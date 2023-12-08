#pragma once

#include <mutex>
#include <vector>

namespace languid
{
    class Buffer
    {
    public:
        void write(uint8_t *data, size_t size);
        std::vector<uint8_t> read(size_t size);
        bool empty();
        size_t size();

    private:
        std::mutex _data_mutex;
        std::vector<uint8_t> _data;
    };
}