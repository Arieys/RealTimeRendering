#pragma once
#include "material.h"

#include "base/light.h"
#include "model.h"
class Scene
{
public:
    // lights
    //AmbientLight                  ambientLight;
    std::vector<DirectionalLight> directionalLights;
    std::vector<PointLight>       pointLights;
    std::vector<SpotLight>        spotLights;

    // models
    std::vector<AssimpModel> models;

    void addToScene(AssimpModel& model) {
        models.push_back(std::move(model));
    }

};