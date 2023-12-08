#include "buffer.h"

void languid::Buffer::write(uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> guard(this->_data_mutex);
    this->_data.insert(this->_data.end(), data, data + size);
}

std::vector<uint8_t> languid::Buffer::read(size_t size)
{
    std::vector<uint8_t> result;

    std::lock_guard<std::mutex> guard(this->_data_mutex);
    result.insert(
        result.end(),
        this->_data.begin(),
        size > this->_data.end() - this->_data.begin() ? this->_data.end() : this->_data.begin() + size);

    this->_data.erase(this->_data.begin(), this->_data.begin() + size);

    return std::move(result);
}

bool languid::Buffer::empty()
{
    std::lock_guard<std::mutex> guard(this->_data_mutex);
    return this->_data.empty();
}

size_t languid::Buffer::size()
{
    std::lock_guard<std::mutex> guard(this->_data_mutex);
    return this->_data.size();
}