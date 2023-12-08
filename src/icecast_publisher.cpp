#include <iostream>
#include <shout/shout.h>
#include "icecast_publisher.h"
#include "constants.h"

void languid::IcecastPublisher::run(
    std::shared_ptr<Buffer> in_buffer,
    std::string icecast_host,
    int icecast_protocol,
    int icecast_port,
    std::string icecast_username,
    std::string icecast_password,
    std::string icecast_stream_name,
    int icecast_format,
    bool &stopped)
{
    shout_t *shout;
    int ret;

    shout_init();

    if (!(shout = shout_new()))
    {
        std::cout << "Could not allocate shout_t." << std::endl;
        return;
    }

    if (shout_set_host(shout, icecast_host.c_str()) != SHOUTERR_SUCCESS)
    {
        std::cout << "Error setting hostname: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_protocol(shout, icecast_protocol) != SHOUTERR_SUCCESS)
    {
        std::cout << "Error setting protocol: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_port(shout, icecast_port) != SHOUTERR_SUCCESS)
    {
        std::cout << "Error setting port: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_password(shout, icecast_password.c_str()) != SHOUTERR_SUCCESS)
    {
        std::cout << "Error setting password: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_mount(shout, icecast_stream_name.c_str()) != SHOUTERR_SUCCESS)
    {
        std::cout << "Error setting mount: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_user(shout, icecast_username.c_str()) != SHOUTERR_SUCCESS)
    {
        std::cout << "Error setting user: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_content_format(shout, icecast_format, SHOUT_USAGE_UNKNOWN, NULL) != SHOUTERR_SUCCESS)
    {
        std::cout << "Error setting format: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_open(shout) == SHOUTERR_SUCCESS)
    {
        std::cout << "Connected to icecast server..." << std::endl;
        size_t read = 0, total = 0;
        unsigned char buff[constants::ICECAST_BUFFER_SIZE];
        size_t buffer_size;
        while (!stopped)
        {
            buffer_size = in_buffer->size();

            if (buffer_size >= constants::ICECAST_BUFFER_SIZE)
            {
                auto index = 0;
                std::memcpy(buff, in_buffer->read(constants::ICECAST_BUFFER_SIZE).data(), constants::ICECAST_BUFFER_SIZE);

                ret = shout_send(shout, buff, 4096);

                if (ret != SHOUTERR_SUCCESS)
                {
                    std::cout << "send error: " << shout_get_error(shout) << std::endl;
                    break;
                }

                shout_sync(shout);
            }
        }

        while (true)
        {
            buffer_size = in_buffer->size();

            if (buffer_size == 0)
            {
                break;
            }

            auto bytes_to_read = buffer_size > constants::ICECAST_BUFFER_SIZE ? buffer_size % constants::ICECAST_BUFFER_SIZE : buffer_size;

            std::memcpy(buff, in_buffer->read(bytes_to_read).data(), bytes_to_read);

            ret = shout_send(shout, buff, bytes_to_read);

            if (ret != SHOUTERR_SUCCESS)
            {
                std::cout << "send error: " << shout_get_error(shout) << std::endl;
            }

            shout_sync(shout);
        }
    }
    else
    {
        std::cout << "Error connecting to icecast: " << shout_get_error(shout) << std::endl;
    }
}