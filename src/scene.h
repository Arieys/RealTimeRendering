#pragma once
#include "material.h"

#include "base/light.h"
#include "model.h"
#include "base/bounding_box.h"
class Scene
{
public:
    // lights
    //AmbientLight                  ambientLight;
    std::vector<DirectionalLight> directionalLights;
    std::vector<PointLight>       pointLights;
    std::vector<SpotLight>        spotLights;

    //boundingbox
    BoundingBox box;

    // models
    std::vector<AssimpModel> models;

    void addToScene(AssimpModel& model) {
        box += model.box;
        models.push_back(std::move(model));
        updateDirectionalLight();
    }

    void updateDirectionalLight()
    {
        for (auto& l : directionalLights) 
        {
            l.position = box.max * 1.5f;
            l.direction = glm::normalize(l.position);
        }
    }

};