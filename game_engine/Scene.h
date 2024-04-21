#pragma once

#include "Utilities.h"
#include "ImageDB.h"
#include "Actor.h"
#include "AudioDB.h"
#include "ComponentManager.h"

using namespace std;
using ActorRef = Actor*; // Pair of actor pointer and index
using ActorMap = std::unordered_map<uint64_t, std::vector<ActorRef>>;

enum ACTOR_STATE {ACTIVE, DESTROYED, IMMORTAL};

struct ActorStore {
    shared_ptr<Actor> pointer;
    int actorID = -1;
    ACTOR_STATE state = ACTOR_STATE::ACTIVE;
};

struct ActorRefHash {
    size_t operator()(const ActorRef& ref) const {
        // Use std::hash to hash the actorID
        return std::hash<int>()(ref->actorID);
    }
};

// Custom equality functor
struct ActorRefEqual {
    bool operator()(const ActorRef& lhs, const ActorRef& rhs) const {
        return lhs->actorID == rhs->actorID;
    }
};

class Scene {
private:
    // Private constructor
    Scene();
    ~Scene();
    // Disable copy constructor and assignment
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    static inline int nextActorID = 0;
    static inline rapidjson::Document sceneDoc;

    static inline string sceneName;

    static void LoadScene();
    static void LoadLayer(int newActors, int layerNum);
    static void DoTemplate(string templateName, ActorRef actor);

    void RemoveActorsFromAdd();

public:
    static inline vector<ActorStore> actors;
    static inline vector<ActorStore> actorsToAdd;
    static inline vector<ActorStore> actorsDontDestroyOnLoad;

    static inline vector<pair<int, string>> indexandLocationofActorsToDestroy;

    static Scene& GetInstance();

    void RunStartFunctions();

    static void GetNewScene(string sceneName);

    static inline luabridge::LuaRef InstantiateActor(const string& actorTemplateName)
    {
        Actor newActor;
        DoTemplate(actorTemplateName, &newActor);
        newActor.actorID = nextActorID++;
        shared_ptr<Actor> actor = make_shared<Actor>(newActor);
        ActorStore actorNew;
        actorNew.pointer = actor;
        actorNew.actorID = actor->actorID;
        actorNew.state = ACTOR_STATE::ACTIVE;
        actorsToAdd.push_back(actorNew);
        return luabridge::LuaRef(ComponentManager::lua_state, *actor);
    }

    static inline luabridge::LuaRef cppFind(string name) {
        for (const auto& actor : actors) {
            if (actor.pointer->actor_name == name && actor.state != ACTOR_STATE::DESTROYED) {
                return luabridge::LuaRef(ComponentManager::lua_state, *actor.pointer);
            }
        }
        for (const auto& actor : actorsToAdd) {
            if (actor.pointer->actor_name == name && actor.state != ACTOR_STATE::DESTROYED) {
                return luabridge::LuaRef(ComponentManager::lua_state, *actor.pointer);
            }
        }
        return luabridge::LuaRef(ComponentManager::lua_state); // Return nil if no actor found
    }

    static inline luabridge::LuaRef cppFindAll(const std::string& name) {
        luabridge::LuaRef table = luabridge::newTable(ComponentManager::lua_state);
        int index = 1;

        for (const auto& actor : actors) {
            if (actor.pointer->actor_name == name && actor.state != ACTOR_STATE::DESTROYED) {
                table[index++] = *actor.pointer;
            }
        }
        for (const auto& actor : actorsToAdd) {
            if (actor.pointer->actor_name == name && actor.state != ACTOR_STATE::DESTROYED) {
                table[index++] = *actor.pointer;
            }
        }

        return table;
    }

    static inline void cppActorDestroy(Actor* actor) {
        int actorID = actor->actorID;
        int index = 0;
        string location = "";
        bool found = false;
        for (int i = 0; i < actorsToAdd.size(); ++i) {
            if (actorsToAdd[i].actorID == actorID) {
                index = i;
                location = "actorsToAdd";
                found = true;
                actorsToAdd[i].state = ACTOR_STATE::DESTROYED;
                break;
            }
        }
        if (!found) {
            for (int i = 0; i < actors.size(); ++i) {
                if (actors[i].actorID == actorID) {
                    index = i;
                    location = "actors";
                    found = true;
                    actors[i].state = ACTOR_STATE::DESTROYED;
                    break;
                }
            }
        }
        indexandLocationofActorsToDestroy.push_back(pair<int, string>(index, location));
    }

    static inline void ProcessDestroyedActors() {
        for (int i = 0; i < indexandLocationofActorsToDestroy.size(); ++i) {

            //call the actors on destroy functions
            actors[indexandLocationofActorsToDestroy[i].first].pointer->cppOnDestroy();

            if (indexandLocationofActorsToDestroy[i].second == "actors") {
                actors.erase(actors.begin() + indexandLocationofActorsToDestroy[i].first);
            }
            else if (indexandLocationofActorsToDestroy[i].second == "actorsToAdd") {
                actorsToAdd.erase(actorsToAdd.begin() + indexandLocationofActorsToDestroy[i].first);
            }
        }
        indexandLocationofActorsToDestroy.clear();
    }

    static inline string GetCurrentScene() {
        return sceneName;
    }

    static inline void DontDestroy(Actor* actor) {
        for (int i = 0; i < actors.size(); ++i) {
            if (actor->actorID == actors[i].actorID) {
                actorsDontDestroyOnLoad.push_back(actors[i]);
                return;
            }
        }
        for (int i = 0; i < actorsToAdd.size(); ++i) {
            if (actor->actorID == actorsToAdd[i].actorID) {
                actorsDontDestroyOnLoad.push_back(actorsToAdd[i]);
                return;
            }
        }
    }
};