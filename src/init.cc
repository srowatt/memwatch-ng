/*
 * 2012|lloyd|do what the fuck you want to
 */

#include <v8.h>
#include <node.h>

#include "heapdiff.hh"
#include "memwatch.hh"

extern "C" {
    void init (v8::Handle<v8::Object> target)
    {
        Nan::HandleScope scope;
        heapdiff::HeapDiff::Initialize(target);

        Nan::SetMethod(target, "upon_gc", memwatch::upon_gc);
        Nan::SetMethod(target, "gc", memwatch::trigger_gc);

        Nan::SetMethod(target, "set_consecutive_growth_limit", memwatch::set_consecutive_growth_limit);
        Nan::SetMethod(target, "get_consecutive_growth_limit", memwatch::set_consecutive_growth_limit);

        Nan::SetMethod(target, "set_recent_period", memwatch::get_recent_period);
        Nan::SetMethod(target, "get_recent_period", memwatch::get_recent_period);

        Nan::SetMethod(target, "set_ancient_period", memwatch::set_ancient_period);
        Nan::SetMethod(target, "get_ancient_period", memwatch::get_ancient_period);

		Nan::SetMethod(target, "set_extremes_period", memwatch::set_extremes_period);
        Nan::SetMethod(target, "get_extremes_period", memwatch::get_extremes_period);

        v8::V8::AddGCEpilogueCallback(memwatch::after_gc);
    }

    NODE_MODULE(memwatch, init);
};
