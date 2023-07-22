#pragma once

#include <glm/glm.hpp>

enum MaterialType
{
    FlatPoint,
    FlatLine,
    Phong,
};

struct Material
{
    virtual ~Material() = default;

    virtual MaterialType getType() const noexcept = 0;
};

struct PointMaterial : public Material
{
    glm::vec3 color{0.8f};
    float     pointSize{1.0f};

    PointMaterial() = default;

    PointMaterial(const glm::vec3& color, float pointSize)
        : color(color)
        , pointSize(pointSize)
    {}

    MaterialType getType() const noexcept override
    {
        return MaterialType::FlatPoint;
    }
};

struct LineMaterial : public Material
{
    LineMaterial() = default;

    LineMaterial(const glm::vec3& color, float lineWidth)
        : color(color)
        , lineWidth(lineWidth)
    {}

    glm::vec3 color{0.8f};
    float     lineWidth{1.0f};

    MaterialType getType() const noexcept override
    {
        return MaterialType::FlatLine;
    }
};

struct PhongMaterial : public Material
{
    glm::vec3 ka{0.05f, 0.05f, 0.05f};
    glm::vec3 kd{1.0f, 0.5f, 0.31f};
    glm::vec3 ks{0.5f, 0.5f, 0.5f};
    float     ns{32.0f};

    PhongMaterial() = default;

    PhongMaterial(const glm::vec3& ka, const glm::vec3& kd, const glm::vec3& ks, float ns)
        : ka(ka)
        , kd(kd)
        , ks(ks)
        , ns(ns)
    {}

    MaterialType getType() const noexcept override
    {
        return MaterialType::Phong;
    };
};