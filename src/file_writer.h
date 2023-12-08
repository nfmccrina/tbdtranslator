#pragma once

#include <fstream>
#include "buffer.h"

namespace languid
{
    class FileWriter
    {
    public:
        static void run(std::shared_ptr<Buffer> in_buffer, std::string file_name, bool &stopped);

    private:
        static void write_data(std::shared_ptr<Buffer> in_buffer, std::ofstream &fout);
    };
}