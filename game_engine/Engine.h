#pragma once

#include "Camera.h"
#include "Scene.h"
#include "Renderer.h"
#include "ImageDB.h"
#include "TextDB.h"
#include "AudioDB.h"
#include "Input.h"
#include "ComponentManager.h"


using namespace std;

class RayCastClosestCallback : public b2RayCastCallback {
public:
	HitResult hitResult;
	bool hit;
	float closestFraction;

	RayCastClosestCallback() : hit(false), closestFraction(1.0f) {}

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
		// Check if this hit is closer than previous hits
		if (fraction < closestFraction) {
			hit = true;
			closestFraction = fraction; // Update the closest fraction

			Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
			if (actor == nullptr) return -1; // Continue if actor is nullptr

			hitResult.actor = actor;
			hitResult.point = point;
			hitResult.normal = normal;
			hitResult.is_trigger = fixture->IsSensor();
		}

		return fraction; // Continue the raycast to look for closer hits
	}
};

class RayCastAllCallback : public b2RayCastCallback {
public:
	struct Hit {
		HitResult result;
		float fraction;

		Hit(HitResult hitResult, float frac) : result(hitResult), fraction(frac) {}
	};

	std::vector<Hit> hits;

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
		Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);

		// You might want to filter out certain fixtures here
		if (actor != nullptr) { // Proceed only if actor is not null
			HitResult hitResult;
			hitResult.actor = actor;
			hitResult.point = point;
			hitResult.normal = normal;
			hitResult.is_trigger = fixture->IsSensor();
			hits.emplace_back(hitResult, fraction);
		}
		else {
			return -1;
		}

		return 1; // Always return 1 to collect all hits
	}
};


class Engine
{
public:
	Engine();
	~Engine();
	void Initialize();
	void GameLoop();
	void Update();
	void Render();

	static inline void cppQuit() {
		exit(0);
	}

	static inline void cppSleep(int milliseconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

	static inline int cppGetFrame() {
		return Helper::GetFrameNumber();
	}

	static inline void cppOpenUrl(const std::string& url) {
		std::string command;
#ifdef _WIN32
		// Windows has a 'start' command that can be used to open URLs in the default browser.
		command = "start " + url;
#elif __APPLE__
		// macOS uses the 'open' command for opening URLs.
		command = "open " + url;
#elif __linux__
		// Linux desktop environments typically support 'xdg-open' for opening URLs.
		command = "xdg-open " + url;
#else
		// Unsupported OS
#error "Open URL command is not defined for this OS."
#endif

// Execute the command
		std::system(command.c_str());
	}

private:
	bool gameRunning = true;

	static inline bool proceedToNewScene = false;
	static inline string newSceneName = "";

	EngineUtils utils;

	rapidjson::Document gameConfig;

	Scene& scene;
	Camera& camera;
	Renderer renderer;
	ImageDB imageDB;
	TextDB text;
	AudioDB audio;
	ComponentManager comp;
	ContactListner listner;

	void RenderTexts();
	void RenderImages();
	void RenderUI();
	void RenderPixels();

	static inline void LoadScene(string scene_name) {
		proceedToNewScene = true;
		newSceneName = scene_name;
	}

	int heightWindow = 0;

	void InjectDebugClass();

	void InjectActorClass();

	void InjectApplicationNameSpace();

	void InjectInputClass();

	void InjectTextNamespace();

	void InjectAudioNamespace();

	void InjectImageNamespace();

	void InjectCameraNamespace();

	void InjectSceneNamespace();

	void InjectVector2Class();

	void InjectRigidbodyComponent();

	static inline luabridge::LuaRef Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
		RayCastClosestCallback callback;
		dir.Normalize();
		Rigidbody::world->RayCast(&callback, pos, pos + dist * dir);

		if (callback.hit) {
			luabridge::LuaRef hitResultTable = luabridge::LuaRef::newTable(ComponentManager::lua_state);
			if (callback.hitResult.actor == nullptr) {
				return luabridge::LuaRef(ComponentManager::lua_state);
			}
			hitResultTable["actor"] = callback.hitResult.actor;
			hitResultTable["point"] = callback.hitResult.point;
			hitResultTable["normal"] = callback.hitResult.normal;
			hitResultTable["is_trigger"] = callback.hitResult.is_trigger;

			return hitResultTable;
		}
		else {
			return luabridge::LuaRef(ComponentManager::lua_state);
		}
	}

	static inline luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {
		RayCastAllCallback callback;
		dir.Normalize();
		Rigidbody::world->RayCast(&callback, pos, pos + dist * dir);

		// Sort hits by fraction (distance)
		std::sort(callback.hits.begin(), callback.hits.end(),
			[](const RayCastAllCallback::Hit& a, const RayCastAllCallback::Hit& b) {
				return a.fraction < b.fraction;
			});

		// Convert sorted hits to Lua table
		luabridge::LuaRef hitsTable = luabridge::LuaRef::newTable(ComponentManager::lua_state);
		for (size_t i = 0; i < callback.hits.size(); ++i) {
			luabridge::LuaRef hitTable = luabridge::LuaRef::newTable(ComponentManager::lua_state);
			hitTable["actor"] = callback.hits[i].result.actor;
			hitTable["point"] = callback.hits[i].result.point;
			hitTable["normal"] = callback.hits[i].result.normal;
			hitTable["is_trigger"] = callback.hits[i].result.is_trigger;
			hitsTable[i + 1] = hitTable; // Lua is 1-indexed
		}

		return hitsTable;
	}
};