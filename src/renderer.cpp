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
    _options = std::make_shared<UIOptions>(options);

    if (options.renderType == RenderType::FORAWRD)
    {
        forwardShading(camera, scene);
    }
    else if (options.renderType == RenderType::DEFERRED)
    {
        deferredShading(camera, scene);
    }
}

void Renderer::forwardShading(unique_ptr<PerspectiveCamera>& camera, const Scene& scene)
{
    //set current main pipeline shader
    if (_options->fShaderType == ForwardShaderType::Phong)
    {
        _currentShader = _phongShader;
    }
    else if (_options->fShaderType == ForwardShaderType::CSM)
    {
        _currentShader = _csmShader;
    }

    //update ui options
    _currentShader->setRenderOptions(*_options);

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
    if (_options->useShadow)
    {
        _currentShader->genDepthMap(scene.directionalLights[0], camera, scene.models);
    }

    //render model
    for (const auto& model : scene.models)
    {
        if (model.display == false)
            continue;

        if (_options->displayFacet)
        {
            _currentShader->renderFacet(model);
        }
        else break;

        if (_options->displayNormal)
        {
            renderNormal(model);
        }
    }
    //render light
    renderLight(scene.directionalLights[0]);

    //render background
    _currentShader->renderBackground();

    //set wire option
    if (_options->wire)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (_options->useShadow) {
        _currentShader->deleteBuffer();
    }
}

void Renderer::deferredShading(unique_ptr<PerspectiveCamera>& camera, const Scene& scene)
{
    //update camera
    _deferredShader->use();
    _deferredShader->setUniformMat4("projection", camera->getProjectionMatrix());
    _deferredShader->setUniformMat4("view", camera->getViewMatrix());
    _deferredShader->unuse();

    _gBuffer.reset(new GBuffer());

    if (!_gBuffer->Init(screenWidth, screenHeight))
    {
        cout << "init gbuufer failed" << endl;
    }

    _gBuffer->BindForWriting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& model : scene.models)
    {
        if (model.display == false)
            continue;

        if (_options->displayFacet)
        {
            renderGbuffer(model);
        }

    }
    
    //bind current drawing framebuffer to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (_options->dShaderType == DeferredShaderType::GBufferDisplay)
    {
        renderGbufferToScreen();
    }
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
    const std::string phongVertShaderRelPath = "/phong/phong.vert";
    const std::string phongFragShaderRelPath = "/phong/phong.frag";

    const std::string csmVertShaderRelPath = "/csm/csm.vert";
    const std::string csmFragShaderRelPath = "/csm/csm.frag";

    _phongShader.reset(new PhongShader(screenWidth, screenHeight, shaderBasePath,phongVertShaderRelPath, phongFragShaderRelPath));
    _csmShader.reset(new CSMShader(screenWidth, screenHeight, shaderBasePath,csmVertShaderRelPath, csmFragShaderRelPath));

    initPerShader(_flatShader, shaderBasePath, "flat", false);
    initPerShader(_normalShader, shaderBasePath, "normal", true);
    initPerShader(_deferredShader, shaderBasePath, "gbufferGen", false);
    //initPerShader(_deferredLightShader, shaderBasePath, "deferred", false);
}

void Renderer::initPerShader(std::shared_ptr<GLSLProgram> &shader,const std::string shader_path, const std::string shader_name, bool use_geometry)
{
    shader = std::make_shared<GLSLProgram>();

    const std::string VertShaderRelPath = shader_path + "/" + shader_name + "/" + shader_name + ".vert";
    const std::string FragShaderRelPath = shader_path + "/" + shader_name + "/" + shader_name + ".frag";
    const std::string GeomShaderRelPath = shader_path + "/" + shader_name + "/" + shader_name + ".geom";

    shader->attachVertexShaderFromFile(VertShaderRelPath);
    shader->attachFragmentShaderFromFile(FragShaderRelPath);
    if(use_geometry) shader->attachGeometryShaderFromFile(GeomShaderRelPath);
    shader->link();
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
    if (_gBuffer == nullptr) {
        std::cout << "render gbuffer error : _gBuffer nullptr" << std::endl;
    }

    //bind current reading framebuffer
    _gBuffer->BindForReading();

    //copy framebuffer 

    switch (_options->gbufferDisplayType) {
    case GbufferDisplayType::POSITION:
        _gBuffer->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
        glBlitFramebuffer(0, 0, screenWidth, screenHeight,
            0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        break;
    case GbufferDisplayType::DIFFUSE:
        _gBuffer->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
        glBlitFramebuffer(0, 0, screenWidth, screenHeight,
            0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        break;
    case GbufferDisplayType::NORMAL:
        _gBuffer->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
        glBlitFramebuffer(0, 0, screenWidth, screenHeight,
            0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        break;
    case GbufferDisplayType::TEXCOORDS:
        _gBuffer->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
        glBlitFramebuffer(0, 0, screenWidth, screenHeight,
            0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        break;
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        const char* errorString;
        switch (error) {
        case GL_INVALID_ENUM:
            errorString = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorString = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errorString = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            errorString = "GL_OUT_OF_MEMORY";
            break;
        default:
            errorString = "Unknown error";
            break;
        }
        cout << errorString << endl;
    }
}

void Renderer::renderGbuffer(const AssimpModel& model)
{
    for (const auto& mesh : model.meshes) {
        _deferredShader->use();
        _deferredShader->setUniformMat4("model", model.transform.getLocalMatrix());

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        const PhongMaterial* material = static_cast<PhongMaterial*>(model.facetMaterial.get());

        _deferredShader->setUniformVec3("material.diffuse", material->kd);

        bool use_texture_kd = false;
        bool use_texture_ks = false;
        bool use_texture_normal = false;

        for (unsigned int i = 0; i < mesh.textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i + 1); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = mesh.textures[i].type;
            if (name == "texture_diffuse")
            {
                number = std::to_string(diffuseNr++);
                use_texture_kd = true;
                // now set the sampler to the correct texture unit
                _deferredShader->setUniformInt((name + number).c_str(), i + 1);
            }
            else if (name == "texture_normal")
            {
                number = std::to_string(normalNr++); // transfer unsigned int to string
                use_texture_normal = true;
                // now set the sampler to the correct texture unit
                _deferredShader->setUniformInt((name + number).c_str(), i + 1);
            }
            else if (name == "texture_specular")
            {
                number = std::to_string(specularNr++); // transfer unsigned int to string
                use_texture_ks = true;
                _deferredShader->setUniformInt((name + number).c_str(), i + 1);
            }

            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }

        _deferredShader->setUniformBool("use_texture_kd", use_texture_kd);
        _deferredShader->setUniformBool("use_texture_ks", use_texture_ks);
        _deferredShader->setUniformBool("use_texture_normal", _options->useNormalMap && use_texture_normal);


        // draw mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);

        _deferredShader->unuse();
    }
}
