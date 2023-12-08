#include "wav_header_remover.h"

void languid::WavHeaderRemover::remove_header(std::vector<uint8_t> &wav_data)
{
    wav_data.erase(wav_data.begin(), wav_data.begin() + 44);
}