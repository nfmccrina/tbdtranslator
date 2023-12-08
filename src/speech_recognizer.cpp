#include "speech_recognizer.h"

languid::SpeechRecognizer::SpeechRecognizer(const LanguidConfig &config, std::shared_ptr<Buffer> wav_buffer, WavHeaderRemover wav_header_remover)
    : _config{config}, _wav_buffer{wav_buffer}, _wav_header_remover{wav_header_remover}
{
}

void languid::SpeechRecognizer::start()
{
    if (this->_wav_buffer == nullptr)
    {
        throw std::invalid_argument("SpeechRecognizer: buffer is null");
    }

    this->_recognizer = TranslationRecognizer::FromConfig(this->get_translation_config(), this->get_audio_config());

    this->_recognizer->Synthesizing += [this](const TranslationSynthesisEventArgs &args) mutable
    {
        this->synthesized_event_callback(args);
    };

    this->_recognizer->Recognized += [](const TranslationRecognitionEventArgs &evt) {
    };

    this->_recognizer->StartContinuousRecognitionAsync().get();
}

void languid::SpeechRecognizer::stop()
{
    if (this->_recognizer != nullptr)
    {
        this->_recognizer->StopContinuousRecognitionAsync().get();
    }
}

std::shared_ptr<SpeechTranslationConfig> languid::SpeechRecognizer::get_translation_config()
{
    auto config = SpeechTranslationConfig::FromSubscription(this->_config.get_speech_key(), this->_config.get_speech_region());
    config->SetSpeechRecognitionLanguage(this->_config.get_source_language());
    config->AddTargetLanguage(this->_config.get_target_language());
    config->SetVoiceName(this->_config.get_voice_name());
    config->SetSpeechSynthesisOutputFormat(SpeechSynthesisOutputFormat::Riff16Khz16BitMonoPcm);

    return config;
}

std::shared_ptr<AudioConfig> languid::SpeechRecognizer::get_audio_config()
{
    return AudioConfig::FromDefaultMicrophoneInput();
}

void languid::SpeechRecognizer::synthesized_event_callback(const TranslationSynthesisEventArgs &args)
{
    std::vector<uint8_t> audio = args.Result->Audio;
    if (audio.size() > 0 && this->_wav_buffer != nullptr)
    {
        this->_wav_header_remover.remove_header(audio);
        this->_wav_buffer->write((uint8_t *)&audio[0], audio.size());
    }
}