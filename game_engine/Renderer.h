#pragma once
#include "Utilities.h"
#include "Actor.h"
#include "Scene.h"

class Renderer
{
private:
	SDL_Window* window = nullptr;
	int windowWidth = 640;
	int windowHeight = 360;
	static inline bool xScaleActorFlippingOnMovement = false;
	

public:
	static inline SDL_Renderer* renderer = nullptr;
	static inline float zoomFactor = 1;
	

	void Initialize(const char* gameTitle="", int width=640, int height=360) {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			std::cout << "SDL could not initialize! SDL_ERROR: " << SDL_GetError() << std::endl;
			return;
		}
		windowWidth = width;
		windowHeight = height;
		window = Helper::SDL_CreateWindow498(gameTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
		if (window == nullptr) {
			std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
			SDL_Quit();
			return;
		}
	}

	void CreateRender(Uint8 r=255, Uint8 g=255, Uint8 b=255, Uint8 a=255) {
		Uint32 flags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED;
		renderer = Helper::SDL_CreateRenderer498(window, -1, flags);
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
		SDL_RenderClear(renderer);
	}

	void ClearRender() {
		SDL_RenderClear(renderer);
	}

	void GetTextureDimensions(SDL_Texture* texture, int& width, int& height) {
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
	}

	static inline void RenderText(TTF_Font* font, SDL_Color textColor, const string& text, int x, int y) {
		if (font == nullptr) {
			return;
		}
		if (text == "") {
			return;
		}
		// Create a surface from the text
		SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
		if (textSurface == nullptr) {
			cout << "Unable to create text surface! SDL_ttf Error: " << TTF_GetError();
			return;
		}

		// Create a texture from the surface
		SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (textTexture == nullptr) {
			cout << "Unable to create texture from surface! SDL Error: " << SDL_GetError();
			SDL_FreeSurface(textSurface); // Don't forget to free the surface
			return;
		}

		int textureWidth = 0;
		int textureHeight = 0;
		SDL_QueryTexture(textTexture, 0, 0, &textureWidth, &textureHeight);

		SDL_Rect renderQuad = { x, y, textureWidth, textureHeight };

		// Render to screen
		SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

		SDL_FreeSurface(textSurface);
	}

	void SetZoomFactor(float zoomIn) {
		zoomFactor = zoomIn;
	}

	void SetXScaleFlipping() {
		xScaleActorFlippingOnMovement = true;
	}

	void RenderUIElement(SDL_Texture* texture, int x, int y) {
		// Assuming you do not need to rotate or scale the image, prepare the SDL_Rect for the position
		SDL_Rect renderQuad = { x, y, 0, 0 };
		// Query the texture to get its width and height to fill the SDL_Rect
		SDL_QueryTexture(texture, NULL, NULL, &renderQuad.w, &renderQuad.h);

		// Render texture to screen
		SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
	}

	void RenderUIElementEx(SDL_Texture* texture, int x, int y, int r, int g, int b, int a) {
		// Set texture color modulation (color tint)
		SDL_SetTextureColorMod(texture, r, g, b);

		// Set texture alpha blending
		SDL_SetTextureAlphaMod(texture, a);

		// Prepare the SDL_Rect for the position and size
		SDL_Rect renderQuad = { x, y, 0, 0 };
		// Query the texture to get its width and height to fill the SDL_Rect
		SDL_QueryTexture(texture, NULL, NULL, &renderQuad.w, &renderQuad.h);

		// Render texture to screen
		SDL_RenderCopy(renderer, texture, NULL, &renderQuad);

		// Reset color modulation and alpha blending to default values
		SDL_SetTextureColorMod(texture, 255, 255, 255);
		SDL_SetTextureAlphaMod(texture, 255);
	}

	void RenderImage(SDL_Texture* texture, float x, float y) {
		int textureWidth, textureHeight;
		// Assuming GetTextureDimensions is a function that retrieves the texture's width and height
		GetTextureDimensions(texture, textureWidth, textureHeight);

		// Pivot point in fractions, with example values (can be parameters)
		float pivotXFraction = 0.5f; // Assuming pivot is in the middle
		float pivotYFraction = 0.5f;

		// Convert scene units to pixels
		float xPosInPixels = x * 100; // Convert x position from scene units to pixels
		float yPosInPixels = y * 100; // Convert y position from scene units to pixels
		float camPosXInPixels = camPosX * 100; // Convert camera X position from scene units to pixels
		float camPosYInPixels = camPosY * 100; // Convert camera Y position from scene units to pixels

		// Adjusting texture dimensions and position according to the scale and zoom factor
		int destW = static_cast<int>((textureWidth));
		int destH = static_cast<int>((textureHeight));

		// Calculating the final pivot in pixels after applying scale
		int finalPivotX = static_cast<int>((pivotXFraction * destW));
		int finalPivotY = static_cast<int>((pivotYFraction * destH));

		// Adjusting the actor's position to be relative to the camera and applying zoom factor
		float destX = (xPosInPixels - camPosXInPixels) - finalPivotX + ((windowWidth / 2) / zoomFactor);
		float destY = (yPosInPixels - camPosYInPixels) - finalPivotY + ((windowHeight / 2) / zoomFactor);

		SDL_Rect destRect = { static_cast<int>(destX), static_cast<int>(destY), destW, destH };

		SDL_RendererFlip flip = SDL_FLIP_NONE; // Assuming no flipping required; adjust as needed

		// Rotation point - Now calculated with scaled pivot values
		SDL_Point rotationPoint = { finalPivotX, finalPivotY };
		// Render the actor with adjustments
		Helper::SDL_RenderCopyEx498(-1, "bob", renderer, texture, nullptr, &destRect, 0, &rotationPoint, flip);
	}

	void RenderImageEx(SDL_Texture* texture, float x, float y, int rotation_degrees, float scale_x, float scale_y,
		float pivot_x, float pivot_y, int r, int g, int b, int a) {

		const int pixels_per_meter = 100;

		glm::vec2 final_rendering_position = glm::vec2(x, y) - glm::vec2(camPosX, camPosY);

		SDL_Rect tex_rect;
		SDL_QueryTexture(texture, NULL, NULL, &tex_rect.w, &tex_rect.h);

		int flip_mode = SDL_FLIP_NONE;
		if (scale_x < 0) {
			flip_mode |= SDL_FLIP_HORIZONTAL;
		}
		if (scale_y < 0) {
			flip_mode |= SDL_FLIP_VERTICAL;
		}

		float x_scale = std::abs(scale_x);
		float y_scale = std::abs(scale_y);

		tex_rect.w *= x_scale;
		tex_rect.h *= y_scale;

		SDL_Point pivot_point = { static_cast<int>(pivot_x * tex_rect.w), static_cast<int>(pivot_y * tex_rect.h) };

		glm::ivec2 cam_dimensions = glm::ivec2(windowWidth, windowHeight);

		tex_rect.x = static_cast<int>(final_rendering_position.x * pixels_per_meter + cam_dimensions.x * 0.5f * (1.0f / zoomFactor) - pivot_point.x);
		tex_rect.y = static_cast<int>(final_rendering_position.y * pixels_per_meter + cam_dimensions.y * 0.5f * (1.0f / zoomFactor) - pivot_point.y);

		SDL_SetTextureColorMod(texture, r, g, b);
		SDL_SetTextureAlphaMod(texture, a);

		Helper::SDL_RenderCopyEx498(-1, "bob", renderer, texture, NULL, &tex_rect, rotation_degrees, &pivot_point, static_cast<SDL_RendererFlip>(flip_mode));

		SDL_SetTextureColorMod(texture, 255, 255, 255);
		SDL_SetTextureAlphaMod(texture, 255);
	}

	static inline float camPosX = 0;
	static inline float camPosY = 0;

	static inline void SetPosition(float x, float y) {
		camPosX = x;
		camPosY = y;
	}

	static inline float GetPositionX() {
		return camPosX;
	}

	static inline float GetPositionY() {
		return camPosY;
	}

	static inline void SetZoom(float zoom_factor) {
		zoomFactor = zoom_factor;
	}

	static inline float GetZoom() {
		return zoomFactor;
	}
};

