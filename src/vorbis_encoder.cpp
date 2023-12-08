#include <thread>
#include <vorbis/vorbisenc.h>
#include "vorbis_encoder.h"
#include "constants.h"

void languid::VorbisEncoder::run(std::shared_ptr<Buffer> in_buffer, std::shared_ptr<Buffer> out_buffer, bool &stopped)
{
    if (in_buffer == nullptr || out_buffer == nullptr)
    {
        throw std::invalid_argument("VorbisEncoder: buffer is null");
    }
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
    vorbis_comment_add_tag(&vc, "ENCODER", "Languid");

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
            int result = ogg_stream_flush(&os, &og);
            if (result == 0)
                break;
            for (auto i = 0; i < og.header_len; ++i)
            {
                auto value = ((uint8_t *)og.header)[i];
                out_buffer->write(&value, 1);
            }

            for (auto i = 0; i < og.body_len; ++i)
            {
                auto value = ((uint8_t *)og.body)[i];
                out_buffer->write(&value, 1);
            }
        }
    }

    while (!eos)
    {
        long i;

        if (!stopped && in_buffer->size() < (constants::SAMPLES_PER_PERIOD * constants::SAMPLEWIDTH_IN_BYTES))
        {
            continue;
        }

        // long bytes = fread(readbuffer, 1, READ * 4, stdin); /* stereo hardwired here */
        std::vector<uint8_t> bytes = stopped ? std::vector<uint8_t>() : in_buffer->read(constants::SAMPLES_PER_PERIOD * constants::SAMPLEWIDTH_IN_BYTES);

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
            float **buffer = vorbis_analysis_buffer(&vd, constants::SAMPLES_PER_PERIOD);

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

            for (i = 0; i < constants::SAMPLES_PER_PERIOD; i++)
            {
                short original_sample = ((short)(bytes[(i * 2) + 1]) << 8) |
                                        (0x00ff & (short)bytes[i * 2]);
                float converted_sample = original_sample / 32768.f;

                buffer[0][i] = converted_sample;
            }

            /* tell the library how much we actually submitted */
            // vorbis_analysis_wrote(&vd, i);
            vorbis_analysis_wrote(&vd, constants::SAMPLES_PER_PERIOD);
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
                    int result = ogg_stream_pageout(&os, &og);
                    if (result == 0)
                    {
                        if (ogg_stream_flush(&os, &og) == 0)
                        {
                            break;
                        }
                    }

                    for (auto i = 0; i < og.header_len; ++i)
                    {
                        auto value = ((uint8_t *)og.header)[i];
                        out_buffer->write(&value, 1);
                    }

                    for (auto i = 0; i < og.body_len; ++i)
                    {
                        auto value = ((uint8_t *)og.body)[i];
                        out_buffer->write(&value, 1);
                    }

                    /* this could be set above, but for illustrative purposes, I do
                       it here (to show that vorbis does know where the stream ends) */

                    if (ogg_page_eos(&og))
                        eos = 1;
                }
            }
        }
    }

    /* clean up and exit.  vorbis_info_clear() must be called last */

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
}