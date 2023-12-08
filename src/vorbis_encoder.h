#pragma once

#include <memory>
#include "buffer.h"

namespace languid
{
    class VorbisEncoder
    {
    public:
        static void run(std::shared_ptr<Buffer> in_buffer, std::shared_ptr<Buffer> out_buffer, bool &stopped);
    };
}