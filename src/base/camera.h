#pragma once

#include "frustum.h"
#include "transform.h"
#include "bounding_box.h"

class Camera {
public:
    Transform transform;
    float znear;
    float zfar;

public:
    virtual ~Camera() = default;

    glm::mat4 getViewMatrix() const;

    virtual glm::mat4 getProjectionMatrix() const = 0;

    virtual Frustum getFrustum() const = 0;
};

class PerspectiveCamera : public Camera {
public:
    float fovy;
    float aspect;

public:
    PerspectiveCamera(float fovy, float aspect, float znear, float zfar);

    ~PerspectiveCamera() = default;

    glm::mat4 getProjectionMatrix() const override;

    virtual Frustum getFrustum() const override;
};

class OrthographicCamera : public Camera {
public:
    float left;
    float right;
    float bottom;
    float top;

public:
    OrthographicCamera(float left, float right, float bottom, float top, float znear, float zfar);

    ~OrthographicCamera() = default;

    glm::mat4 getProjectionMatrix() const override;

    virtual Frustum getFrustum() const override;
};