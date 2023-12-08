#include "languid_controller.h"
#include "audio_stream_normalizer.h"
#include "vorbis_encoder.h"
#include "file_writer.h"
#include "icecast_publisher.h"

using namespace std::chrono_literals;

languid::LanguidController::LanguidController(const LanguidConfig &config)
    : _config{config},
      _pcm_buffer{std::make_shared<Buffer>()},
      _normalized_pcm_buffer{std::make_shared<Buffer>()},
      _ogg_buffer{std::make_shared<Buffer>()},
      _speech_recognizer{_config, _pcm_buffer, WavHeaderRemover()}
{
}

void languid::LanguidController::run()
{
    bool stopped = false;
    this->_speech_recognizer.start();
    std::thread audio_stream_normalizer_thread(AudioStreamNormalizer::run, this->_pcm_buffer, this->_normalized_pcm_buffer, std::ref(stopped));
    std::thread vorbis_encoder_thread(VorbisEncoder::run, this->_normalized_pcm_buffer, this->_ogg_buffer, std::ref(stopped));

    std::thread file_writer_thread;
    if (!this->_config.get_output_file_name().empty())
    {
        file_writer_thread = std::thread(FileWriter::run, this->_ogg_buffer, this->_config.get_output_file_name(), std::ref(stopped));
    }

    std::thread icecast_publisher_thread;
    if (!this->_config.get_icecast_stream_name().empty())
    {
        icecast_publisher_thread = std::thread(
            IcecastPublisher::run,
            this->_ogg_buffer,
            this->_config.get_icecast_host(),
            this->_config.get_icecast_protocol(),
            this->_config.get_icecast_port(),
            this->_config.get_icecast_username(),
            this->_config.get_icecast_password(),
            this->_config.get_icecast_stream_name(),
            this->_config.get_icecast_format(),
            std::ref(stopped));
    }

    std::cin.get();

    stopped = true;

    this->_speech_recognizer.stop();

    audio_stream_normalizer_thread.join();
    vorbis_encoder_thread.join();

    if (file_writer_thread.joinable())
    {
        file_writer_thread.join();
    }
}