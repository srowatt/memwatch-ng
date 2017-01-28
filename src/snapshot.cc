#include "node.h"

#include "compat-inl.hh"
#include "uv.h"
#include "v8-profiler.h"
#include "v8.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

namespace {
    static const int kMaxPath = 4096;
    static const int kForkFlag = 1;
    static const int kSignalFlag = 2;

    inline bool WriteSnapshot(v8::Isolate* isolate, const char* filename);
    inline bool WriteSnapshotHelper(v8::Isolate* isolate, const char* filename);
    inline void InvokeCallback(const char* filename);
    inline void PlatformInit(v8::Isolate* isolate, int flags);
    inline void RandomSnapshotFilename(char* buffer, size_t size);
    static ::compat::Persistent<v8::Function> on_complete_callback;

}  // namespace anonymous

#ifdef _WIN32
    #include "snapshot-win32.hh"
#else
    #include "snapshot-posix.hh"
#endif

namespace {

    using v8::Context;
    using v8::Function;
    using v8::FunctionTemplate;
    using v8::Handle;
    using v8::HandleScope;
    using v8::HeapSnapshot;
    using v8::Isolate;
    using v8::Local;
    using v8::Object;
    using v8::OutputStream;
    using v8::Persistent;
    using v8::String;
    using v8::V8;
    using v8::Value;

    namespace C = ::compat;

    inline C::ReturnType WriteSnapshot(const C::ArgumentType& args) {
        C::ReturnableHandleScope handle_scope(args);
        Isolate* const isolate = args.GetIsolate();

        Local<Value> maybe_function = args[0];
        if (1 < args.Length()) {
            maybe_function = args[1];
        }

        if (maybe_function->IsFunction()) {
            Local<Function> function = maybe_function.As<Function>();
            on_complete_callback.Reset(isolate, function);
        }

        char filename[kMaxPath];
        if (args[0]->IsString()) {
            String::Utf8Value filename_string(args[0]);
            snprintf(filename, sizeof(filename), "%s", *filename_string);
        } else {
            RandomSnapshotFilename(filename, sizeof(filename));
        }

        // Throwing on error is too disruptive, just return a boolean indicating success.
        const bool success = WriteSnapshot(isolate, filename);
        return handle_scope.Return(success);
    }

    inline bool WriteSnapshotHelper(Isolate* isolate, const char* filename) {
        FILE* fp = fopen(filename, "w");
        if (fp == NULL) {
            return false;
        }
        const HeapSnapshot* const snap = C::HeapProfiler::TakeHeapSnapshot(isolate);
        FileOutputStream stream(fp);
        snap->Serialize(&stream, HeapSnapshot::kJSON);
        fclose(fp);
        // Work around a deficiency in the API.  The HeapSnapshot object is const
        // but we cannot call HeapProfiler::DeleteAllHeapSnapshots() because that
        // invalidates _all_ snapshots, including those created by other tools.
        const_cast<HeapSnapshot*>(snap)->Delete();
        return true;
    }

    inline void InvokeCallback(const char* filename) {
        if (on_complete_callback.IsEmpty()) {
            return;
        }
        Isolate* isolate = Isolate::GetCurrent();
        C::HandleScope handle_scope(isolate);
        Local<Function> callback = on_complete_callback.ToLocal(isolate);
        Local<Value> argv[] = {
            C::Null(isolate),
            C::String::NewFromUtf8(isolate, filename)
        };
        const int argc = sizeof(argv) / sizeof(*argv);
        #if !NODE_VERSION_AT_LEAST(0, 11, 0)
            node::MakeCallback(Context::GetCurrent()->Global(), callback, argc, argv);
        #else
            node::MakeCallback(isolate, isolate->GetCurrentContext()->Global(), callback, argc, argv);
        #endif
    }

    inline void RandomSnapshotFilename(char* buffer, size_t size) {
        const uint64_t now = uv_hrtime();
        const unsigned long sec = static_cast<unsigned long>(now / 1000000);
        const unsigned long usec = static_cast<unsigned long>(now % 1000000);
        snprintf(buffer, size, "heapdump-%lu.%lu.heapsnapshot", sec, usec);
    }

    inline C::ReturnType Configure(const C::ArgumentType& args) {
        C::ReturnableHandleScope handle_scope(args);
        PlatformInit(args.GetIsolate(), args[0]->Int32Value());
        return handle_scope.Return();
    }

    inline void Initialize(Local<Object> binding) {
      Isolate* const isolate = Isolate::GetCurrent();
      binding->Set(C::String::NewFromUtf8(isolate, "kForkFlag"),
                   C::Integer::New(isolate, kForkFlag));
      binding->Set(C::String::NewFromUtf8(isolate, "kSignalFlag"),
                   C::Integer::New(isolate, kSignalFlag));
      binding->Set(C::String::NewFromUtf8(isolate, "configure"),
                   C::FunctionTemplate::New(isolate, Configure)->GetFunction());
      binding->Set(C::String::NewFromUtf8(isolate, "writeSnapshot"),
                   C::FunctionTemplate::New(isolate, WriteSnapshot)->GetFunction());
    }

}  // namespace anonymous