#pragma once

#include "Utilities.h"
#include "Scene.h"

const int CAMERA_WIDTH = 13;
const int CAMERA_HEIGHT = 9;

class Camera
{
private:
    // Private constructor
    Camera();
    // Delete copy constructor and assignment operator
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    Scene& scene;

public:
    int camWidth;
    int camHeight;

    static Camera& GetInstance();

    rapidjson::Document renderingConfig;
    bool usingRenderConfig = false;
};
