#pragma once

#include "Utilities.h"

static int globalRequestCounter = 0;

struct UIRequest {
	SDL_Texture* texture;
	int x, y, r, g, b, a, sorting_order;
	int insertionOrder;

	bool extended = false;

	UIRequest(SDL_Texture* _texture, int _x, int _y, int _r, int _g, int _b, int _a, int _sorting_order) : 
		texture(_texture), x(_x), y(_y), r(_r), g(_g), b(_b), a(_a), sorting_order(_sorting_order) {
		insertionOrder = globalRequestCounter++;
	}

	UIRequest(SDL_Texture* _texture, int _x, int _y) : texture(_texture), x(_x), y(_y) {
		r = 255;
		g = 255;
		b = 255;
		a = 255;
		sorting_order = 0;
		insertionOrder = globalRequestCounter++;
	}
};

struct ImageRequest {
	SDL_Texture* texture;
	int rotation_degrees, r, g, b, a, sorting_order;
	float x, y, scale_y, pivot_x, scale_x, pivot_y;

	int insertionOrder;

	bool extended = false;

	ImageRequest(SDL_Texture* _texture, float _x, float _y, int _rotation_degrees,
		float _scale_x, float _scale_y, float _pivot_x, float _pivot_y, int _r, int _g, int _b, int _a, int _sorting_order) :
		texture(_texture), x(_x), y(_y), rotation_degrees(_rotation_degrees), 
		scale_x(_scale_x), scale_y(_scale_y), pivot_x(_pivot_x), pivot_y(_pivot_y), r(_r), g(_g), b(_b), a(_a), sorting_order(_sorting_order) {
		insertionOrder = globalRequestCounter++;
	}

	ImageRequest(SDL_Texture* _texture, float _x, float _y) : texture(_texture), x(_x), y(_y) {
		rotation_degrees = 0;
		scale_x = 1;
		scale_y = 1;
		pivot_x = 0.5f;
		pivot_y = 0.5f;
		r = 255;
		g = 255;
		b = 255;
		a = 255;
		sorting_order = 0;
		insertionOrder = globalRequestCounter++;
	}
};

struct PixelRequest {
	int x, y, r, g, b, a;
	PixelRequest(int _x, int _y, int _r, int _g, int _b, int _a) : x(_x), y(_y), r(_r), g(_g), b(_b), a(_a) {}
};

struct UIRequestCompare {
	bool operator()(const UIRequest& lhs, const UIRequest& rhs) const {
		if (lhs.sorting_order == rhs.sorting_order)
			return lhs.insertionOrder > rhs.insertionOrder; // Ensure newer elements come later
		return lhs.sorting_order > rhs.sorting_order;
	}
};

struct ImageRequestCompare {
	bool operator()(const ImageRequest& lhs, const ImageRequest& rhs) const {
		if (lhs.sorting_order == rhs.sorting_order)
			return lhs.insertionOrder > rhs.insertionOrder; // Ensure newer elements come later
		return lhs.sorting_order > rhs.sorting_order;
	}
};

class ImageDB
{
private:
	static inline std::unordered_map<std::string, SDL_Texture*> textureCache;

public:
	static inline SDL_Renderer* renderer = nullptr;
	static inline std::priority_queue<UIRequest, std::vector<UIRequest>, UIRequestCompare> uiRequests;
	static inline std::priority_queue<ImageRequest, std::vector<ImageRequest>, ImageRequestCompare> imageRequests;
	static inline queue<PixelRequest> renderRequestPixels;

	static inline SDL_Texture* GetImage(const std::string& imageName) {
		// Check if the texture is already loaded
		auto it = textureCache.find(imageName);
		if (it != textureCache.end()) {
			// Texture is already loaded, return it
			return it->second;
		}
		else {
			EngineUtils eng;
			eng.CheckForImage(imageName);
			std::string filePath = "resources/images/" + imageName + ".png";
			// Load the texture, add it to the map, and return it
			SDL_Texture* texture = IMG_LoadTexture(renderer, filePath.c_str());
			textureCache[imageName] = texture;
			return texture;
		}
	}

	static void ClearTextureCache() {
		for (auto& pair : textureCache) {
			SDL_DestroyTexture(pair.second);
		}
		textureCache.clear();
	}

	static inline void DrawUI(const string& image_name, float x, float y) {
		SDL_Texture* image = GetImage(image_name);
		UIRequest newRequest = UIRequest(image, x, y);
		uiRequests.push(newRequest);
	}

	static inline void DrawUIEx(const string& image_name, float x, float y, float r, float g, float b, float a, float sorting_order) {
		SDL_Texture* image = GetImage(image_name);
		UIRequest newRequest = UIRequest(image, x, y, r, g, b, a, sorting_order);
		newRequest.extended = true;
		uiRequests.push(newRequest);
	}

	static inline void Draw(const string& image_name, float x, float y) {
		SDL_Texture* image = GetImage(image_name);
		ImageRequest newRequest = ImageRequest(image, x, y);
		imageRequests.push(newRequest);
	}

	static inline void DrawEx(const string& image_name, float x, float y, float rotation_degrees,
		float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order) {
		SDL_Texture* image = GetImage(image_name);
		ImageRequest newRequest = ImageRequest(image, x, y, rotation_degrees, scale_x, scale_y, pivot_x, pivot_y, r, g, b, a, sorting_order);
		newRequest.extended = true;
		imageRequests.push(newRequest);
	}

	static inline void DrawPixel(float x, float y, float r, float g, float b, float a) {
		PixelRequest newRequest = PixelRequest(static_cast<int>(x), static_cast<int>(y), static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a));
		renderRequestPixels.push(newRequest);
	}

	static inline pair<int, int> GetImageWidthandHeight(string imageName) {
		auto it = textureCache.find(imageName);
		if (it == textureCache.end()) {
			EngineUtils eng;
			eng.CheckForImage(imageName);
			std::string filePath = "resources/images/" + imageName + ".png";
			// Load the texture, add it to the map, and return it
			SDL_Texture* texture = IMG_LoadTexture(renderer, filePath.c_str());
			textureCache[imageName] = texture;
		}
		int width = 0;
		int height = 0;
		SDL_QueryTexture(textureCache[imageName], NULL, NULL, &width, &height);

		return pair<int, int>(width, height);
	}
};