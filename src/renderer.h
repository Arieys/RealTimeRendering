#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "base/camera.h"
#include "base/glsl_program.h"
#include "base/index_buffer.h"
#include "base/light.h"
#include "base/texture.h"
#include "base/uniform_buffer.h"
#include "base/vertex.h"
#include "base/vertex_array.h"
#include "base/vertex_buffer.h"

#include "scene.h"
#include "base/application.h"

#include "ui_options.h"
#include "shader.h"
#include "gbuffer.h"

class Renderer
{
public:

    Renderer(const std::string& shaderBasePath);

    void setScreenSize(int height, int width) {
        screenHeight = height, screenWidth = width;
    }

    void render(unique_ptr<PerspectiveCamera>& _camera, const Scene& scene, const UIOptions& options);

    ~Renderer();

private:
    //current used shader
    std::shared_ptr<Shader> _currentShader;

    //functional shader
    std::unique_ptr<GLSLProgram> _flatShader;
    std::unique_ptr<GLSLProgram> _normalShader;

    //main forward shader
    std::shared_ptr<PhongShader> _phongShader;  
    std::shared_ptr<CSMShader> _csmShader;

    //deferred shader
    std::unique_ptr<GLSLProgram> _deferredShader;

    //screen info
    int screenWidth;
    int screenHeight;

    //plane render
    GLuint planeVAO;
    GLuint planeVBO;

    //gbuffer
    std::unique_ptr<GBuffer> _gBuffer;

    //render type
    RenderType currentRenderType;

    //base functions
    void initShaders(const std::string& shaderBasePath);
    void initBackground();
    void updateCamera(unique_ptr<PerspectiveCamera>& camera);
    void updateDirectionalLight(const DirectionalLight& light);
    void renderLight(const DirectionalLight& pointlight);
    void renderNormal(const AssimpModel& model);

    void renderGbufferToScreen();
    void renderGbuffer(const AssimpModel& model);

    void forwardShading(unique_ptr<PerspectiveCamera>& _camera, const Scene& scene, const UIOptions& options);

    void deferredShading(unique_ptr<PerspectiveCamera>& _camera, const Scene& scene, const UIOptions& options);

};