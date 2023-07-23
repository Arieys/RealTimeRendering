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
        bool useCSM;
        bool CSMDebug;
        bool showNormal;
    };

    Renderer(const std::string& shaderBasePath);

    void setScreenSize(int height, int width) {
        screenHeight = height, screenWidth = width;
    }

    void render(unique_ptr<PerspectiveCamera>& _camera, const Scene& scene, const Options& options);

    ~Renderer();

private:
    std::unique_ptr<GLSLProgram> _flatShader;
    std::unique_ptr<GLSLProgram> _phongShader;
    std::unique_ptr<GLSLProgram> _csmShader;
    std::unique_ptr<GLSLProgram> _shadowShader;
    std::unique_ptr<GLSLProgram> _normalShader;
    std::unique_ptr<GLSLProgram> _debugShader;

    int screenWidth;
    int screenHeight;
    
    //depth map resolution
    const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

    //plane render
    GLuint planeVAO;
    GLuint planeVBO;

    //for shadow map
    GLuint depthMapFBO;
    GLuint depthMap;
    glm::mat4 lightSpaceMatrix;
    void genDepthMap(const DirectionalLight& l, const std::vector<AssimpModel>& models, const Options& options);

    //for cascade shadow map
    std::vector<float> shadowCascadeLevels;
    std::vector<glm::mat4> lightspace_matrics;
    void genCascadeDepthMap(const DirectionalLight& l, std::unique_ptr<PerspectiveCamera>& _camera, const std::vector<AssimpModel>& models, const Options& options);
    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
    glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane, std::unique_ptr<PerspectiveCamera>& _camera, const DirectionalLight& l);
    std::vector<glm::mat4> getLightSpaceMatrices(std::unique_ptr<PerspectiveCamera>& _camera, const DirectionalLight& l);
    void renderFacetCSM(const AssimpModel& model);
    void renderBackgroundCSM();
    void renderDubugInfo();

    //base functions
    void initShaders(const std::string& shaderBasePath);
    void initBackground();
    void updateCamera(unique_ptr<PerspectiveCamera>& camera);
    void updateDirectionalLight(const DirectionalLight& light);

    //phong_shading based functions
    void renderLight(const DirectionalLight& pointlight);
    void renderBackground();
    void renderFacets(const AssimpModel& model);
    void renderNormal(const AssimpModel& model);

};