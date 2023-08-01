#include "renderer.h"

Renderer::Renderer(const std::string& shaderBasePath)
{
    initShaders(shaderBasePath);
    initBackground();
}

Renderer::~Renderer() 
{
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
}

void Renderer::render(unique_ptr<PerspectiveCamera>& camera, const Scene& scene, const UIOptions& options)
{
    if (options.renderType == RenderType::FORAWRD)
    {
        forwardShading(camera, scene, options);
    }
    else if (options.renderType == RenderType::DEFERRED)
    {
        deferredShading(camera, scene, options);
    }
}

void Renderer::forwardShading(unique_ptr<PerspectiveCamera>& camera, const Scene& scene, const UIOptions& options)
{
    //set current main pipeline shader
    if (options.useShadow) {
        if (options.useCSM) _currentShader = _csmShader;
        else _currentShader = _phongShader;
    }
    else _currentShader = _phongShader;

    //update ui options
    _currentShader->setRenderOptions(options);

    //update screen size
    _currentShader->setScreenSize(this->screenWidth, this->screenHeight);
    _currentShader->setBackgroundVAOVBO(this->planeVAO, this->planeVBO);
    // update camera
    updateCamera(camera);

    // update lights
    if (scene.directionalLights.size() == 0)
    {
        std::cout << "No light" << std::endl;
        return;
    }
    else
    {
        updateDirectionalLight(scene.directionalLights[0]);
    }

    //gen shadow map
    if (options.useShadow)
    {
        _currentShader->genDepthMap(scene.directionalLights[0], camera, scene.models);
    }

    //render model
    for (const auto& model : scene.models)
    {
        if (model.display == false)
            continue;

        if (options.displayFacet)
        {
            if (options.useShadow && options.useCSM && options.CSMDebug) {
                std::dynamic_pointer_cast<CSMShader>(_currentShader)->renderDubugInfo();
            }
            else _currentShader->renderFacet(model);
        }

        if (options.displayNormal)
        {
            renderNormal(model);
        }
    }
    //render light
    renderLight(scene.directionalLights[0]);

    //render background
    _currentShader->renderBackground();

    //set wire option
    if (options.wire)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (options.useShadow) {
        _currentShader->deleteBuffer();
    }
}

void Renderer::deferredShading(unique_ptr<PerspectiveCamera>& camera, const Scene& scene, const UIOptions& options)
{
    //update camera
    _deferredShader->use();
    _deferredShader->setUniformMat4("projection", camera->getProjectionMatrix());
    _deferredShader->setUniformMat4("view", camera->getViewMatrix());
    _deferredShader->unuse();


    _gBuffer.reset(new GBuffer());

    _gBuffer->Init(screenWidth, screenHeight);

    _gBuffer->BindForWriting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& model : scene.models)
    {
        if (model.display == false)
            continue;

        if (options.displayFacet)
        {
            renderGbuffer(model);
        }

    }
    
    renderGbufferToScreen();
}

void Renderer::updateCamera(unique_ptr<PerspectiveCamera>& camera)
{
    _currentShader->updateCamera(camera);

    _normalShader->use();
    _normalShader->setUniformMat4("projection", camera->getProjectionMatrix());
    _normalShader->setUniformMat4("view", camera->getViewMatrix());
    _normalShader->unuse();

    _flatShader->use();
    _flatShader->setUniformMat4("projection", camera->getProjectionMatrix());
    _flatShader->setUniformMat4("view", camera->getViewMatrix());
    _flatShader->unuse();
}

void Renderer::updateDirectionalLight(const DirectionalLight&light)
{
    _currentShader->updateLight(light);
}


void Renderer::initShaders(const std::string& shaderBasePath)
{
    const std::string flatVertShaderRelPath = shaderBasePath + "/flat/flat.vert";
    const std::string flatFragShaderRelPath = shaderBasePath + "/flat/flat.frag";

    const std::string phongVertShaderRelPath = "/phong/phong.vert";
    const std::string phongFragShaderRelPath = "/phong/phong.frag";

    const std::string csmVertShaderRelPath = "/csm/csm.vert";
    const std::string csmFragShaderRelPath = "/csm/csm.frag";

    const std::string normalVertShaderRelPath = shaderBasePath + "/normal/normal_visualization.vert";
    const std::string normalFragShaderRelPath = shaderBasePath + "/normal/normal_visualization.frag";
    const std::string normalGeomShaderRelPath = shaderBasePath + "/normal/normal_visualization.geom";

    const std::string gBufferGenVertShaderRelPath = shaderBasePath + "/gbufferGen/gbufferGen.vert";
    const std::string gBufferGenFragShaderRelPath = shaderBasePath + "/gbufferGen/gbufferGen.frag";

    _flatShader.reset(new GLSLProgram);
    _normalShader.reset(new GLSLProgram);
    _deferredShader.reset(new GLSLProgram);

    _phongShader.reset(new PhongShader(screenWidth, screenHeight, shaderBasePath,phongVertShaderRelPath, phongFragShaderRelPath));
    _csmShader.reset(new CSMShader(screenWidth, screenHeight, shaderBasePath,csmVertShaderRelPath, csmFragShaderRelPath));

    _flatShader->attachVertexShaderFromFile(flatVertShaderRelPath);
    _flatShader->attachFragmentShaderFromFile(flatFragShaderRelPath);
    _flatShader->link();

    _normalShader->attachVertexShaderFromFile(normalVertShaderRelPath);
    _normalShader->attachFragmentShaderFromFile(normalFragShaderRelPath);
    _normalShader->attachGeometryShaderFromFile(normalGeomShaderRelPath);
    _normalShader->link();

    _deferredShader->attachVertexShaderFromFile(gBufferGenVertShaderRelPath);
    _deferredShader->attachFragmentShaderFromFile(gBufferGenFragShaderRelPath);
    _deferredShader->link();
    
}

void Renderer::initBackground()
{
    //add scene plane
    float planeVertices[] = {
        // positions            // normals         // texcoords
        125.0f, -1.0f, 125.0f, 0.0f, 1.0f, 0.0f,   25.0f,  0.0f,
        -125.0f, -1.0f, 125.0f,  0.0f, 1.0f, 0.0f,  0.0f,   0.0f,
        -125.0f, -1.0f, -125.0f, 0.0f, 1.0f, 0.0f,  0.0f,   25.0f,

        125.0f, -1.0f, 125.0f, 0.0f, 1.0f, 0.0f,   25.0f,  0.0f,
        -125.0f, -1.0f, -125.0f, 0.0f, 1.0f, 0.0f,  0.0f,   25.0f,
        125.0f,  -1.0f, -125.0f, 0.0f, 1.0f, 0.0f,  25.0f,  25.0f
    };
    // plane VAO
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}


void Renderer::renderNormal(const AssimpModel& model)
{
    _normalShader->use();
    _normalShader->setUniformMat4("model", model.transform.getLocalMatrix());

    for (const auto& mesh : model.meshes)
    {
        // draw mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    _normalShader->unuse();

}

void Renderer::renderLight(const DirectionalLight& light)
{
    //render light
    if (1)
    {
        float* temp = new float[3];
        temp[0]     = light.position.x;
        temp[1]     = light.position.y;
        temp[2]     = light.position.z;
        GLuint light_vao, light_vbo;
        glGenVertexArrays(1, &light_vao);
        glGenBuffers(1, &light_vbo);
        glBindVertexArray(light_vao);
        glBindBuffer(GL_ARRAY_BUFFER, light_vbo);

        glBufferData(
            GL_ARRAY_BUFFER, 3 * sizeof(float), temp, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glm::mat4 model(1.0f);
        _flatShader->use();
        _flatShader->setUniformMat4("model", model);
        _flatShader->setUniformVec3("material.color",glm::vec3(1.0f,1.0f,1.0f));
        glPointSize(5.0f);
        glDrawArrays(GL_POINTS, 0, 1);
        _flatShader->unuse();

        //memory clear
        delete[] temp;
        glDeleteVertexArrays(1, &light_vao);
        glDeleteBuffers(1, &light_vbo);
    }
}

void Renderer::renderGbufferToScreen()
{
    //bind current drawing framebuffer to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (_gBuffer == nullptr) {
        std::cout << "render gbuffer error : _gBuffer nullptr" << std::endl;
    }

    //bind current reading framebuffer
    _gBuffer->BindForReading();

    //copy framebuffer 
    GLsizei HalfWidth = (GLsizei)(this->screenWidth / 2.0f);
    GLsizei HalfHeight = (GLsizei)(this->screenHeight / 2.0f);
    _gBuffer->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
    glBlitFramebuffer(0, 0, screenWidth, screenHeight,
        0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    _gBuffer->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, screenWidth, screenHeight,
        0, HalfHeight, HalfWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    _gBuffer->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, screenWidth, screenHeight,
        HalfWidth, HalfHeight, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    _gBuffer->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
    glBlitFramebuffer(0, 0, screenWidth, screenHeight,
        HalfWidth, 0, screenWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void Renderer::renderGbuffer(const AssimpModel& model)
{
    for (const auto& mesh : model.meshes) {
        _deferredShader->use();
        _deferredShader->setUniformMat4("model", model.transform.getLocalMatrix());

        // draw mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        _deferredShader->unuse();
    }
}
