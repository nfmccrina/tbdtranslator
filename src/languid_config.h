#pragma once

#include <string>

namespace languid
{
    class LanguidConfig
    {
    public:
        static LanguidConfig init(int argc, char *argv[]);

        LanguidConfig(
            std::string speech_key,
            std::string speech_region);

        std::string get_speech_key() const;
        std::string get_speech_region() const;
        std::string print() const;

    private:
        static std::string get_value_from_args_or_env(
            const std::vector<std::string> &args,
            std::string short_flag,
            std::string long_flag,
            std::string env_var);

        std::string _speech_key;
        std::string _speech_region;
    };
}