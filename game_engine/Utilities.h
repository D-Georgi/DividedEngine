#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <optional>
#include <list>
#include <set>
#include <map>
#include <memory>
#include <thread>
#include <cstdlib>
#include <queue>

#include "glm/glm/glm.hpp" // Student : You need to get glm added to your project source code or this line will fail.
// Search for your IDE's "include / header directories" option (it's called something different everywhere)
// Ensure you have the glm folder listed so your compiler can actually find these glm headers.
#include "glm/glm/geometric.hpp"
#include "glm/glm/vec2.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"

#include "Helper.h"
#include "AudioHelper.h"

#include "Lua/lua.hpp"
#include "LuaBridge.h"

#include "box2d/box2d.h"

using namespace std;

enum Command {
    North = 'n',
    South = 's',
    East = 'e',
    West = 'w',
    Quit = 'q' // Assuming you change "quit" to just 'q' for simplicity
};

class EngineUtils {
public:
	static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document)
	{
		FILE* file_pointer = nullptr;
#ifdef _WIN32
		fopen_s(&file_pointer, path.c_str(), "rb");
#else
		file_pointer = fopen(path.c_str(), "rb");
#endif
		char buffer[65536];
		rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
		out_document.ParseStream(stream);
		std::fclose(file_pointer);

		if (out_document.HasParseError()) {
			rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
			std::cout << "error parsing json at [" << path << "]" << std::endl;
			exit(0);
		}
	}
	//test grade 4
	void CheckForFile(string filepath) {
		const std::filesystem::path path(filepath);

		if (!std::filesystem::exists(path)) {
			std::cout << "error: "<< filepath << " missing";
			exit(0);
		}
	}

	bool CheckForScene(string scene_name) {
		string fullPath = "resources/scenes/" + scene_name + ".scene";
		const std::filesystem::path path(fullPath);

		if (!std::filesystem::exists(path)) {
			std::cout << "error: scene " << scene_name << " is missing";
			exit(0);
		}
		return true;
	}

	void CheckForImage(string imageName) {
		string fullPath = "resources/images/" + imageName + ".png";
		const std::filesystem::path path(fullPath);
		if (!std::filesystem::exists(path)) {
			std::cout << "error: missing image " << imageName;
			exit(0);
		}
	}

	void CheckForFont(string filename) {
		string fullPath = "resources/fonts/" + filename + ".ttf";
		const std::filesystem::path path(fullPath);
		if (!std::filesystem::exists(path)) {
			std::cout << "error: font " << filename << " missing";
			exit(0);
		}
	}

	string CheckForAudio(string filename) {
		string wavFull = "resources/audio/" + filename + ".wav";
		string oggFull = "resources/audio/" + filename + ".ogg";
		const std::filesystem::path wavPath(wavFull);
		const std::filesystem::path oggPath(oggFull);
		if (!std::filesystem::exists(wavPath) && !std::filesystem::exists(oggPath)) {
			std::cout << "error: audiofile " << filename << " missing";
			exit(0);
		}
		if (std::filesystem::exists(wavPath)) {
			return ".wav";
		}
		else if (std::filesystem::exists(oggPath)) {
			return ".ogg";
		}
        return "";
	}

	bool DoesFileExist(string fileName) {
		string fullPath = "resources/" + fileName;
		const std::filesystem::path path(fullPath);

		if (!std::filesystem::exists(path)) {
			return false;
		}
		return true;
	}

	string ObtainWordAfterPhrase(const string& input, const string& phrase) {
		size_t pos = input.find(phrase);
		if (pos == std::string::npos) return "";

		pos += phrase.length();
		while (pos < input.size() && std::isspace(input[pos])) {
			++pos;
		}

		if (pos == input.size()) return "";

		size_t endPos = pos;
		while (endPos < input.size() && !std::isspace(input[endPos])) {
			++endPos;
		}

		return input.substr(pos, endPos - pos);
	}
};

static uint64_t create_composite_key(int x, int y) {
	uint32_t ux = static_cast<uint32_t>(x);
	uint32_t uy = static_cast<uint32_t>(y);

	uint64_t result = static_cast<uint64_t>(ux);
	
	result = result << 32;

	result = result | static_cast<uint64_t>(uy);

	return result;
}

inline bool compareLuaRefByKey(const std::shared_ptr<luabridge::LuaRef> a_ptr, const std::shared_ptr<luabridge::LuaRef> b_ptr) {
	const luabridge::LuaRef a = *a_ptr;
	const luabridge::LuaRef b = *b_ptr;

	if (!a.isTable() || !b.isTable()) return false;

	std::string keyA = a["key"].cast<std::string>();
	std::string keyB = b["key"].cast<std::string>();
	return keyA < keyB;
}

inline void ReportError(const std::string& actor_name, const luabridge::LuaException& e) {
	std::string error_message = e.what();

	std::replace(error_message.begin(), error_message.end(), '\\', '/');

	std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << std::endl;
}