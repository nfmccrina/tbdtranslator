#pragma once

#include "buffer.h"

namespace languid
{
    // the speech sdk only returns wav files at intermittent intervals,
    // so we need to turn that into a continuous stream of audio samples
    // (adding silence as necessary).
    class AudioStreamNormalizer
    {
    public:
        static void run(std::shared_ptr<Buffer> in_buffer, std::shared_ptr<Buffer> out_buffer, bool &stopped);
    };
}