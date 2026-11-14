/*
 * DlsTranscoder.cpp
 */

#include <odOsg/audio/music/DlsTranscoder.h>

#include <algorithm>
#include <cstring>
#include <map>

#include <odCore/Panic.h>

namespace odOsg
{

    namespace
    {

        constexpr uint16_t WAVE_FORMAT_PCM = 1;
        constexpr uint16_t WAVE_FORMAT_ADPCM = 2;

        uint16_t readU16(const char *p)
        {
            uint8_t b[2];
            std::memcpy(b, p, 2);
            return static_cast<uint16_t>(b[0] | (b[1] << 8));
        }

        uint32_t readU32(const char *p)
        {
            uint8_t b[4];
            std::memcpy(b, p, 4);
            return static_cast<uint32_t>(b[0]) | (static_cast<uint32_t>(b[1]) << 8) | (static_cast<uint32_t>(b[2]) << 16) | (static_cast<uint32_t>(b[3]) << 24);
        }

        void appendU16(std::vector<char> &out, uint16_t v)
        {
            out.push_back(static_cast<char>(v & 0xff));
            out.push_back(static_cast<char>((v >> 8) & 0xff));
        }

        void appendU32(std::vector<char> &out, uint32_t v)
        {
            for(int i = 0; i < 4; ++i) out.push_back(static_cast<char>((v >> (8*i)) & 0xff));
        }

        void patchU32(std::vector<char> &out, size_t pos, uint32_t v)
        {
            for(int i = 0; i < 4; ++i) out[pos + i] = static_cast<char>((v >> (8*i)) & 0xff);
        }

        void appendId(std::vector<char> &out, const char *id)
        {
            out.insert(out.end(), id, id + 4);
        }

        void appendBytes(std::vector<char> &out, const char *data, size_t size)
        {
            out.insert(out.end(), data, data + size);
        }

        void pad(std::vector<char> &out)
        {
            if(out.size() & 1) out.push_back(0);
        }

        /**
         * A view on one RIFF chunk inside a buffer. For LIST/RIFF chunks, 'dataOffset' points to the list type id.
         */
        struct Chunk
        {
            char id[4];
            size_t headerOffset;
            size_t dataOffset;
            size_t dataSize;
            size_t nextOffset; // including padding

            bool is(const char *s) const { return std::memcmp(id, s, 4) == 0; }
            bool isList() const { return is("LIST") || is("RIFF"); }
        };

        bool readChunk(const std::vector<char> &buf, size_t offset, size_t end, Chunk &chunk)
        {
            if(offset + 8 > end) return false;

            std::memcpy(chunk.id, &buf[offset], 4);
            chunk.headerOffset = offset;
            chunk.dataOffset = offset + 8;
            chunk.dataSize = readU32(&buf[offset + 4]);
            if(chunk.dataOffset + chunk.dataSize > end)
            {
                throw DlsError(std::string() + "Chunk in DLS exceeds bounds of its parent");
            }
            chunk.nextOffset = std::min(chunk.dataOffset + chunk.dataSize + (chunk.dataSize & 1), buf.size());
            return true;
        }

        bool listTypeIs(const std::vector<char> &buf, const Chunk &chunk, const char *type)
        {
            return chunk.isList() && chunk.dataSize >= 4 && std::memcmp(&buf[chunk.dataOffset], type, 4) == 0;
        }

        // starts a chunk in the output. returns the position of the size field to be patched using endChunk
        size_t beginChunk(std::vector<char> &out, const char *id)
        {
            appendId(out, id);
            size_t sizePos = out.size();
            appendU32(out, 0);
            return sizePos;
        }

        void endChunk(std::vector<char> &out, size_t sizePos)
        {
            patchU32(out, sizePos, static_cast<uint32_t>(out.size() - sizePos - 4));
            pad(out);
        }

        /**
         * Regions in Drakan's DLS files have a velocity range of 0-0, which DirectMusic evidently treats as "unspecified"
         * (i.e. the full range). Other synthesizers take it literally, so the region would only match velocity 0 (which
         * is a note off) and never sound. Replace such ranges by the full range.
         */
        void fixRegionVelocityRanges(std::vector<char> &buf, size_t offset, size_t end)
        {
            Chunk c;
            while(readChunk(buf, offset, end, c))
            {
                if(c.isList())
                {
                    // only look into the lists that can contain instrument regions. (the wave pool and the lists within it are not
                    //  interesting here, and some files have sloppy chunk sizes in them)
                    if(c.dataSize >= 4 && (listTypeIs(buf, c, "lins") || listTypeIs(buf, c, "ins ") || listTypeIs(buf, c, "lrgn") || listTypeIs(buf, c, "rgn ")))
                    {
                        fixRegionVelocityRanges(buf, c.dataOffset + 4, c.dataOffset + c.dataSize);
                    }

                }else if(c.is("rgnh") && c.dataSize >= 8)
                {
                    char *velocityRange = &buf[c.dataOffset + 4];
                    uint16_t low = readU16(velocityRange);
                    uint16_t high = readU16(velocityRange + 2);
                    if(low == 0 && high == 0)
                    {
                        velocityRange[2] = 127;
                        velocityRange[3] = 0;
                    }
                }

                offset = c.nextOffset;
            }
        }

        // converts one 'wave' LIST. appends the converted list to out
        void convertWave(const std::vector<char> &in, const Chunk &waveList, std::vector<char> &out)
        {
            // first pass: find format chunk
            bool isAdpcm = false;
            uint16_t channels = 0;
            uint32_t sampleRate = 0;
            uint16_t blockAlign = 0;
            std::vector<std::pair<int16_t, int16_t>> coefficients;

            size_t offset = waveList.dataOffset + 4;
            size_t end = waveList.dataOffset + waveList.dataSize;
            Chunk c;
            while(readChunk(in, offset, end, c))
            {
                if(c.is("fmt ") && c.dataSize >= 16)
                {
                    const char *f = &in[c.dataOffset];
                    uint16_t tag = readU16(f);
                    if(tag == WAVE_FORMAT_ADPCM)
                    {
                        isAdpcm = true;
                        channels = readU16(f + 2);
                        sampleRate = readU32(f + 4);
                        blockAlign = readU16(f + 12);

                        // extra data: cbSize, samplesPerBlock, numCoef, coefficient pairs
                        if(c.dataSize < 22)
                        {
                            throw DlsError(std::string() + "Malformed ADPCM format chunk in DLS");
                        }
                        uint16_t numCoef = readU16(f + 20);
                        if(c.dataSize < 22u + numCoef*4u)
                        {
                            throw DlsError(std::string() + "ADPCM format chunk in DLS too small for its coefficient table");
                        }
                        for(uint16_t i = 0; i < numCoef; ++i)
                        {
                            coefficients.emplace_back(static_cast<int16_t>(readU16(f + 22 + i*4)),
                                                      static_cast<int16_t>(readU16(f + 24 + i*4)));
                        }
                    }
                }
                offset = c.nextOffset;
            }

            if(!isAdpcm)
            {
                // nothing to do. copy as is
                appendBytes(out, &in[waveList.headerOffset], waveList.nextOffset - waveList.headerOffset);
                return;
            }

            if(channels != 1)
            {
                throw DlsError(std::string() + "Only mono ADPCM waves in DLS are supported");
            }

            // second pass: write new list
            size_t listSizePos = beginChunk(out, "LIST");
            appendId(out, "wave");

            offset = waveList.dataOffset + 4;
            while(readChunk(in, offset, end, c))
            {
                if(c.is("fmt "))
                {
                    size_t sizePos = beginChunk(out, "fmt ");
                    appendU16(out, WAVE_FORMAT_PCM);
                    appendU16(out, 1);
                    appendU32(out, sampleRate);
                    appendU32(out, sampleRate * 2);
                    appendU16(out, 2);  // block align
                    appendU16(out, 16); // bits per sample
                    endChunk(out, sizePos);

                }else if(c.is("data"))
                {
                    auto pcm = DlsTranscoder::decodeMsAdpcm(reinterpret_cast<const uint8_t*>(&in[c.dataOffset]), c.dataSize, blockAlign, coefficients);

                    size_t sizePos = beginChunk(out, "data");
                    for(int16_t s : pcm) appendU16(out, static_cast<uint16_t>(s));
                    endChunk(out, sizePos);

                }else
                {
                    appendBytes(out, &in[c.headerOffset], c.nextOffset - c.headerOffset);
                }

                offset = c.nextOffset;
            }

            endChunk(out, listSizePos);
        }

    }


    std::vector<int16_t> DlsTranscoder::decodeMsAdpcm(const uint8_t *data, size_t size, size_t blockAlign,
            const std::vector<std::pair<int16_t, int16_t>> &coefficients)
    {
        static const int adaptationTable[16] = { 230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512, 409, 307, 230, 230, 230 };

        constexpr size_t HEADER_SIZE = 7; // for mono

        if(blockAlign <= HEADER_SIZE)
        {
            throw DlsError("Invalid ADPCM block size " + std::to_string(blockAlign));
        }

        std::vector<int16_t> out;
        out.reserve((size/blockAlign + 1) * (blockAlign - HEADER_SIZE) * 2);

        for(size_t blockStart = 0; blockStart + HEADER_SIZE <= size; blockStart += blockAlign)
        {
            size_t blockSize = std::min(blockAlign, size - blockStart);
            const uint8_t *b = data + blockStart;

            size_t predictorIndex = b[0];
            if(predictorIndex >= coefficients.size())
            {
                throw DlsError(std::string() + "ADPCM predictor index out of range");
            }
            int c1 = coefficients[predictorIndex].first;
            int c2 = coefficients[predictorIndex].second;

            int delta = static_cast<int16_t>(b[1] | (b[2] << 8));
            int sample1 = static_cast<int16_t>(b[3] | (b[4] << 8));
            int sample2 = static_cast<int16_t>(b[5] | (b[6] << 8));

            // the header contains the first two samples. the older one comes first
            out.push_back(static_cast<int16_t>(sample2));
            out.push_back(static_cast<int16_t>(sample1));

            auto decodeNibble = [&](unsigned nibble)
            {
                int signedNibble = (nibble & 0x8) ? static_cast<int>(nibble) - 16 : static_cast<int>(nibble);

                int predicted = (sample1*c1 + sample2*c2) / 256;
                int value = predicted + signedNibble*delta;
                value = std::max(-32768, std::min(32767, value));

                out.push_back(static_cast<int16_t>(value));

                sample2 = sample1;
                sample1 = value;

                delta = (adaptationTable[nibble]*delta) / 256;
                if(delta < 16) delta = 16;
            };

            for(size_t i = HEADER_SIZE; i < blockSize; ++i)
            {
                decodeNibble(b[i] >> 4);
                decodeNibble(b[i] & 0x0f);
            }
        }

        return out;
    }


    std::vector<char> DlsTranscoder::convertAdpcmToPcm(const std::vector<char> &in)
    {
        Chunk root;
        if(!readChunk(in, 0, in.size(), root) || !listTypeIs(in, root, "DLS ") || !root.is("RIFF"))
        {
            throw DlsError(std::string() + "Data is not a DLS file");
        }

        std::vector<char> out;
        size_t rootSizePos = beginChunk(out, "RIFF");
        appendId(out, "DLS ");

        // offsets in the pool table are relative to the start of the wave pool's data (after the 'wvpl' type id)
        //  and have to be remapped once the pool was rewritten
        size_t ptblOutOffset = 0; // position of the ptbl chunk's data in the output (0 = none seen)
        size_t ptblSize = 0;

        size_t offset = root.dataOffset + 4;
        size_t end = root.dataOffset + root.dataSize;
        Chunk c;
        while(readChunk(in, offset, end, c))
        {
            if(c.is("ptbl"))
            {
                ptblOutOffset = out.size() + 8;
                ptblSize = c.dataSize;
                appendBytes(out, &in[c.headerOffset], c.nextOffset - c.headerOffset);

            }else if(listTypeIs(in, c, "wvpl"))
            {
                size_t poolSizePos = beginChunk(out, "LIST");
                appendId(out, "wvpl");
                size_t newPoolDataStart = out.size();
                size_t oldPoolDataStart = c.dataOffset + 4;

                std::map<uint32_t, uint32_t> offsetMap;

                size_t waveOffset = c.dataOffset + 4;
                size_t poolEnd = c.dataOffset + c.dataSize;
                Chunk w;
                while(readChunk(in, waveOffset, poolEnd, w))
                {
                    offsetMap[static_cast<uint32_t>(w.headerOffset - oldPoolDataStart)] = static_cast<uint32_t>(out.size() - newPoolDataStart);

                    if(listTypeIs(in, w, "wave"))
                    {
                        convertWave(in, w, out);

                    }else
                    {
                        appendBytes(out, &in[w.headerOffset], w.nextOffset - w.headerOffset);
                    }

                    waveOffset = w.nextOffset;
                }

                endChunk(out, poolSizePos);

                if(ptblOutOffset == 0)
                {
                    throw DlsError(std::string() + "DLS has a wave pool, but no pool table before it");
                }

                // patch pool table
                uint32_t cueCount = readU32(&out[ptblOutOffset + 4]);
                if(ptblSize < 8 + cueCount*4u)
                {
                    throw DlsError(std::string() + "Pool table in DLS is too small for its cue count");
                }
                for(uint32_t i = 0; i < cueCount; ++i)
                {
                    size_t pos = ptblOutOffset + 8 + i*4;
                    uint32_t oldOffset = readU32(&out[pos]);
                    auto it = offsetMap.find(oldOffset);
                    if(it == offsetMap.end())
                    {
                        throw DlsError(std::string() + "Pool table entry in DLS does not point to a wave");
                    }
                    patchU32(out, pos, it->second);
                }

            }else
            {
                appendBytes(out, &in[c.headerOffset], c.nextOffset - c.headerOffset);
            }

            offset = c.nextOffset;
        }

        patchU32(out, rootSizePos, static_cast<uint32_t>(out.size() - rootSizePos - 4));

        fixRegionVelocityRanges(out, 12, out.size());

        return out;
    }

}
