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
            int icecast_format);

        std::string get_speech_key() const;
        std::string get_speech_region() const;
        std::string get_source_language() const;
        std::string get_target_language() const;
        std::string get_voice_name() const;
        std::string get_output_file_name() const;
        std::string get_icecast_host() const;
        int get_icecast_protocol() const;
        int get_icecast_port() const;
        std::string get_icecast_username() const;
        std::string get_icecast_password() const;
        std::string get_icecast_stream_name() const;
        int get_icecast_format() const;
        std::string print() const;

    private:
        static std::string get_value_from_args_or_env(
            const std::vector<std::string> &args,
            std::string short_flag,
            std::string long_flag,
            std::string env_var);

        std::string _speech_key;
        std::string _speech_region;
        std::string _source_language;
        std::string _target_language;
        std::string _voice_name;
        std::string _output_file_name;
        std::string _icecast_host;
        int _icecast_protocol;
        int _icecast_port;
        std::string _icecast_username;
        std::string _icecast_password;
        std::string _icecast_stream_name;
        int _icecast_format;
    };
}