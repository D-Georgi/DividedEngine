#pragma once

#include "Utilities.h"
#include "ComponentManager.h"
#include "Rigidbody.h"

using namespace std;

struct collision {
	Actor* other = nullptr;
	b2Vec2 point = b2Vec2(0, 0);
	b2Vec2 relative_velocity = b2Vec2(0, 0);
	b2Vec2 normal = b2Vec2(0, 0);
};

struct HitResult {
	Actor* actor = nullptr;
	b2Vec2 point = b2Vec2(0, 0);
	b2Vec2 normal = b2Vec2(0, 0);
	bool is_trigger = true;
};

class Actor
{
public:
	string actor_name = "";
	int actorID = -1;

	Actor* thisActor = nullptr;

	bool firstUpdate = true;
	bool firstLateUpdate = true;
	bool firstOnCollisionEnter = true;
	bool firstOnCollisionExit = true;
	bool firstOnTriggerEnter = true;
	bool firstOnTriggerExit = true;
	bool firstOnDestroyExit = true;

	//string is the local key on the actor, NOT component name
	
	unordered_map<string, std::shared_ptr<luabridge::LuaRef>> componentsByKey;
	unordered_map<string, vector<std::shared_ptr<luabridge::LuaRef>>> componentsByType;
	unordered_map<string, string> keyToType;

	//Component name, key
	map<pair<string, string>, std::shared_ptr<luabridge::LuaRef>> componentsToAdd;
	set<pair<string, string>> componentsToRemove;

	map<string, std::shared_ptr<luabridge::LuaRef>> orderedStartComponents;
	map<string, std::shared_ptr<luabridge::LuaRef>> orderedUpdateComponents;
	map<string, std::shared_ptr<luabridge::LuaRef>> orderedLateUpdateComponents;
	
	map<string, std::shared_ptr<luabridge::LuaRef>> orderedOnCollisionEnterComponents;
	map<string, std::shared_ptr<luabridge::LuaRef>> orderedOnCollisionExitComponents;

	map<string, std::shared_ptr<luabridge::LuaRef>> orderedOnTriggerEnterComponents;
	map<string, std::shared_ptr<luabridge::LuaRef>> orderedOnTriggerExitComponents;

	map<string, std::shared_ptr<luabridge::LuaRef>> onDestroycomponents;

	void ProcessAddedComponents() {
		for (auto& component : componentsToAdd) {
			orderedStartComponents[component.first.second] = component.second;
			componentsByKey[component.first.second] = component.second;
			componentsByType[component.first.first].push_back(component.second);
		}
		componentsToAdd.clear();
		cppOnStart();
	}

	void ProcessRemovedComponents() {
		for (auto& component : componentsToRemove) {
			auto& luaTable = *componentsByKey[component.second];
			luabridge::LuaRef onDestroyFunction = luaTable["OnDestroy"];

			if (onDestroyFunction.isFunction()) {
				onDestroyFunction(luaTable);
			}

			componentsByKey.erase(component.second);
			componentsByType.erase(component.first);
			orderedStartComponents.erase(component.second);
			orderedUpdateComponents.erase(component.second);
			orderedLateUpdateComponents.erase(component.second);
		}
		componentsToRemove.clear();
	}

	void AddComponentToActor(const std::string& compName, const std::string& compKey) {
		auto instanceTable = std::make_shared<luabridge::LuaRef>(luabridge::newTable(ComponentManager::lua_state));
		if (compName == "Rigidbody") {
			instanceTable = Rigidbody::GetRigidComponent(ComponentManager::lua_state);
		}
		else {
			auto parentTable = ComponentManager::GetBaseComponent(compName);
			ComponentManager::EstablishInheritance(*instanceTable, *parentTable, false, compName);
		}

		(*instanceTable)["key"] = compKey;
		(*instanceTable)["enabled"] = true;
		if (compName == "Rigidbody") {
			(*instanceTable)["type"] = compName;
		}

		orderedStartComponents[compKey] = instanceTable;
		componentsByKey[compKey] = instanceTable;
		componentsByType[compName].push_back(instanceTable);
	}

	luabridge::LuaRef AddComponentToActorRuntime(const std::string& compName) {
		auto parentTable = ComponentManager::GetBaseComponent(compName);
		auto instanceTable = std::make_shared<luabridge::LuaRef>(luabridge::newTable(ComponentManager::lua_state));
		ComponentManager::EstablishInheritance(*instanceTable, *parentTable, true, compName);
		
		string key = "r" + to_string(ComponentManager::GetRuntimeAdditionsOfComponent(compName) - 1);
		(*instanceTable)["key"] = key;
		(*instanceTable)["enabled"] = true;
		(*instanceTable)["actor"] = this;

		componentsToAdd[pair<string, string>(compName, key)] = instanceTable;

		return *instanceTable;
	}

	void RemoveComponent(luabridge::LuaRef component) {
		string key = component["key"];
		string type = keyToType[key];
		componentsToRemove.insert(pair<string, string>(type, key));
		component["enabled"] = false;
	}

	luabridge::LuaRef cppGetComponentByKey(string key) {
		auto it = componentsByKey.find(key);
		if (it != componentsByKey.end()) {
			luabridge::LuaRef component = *it->second;
			if (component["enabled"]) {
				return component;
			}
		}
		return luabridge::LuaRef(ComponentManager::lua_state);
	}

	luabridge::LuaRef cppGetComponent(const std::string& type_name) {
		// Assuming componentsByType is a map of string to vector of LuaRef
		auto it = componentsByType.find(type_name);
		if (it != componentsByType.end()) {
			auto& list = it->second;

			// Ensure list has elements and each is a Lua table
			if (!list.empty()) {
				std::sort(list.begin(), list.end(), compareLuaRefByKey);
				luabridge::LuaRef component = *list.front();
				// Return the first element of the sorted list
				if (component["enabled"]) {
					return component;
				}
			}
		}
		// Return an empty LuaRef if no component found
		return luabridge::LuaRef(ComponentManager::lua_state);
	}

	luabridge::LuaRef cppGetComponents(const std::string& type_name) {
		auto lua_state = ComponentManager::lua_state; // Assuming this is your Lua state
		luabridge::LuaRef table = luabridge::newTable(lua_state);

		auto it = componentsByType.find(type_name);
		if (it != componentsByType.end()) {
			auto& list = it->second; // Make a copy to sort

			// Sort the list of components by their "key"
			std::sort(list.begin(), list.end(), compareLuaRefByKey);

			// Populate the Lua table
			int index = 1; // Lua tables are 1-indexed
			for (auto& componentPtr : list) {
				auto& component = *componentPtr;
				if (component["enabled"]) {
					table[index++] = *componentPtr;
				}
			}
		}

		// Return the populated table (or an empty table if no components of the desired type exist)
		return table;
	}

	string GetName() const {
		return actor_name;
	}

	int GetID() const {
		return actorID;
	}

	void cppOnStart() {
		for (auto& componentPair : orderedStartComponents) {
			// Extract the LuaRef from the shared_ptr.
			auto& component = *componentPair.second;

			component["actor"] = this;
			component["actorPtr"] = this;
			thisActor = component["actor"];

			// Assuming the "start" function is defined at the Lua table's root.
			luabridge::LuaRef startFunction = component["OnStart"];

			if (startFunction.isFunction() && component["enabled"]) {
				try {
					// Call the "start" function with no arguments. Adjust as necessary for your function signatures.
					startFunction(component);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}

			luabridge::LuaRef updateFunction = component["OnUpdate"];
			if (updateFunction.isFunction()) {
				orderedUpdateComponents[componentPair.first] = componentPair.second;
			}
			luabridge::LuaRef lateUpdateFunction = component["OnLateUpdate"];
			if (lateUpdateFunction.isFunction()) {
				orderedLateUpdateComponents[componentPair.first] = componentPair.second;
			}
			luabridge::LuaRef onCollisionEnterFunction = component["OnCollisionEnter"];
			if (onCollisionEnterFunction.isFunction()) {
				orderedOnCollisionEnterComponents[componentPair.first] = componentPair.second;
			}
			luabridge::LuaRef onCollisionExitFunction = component["OnCollisionExit"];
			if (onCollisionExitFunction.isFunction()) {
				orderedOnCollisionExitComponents[componentPair.first] = componentPair.second;
			}
			luabridge::LuaRef onTriggerEnterFunction = component["OnTriggerEnter"];
			if (onTriggerEnterFunction.isFunction()) {
				orderedOnTriggerEnterComponents[componentPair.first] = componentPair.second;
			}
			luabridge::LuaRef onTriggerExitFunction = component["OnTriggerExit"];
			if (onTriggerExitFunction.isFunction()) {
				orderedOnTriggerExitComponents[componentPair.first] = componentPair.second;
			}
			luabridge::LuaRef onDestroyFunction = component["OnDestroy"];
			if (onDestroyFunction.isFunction()) {
				onDestroycomponents[componentPair.first] = componentPair.second;
			}
		}
		orderedStartComponents.clear();
	}

	void cppOnUpdate() {
		for (auto& componentPair : orderedUpdateComponents) {
			// Extract the LuaRef from the shared_ptr.
			auto& component = *componentPair.second;

			if (firstUpdate) {
				component["actor"] = thisActor;
			}

			// Assuming the "start" function is defined at the Lua table's root.
			luabridge::LuaRef updateFunction = component["OnUpdate"];

			if (updateFunction.isFunction() && component["enabled"]) {
				try {
					// Call the "start" function with no arguments. Adjust as necessary for your function signatures.
					updateFunction(component);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
		firstUpdate = false;
	}

	void cppOnLateUpdate() {
		for (auto& componentPair : orderedLateUpdateComponents) {
			// Extract the LuaRef from the shared_ptr.
			auto& component = *componentPair.second;

			if (firstLateUpdate) {
				component["actor"] = thisActor;
			}

			// Assuming the "start" function is defined at the Lua table's root.
			luabridge::LuaRef lateUpdateFunction = component["OnLateUpdate"];

			if (lateUpdateFunction.isFunction() && component["enabled"]) {
				try {
					// Call the "start" function with no arguments. Adjust as necessary for your function signatures.
					lateUpdateFunction(component);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
		firstLateUpdate = false;
	}

	void cppOnDestroy() {
		for (auto& componentPair : onDestroycomponents) {
			auto& component = *componentPair.second;

			if (firstOnDestroyExit) {
				component["actor"] = thisActor;
			}

			luabridge::LuaRef onDestroyFunction = component["OnDestroy"];
			if (onDestroyFunction.isFunction()) {
				try {
					onDestroyFunction(component);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
		firstOnDestroyExit = false;
	}

	void cppOnCollisionEnter(collision& col) {
		for (auto& componentPair : orderedOnCollisionEnterComponents) {
			auto& component = *componentPair.second;

			if (firstOnCollisionEnter) {
				component["actor"] = thisActor;
			}

			luabridge::LuaRef onCollisionEnterFunction = component["OnCollisionEnter"];
			if (onCollisionEnterFunction.isFunction() && component["enabled"]) {
				try {
					onCollisionEnterFunction(component, col);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
		firstOnCollisionEnter = false;
	}

	void cppOnCollisionExit(collision col) {
		for (auto& componentPair : orderedOnCollisionExitComponents) {
			auto& component = *componentPair.second;

			if (firstOnCollisionExit) {
				component["actor"] = thisActor;
			}

			luabridge::LuaRef onCollisionExitFunction = component["OnCollisionExit"];
			if (onCollisionExitFunction.isFunction() && component["enabled"]) {
				try {
					onCollisionExitFunction(component, col);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
		firstOnCollisionExit = false;
	}

	void cppOnTriggerEnter(collision& col) {
		for (auto& componentPair : orderedOnTriggerEnterComponents) {
			auto& component = *componentPair.second;

			if (firstOnTriggerEnter) {
				component["actor"] = thisActor;
			}

			luabridge::LuaRef onTriggerEnterFunction = component["OnTriggerEnter"];
			if (onTriggerEnterFunction.isFunction() && component["enabled"]) {
				try {
					onTriggerEnterFunction(component, col);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
		firstOnTriggerEnter = false;
	}

	void cppOnTriggerExit(collision col) {
		for (auto& componentPair : orderedOnTriggerExitComponents) {
			auto& component = *componentPair.second;

			if (firstOnTriggerExit) {
				component["actor"] = thisActor;
			}

			luabridge::LuaRef onTriggerExitFunction = component["OnTriggerExit"];
			if (onTriggerExitFunction.isFunction() && component["enabled"]) {
				try {
					onTriggerExitFunction(component, col);
				}
				catch (luabridge::LuaException const& e) {
					ReportError(actor_name, e);
				}
			}
		}
		firstOnTriggerExit = false;
	}
};

class ContactListner : public b2ContactListener {
	void BeginContact(b2Contact* contact) override {
		auto* fixtureA = contact->GetFixtureA();
		auto* fixtureB = contact->GetFixtureB();

		Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
		Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);

		if (actorA && actorB) {
			b2WorldManifold world_manifold;
			contact->GetWorldManifold(&world_manifold);

			collision aCol;
			aCol.other = actorB;
			aCol.point = world_manifold.points[0];
			aCol.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
			aCol.normal = world_manifold.normal;

			collision bCol;
			bCol.other = actorA;
			bCol.point = world_manifold.points[0];
			bCol.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
			bCol.normal = world_manifold.normal;

			if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
				actorA->cppOnCollisionEnter(aCol);
				actorB->cppOnCollisionEnter(bCol);
			}
			else if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
				aCol.point = b2Vec2(-999.0f, -999.0f);
				aCol.normal = b2Vec2(-999.0f, -999.0f);
				actorA->cppOnTriggerEnter(aCol);

				bCol.point = b2Vec2(-999.0f, -999.0f);
				bCol.normal = b2Vec2(-999.0f, -999.0f);
				actorB->cppOnTriggerEnter(bCol);
			}
		}
	}

	void EndContact(b2Contact* contact) override {
		auto* fixtureA = contact->GetFixtureA();//dummy
		auto* fixtureB = contact->GetFixtureB();

		Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
		Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);

		if (actorA && actorB) {
			collision aCol;
			aCol.other = actorB;
			aCol.point = b2Vec2(-999.0f, -999.0f); // Sentinel values
			aCol.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
			aCol.normal = b2Vec2(-999.0f, -999.0f); // Sentinel values

			collision bCol;
			bCol.other = actorA;
			bCol.point = b2Vec2(-999.0f, -999.0f); // Sentinel values
			bCol.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
			bCol.normal = b2Vec2(-999.0f, -999.0f); // Sentinel values
			
			if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
				actorA->cppOnCollisionExit(aCol);
				actorB->cppOnCollisionExit(bCol);
			}
			else if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
				actorA->cppOnTriggerExit(aCol);
				actorB->cppOnTriggerExit(bCol);
			}
		}
	}
};