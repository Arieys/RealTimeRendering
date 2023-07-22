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

class Renderer
{
public:
    struct Options
    {
        bool showFacet;
        bool wire;
        bool useShadow;
        bool showNormal;
    };

    Renderer(const std::string& shaderBasePath);

    void setScreenSize(int height, int width) {
        screenHeight = height, screenWidth = width;
    }

    void render(const Camera& camera, const Scene& scene, const Options& options);

    ~Renderer();

private:
    std::unique_ptr<GLSLProgram> _flatShader;
    std::unique_ptr<GLSLProgram> _phongShader;
    std::unique_ptr<GLSLProgram> _shadowShader;
    std::unique_ptr<GLSLProgram> _normalShader;

    int screenWidth;
    int screenHeight;

    GLuint depthMapFBO;
    GLuint depthMap;
    GLuint planeVAO;
    GLuint planeVBO;

    glm::mat4 lightSpaceMatrix;

    void initShaders(const std::string& shaderBasePath);

    void initBackground();

    void updateCamera(const Camera& camera);

    void updatePointLight(const PointLight& light);

    void genDepthMap(const PointLight& l, const std::vector<AssimpModel>& models, const Options& options);

    void renderLight(const PointLight& pointlight);

    void renderBackground();

    void renderFacets(const AssimpModel& model);

    void renderNormal(const AssimpModel& model);
};