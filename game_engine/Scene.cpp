#include "Scene.h"
Scene::Scene() 
{
}

Scene::~Scene()
{
}

Scene& Scene::GetInstance()
{
    static Scene instance;
    return instance;
}

void Scene::RemoveActorsFromAdd()
{
    if (actorsToAdd.empty()) {
        return; // Nothing to do
    }

    // Move actors from actorsToAdd to the end of actors
    std::move(actorsToAdd.begin(), actorsToAdd.end(), std::back_inserter(actors));

    // Clear the actorsToAdd list after moving
    actorsToAdd.clear();
}

void Scene::RunStartFunctions()
{
    //call start functions of actors' components
    for (auto& actor : actorsToAdd) {
        actor.pointer->cppOnStart();
    }
    RemoveActorsFromAdd();
}

void Scene::LoadScene()
{
    actors.clear();
    
    for (int i = 0; i < actorsDontDestroyOnLoad.size(); ++i) {
        actors.push_back(actorsDontDestroyOnLoad[i]);
    }
    int numLayers = sceneDoc["layers"].Size();
    int newActors = 0;

    for (int i = 0; i < numLayers; ++i) {
        newActors += sceneDoc["layers"][i]["objects"].Size();
    }
    actors.reserve(newActors + actorsDontDestroyOnLoad.size());
    actorsToAdd.reserve(newActors + actorsDontDestroyOnLoad.size());

    for (int i = 0; i < numLayers; ++i) {
        int actorsInLayer = sceneDoc["layers"][i]["objects"].Size();
        LoadLayer(actorsInLayer, i);
    }
}

void Scene::LoadLayer(int newActors, int layerNum)
{
    for (int i = 0; i < newActors; ++i) {
        Actor newActor;
        newActor.actorID = nextActorID++;

        bool hasTemplate = false;
        if (sceneDoc["layers"][layerNum]["objects"][i].HasMember("name")) {
            newActor.actor_name = sceneDoc["layers"][layerNum]["objects"][i]["name"].GetString();
        }
        if (sceneDoc["layers"][layerNum]["objects"][i].HasMember("properties")) {
            for (auto& property : sceneDoc["layers"][layerNum]["objects"][i]["properties"].GetArray()) {
                std::string key = property["name"].GetString(); // The key, e.g., "1", "2".
                std::string type;
                rapidjson::Document parsedValue;
                std::string valueStr = property["value"].GetString();
                std::string jsonStr = std::string("{") + valueStr + std::string("}");
                // Parse the string as JSON
                    
                if (parsedValue.Parse(jsonStr.c_str()).HasParseError()) {
                    std::cerr << "Error parsing value: " << valueStr << std::endl;
                    continue;
                }

                if (!hasTemplate || newActor.componentsByKey.find(key) == newActor.componentsByKey.end()) {
                    if (parsedValue.HasMember("type")) {
                        type = parsedValue["type"].GetString(); // Getting the type from the component.
                    }
                    newActor.AddComponentToActor(type, key);
                }
                luabridge::LuaRef table = *(newActor.componentsByKey[key]);
                for (auto& m : parsedValue.GetObject()) {
                    string memberName = m.name.GetString();
                    if (memberName != "type") {
                        if (m.value.IsString()) {
                            table[memberName] = m.value.GetString();
                        }
                        else if (m.value.IsBool()) {
                            table[memberName] = m.value.GetBool();
                        }
                        else if (m.value.IsInt()) {
                            table[memberName] = m.value.GetInt();
                        }
                        else if (m.value.IsFloat()) {
                            table[memberName] = m.value.GetFloat();
                        }
                    }
                }
                if (type == "Rigidbody") {
                    const float degreeToRadians = b2_pi / 180.0f;

                    // Calculating x, y considering rotation
                    float rotation = sceneDoc["layers"][layerNum]["objects"][i]["rotation"].GetFloat();
                    float rotationInRadians = 0.0f;
                    if (rotation != 0) {
                        rotationInRadians = (rotation - 360.0f) * degreeToRadians;
                    }
                    
                    float bottomLeftX = sceneDoc["layers"][layerNum]["objects"][i]["x"].GetFloat();
                    float bottomLeftY = sceneDoc["layers"][layerNum]["objects"][i]["y"].GetFloat();

                    float rotatedX = bottomLeftX + (sceneDoc["layers"][layerNum]["objects"][i]["width"].GetFloat() / 2) * cos(rotationInRadians) - (sceneDoc["layers"][layerNum]["objects"][i]["height"].GetFloat() / 2) * sin(rotationInRadians);
                    float rotatedY = bottomLeftY + (sceneDoc["layers"][layerNum]["objects"][i]["width"].GetFloat() / 2) * sin(rotationInRadians) - (sceneDoc["layers"][layerNum]["objects"][i]["height"].GetFloat() / 2) * cos(rotationInRadians);

                    // Adjusting x, y for scale and rotation
                    float x = rotatedX / 100.0f;
                    float y = rotatedY / 100.0f;

                    // Assign the calculated positions to the table
                    table["x"] = x;
                    table["y"] = y;
                    table["width"] = sceneDoc["layers"][layerNum]["objects"][i]["width"].GetFloat() / 100.0f;
                    table["height"] = sceneDoc["layers"][layerNum]["objects"][i]["height"].GetFloat() / 100.0f;
                    table["trigger_width"] = sceneDoc["layers"][layerNum]["objects"][i]["width"].GetFloat() / 100.0f;
                    table["trigger_height"] = sceneDoc["layers"][layerNum]["objects"][i]["height"].GetFloat() / 100.0f;
                    table["radius"] = sceneDoc["layers"][layerNum]["objects"][i]["width"].GetFloat() / 200.0f;
                    table["trigger_radius"] = sceneDoc["layers"][layerNum]["objects"][i]["width"].GetFloat() / 200.0f;
                    table["rotation"] = sceneDoc["layers"][layerNum]["objects"][i]["rotation"].GetFloat();
                }
                if (type == "SpriteRenderer") {
                    pair<int, int> imageDims = ImageDB::GetImageWidthandHeight(table["sprite"]);
                    table["scale_x"] = sceneDoc["layers"][layerNum]["objects"][i]["width"].GetFloat() / imageDims.first;
                    table["scale_y"] = sceneDoc["layers"][layerNum]["objects"][i]["height"].GetFloat() / imageDims.second;
                    table["sorting_order"] = layerNum;
                }
            }
        }
        shared_ptr<Actor> actor = make_shared<Actor>(newActor);
        ActorStore actorNew;
        actorNew.pointer = actor;
        actorNew.actorID = actor->actorID;
        actorNew.state = ACTOR_STATE::ACTIVE;
        actorsToAdd.push_back(actorNew);
    }
}

void Scene::DoTemplate(string templateName, ActorRef actor)
{
    EngineUtils utils;
    if (utils.DoesFileExist("actor_templates/" + templateName + ".template")) {
        rapidjson::Document tempDoc;
        utils.ReadJsonFile("resources/actor_templates/" + templateName + ".template", tempDoc);
        if (tempDoc.HasMember("name")) {
            actor->actor_name = tempDoc["name"].GetString();
        }
        if (tempDoc.HasMember("components")) {
            for (auto& componentKV : tempDoc["components"].GetObject()) {
                std::string key = componentKV.name.GetString(); // The key, e.g., "1", "2".
                std::string type = componentKV.value["type"].GetString(); // Getting the type from the component.
                actor->AddComponentToActor(type, key);
                for (auto& m : componentKV.value.GetObject()) {
                    string memberName = m.name.GetString();
                    if (memberName != "type") {
                        luabridge::LuaRef table = *actor->componentsByKey[key];
                        if (m.value.IsString()) {
                            table[memberName] = m.value.GetString();
                        }
                        else if (m.value.IsBool()) {
                            table[memberName] = m.value.GetBool();
                        }
                        else if (m.value.IsInt()) {
                            table[memberName] = m.value.GetInt();
                        }
                        else if (m.value.IsFloat()) {
                            table[memberName] = m.value.GetFloat();
                        }
                    }
                }
            }
        }
    }
}

void Scene::GetNewScene(string scene_name)
{
    EngineUtils utils;
    
    if (utils.CheckForScene(scene_name)) {
        utils.ReadJsonFile("resources/scenes/" + scene_name + ".scene", sceneDoc);
        sceneName = scene_name;
        LoadScene();
    }
}