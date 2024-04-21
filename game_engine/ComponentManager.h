#pragma once
#include "Utilities.h"

class ComponentManager
{
private:
	//baseComponents[name of the component] = ref to table
	static inline unordered_map<string, std::shared_ptr<luabridge::LuaRef>> baseComponents;

    static inline unordered_map<string, int> runtimeUsesOfComponentByName;

public:
	static inline lua_State* lua_state;

	ComponentManager() {
		lua_state = luaL_newstate();
		luaL_openlibs(lua_state);
	}

    void InitializeComponents(const std::string& path) {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.path().extension() == ".lua") {
                    std::string filepath = entry.path().string();
                    if (luaL_dofile(lua_state, filepath.c_str()) == LUA_OK) {
                        std::string componentName = entry.path().stem().string();

                        auto table = luabridge::getGlobal(lua_state, componentName.c_str());

                        baseComponents[componentName] = std::make_shared<luabridge::LuaRef>(table);
                        runtimeUsesOfComponentByName[componentName] = 0;
                    }
                    else {
                        cout << "problem with lua file " << entry.path().stem().string();
                        exit(0);
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error& err) {
            std::cerr << "Filesystem error: " << err.what() << std::endl;
        }
        catch (const std::exception& err) {
            std::cerr << "General error: " << err.what() << std::endl;
        }
    }

    static inline void EstablishInheritance(luabridge::LuaRef & instanceTable, luabridge::LuaRef & parentTable, bool runtime, const string& name) {
        if (runtime) {
            runtimeUsesOfComponentByName[name] += 1;
        }
        luabridge::LuaRef newMetatable = luabridge::newTable(lua_state);
        newMetatable["__index"] = parentTable;

        instanceTable.push(lua_state);
        newMetatable.push(lua_state);
        lua_setmetatable(lua_state, -2);
        lua_pop(lua_state, 1);
    }

    static inline std::shared_ptr<luabridge::LuaRef> GetBaseComponent(string name) {
        auto it = baseComponents.find(name);
        if (it != baseComponents.end()) {
            return it->second;
        }
        else {
            cout << "error: failed to locate component " << name;
            exit(0);
        }
    }

    static inline void CppLog(string message) {
        cout << message << endl;
    }
    static inline void CppLogError(string message) {
        cerr << message << endl;
    }

    static inline int GetRuntimeAdditionsOfComponent(const string& name) {
        return runtimeUsesOfComponentByName[name];
    }
};