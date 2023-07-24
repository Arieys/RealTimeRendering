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

#include "renderer_options.h"
#include "shader.h"

class Renderer
{
public:

    Renderer(const std::string& shaderBasePath);

    void setScreenSize(int height, int width) {
        screenHeight = height, screenWidth = width;
    }

    void render(unique_ptr<PerspectiveCamera>& _camera, const Scene& scene, const RendererOptions& options);

    ~Renderer();

private:
    //current used shader
    std::shared_ptr<Shader> _currentShader;

    //functional shader
    std::unique_ptr<GLSLProgram> _flatShader;
    std::unique_ptr<GLSLProgram> _normalShader;

    //main shader
    std::shared_ptr<PhongShader> _phongShader;  
    std::shared_ptr<CSMShader> _csmShader;

    //screen info
    int screenWidth;
    int screenHeight;

    //plane render
    GLuint planeVAO;
    GLuint planeVBO;

    //base functions
    void initShaders(const std::string& shaderBasePath);
    void initBackground();
    void updateCamera(unique_ptr<PerspectiveCamera>& camera);
    void updateDirectionalLight(const DirectionalLight& light);
    void renderLight(const DirectionalLight& pointlight);
    void renderNormal(const AssimpModel& model);

};