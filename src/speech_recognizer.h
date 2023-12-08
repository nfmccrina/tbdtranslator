#pragma once

#include <speechapi_cxx.h>
#include "languid_config.h"
#include "buffer.h"
#include "wav_header_remover.h"

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;
using namespace Microsoft::CognitiveServices::Speech::Translation;

namespace languid
{
    class SpeechRecognizer
    {
    public:
        SpeechRecognizer(
            const LanguidConfig &config,
            std::shared_ptr<Buffer> wav_buffer,
            WavHeaderRemover wav_header_remover);
        void start();
        void stop();

    private:
        std::shared_ptr<SpeechTranslationConfig> get_translation_config();
        std::shared_ptr<AudioConfig> get_audio_config();
        void synthesized_event_callback(const TranslationSynthesisEventArgs &args);

        const LanguidConfig &_config;
        std::shared_ptr<Buffer> _wav_buffer;
        std::shared_ptr<TranslationRecognizer> _recognizer;
        WavHeaderRemover _wav_header_remover;
    };
}