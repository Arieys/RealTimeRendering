#pragma once

#include "transform.h"

struct Light {
    //Transform transform;
    //float intensity = 1.0f;
    //glm::vec3 color = {1.0f, 1.0f, 1.0f};
    //glm::vec3 ambient;
    //glm::vec3 diffuse;
    //glm::vec3 specular;
};

//struct AmbientLight : public Light {};

struct DirectionalLight : public Light {
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
};

struct PointLight : public Light {
    glm::vec3 position = glm::vec3(-1.525f, 15.864f, 2.203f);
    glm::vec3 ambient  = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuse  = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    float kc = 0.3f;
    float kl = 0.09f;
    float kq = 0.0032f;
};

struct SpotLight : public Light {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float angle = glm::radians(60.0f);
    float kc = 1.0f;
    float kl = 0.0f;
    float kq = 1.0f;
};