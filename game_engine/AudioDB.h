#pragma once

#include "Utilities.h"

class AudioDB
{
private:
    static inline unordered_map<string, Mix_Chunk*> audioClips;
public:

	AudioDB() {
		AudioHelper::Mix_OpenAudio498(44100, MIX_DEFAULT_FORMAT, 1, 2048);
		AudioHelper::Mix_AllocateChannels498(50);
	}
	~AudioDB() {

	}

    static void SetupAudio(rapidjson::Document& config, const std::string& name) {
        // Convert std::string to const char* for rapidjson
        const char* key = name.c_str();

        // Make sure the key exists and is a string before attempting to access it
        if (config.HasMember(key) && config[key].IsString()) {
            std::string audioName = config[key].GetString();
            EngineUtils engin;
            std::string type = engin.CheckForAudio(audioName);
            std::string audioPath = "resources/audio/" + audioName + type;

            //TODO: actually store the audio
        }
        else {
            // Handle the case where the key doesn't exist or isn't a string
            std::cerr << "Error: Config key '" << name << "' is missing or not a string." << std::endl;
        }
    }

    static inline Mix_Chunk* GetClip(string clip_name) {
        auto it = audioClips.find(clip_name);
        if (it != audioClips.end()) {
            return it->second;
        }
        else {
            EngineUtils engin;
            std::string type = engin.CheckForAudio(clip_name);
            std::string audioPath = "resources/audio/" + clip_name + type;
            audioClips[clip_name] = AudioHelper::Mix_LoadWAV498(audioPath.c_str());
            return audioClips[clip_name];
        }
    }

	static inline void PlayAudio(int channelNum, string clip_name, bool _loops) {
        Mix_Chunk* audioClip = GetClip(clip_name);
        int loops = 0;
        if (_loops) {
            loops = -1;
        }
		AudioHelper::Mix_PlayChannel498(channelNum, audioClip, loops);
	}

    static inline void cppHalt(int channel) {
        AudioHelper::Mix_HaltChannel498(channel);
    }

    static inline void SetVolume(int channel, int volume) {
        AudioHelper::Mix_Volume498(channel, volume);
    }
};

