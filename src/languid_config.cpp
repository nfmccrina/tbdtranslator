#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <shout/shout.h>
#include "languid_config.h"

languid::LanguidConfig::LanguidConfig(
    std::string speech_key,
    std::string speech_region,
    std::string source_language,
    std::string target_language,
    std::string voice_name,
    std::string output_file_name,
    std::string icecast_host,
    int icecast_protocol,
    int icecast_port,
    std::string icecast_username,
    std::string icecast_password,
    std::string icecast_stream_name,
    int icecast_format)
    : _speech_key{speech_key},
      _speech_region{speech_region},
      _source_language{source_language},
      _target_language{target_language},
      _voice_name{voice_name},
      _output_file_name{output_file_name},
      _icecast_host{icecast_host},
      _icecast_protocol{icecast_protocol},
      _icecast_port{icecast_port},
      _icecast_username{icecast_username},
      _icecast_password{icecast_password},
      _icecast_stream_name{icecast_stream_name},
      _icecast_format{icecast_format}
{
}

languid::LanguidConfig languid::LanguidConfig::init(int argc, char *argv[])
{
    std::vector<std::string> args;
    std::string speech_key;
    std::string speech_region;
    std::string source_language;
    std::string target_language;
    std::string voice_name;
    std::string output_file_name;
    std::string icecast_host;
    std::string icecast_protocol;
    std::string icecast_port;
    std::string icecast_username;
    std::string icecast_password;
    std::string icecast_stream_name;
    std::string icecast_format;

    for (auto i = 0; i < argc; ++i)
    {
        args.push_back(std::string(argv[i]));
    }

    speech_key = get_value_from_args_or_env(args, "-k", "--speech-key", "LANGUID_SPEECH_KEY");
    speech_region = get_value_from_args_or_env(args, "-r", "--speech-region", "LANGUID_SPEECH_REGION");
    source_language = get_value_from_args_or_env(args, "", "--source-language", "LANGUID_SOURCE_LANGUAGE");
    target_language = get_value_from_args_or_env(args, "-t", "--target-language", "LANGUID_TARGET_LANGUAGE");
    voice_name = get_value_from_args_or_env(args, "-v", "--voice-name", "LANGUID_VOICE_NAME");
    output_file_name = get_value_from_args_or_env(args, "-o", "--output-file-name", "LANGUID_OUTPUT_FILE_NAME");
    icecast_host = get_value_from_args_or_env(args, "", "--icecast-host", "LANGUID_ICECAST_HOST");
    icecast_protocol = get_value_from_args_or_env(args, "", "--icecast-protocol", "LANGUID_ICECAST_PROTOCOL");
    icecast_port = get_value_from_args_or_env(args, "", "--icecast-port", "LANGUID_ICECAST_PORT");
    icecast_username = get_value_from_args_or_env(args, "", "--icecast-username", "LANGUID_ICECAST_USERNAME");
    icecast_password = get_value_from_args_or_env(args, "-p", "--icecast-password", "LANGUID_ICECAST_PASSWORD");
    icecast_stream_name = get_value_from_args_or_env(args, "-s", "--icecast-stream-name", "LANGUID_ICECAST_STREAM_NAME");
    icecast_format = get_value_from_args_or_env(args, "", "--icecast-format", "LANGUID_ICECAST_FORMAT");

    if (speech_key.empty())
    {
        throw std::logic_error("Speech subscription key was not provided.");
    }

    if (speech_region.empty())
    {
        throw std::logic_error("Speech region was not provided.");
    }

    if (source_language.empty())
    {
        std::cout << "Source language was not provided, defaulting to \"en-US\"." << std::endl;
        source_language = "en-US";
    }

    if (target_language.empty())
    {
        throw std::logic_error("Target language was not provided.");
    }

    if (voice_name.empty())
    {
        throw std::logic_error("Voice name was not provided.");
    }

    if (output_file_name.empty() && icecast_stream_name.empty())
    {
        throw std::logic_error("Either an output file or an icecast stream must be specified.");
    }

    if (!icecast_stream_name.empty() && icecast_host.empty())
    {
        std::cout << "Icecast host was not provided, defaulting to \"127.0.0.1\"." << std::endl;
        icecast_host = "127.0.0.1";
    }

    if (!icecast_stream_name.empty() && icecast_format.empty())
    {
        std::cout << "Icecast format was not provided, defaulting to \"SHOUT_FORMAT_OGG\"." << std::endl;
        icecast_format = std::to_string(SHOUT_FORMAT_OGG);
    }

    if (!icecast_stream_name.empty() && icecast_port.empty())
    {
        std::cout << "Icecast port was not provided, defaulting to \"8000\"." << std::endl;
        icecast_port = std::to_string(8000);
    }

    if (!icecast_stream_name.empty() && icecast_protocol.empty())
    {
        std::cout << "Icecast protocol was not provided, defaulting to \"SHOUT_PROTOCOL_HTTP\"." << std::endl;
        icecast_protocol = std::to_string(SHOUT_PROTOCOL_HTTP);
    }

    if (!icecast_stream_name.empty() && icecast_username.empty())
    {
        std::cout << "Icecast username was not provided, defaulting to \"source\"." << std::endl;
        icecast_username = "source";
    }

    if (!icecast_stream_name.empty() && icecast_password.empty())
    {
        throw std::logic_error("Icecast password was not provided.");
    }

    return languid::LanguidConfig(
        speech_key,
        speech_region,
        source_language,
        target_language,
        voice_name,
        output_file_name,
        icecast_host,
        std::stoi(icecast_protocol),
        std::stoi(icecast_port),
        icecast_username,
        icecast_password,
        icecast_stream_name,
        std::stoi(icecast_format));
}

std::string languid::LanguidConfig::get_speech_key() const
{
    return this->_speech_key;
}

std::string languid::LanguidConfig::get_speech_region() const
{
    return this->_speech_region;
}

std::string languid::LanguidConfig::get_source_language() const
{
    return this->_source_language;
}

std::string languid::LanguidConfig::get_target_language() const
{
    return this->_target_language;
}

std::string languid::LanguidConfig::get_voice_name() const
{
    return this->_voice_name;
}

std::string languid::LanguidConfig::get_output_file_name() const
{
    return this->_output_file_name;
}

std::string languid::LanguidConfig::get_icecast_host() const
{
    return this->_icecast_host;
}

int languid::LanguidConfig::get_icecast_protocol() const
{
    return this->_icecast_protocol;
}

int languid::LanguidConfig::get_icecast_port() const
{
    return this->_icecast_port;
}

std::string languid::LanguidConfig::get_icecast_username() const
{
    return this->_icecast_username;
}

std::string languid::LanguidConfig::get_icecast_password() const
{
    return this->_icecast_password;
}

std::string languid::LanguidConfig::get_icecast_stream_name() const
{
    return this->_icecast_stream_name;
}

int languid::LanguidConfig::get_icecast_format() const
{
    return this->_icecast_format;
}

std::string languid::LanguidConfig::print() const
{
    std::stringstream ss;

    ss << "speech key: " << this->_speech_key << std::endl;
    ss << "speech region: " << this->_speech_region << std::endl;
    ss << "source language: " << this->_source_language << std::endl;
    ss << "target language: " << this->_target_language << std::endl;
    ss << "voice name: " << this->_voice_name << std::endl;
    ss << "output file name: " << this->_output_file_name << std::endl;
    ss << "Icecast host: " << this->_icecast_host << std::endl;
    ss << "Icecast protocol: " << this->_icecast_protocol << std::endl;
    ss << "Icecast port: " << this->_icecast_port << std::endl;
    ss << "Icecast username: " << this->_icecast_username << std::endl;
    ss << "Icecast password: " << this->_icecast_password << std::endl;
    ss << "Icecast stream name: " << this->_icecast_stream_name << std::endl;
    ss << "Icecast format: " << this->_icecast_format << std::endl;

    return ss.str();
}

std::string languid::LanguidConfig::get_value_from_args_or_env(
    const std::vector<std::string> &args,
    std::string short_flag,
    std::string long_flag,
    std::string env_var)
{
    std::vector<std::string>::const_iterator key_it;

    if (!short_flag.empty())
    {
        key_it = std::find(args.begin(), args.end(), short_flag);

        if (key_it != std::end(args))
        {
            if ((key_it + 1) != args.end())
            {
                return *(key_it + 1);
            }
        }
    }

    if (!long_flag.empty())
    {
        auto is_long_flag = [long_flag](std::string s)
        { return s.compare(0, long_flag.size(), long_flag) == 0; };
        key_it = std::find_if(args.begin(), args.end(), is_long_flag);

        if (key_it != std::end(args))
        {
            auto delimiter_pos = key_it->find('=');
            return key_it->substr(delimiter_pos + 1, key_it->size());
        }
    }

    if (!env_var.empty())
    {
        char *e = std::getenv(env_var.c_str());

        if (e != nullptr)
        {
            return std::string(e);
        }
    }

    return std::string();
}