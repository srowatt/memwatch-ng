#ifndef SRC_HEAPDUMP_WIN32_H_
#define SRC_HEAPDUMP_WIN32_H_

namespace snapshot {

    inline void PlatformInit(v8::Isolate*, int) {
    }

    inline bool WriteSnapshot(v8::Isolate* isolate, const char* filename) {
        bool result = WriteSnapshotHelper(isolate, filename);
        InvokeCallback(filename);
        return result;
    }

    // Emulate snprintf() on windows, _snprintf() doesn't zero-terminate the buffer on overflow.
    inline int snprintf(char* buf, size_t len, const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        const int n = _vsprintf_p(buf, len, fmt, ap);
        if (len) buf[len - 1] = '\0';
            va_end(ap);
        return n;
    }

}

#endif  // SRC_HEAPDUMP_WIN32_H_