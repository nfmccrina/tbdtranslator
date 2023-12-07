#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <iomanip>
#include <speechapi_cxx.h>
#include <shout/shout.h>
#include <sstream>
#include <chrono>
#include <vorbis/vorbisenc.h>

// 00007000: 4200 4b00 4500 4b00 4d00 4b00 5300 5100  B.K.E.K.M.K.S.Q.

// 00045f80: 4200 4b00 4500 4b00 4d00 4b00 5300 5100  B.K.E.K.M.K.S.Q.

using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;
using namespace Microsoft::CognitiveServices::Speech::Translation;
using namespace std::chrono_literals;

std::mutex pcm_buffer_mutex;
std::mutex ogg_buffer_mutex;
int chunks = 0;

void get_pcm_from_synthesized_audio(const std::vector<uint8_t> &synthesized_audio, std::shared_ptr<std::vector<uint8_t>> pcm_buffer)
{
    std::stringstream ss;

    ss << "original" << chunks << ".wav";
    std::ofstream wav_file(ss.str().c_str(), std::ios::binary);

    wav_file.write((const char *)&synthesized_audio[0], synthesized_audio.size());
    chunks++;

    // std::cout << "buffer size before write: " << pcm_buffer.get()->size() << std::endl;
    std::lock_guard<std::mutex> guard(pcm_buffer_mutex);
    pcm_buffer.get()->insert(pcm_buffer.get()->end(), synthesized_audio.begin() + 44, synthesized_audio.end());
    // std::cout << "buffer size after write: " << pcm_buffer.get()->size() << std::endl;
}

// std::vector<uint8_t> encode_pcm_chunk(lame_global_flags *lame, const std::vector<uint8_t> &pcm_chunk)
// {
//     const int nsamples = 16000 / (1000 / 20);
//     const int MP3_SIZE = (2 * nsamples) + 7200;

//     unsigned char mp3_buffer[MP3_SIZE];
//     int mp3_bytes_written;

//     std::vector<short> sample_buffer(nsamples);

//     if (pcm_chunk.size() != 0)
//     {
//         bool is_high_byte = true;
//         for (auto it = pcm_chunk.begin(); it != pcm_chunk.end(); ++it)
//         {
//             if (!is_high_byte)
//             {
//                 is_high_byte = !is_high_byte;
//                 continue;
//             }

//             short sample = 0;
//             // sample = static_cast<short>(*it) << 8;
//             // sample = sample | static_cast<short>(*(it + 1));
//             sample = static_cast<short>(*(it + 1)) << 8;
//             sample = sample | static_cast<short>(*it);

//             sample_buffer.push_back(sample);

//             is_high_byte = !is_high_byte;
//         }

//         mp3_bytes_written = lame_encode_buffer(lame, sample_buffer.data(), NULL, nsamples, mp3_buffer, MP3_SIZE);
//     }
//     else
//     {
//         mp3_bytes_written = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
//     }

//     std::vector<uint8_t> mp3_data;
//     if (mp3_bytes_written < 0)
//     {
//         // std::cout << "lame encoding error: " << mp3_bytes_written << std::endl;
//     }
//     else if (mp3_bytes_written > 0)
//     {
//         mp3_data.assign(mp3_buffer, mp3_buffer + mp3_bytes_written);
//     }

//     return std::move(mp3_data);
// }

std::shared_ptr<SpeechTranslationConfig> get_translation_config(std::string speech_key, std::string speech_region)
{
    auto config = SpeechTranslationConfig::FromSubscription(speech_key, speech_region);
    config->SetSpeechRecognitionLanguage("en-US");
    config->AddTargetLanguage("uk");
    config->SetVoiceName("uk-UA-OstapNeural");
    config->SetSpeechSynthesisOutputFormat(SpeechSynthesisOutputFormat::Riff16Khz16BitMonoPcm);

    return config;
}

std::shared_ptr<AudioConfig> get_audio_config()
{
    return AudioConfig::FromDefaultMicrophoneInput();
}

std::vector<uint8_t> get_next_pcm_chunk(std::shared_ptr<std::vector<uint8_t>> pcm_buffer)
{
    auto chunk_size = (16000 / (1000 / 20)) * 1 * 2; // 16000 samples per second, times 20/1000ths of a second, times one channel, times two bytes per sample

    std::vector<uint8_t> pcm_chunk;

    std::lock_guard<std::mutex> guard(pcm_buffer_mutex);

    auto bytes_available = pcm_buffer.get()->size();

    auto bytes_read_from_buffer = chunk_size > bytes_available ? bytes_available : chunk_size;

    // std::cout << "Reading from buffer..." << std::endl
    //           << "chunk size: " << chunk_size << std::endl
    //           << "bytes available: " << bytes_available << std::endl
    //           << "bytes read: " << bytes_read_from_buffer << std::endl;

    pcm_chunk.assign(pcm_buffer.get()->begin(), pcm_buffer.get()->begin() + bytes_read_from_buffer);
    pcm_chunk.insert(pcm_chunk.end(), chunk_size - bytes_read_from_buffer, 0);

    // std::cout << std::hex << "data : ";

    // for (auto it = pcm_chunk.begin(); it != pcm_chunk.end(); ++it)
    // {
    //     std::cout << std::setw(2) << std::setfill('0') << (int)*it << " ";
    // }

    // std::cout << std::endl;
    // std::cout << std::dec << std::endl;

    pcm_buffer.get()->erase(pcm_buffer.get()->begin(), pcm_buffer.get()->begin() + bytes_read_from_buffer);

    return std::move(pcm_chunk);
}

std::shared_ptr<TranslationRecognizer> init_translation_recognizer(std::string speech_key, std::string speech_region, std::shared_ptr<std::vector<uint8_t>> pcm_buffer)
{
    auto recognizer = TranslationRecognizer::FromConfig(get_translation_config(speech_key, speech_region), get_audio_config());

    recognizer->Synthesizing += [pcm_buffer](const TranslationSynthesisEventArgs &args) mutable
    {
        auto audio = args.Result->Audio;

        if (audio.size() > 0)
        {
            get_pcm_from_synthesized_audio(audio, pcm_buffer);
        }
    };

    recognizer->Recognized += [](const TranslationRecognitionEventArgs &evt)
    {
        // if (evt.Result->Reason == ResultReason::TranslatedSpeech)
        // {
        //     for (const auto &translation : evt.Result->Translations)
        //     {
        //         std::cout << "TRANSLATED [" << translation.first << "]:" << translation.second << std::endl;
        //     }
        // }
    };

    return recognizer;
}

void encode_pcm_as_vorbis(std::shared_ptr<std::vector<uint8_t>> ogg_buffer, std::function<std::vector<uint8_t>()> get_data)
{
    // std::ofstream pcm_file("output.pcm", std::ios::binary);
    // std::ofstream original_pcm_file("original_output.pcm", std::ios::binary);
    // FILE *fout = fopen("original_output.ogg", "wb");

    ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
    ogg_page og;         /* one Ogg bitstream page.  Vorbis packets are inside */
    ogg_packet op;       /* one raw packet of data for decode */

    vorbis_info vi;    /* struct that stores all the static vorbis bitstream
                          settings */
    vorbis_comment vc; /* struct that stores all the user comments */

    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block vb;     /* local working space for packet->PCM decode */

    int eos = 0, ret;
    int i, founddata;

#if defined(macintosh) && defined(__MWERKS__)
    int argc = 0;
    char **argv = NULL;
    argc = ccommand(&argv); /* get a "command line" from the Mac user */
                            /* this also lets the user set stdin and stdout */
#endif

    /* we cheat on the WAV header; we just bypass 44 bytes (simplest WAV
       header is 44 bytes) and assume that the data is 44.1khz, stereo, 16 bit
       little endian pcm samples. This is just an example, after all. */

#ifdef _WIN32 /* We need to set stdin/stdout to binary mode. Damn windows. */
    /* if we were reading/writing a file, it would also need to in
       binary mode, eg, fopen("file.wav","wb"); */
    /* Beware the evil ifdef. We avoid these where we can, but this one we
       cannot. Don't add any more, you'll probably go to hell if you do. */
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    /* we cheat on the WAV header; we just bypass the header and never
       verify that it matches 16bit/stereo/44.1kHz.  This is just an
       example, after all. */

    // readbuffer[0] = '\0';
    // for (i = 0, founddata = 0; i < 30 && !feof(stdin) && !ferror(stdin); i++)
    // {
    //     fread(readbuffer, 1, 2, stdin);

    //     if (!strncmp((char *)readbuffer, "da", 2))
    //     {
    //         founddata = 1;
    //         fread(readbuffer, 1, 6, stdin);
    //         break;
    //     }
    // }

    /********** Encode setup ************/

    vorbis_info_init(&vi);

    /* choose an encoding mode.  A few possibilities commented out, one
       actually used: */

    /*********************************************************************
     Encoding using a VBR quality mode.  The usable range is -.1
     (lowest quality, smallest file) to 1. (highest quality, largest file).
     Example quality mode .4: 44kHz stereo coupled, roughly 128kbps VBR

     ret = vorbis_encode_init_vbr(&vi,2,44100,.4);

     ---------------------------------------------------------------------

     Encoding using an average bitrate mode (ABR).
     example: 44kHz stereo coupled, average 128kbps VBR

     ret = vorbis_encode_init(&vi,2,44100,-1,128000,-1);

     ---------------------------------------------------------------------

     Encode using a quality mode, but select that quality mode by asking for
     an approximate bitrate.  This is not ABR, it is true VBR, but selected
     using the bitrate interface, and then turning bitrate management off:

     ret = ( vorbis_encode_setup_managed(&vi,2,44100,-1,128000,-1) ||
             vorbis_encode_ctl(&vi,OV_ECTL_RATEMANAGE2_SET,NULL) ||
             vorbis_encode_setup_init(&vi));

     *********************************************************************/

    // ret = vorbis_encode_init_vbr(&vi, 2, 44100, 0.1);
    ret = vorbis_encode_init_vbr(&vi, 1, 16000, 0.1);

    /* do not continue if setup failed; this can happen if we ask for a
       mode that libVorbis does not support (eg, too low a bitrate, etc,
       will return 'OV_EIMPL') */

    if (ret)
        exit(1);

    /* add a comment */
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "encoder_example.c");

    /* set up the analysis state and auxiliary encoding storage */
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    /* set up our packet->stream encoder */
    /* pick a random serial number; that way we can more likely build
       chained streams just by concatenation */
    srand(time(NULL));
    ogg_stream_init(&os, rand());

    /* Vorbis streams begin with three headers; the initial header (with
       most of the codec setup parameters) which is mandated by the Ogg
       bitstream spec.  The second header holds any comment fields.  The
       third header holds the bitstream codebook.  We merely need to
       make the headers, then pass them to libvorbis one at a time;
       libvorbis handles the additional Ogg bitstream constraints */

    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header); /* automatically placed in its own
                                              page */
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);

        /* This ensures the actual
         * audio data will start on a new page, as per spec
         */
        while (!eos)
        {
            std::lock_guard<std::mutex> guard(ogg_buffer_mutex);
            int result = ogg_stream_flush(&os, &og);
            if (result == 0)
                break;
            // fwrite(og.header, 1, og.header_len, fout);
            for (auto i = 0; i < og.header_len; ++i)
            {
                auto value = ((uint8_t *)og.header)[i];
                // std::cout << std::hex << std::setw(2) << std::setfill('0') << "pushed " << (int)value << std::dec << std::endl;
                ogg_buffer.get()->push_back(value);
            }
            // fwrite(og.body, 1, og.body_len, fout);
            for (auto i = 0; i < og.body_len; ++i)
            {
                auto value = ((uint8_t *)og.body)[i];
                // std::cout << std::hex << std::setw(2) << std::setfill('0') << "pushed " << (int)value << std::dec << std::endl;
                ogg_buffer.get()->push_back(value);
            }
            // fout.write((char *)&og.header, og.header_len);
            // fout.write((char *)&og.body, og.body_len);
        }
    }

    while (!eos)
    {
        auto start = std::chrono::high_resolution_clock::now();
        long i;
        // long bytes = fread(readbuffer, 1, READ * 4, stdin); /* stereo hardwired here */
        std::vector<uint8_t> bytes = get_data();

        if (bytes.size() == 0)
        {
            /* end of file.  this can be done implicitly in the mainline,
               but it's easier to see here in non-clever fashion.
               Tell the library we're at end of stream so that it can handle
               the last frame and mark end of stream in the output properly */
            vorbis_analysis_wrote(&vd, 0);
        }
        else
        {
            /* data to encode */

            /* expose the buffer to submit data */
            // float **buffer = vorbis_analysis_buffer(&vd, READ);
            float **buffer = vorbis_analysis_buffer(&vd, 320);

            /* uninterleave samples */
            // for (i = 0; i < bytes / 4; i++)
            // {
            //     buffer[0][i] = ((readbuffer[i * 4 + 1] << 8) |
            //                     (0x00ff & (int)readbuffer[i * 4])) /
            //                    32768.f;
            //     buffer[1][i] = ((readbuffer[i * 4 + 3] << 8) |
            //                     (0x00ff & (int)readbuffer[i * 4 + 2])) /
            //                    32768.f;
            // }

            // std::cout << std::hex;

            // original_pcm_file.write((char *)&bytes[0], bytes.size());

            for (i = 0; i < 320; i++)
            {
                short original_sample = ((short)(bytes[(i * 2) + 1]) << 8) |
                                        (0x00ff & (short)bytes[i * 2]);
                float converted_sample = original_sample / 32768.f;

                // if (original_sample != 0)
                // {
                //     std::cout << "original sample: " << original_sample << ", converted sample: " << converted_sample << std::endl;
                // }
                buffer[0][i] = converted_sample;

                // std::cout << std::setw(2) << std::setfill('0') << (int)bytes[(i * 2) + 1] << std::setw(2) << std::setfill('0') << (int)bytes[i * 2] << " ";
            }

            // for (i = 0; i < 320; i++)
            // {

            // }

            // pcm_file.write((char *)&buffer[0][0], 320 * 4);

            // std::cout << std::dec << std::endl;

            /* tell the library how much we actually submitted */
            // vorbis_analysis_wrote(&vd, i);
            vorbis_analysis_wrote(&vd, 320);
        }

        /* vorbis does some data preanalysis, then divvies up blocks for
           more involved (potentially parallel) processing.  Get a single
           block for encoding now */
        while (vorbis_analysis_blockout(&vd, &vb) == 1)
        {

            /* analysis, assume we want to use bitrate management */
            vorbis_analysis(&vb, NULL);
            vorbis_bitrate_addblock(&vb);

            while (vorbis_bitrate_flushpacket(&vd, &op))
            {

                /* weld the packet into the bitstream */
                ogg_stream_packetin(&os, &op);

                /* write out pages (if any) */
                while (!eos)
                {
                    std::lock_guard<std::mutex> guard(ogg_buffer_mutex);
                    int result = ogg_stream_pageout(&os, &og);
                    if (result == 0)
                    {
                        if (ogg_stream_flush(&os, &og) == 0)
                        {
                            break;
                        }
                    }
                    // fwrite(og.header, 1, og.header_len, fout);
                    for (auto i = 0; i < og.header_len; ++i)
                    {
                        auto value = ((uint8_t *)og.header)[i];
                        // std::cout << std::hex << std::setw(2) << std::setfill('0') << "pushed " << (int)value << std::dec << std::endl;
                        ogg_buffer.get()->push_back(value);
                    }
                    // fwrite(og.body, 1, og.body_len, fout);
                    for (auto i = 0; i < og.body_len; ++i)
                    {
                        auto value = ((uint8_t *)og.body)[i];
                        // std::cout << std::hex << std::setw(2) << std::setfill('0') << "pushed " << (int)value << std::dec << std::endl;
                        ogg_buffer.get()->push_back(value);
                    }
                    // fout.write((char *)&og.header, og.header_len);
                    // fout.write((char *)&og.body, og.body_len);

                    /* this could be set above, but for illustrative purposes, I do
                       it here (to show that vorbis does know where the stream ends) */

                    if (ogg_page_eos(&og))
                        eos = 1;
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed = std::chrono::duration<double, std::milli>(end - start);
        auto target = std::chrono::duration<double, std::milli>(20.0);

        std::this_thread::sleep_for(target - elapsed);
    }

    /* clean up and exit.  vorbis_info_clear() must be called last */

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
}

void encode_audio(std::shared_ptr<std::vector<uint8_t>> pcm_buffer, std::shared_ptr<std::vector<uint8_t>> ogg_buffer, bool &running)
{
    auto callback = [&running, &pcm_buffer]() -> std::vector<uint8_t>
    {
        if (!running)
        {
            return std::vector<uint8_t>();
        }
        else
        {
            return get_next_pcm_chunk(pcm_buffer);
        }
    };

    encode_pcm_as_vorbis(ogg_buffer, callback);
}

void send_audio(std::shared_ptr<std::vector<uint8_t>> ogg_buffer, bool &running)
{
    shout_t *shout;
    int ret;

    // std::ofstream pcm_file("output.pcm", std::ios::binary);
    // std::ofstream fout("output.mp3", std::ios::binary);

    shout_init();

    if (!(shout = shout_new()))
    {
        // std::cout << "Could not allocate shout_t" << std::endl;
        return;
    }

    if (shout_set_host(shout, "127.0.0.1") != SHOUTERR_SUCCESS)
    {
        // std::cout << "Error setting hostname: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS)
    {
        // std::cout << "Error setting protocol: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_port(shout, 8000) != SHOUTERR_SUCCESS)
    {
        // std::cout << "Error setting port: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_password(shout, "CorrectHorseBatteryStaple") != SHOUTERR_SUCCESS)
    {
        // std::cout << "Error setting password: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_mount(shout, "/stream.ogg") != SHOUTERR_SUCCESS)
    {
        // std::cout << "Error setting mount: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_user(shout, "source") != SHOUTERR_SUCCESS)
    {
        // std::cout << "Error setting user: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_set_content_format(shout, SHOUT_FORMAT_OGG, SHOUT_USAGE_UNKNOWN, NULL) != SHOUTERR_SUCCESS)
    {
        // std::cout << "Error setting format: " << shout_get_error(shout) << std::endl;
        return;
    }

    if (shout_open(shout) == SHOUTERR_SUCCESS)
    {
        FILE *fout = fopen("output.ogg", "wb");
        std::cout << "Connected to server..." << std::endl;
        size_t read = 0, total = 0;
        unsigned char buff[4096];
        size_t buffer_size;
        while (running)
        {
            {
                std::lock_guard<std::mutex> guard(ogg_buffer_mutex);
                buffer_size = ogg_buffer.get()->size();

                // std::cout << buffer_size << std::endl;

                if (buffer_size >= 4096)
                {
                    auto index = 0;
                    for (auto it = ogg_buffer.get()->begin(); it != (ogg_buffer.get()->begin() + 4096); ++it)
                    {
                        auto value = *it;
                        // std::cout << std::hex << std::setw(2) << std::setfill('0') << "popped " << (int)value << std::dec << std::endl;
                        buff[index] = value;
                        fwrite(&value, 1, 1, fout);
                        index++;
                    }
                }
            }

            if (buffer_size >= 4096)
            {
                ret = shout_send(shout, buff, 4096);

                if (ret != SHOUTERR_SUCCESS)
                {
                    std::cout << "send error: " << shout_get_error(shout) << std::endl;
                    break;
                }

                ogg_buffer.get()->erase(ogg_buffer.get()->begin(), ogg_buffer.get()->begin() + 4096);

                shout_sync(shout);
            }
        }

        while (true)
        {

            {
                std::lock_guard<std::mutex> guard(ogg_buffer_mutex);
                buffer_size = ogg_buffer.get()->size();

                if (buffer_size == 0)
                {
                    break;
                }

                auto index = 0;
                for (auto it = ogg_buffer.get()->begin(); it != ogg_buffer.get()->begin() + (buffer_size % 4096); ++it)
                {
                    auto value = *it;
                    // std::cout << std::hex << std::setw(2) << std::setfill('0') << "popped " << (int)value << std::dec << std::endl;
                    buff[index] = value;
                    fwrite(&value, 1, 1, fout);
                    index++;
                }
            }

            ret = shout_send(shout, buff, buffer_size % 4096);

            if (ret != SHOUTERR_SUCCESS)
            {
                std::cout << "send error: " << shout_get_error(shout) << std::endl;
            }

            ogg_buffer.get()->erase(ogg_buffer.get()->begin(), ogg_buffer.get()->begin() + (buffer_size % 4096));

            shout_sync(shout);
        }

        // if (ogg_buffer.get()->size() > 0)
        // {
        //     std::cout << "ogg buffer size: " << ogg_buffer.get()->size() << std::endl;
        //     fwrite((char *)ogg_buffer.get(), 1, ogg_buffer.get()->size(), fout);
        //     ret = shout_send(shout, (unsigned char *)ogg_buffer.get(), ogg_buffer.get()->size());
        //     if (ret != SHOUTERR_SUCCESS)
        //     {
        //         printf("DEBUG: Send error: %s\n", shout_get_error(shout));
        //         break;
        //     }

        //     shout_sync(shout);
        // }
    }
    else
    {
        // std::cout << "Error connecting: " << shout_get_error(shout) << std::endl;
    }
}

int main()
{
    auto speech_key = std::string(std::getenv("SPEECH_KEY"));       // 0d3c308c3e4146bf9d03d04a46984922
    auto speech_region = std::string(std::getenv("SPEECH_REGION")); // centralus

    std::shared_ptr<std::vector<uint8_t>> pcm_buffer = std::make_shared<std::vector<uint8_t>>();
    std::shared_ptr<std::vector<uint8_t>> ogg_buffer = std::make_shared<std::vector<uint8_t>>();
    bool running = true;

    auto recognizer = init_translation_recognizer(speech_key, speech_region, pcm_buffer);

    recognizer->StartContinuousRecognitionAsync().get();

    std::thread audio_encode_thread(encode_audio, pcm_buffer, ogg_buffer, std::ref(running));
    std::thread audio_broadcast_thread(send_audio, ogg_buffer, std::ref(running));

    // std::cout
    //     << "Press enter to stop recording..." << std::endl;
    std::getchar();

    running = false;

    recognizer->StopContinuousRecognitionAsync().get();
    audio_encode_thread.join();
    audio_broadcast_thread.join();

    // std::cout << "stopped." << std::endl;
    return 0;
}