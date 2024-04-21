#include "Engine.h"


Engine::Engine(): camera(Camera::GetInstance()), scene(Scene::GetInstance())
{

}

Engine::~Engine()
{
}

void Engine::Initialize()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_EVENTS);

    utils.CheckForFile("resources/");
    utils.CheckForFile("resources/game.config");
    utils.ReadJsonFile("resources/game.config", gameConfig);
    
    string initial_scene_name = "";
    if (gameConfig.FindMember("initial_scene") != gameConfig.MemberEnd() && gameConfig["initial_scene"].GetString() != "") {
        initial_scene_name = gameConfig["initial_scene"].GetString();
    }
    else {
        cout << "error: initial_scene unspecified";
        exit(0);
    }

    int height = 360;
    int width = 640;
    Uint8 r = 255;
    Uint8 g = 255; 
    Uint8 b = 255; 
    Uint8 a = 255;
    
    heightWindow = height;

    bool usingRenderConfig = false;
    if (utils.DoesFileExist("rendering.config")) {
        utils.ReadJsonFile("resources/rendering.config", camera.renderingConfig);
        usingRenderConfig = true;
        if (camera.renderingConfig.HasMember("x_resolution")) {
            width = camera.renderingConfig["x_resolution"].GetInt();
        }
        if (camera.renderingConfig.HasMember("y_resolution")) {
            height = camera.renderingConfig["y_resolution"].GetInt();
        }
        if (camera.renderingConfig.HasMember("clear_color_r")) {
            r = camera.renderingConfig["clear_color_r"].GetInt();
        }
        if (camera.renderingConfig.HasMember("clear_color_g")) {
            g = camera.renderingConfig["clear_color_g"].GetInt();
        }
        if (camera.renderingConfig.HasMember("clear_color_b")) {
            b = camera.renderingConfig["clear_color_b"].GetInt();
        }
    }
    camera.usingRenderConfig = usingRenderConfig;
    
    renderer.Initialize(gameConfig["game_title"].GetString(), width, height);
    renderer.CreateRender(r, g, b, a);

    imageDB.renderer = renderer.renderer;

    //initialize all components
    string path = "resources/component_types/";

    InjectDebugClass();
    InjectActorClass();
    InjectApplicationNameSpace();
    InjectInputClass();
    InjectTextNamespace();
    InjectAudioNamespace();
    InjectImageNamespace();
    InjectCameraNamespace();
    InjectSceneNamespace();
    InjectVector2Class();
    InjectRigidbodyComponent();

    comp.InitializeComponents(path);

    scene.GetNewScene(initial_scene_name);
    
    if (Rigidbody::world_initialized) {
        Rigidbody::world->SetContactListener(&listner);
    }

    scene.RunStartFunctions();

}

void Engine::GameLoop()
{
    Initialize();
    while (gameRunning) {
        Update();
        Render();
        if (proceedToNewScene) {
            proceedToNewScene = false;
            scene.GetNewScene(newSceneName);
        }
    }
}

void Engine::Update() {
    SDL_Event next_event;
    while (Helper::SDL_PollEvent498(&next_event)) {
        switch (next_event.type) {
        case SDL_QUIT:
            gameRunning = false;
            break;
        default:
            Input::ProcessEvent(next_event);
        }
    }
    scene.RunStartFunctions();
    for (auto& actor : scene.actors) {
        if (actor.state == ACTOR_STATE::DESTROYED) {
            continue;
        }
        actor.pointer->ProcessAddedComponents();
    }
    for (auto& actor : scene.actors) {
        if (actor.state == ACTOR_STATE::DESTROYED) {
            continue;
        }
        actor.pointer->cppOnUpdate();
    }
    for (auto& actor : scene.actors) {
        if (actor.state == ACTOR_STATE::DESTROYED) {
            continue;
        }
        actor.pointer->cppOnLateUpdate();
    }
    for (auto& actor : scene.actors) {
        if (actor.state == ACTOR_STATE::DESTROYED) {
            continue;
        }
        actor.pointer->ProcessRemovedComponents();
    }
    scene.ProcessDestroyedActors();
    Input::LateUpdate();
    if (Rigidbody::world_initialized) {
        Rigidbody::world->Step(1.0f / 60.0f, 8, 3);
    }
}


void Engine::Render()
{
    renderer.ClearRender();

    SDL_RenderSetScale(renderer.renderer, renderer.zoomFactor, renderer.zoomFactor);
    RenderImages();
    SDL_RenderSetScale(renderer.renderer, 1, 1);
    RenderUI();
    SDL_RenderSetScale(renderer.renderer, renderer.zoomFactor, renderer.zoomFactor);
    RenderTexts();
    RenderPixels();
    SDL_RenderSetScale(renderer.renderer, 1, 1);

    Helper::SDL_RenderPresent498(renderer.renderer);
}

void Engine::RenderTexts()
{
    while (!text.textRequests.empty()) {
        DialogueEntry* top = text.textRequests.front();
        text.textRequests.pop();
        SDL_Color color = { static_cast<Uint8>(top->r), static_cast<Uint8>(top->g), static_cast<Uint8>(top->b), static_cast<Uint8>(top->a) };
        Renderer::RenderText(top->font, color, top->content, top->x, top->y);
        delete top;
        top = nullptr;
    }
}

void Engine::RenderImages()
{
    while (!imageDB.imageRequests.empty()) {
        ImageRequest top = imageDB.imageRequests.top();
        imageDB.imageRequests.pop();
        if (top.extended) {
            renderer.RenderImageEx(top.texture, top.x, top.y, static_cast<int>(top.rotation_degrees), top.scale_x, top.scale_y, top.pivot_x, top.pivot_y, top.r, top.g, top.b, top.a);
        }
        else {
            renderer.RenderImage(top.texture, top.x, top.y);
        }
    }
}

void Engine::RenderUI()
{
    while (!imageDB.uiRequests.empty()) {
        UIRequest top = imageDB.uiRequests.top();
        imageDB.uiRequests.pop();
        if (top.extended) {
            renderer.RenderUIElementEx(top.texture, top.x, top.y, top.r, top.g, top.b, top.a);
        }
        else {
            renderer.RenderUIElement(top.texture, top.x, top.y);
        }
    }
}

void Engine::RenderPixels() {
    // Ensure alpha blending is enabled for drawing pixels with alpha
    SDL_SetRenderDrawBlendMode(renderer.renderer, SDL_BLENDMODE_BLEND);

    while (!imageDB.renderRequestPixels.empty()) {
        // Get the next pixel request from the queue
        PixelRequest top = imageDB.renderRequestPixels.front();
        imageDB.renderRequestPixels.pop();

        // Set the draw color to the requested color, including alpha
        SDL_SetRenderDrawColor(renderer.renderer, top.r, top.g, top.b, top.a);

        // Draw the pixel at the requested coordinates
        SDL_RenderDrawPoint(renderer.renderer, top.x, top.y);
    }

    // Reset the blend mode to default
    SDL_SetRenderDrawBlendMode(renderer.renderer, SDL_BLENDMODE_NONE);
}

void Engine::InjectDebugClass()
{
    luabridge::getGlobalNamespace(comp.lua_state)
        .beginNamespace("Debug")
        .addFunction("Log", ComponentManager::CppLog)
        .addFunction("LogError", ComponentManager::CppLogError)
        .endNamespace();
}

void Engine::InjectActorClass()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::cppGetComponentByKey)
        .addFunction("GetComponent", &Actor::cppGetComponent)
        .addFunction("GetComponents", &Actor::cppGetComponents)
        .addFunction("AddComponent", &Actor::AddComponentToActorRuntime)
        .addFunction("RemoveComponent", &Actor::RemoveComponent)
        .endClass();

    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Actor")
        .addFunction("FindAll", &Scene::cppFindAll)
        .addFunction("Find", &Scene::cppFind)
        .addFunction("Instantiate", &Scene::InstantiateActor)
        .addFunction("Destroy", &Scene::cppActorDestroy)
        .endNamespace();
}

void Engine::InjectApplicationNameSpace()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Application")
        .addFunction("Quit", &Engine::cppQuit)
        .addFunction("Sleep", &Engine::cppSleep)
        .addFunction("GetFrame", &Engine::cppGetFrame)
        .addFunction("OpenURL", &Engine::cppOpenUrl)
        .endNamespace();
}

void Engine::InjectInputClass()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass();

    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Input")
        .addFunction("GetKey", &Input::GetKeyWrapper)
        .addFunction("GetKeyDown", &Input::GetKeyDownWrapper)
        .addFunction("GetKeyUp", &Input::GetKeyUpWrapper)
        .addFunction("GetMouseButton", &Input::GetMouseButtonWrapper)
        .addFunction("GetMouseButtonDown", &Input::GetMouseButtonDownWrapper)
        .addFunction("GetMouseButtonUp", &Input::GetMouseButtonUpWrapper)
        // Expose GetMousePosition and GetMouseScrollDelta directly as they do not need wrappers
        .addFunction("GetMousePosition", &Input::GetMousePosition)
        .addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
        .endNamespace();
}

void Engine::InjectTextNamespace()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Text")
        .addFunction("Draw", &TextDB::AddTextRequest)
        .endNamespace();
}

void Engine::InjectAudioNamespace()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Audio")
        .addFunction("Play", &AudioDB::PlayAudio)
        .addFunction("Halt", &AudioDB::cppHalt)
        .addFunction("SetVolume", &AudioDB::SetVolume)
        .endNamespace();
}

void Engine::InjectImageNamespace()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Image")
        .addFunction("DrawUI", &ImageDB::DrawUI)
        .addFunction("DrawUIEx", &ImageDB::DrawUIEx)
        .addFunction("Draw", &ImageDB::Draw)
        .addFunction("DrawEx", &ImageDB::DrawEx)
        .addFunction("DrawPixel", &ImageDB::DrawPixel)
        .endNamespace();
}

void Engine::InjectCameraNamespace()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Camera")
        .addFunction("SetPosition", &Renderer::SetPosition)
        .addFunction("GetPositionX", &Renderer::GetPositionX)
        .addFunction("GetPositionY", &Renderer::GetPositionY)
        .addFunction("SetZoom", &Renderer::SetZoom)
        .addFunction("GetZoom", &Renderer::GetZoom)
        .endNamespace();
}

void Engine::InjectSceneNamespace()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Scene")
        .addFunction("Load", &Engine::LoadScene)
        .addFunction("GetCurrent", &Scene::GetCurrentScene)
        .addFunction("DontDestroy", &Scene::DontDestroy)
        .endNamespace();
}

void Engine::InjectVector2Class()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*) (float, float)>()
        .addFunction("__add", &b2Vec2::operator_add)
        .addFunction("__sub", &b2Vec2::operator_sub)
        .addFunction("__mul", &b2Vec2::operator_mul)
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addStaticFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
        .addStaticFunction("Distance", b2Distance)
        .endClass();
}

void Engine::InjectRigidbodyComponent()
{
    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginClass<Rigidbody>("Rigidbody")
        .addConstructor<void(*)()>()
        .addData("enabled", &Rigidbody::enabled)
        .addData("key", &Rigidbody::key)
        .addData("type", &Rigidbody::type)
        .addData("actor", &Rigidbody::actor)
        .addData("x", &Rigidbody::x)
        .addData("y", &Rigidbody::y)
        .addData("actorPtr", &Rigidbody::actor)
        .addProperty("body_type", &Rigidbody::body_type)
        .addProperty("precise", &Rigidbody::precise)
        .addProperty("gravity_scale", &Rigidbody::gravity_scale)
        .addProperty("density", &Rigidbody::density)
        .addProperty("angular_friction", &Rigidbody::angular_friction)
        .addProperty("rotation", &Rigidbody::rotation)
        .addProperty("has_collider", &Rigidbody::has_collider)
        .addProperty("has_trigger", &Rigidbody::has_trigger)
        .addProperty("trigger_type", &Rigidbody::trigger_type)
        .addProperty("trigger_width", &Rigidbody::trigger_width)
        .addProperty("trigger_height", &Rigidbody::trigger_height)
        .addProperty("trigger_radius", &Rigidbody::trigger_radius)
        .addProperty("collider_type", &Rigidbody::collider_type)
        .addProperty("width", &Rigidbody::width)
        .addProperty("height", &Rigidbody::height)
        .addProperty("radius", &Rigidbody::radius)
        .addProperty("friction", &Rigidbody::friction)
        .addProperty("bounciness", &Rigidbody::bounciness)
        .addFunction("GetPosition", &Rigidbody::GetPosition)
        .addFunction("GetRotation", &Rigidbody::GetRotation)
        .addFunction("OnStart", &Rigidbody::OnStart)
        .addFunction("AddForce", &Rigidbody::AddForce)
        .addFunction("SetVelocity", &Rigidbody::SetVelocity)
        .addFunction("SetPosition", &Rigidbody::SetPosition)
        .addFunction("SetRotation", &Rigidbody::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
        .addFunction("SetGravityScale", &Rigidbody::SetGravityScalse)
        .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
        .addFunction("GetVelocity", &Rigidbody::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
        .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
        .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
        .addFunction("OnDestroy", &Rigidbody::OnDestroy)
        .endClass();

    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginClass<collision>("collision")
        .addProperty("other", &collision::other)
        .addProperty("point", &collision::point)
        .addProperty("relative_velocity", &collision::relative_velocity)
        .addProperty("normal", &collision::normal)
        .endClass();

    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginClass<HitResult>("HitResult")
        .addProperty("actor", &HitResult::actor)
        .addProperty("point", &HitResult::point)
        .addProperty("normal", &HitResult::normal)
        .addProperty("is_trigger", &HitResult::is_trigger)
        .endClass();

    luabridge::getGlobalNamespace(ComponentManager::lua_state)
        .beginNamespace("Physics")
        .addFunction("Raycast", &Engine::Raycast)
        .addFunction("RaycastAll", &Engine::RaycastAll)
        .endNamespace();
}
