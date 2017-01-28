#include "node.h"

#include "uv.h"
#include "v8-profiler.h"
#include "v8.h"

#include <stdio.h>

namespace {
    using v8::OutputStream;
}

class FileOutputStream : public OutputStream {
    public:
        FileOutputStream(FILE* stream) : stream_(stream) {}

        virtual int GetChunkSize() {
            return 65536;  // big chunks == faster
        }

        virtual void EndOfStream() {}

        virtual WriteResult WriteAsciiChunk(char* data, int size) {
            const size_t len = static_cast<size_t>(size);
            size_t off = 0;

            while (off < len && !feof(stream_) && !ferror(stream_))
                off += fwrite(data + off, 1, len - off, stream_);

            return off == len ? kContinue : kAbort;
        }

    private:
        FILE* stream_;
};