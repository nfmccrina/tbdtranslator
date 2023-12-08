#include <thread>
#include <chrono>
#include "file_writer.h"

using namespace std::chrono_literals;

void languid::FileWriter::run(std::shared_ptr<Buffer> in_buffer, std::string file_name, bool &stopped)
{
    if (in_buffer == nullptr)
    {
        throw std::invalid_argument("FileWriter: buffer is null");
    }

    std::ofstream fout(file_name, std::ios::binary);

    while (!stopped)
    {
        write_data(in_buffer, fout);
    }

    std::this_thread::sleep_for(100ms);

    write_data(in_buffer, fout);
}

void languid::FileWriter::write_data(std::shared_ptr<Buffer> in_buffer, std::ofstream &fout)
{
    const int buffer_size = 256;
    size_t available_bytes = in_buffer->size();
    while (available_bytes > 0)
    {
        std::vector<uint8_t> data;

        if (available_bytes >= buffer_size)
        {
            data = in_buffer->read(buffer_size);
        }
        else
        {
            data = in_buffer->read(available_bytes);
        }

        fout.write((char *)&data[0], data.size());

        available_bytes = in_buffer->size();
    }
}