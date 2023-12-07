#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include "languid_config.h"

languid::LanguidConfig::LanguidConfig(
    std::string speech_key,
    std::string speech_region)
{
    this->_speech_key = speech_key;
    this->_speech_region = speech_region;
}

languid::LanguidConfig languid::LanguidConfig::init(int argc, char *argv[])
{
    std::vector<std::string> args;
    std::string speech_key;
    std::string speech_region;

    for (auto i = 0; i < argc; ++i)
    {
        args.push_back(std::string(argv[i]));
    }

    speech_key = get_value_from_args_or_env(args, "-k", "--speech-key", "SPEECH_KEY");
    speech_region = get_value_from_args_or_env(args, "-r", "--speech-region", "SPEECH_REGION");

    if (speech_key.empty())
    {
        throw std::logic_error("Speech subscription key was not found.");
    }

    if (speech_region.empty())
    {
        throw std::logic_error("Speech region was not found.");
    }

    return languid::LanguidConfig(speech_key, speech_region);
}

std::string languid::LanguidConfig::get_speech_key() const
{
    return this->_speech_key;
}

std::string languid::LanguidConfig::get_speech_region() const
{
    return this->_speech_region;
}

std::string languid::LanguidConfig::print() const
{
    std::stringstream ss;

    ss << "speech key: " << this->_speech_key << std::endl;
    ss << "speech region: " << this->_speech_region << std::endl;

    return ss.str();
}

std::string languid::LanguidConfig::get_value_from_args_or_env(
    const std::vector<std::string> &args,
    std::string short_flag,
    std::string long_flag,
    std::string env_var)
{
    auto key_it = std::find(args.begin(), args.end(), short_flag);

    if (key_it != std::end(args))
    {
        if ((key_it + 1) != args.end())
        {
            return *(key_it + 1);
        }
    }

    auto is_long_flag = [long_flag](std::string s)
    { return s.compare(0, long_flag.size(), long_flag) == 0; };
    key_it = std::find_if(args.begin(), args.end(), is_long_flag);

    if (key_it != std::end(args))
    {
        auto delimiter_pos = key_it->find('=');
        return key_it->substr(delimiter_pos + 1, key_it->size());
    }

    char *e = std::getenv(env_var.c_str());

    if (e != nullptr)
    {
        return std::string(e);
    }

    return std::string();
}