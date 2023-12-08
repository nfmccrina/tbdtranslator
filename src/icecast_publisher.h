#pragma once

#include "buffer.h"

namespace languid
{
    class IcecastPublisher
    {
    public:
        static void run(
            std::shared_ptr<Buffer> in_buffer,
            std::string icecast_host,
            int icecast_protocol,
            int icecast_port,
            std::string icecast_username,
            std::string icecast_password,
            std::string icecast_stream_name,
            int icecast_format,
            bool &stopped);
    };
}