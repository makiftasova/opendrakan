/*
 * FluidSynth.cpp
 *
 *  Created on: 25 Jun 2019
 *      Author: zal
 */

#include <odOsg/audio/music/FluidSynth.h>

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>

#include <unistd.h>

#include <odCore/Logger.h>
#include <odCore/Panic.h>

#include <odCore/db/MusicContainer.h>

#include <odOsg/audio/music/DlsTranscoder.h>

namespace odOsg
{

    class DlsLoaderWrapper
    {
    public:

        DlsLoaderWrapper(odDb::MusicContainer &container, const od::Guid &guid)
        : mContainer(container)
        , mGuid(guid)
        , mInputCursor(mContainer.getCursorForDls(guid))
        , mReader(mInputCursor.getReader())
        , mDataOffset(mInputCursor.getDirIterator()->dataOffset)
        , mDataSize(mInputCursor.getDirIterator()->dataSize)
        {
        }

        void read(void *buf, int count)
        {
            mReader.read(reinterpret_cast<char*>(buf), count);
        }

        /**
         * @brief Returns the complete DLS data of the record.
         */
        std::vector<char> readAll()
        {
            std::vector<char> data(mDataSize);
            mReader.seek(mDataOffset);
            mReader.read(data.data(), data.size());
            return data;
        }

        void seek(long offset, int origin)
        {
            switch(origin)
            {
            case SEEK_SET:
                mReader.seek(offset + mDataOffset);
                break;

            case SEEK_CUR:
                mReader.seek(offset + mReader.tell());
                break;

            case SEEK_END:
                mReader.seek(offset + mDataOffset + mDataSize);
                break;

            default:
                OD_PANIC() << "Unsupported seek offset";
            }
        }

        long tell()
        {
            return mReader.tell() - mDataOffset;
        }

    private:

        odDb::MusicContainer &mContainer;
        od::Guid mGuid;
        od::SrscFile::RecordInputCursor mInputCursor;
        od::DataReader mReader;
        size_t mDataOffset;
        size_t mDataSize;

    };


    static void _errorCheck(int errorCode, const char *errorMessage)
    {
        if(errorCode != FLUID_OK)
        {
            OD_PANIC() << errorMessage;
        }
    }


    /**
     * Live MIDI messages may legitimately be rejected by fluidsynth (e.g. a note off for a note that is not sounding
     * anymore, or messages for a channel without a preset). That is no reason to bring down the game.
     */
    static void _midiMessageCheck(int errorCode, const char *errorMessage)
    {
        if(errorCode != FLUID_OK)
        {
            Logger::debug() << errorMessage;
        }
    }


    FluidSynth::Settings::Settings()
    {
        mInstance = new_fluid_settings();
        if(mInstance == nullptr)
        {
            OD_PANIC() << "Failed to create fluid synth settings";
        }
    }

    FluidSynth::Settings::~Settings()
    {
        delete_fluid_settings(mInstance);
    }

    void FluidSynth::Settings::setString(const char *key, const char *value)
    {
        fluid_settings_setstr(mInstance, key, value);
    }


    FluidSynth::FluidSynth()
    : mSynth(nullptr)
    , mMusicContainer(nullptr)
    {
        mSettings.setString("player.timing-source", "sample");

        mSynth = new_fluid_synth(mSettings.getInstance());
        if(mSynth == nullptr) OD_PANIC() << "Failed to create fluid synth";
    }

    FluidSynth::~FluidSynth()
    {
        delete_fluid_synth(mSynth);

        for(auto &path : mTempDlsFiles)
        {
            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    }

    void FluidSynth::noteOn(uint8_t channel, uint8_t note, uint8_t velocity)
    {
        int result = fluid_synth_noteon(mSynth, channel, note, velocity);
        _midiMessageCheck(result, "Fluidsynth rejected noteon message");
    }

    void FluidSynth::noteOff(uint8_t channel, uint8_t note)
    {
        int result = fluid_synth_noteoff(mSynth, channel, note);
        _midiMessageCheck(result, "Fluidsynth rejected noteoff message");
    }

    void FluidSynth::pitchBend(uint8_t channel, uint16_t value)
    {
        int result = fluid_synth_pitch_bend(mSynth, channel, value);
        _midiMessageCheck(result, "Fluidsynth rejected pitch bend message");
    }

    void FluidSynth::controllerChange(uint8_t channel, uint8_t controller, uint8_t value)
    {
        int result = fluid_synth_cc(mSynth, channel, controller, value);
        _midiMessageCheck(result, "Fluidsynth rejected controller change message");
    }

    void FluidSynth::channelPressure(uint8_t channel, uint8_t value)
    {
        int result = fluid_synth_channel_pressure(mSynth, channel, value);
        _midiMessageCheck(result, "Fluidsynth rejected channel pressure message");
    }

    void FluidSynth::keyPressure(uint8_t channel, uint8_t key, uint8_t value)
    {
        int result = fluid_synth_key_pressure(mSynth, channel, key, value);
        _midiMessageCheck(result, "Fluidsynth rejected key pressure message");
    }

    void FluidSynth::allNotesOff()
    {
    }

    void FluidSynth::fillInterleavedStereoBuffer(int16_t *buffer, size_t size)
    {
        int result = fluid_synth_write_s16(mSynth, size/2, buffer, 0, 2, buffer, 1, 2);
        _errorCheck(result, "Synthesis failed");
    }

    void FluidSynth::preloadDls(const od::Guid &dlsGuid)
    {
        _getOrLoadDls(dlsGuid);
    }

    void FluidSynth::assignPreset(uint8_t channel, uint32_t bank, uint32_t patch, const od::Guid &dlsGuid)
    {
        int sfId = _getOrLoadDls(dlsGuid);
        if(sfId < 0)
        {
            return; // the DLS could not be loaded. nothing to select
        }

        int result = fluid_synth_program_select(mSynth, channel, sfId, bank, patch);
        if(result != FLUID_OK && bank != 0)
        {
            // fluidsynth exposes all presets of a DLS file as bank 0, regardless of the bank number in the file.
            //  since every DLS is loaded as a soundfont of it's own, this is unambiguous
            result = fluid_synth_program_select(mSynth, channel, sfId, 0, patch);
        }
        _errorCheck(result, "Failed to select program");
    }

    int FluidSynth::_getOrLoadDls(const od::Guid &dlsGuid)
    {
        auto it = mSoundFontIdMap.find(dlsGuid);
        if(mSoundFontIdMap.find(dlsGuid) != mSoundFontIdMap.end())
        {
            return it->second;
        }

        Logger::verbose() << "Loading DLS " << dlsGuid;

        if(mMusicContainer == nullptr)
        {
            OD_PANIC() << "No music container provided to FluidSynth when a DLS needed to be loaded";
        }

        // fluidsynth's built-in DLS loader opens the file itself and doesn't go through custom loader callbacks
        //  (as of fluidsynth 2.x), so we dump the DLS data from the music container into a temporary file.
        // Drakan's DLS files contain MS ADPCM waves, which fluidsynth can't load, so we convert them to PCM first.
        DlsLoaderWrapper loaderWrapper(*mMusicContainer, dlsGuid);
        std::vector<char> dlsData;
        try
        {
            dlsData = DlsTranscoder::convertAdpcmToPcm(loaderWrapper.readAll());

        }catch(const DlsError &e)
        {
            // a broken instrument bank must not take the whole game down. the music using it will just be silent
            Logger::error() << "Can't use DLS " << dlsGuid << ": " << e.what();
            mSoundFontIdMap.insert(std::make_pair(dlsGuid, -1));
            return -1;
        }

        std::filesystem::path tempPath = std::filesystem::temp_directory_path() /
                ("opendrakan_" + std::to_string(::getpid()) + "_" + std::to_string(mTempDlsFiles.size()) + ".dls");
        {
            std::ofstream out(tempPath, std::ios::binary);
            out.write(dlsData.data(), dlsData.size());
            if(!out)
            {
                OD_PANIC() << "Failed to write temporary DLS file " << tempPath;
            }
        }
        mTempDlsFiles.push_back(tempPath);

        int sfId = fluid_synth_sfload(mSynth, tempPath.c_str(), 0);
        if(sfId == FLUID_FAILED)
        {
            Logger::error() << "FluidSynth failed to load DLS " << dlsGuid << ". The music using it will be silent";
            mSoundFontIdMap.insert(std::make_pair(dlsGuid, -1));
            return -1;
        }

        mSoundFontIdMap.insert(std::make_pair(dlsGuid, sfId));

        Logger::debug() << "Successfully loaded DLS " << dlsGuid;

        return sfId;
    }

}
