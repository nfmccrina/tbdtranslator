#pragma once

#include "speech_recognizer.h"
#include "languid_config.h"
#include "buffer.h"

namespace languid
{
    class LanguidController
    {
    public:
        LanguidController(const LanguidConfig &config);
        void run();

    private:
        const LanguidConfig &_config;
        std::shared_ptr<Buffer> _pcm_buffer;
        std::shared_ptr<Buffer> _normalized_pcm_buffer;
        std::shared_ptr<Buffer> _ogg_buffer;
        SpeechRecognizer _speech_recognizer;
    };
}