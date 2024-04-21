#include "Camera.h"

Camera::Camera(): scene(Scene::GetInstance()) {
    // Retrieve width from config or use default value
    if (usingRenderConfig && renderingConfig.HasMember("x_resolution")) {
        camWidth = renderingConfig["x_resolution"].GetInt();
    }
    else {
        camWidth = CAMERA_WIDTH; // Default width
    }

    // Retrieve height from config or use default value
    if (usingRenderConfig && renderingConfig.HasMember("y_resolution")) {
        camHeight = renderingConfig["y_resolution"].GetInt();
    }
    else {
        camHeight = CAMERA_HEIGHT; // Default height
    }
}

// Singleton Access Method
Camera& Camera::GetInstance() {
    static Camera instance;
    return instance;
}