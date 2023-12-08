#pragma once

namespace languid::constants
{
    const int PERIOD_LENGTH_IN_MILLISECONDS = 20;
    const int SAMPLERATE = 16000;
    const int SAMPLEWIDTH_IN_BYTES = 2;
    const int SAMPLES_PER_PERIOD = SAMPLERATE / (1000 / PERIOD_LENGTH_IN_MILLISECONDS);
    const int ICECAST_BUFFER_SIZE = 4096;
}