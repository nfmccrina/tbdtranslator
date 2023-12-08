#include <chrono>
#include <thread>
#include <iostream>
#include "audio_stream_normalizer.h"
#include "constants.h"

void languid::AudioStreamNormalizer::run(std::shared_ptr<Buffer> in_buffer, std::shared_ptr<Buffer> out_buffer, bool &stopped)
{
    if (in_buffer == nullptr || out_buffer == nullptr)
    {
        throw std::invalid_argument("AudioStreamNormalizer: buffer is null");
    }

    while (!stopped)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto bytes_needed = constants::SAMPLES_PER_PERIOD * constants::SAMPLEWIDTH_IN_BYTES;
        auto input_buffer_size = in_buffer->size();
        auto bytes_to_read_from_input_buffer = input_buffer_size >= bytes_needed ? bytes_needed : input_buffer_size;

        std::vector<uint8_t> period_buffer(std::move(in_buffer->read(bytes_to_read_from_input_buffer)));

        period_buffer.insert(period_buffer.end(), bytes_needed - bytes_to_read_from_input_buffer, 0);

        out_buffer->write((uint8_t *)&period_buffer[0], bytes_needed);

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double, std::milli>(end - start);
        auto target = std::chrono::duration<double, std::milli>((double)constants::PERIOD_LENGTH_IN_MILLISECONDS);

        std::this_thread::sleep_for(target - elapsed);
    }
}