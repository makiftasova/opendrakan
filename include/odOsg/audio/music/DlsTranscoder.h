/*
 * DlsTranscoder.h
 */

#ifndef INCLUDE_ODOSG_AUDIO_MUSIC_DLSTRANSCODER_H_
#define INCLUDE_ODOSG_AUDIO_MUSIC_DLSTRANSCODER_H_

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace odOsg
{

    /**
     * @brief Utilities for making Drakan's DLS files digestible for synthesizers that only support PCM waves.
     *
     * Drakan's instrument banks store most of their waves as Microsoft ADPCM (WAVE format tag 2, 4 bits per sample),
     * which e.g. FluidSynth can't load.
     */
    /**
     * @brief Thrown when a DLS file is malformed in a way that prevents it from being converted.
     */
    class DlsError : public std::runtime_error
    {
    public:

        explicit DlsError(const std::string &message) : std::runtime_error(message) { }

    };


    class DlsTranscoder
    {
    public:

        /**
         * @brief Rewrites a DLS file, replacing all MS ADPCM waves with 16 bit PCM waves.
         *
         * All other chunks are copied unchanged. The offsets in the DLS file's pool table are adjusted to account for
         * the changed wave sizes. Will throw (via OD_PANIC) if the data is not a well-formed DLS file.
         */
        static std::vector<char> convertAdpcmToPcm(const std::vector<char> &dls);

        /**
         * @brief Decodes mono MS ADPCM data to 16 bit PCM.
         *
         * @param blockAlign        Size in bytes of one ADPCM block.
         * @param coefficients      Pairs of predictor coefficients from the format header.
         */
        static std::vector<int16_t> decodeMsAdpcm(const uint8_t *data, size_t size, size_t blockAlign,
                const std::vector<std::pair<int16_t, int16_t>> &coefficients);

    };

}

#endif /* INCLUDE_ODOSG_AUDIO_MUSIC_DLSTRANSCODER_H_ */
