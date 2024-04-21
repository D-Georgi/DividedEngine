#pragma once

#include "Utilities.h"

struct DialogueEntry {
	std::string content;
	int x;
	int y;
	TTF_Font* font;
	int r;
	int g;
	int b;
	int a;

	DialogueEntry(const std::string& text, int _x, int _y, TTF_Font* _font, int _r, int _g, int _b, int _a) :
		content(text), x(_x), y(_y), font(_font), r(_r), g(_g), b(_b), a(_a) {}
};

class TextDB
{
private:
	static inline TTF_Font* AddFont(const string& fontName, const int& fontSize) {
		string fontPath = "resources/fonts/" + fontName + ".ttf";
		EngineUtils engin;
		engin.CheckForFont(fontName);
		TTF_Font* font = TTF_OpenFont(fontPath.c_str(), fontSize);
		if (!font) {
			cout << "Failed to load font: " << TTF_GetError();
			exit(1); // Or handle the error in a more graceful way
		}
		fonts[fontName][fontSize] = font;
		return font;
	}

public:
	static inline std::unordered_map<string, std::unordered_map<int, TTF_Font*>> fonts;
	static inline SDL_Color textColor = { 255, 255, 255, 255 };

	static inline std::queue<DialogueEntry*> textRequests;

	TextDB() {
		if (TTF_Init() == -1) {
			cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError();
			exit(1);
		}
	}

	~TextDB() {
		if (!fonts.empty()) {
			for (auto& fontSizePair : fonts) {
				for (auto& font : fontSizePair.second) {
					TTF_CloseFont(font.second);
				}
			}
		}
		while (!textRequests.empty()) {
			DialogueEntry* top = textRequests.front();
			textRequests.pop();
			delete top;
			top = nullptr;
		}
		TTF_Quit();
	}

	static inline void AddTextRequest(string content, int x, int y, string font_name, int font_size, int r, int g, int b, int a) {
		TTF_Font* font;
		
		auto it = fonts.find(font_name);
		if (it != fonts.end() && it->second.find(font_size) != it->second.end()) {
			font = fonts[font_name][font_size];
		}
		else {
			font = AddFont(font_name, font_size);
		}

		DialogueEntry* newEntry = new DialogueEntry(content, x, y, font, r, g, b, a);
		textRequests.push(newEntry);
	}
};

