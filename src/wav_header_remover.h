#pragma once

#include <vector>

namespace languid
{
    class WavHeaderRemover
    {
    public:
        void remove_header(std::vector<uint8_t> &wav_data);
    };
}