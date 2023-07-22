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

void Renderer::render(const Camera& camera, const Scene& scene, const Options& options)
{
    // update camera
    updateCamera(camera);

    // update lights
    if (scene.pointLights.size() == 0)
    {
        std::cout << "No light" << std::endl;
    }
    else
    {
        updatePointLight(scene.pointLights[0]);
    }

    if (options.useShadow)
    {
        genDepthMap(scene.pointLights[0], scene.models, options);
    }

    //render model
    for (const auto& model : scene.models)
    {
        if (model.display == false)
            continue;

        if (options.showFacet)
        {
            renderFacets(model);
        }

        if (options.showNormal)
        {
            renderNormal(model);
        }
    }

     //render background
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //always filled mode 
    renderBackground();
    if (options.wire)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    }

    //render light
    renderLight(scene.pointLights[0]);

    //delete shadow related frameBuffer
    if (options.useShadow)
    {
        glDeleteFramebuffers(1, &depthMapFBO);     
        glDeleteTextures(1, &depthMap);
    }
}

void Renderer::updateCamera(const Camera& camera)
{
    _phongShader->use();
    _phongShader->setUniformMat4("projection", camera.getProjectionMatrix());
    _phongShader->setUniformMat4("view", camera.getViewMatrix());
    _phongShader->setUniformVec3("viewPos", camera.transform.position);
    _phongShader->unuse();

    _flatShader->use();
    _flatShader->setUniformMat4("projection", camera.getProjectionMatrix());
    _flatShader->setUniformMat4("view", camera.getViewMatrix());
    _flatShader->unuse();

    _normalShader->use();
    _normalShader->setUniformMat4("projection", camera.getProjectionMatrix());
    _normalShader->setUniformMat4("view", camera.getViewMatrix());
    _normalShader->unuse();
}

void Renderer::updatePointLight(const PointLight &light)
{
    const std::string lightStr = "pointLight";
    _phongShader->use();
    _phongShader->setUniformVec3(lightStr + ".position", light.position);
    _phongShader->setUniformVec3(lightStr + ".ambient", light.ambient);
    _phongShader->setUniformVec3(lightStr + ".diffuse", light.diffuse);
    _phongShader->setUniformVec3(lightStr + ".specular", light.specular);
    _phongShader->setUniformFloat(lightStr + ".constant", light.kc);
    _phongShader->setUniformFloat(lightStr + ".linear", light.kl);
    _phongShader->setUniformFloat(lightStr + ".quadratic", light.kq);
    _phongShader->unuse();
}


void Renderer::initShaders(const std::string& shaderBasePath)
{
    const std::string flatVertShaderRelPath = shaderBasePath + "/flat/flat.vert";
    const std::string flatFragShaderRelPath = shaderBasePath + "/flat/flat.frag";

    const std::string phongVertShaderRelPath = shaderBasePath + "/phong/phong.vert";
    const std::string phongFragShaderRelPath = shaderBasePath + "/phong/phong.frag";

    const std::string shadowVertShaderRelPath = shaderBasePath + "/shadow/shadowvShader.vert";
    const std::string shadowFragShaderRelPath = shaderBasePath + "/shadow/shadowfShader.frag";

    const std::string normalVertShaderRelPath = shaderBasePath + "/normal/normal_visualization.vert";
    const std::string normalFragShaderRelPath = shaderBasePath + "/normal/normal_visualization.frag";
    const std::string normalGeomShaderRelPath = shaderBasePath + "/normal/normal_visualization.geom";

    _flatShader.reset(new GLSLProgram);
    _phongShader.reset(new GLSLProgram);
    _shadowShader.reset(new GLSLProgram);
    _normalShader.reset(new GLSLProgram);

    _flatShader->attachVertexShaderFromFile(flatVertShaderRelPath);
    _flatShader->attachFragmentShaderFromFile(flatFragShaderRelPath);
    _flatShader->link();

    _phongShader->attachVertexShaderFromFile(phongVertShaderRelPath);
    _phongShader->attachFragmentShaderFromFile(phongFragShaderRelPath);
    _phongShader->link();

    _shadowShader->attachVertexShaderFromFile(shadowVertShaderRelPath);
    _shadowShader->attachFragmentShaderFromFile(shadowFragShaderRelPath);
    _shadowShader->link();

    _normalShader->attachVertexShaderFromFile(normalVertShaderRelPath);
    _normalShader->attachFragmentShaderFromFile(normalFragShaderRelPath);
    _normalShader->attachGeometryShaderFromFile(normalGeomShaderRelPath);
    _normalShader->link();


}

void Renderer::renderFacets(const AssimpModel& model)
{
    //if (model.facetMaterial->getType() == MaterialType::Phong)
    //{
    //    const PhongMaterial* material = static_cast<PhongMaterial*>(model.facetMaterial.get());

    //    _phongShader->use();
    //    //_phongShader->setUniformMat4("model", model.transform.getLocalMatrix());

    //    _phongShader->setUniformVec3("material.ambient", material->ka);
    //    _phongShader->setUniformVec3("material.diffuse", material->kd);
    //    _phongShader->setUniformVec3("material.specular", material->ks);
    //    _phongShader->setUniformFloat("material.shininess", material->ns);

    //    _phongShader->setUniformMat4("lightSpaceMatrix", lightSpaceMatrix);

    //    for (const auto& mesh : model.meshes)
    //    {
    //        // draw mesh
    //        glBindVertexArray(mesh.VAO);
    //        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
    //        glBindVertexArray(0);
    //    }

    //    _phongShader->unuse();

    //}
    //else
    //{
        for (const auto& mesh : model.meshes) {
            // bind appropriate textures
            unsigned int diffuseNr = 1;
            unsigned int specularNr = 1;
            unsigned int normalNr = 1;
            unsigned int heightNr = 1;

            const PhongMaterial* material = static_cast<PhongMaterial*>(model.facetMaterial.get());
            _phongShader->use();
            _phongShader->setUniformVec3("material.ambient", material->ka);
            _phongShader->setUniformVec3("material.diffuse", material->kd);
            _phongShader->setUniformVec3("material.specular", material->ks);
            _phongShader->setUniformFloat("material.shininess", material->ns);

            _phongShader->setUniformMat4("lightSpaceMatrix", lightSpaceMatrix);

            bool use_texture_kd = false;
            bool use_texture_ks = false;
            bool use_texture_normal = false;
            bool use_texture_height = false;

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
                }
                else if (name == "texture_specular")
                {
                    number = std::to_string(specularNr++); // transfer unsigned int to string
                    use_texture_ks = true;
                }
                else if (name == "texture_normal")
                {
                    number = std::to_string(normalNr++); // transfer unsigned int to string
                    use_texture_normal = true;
                }
                else if (name == "texture_height")
                {
                    number = std::to_string(heightNr++); // transfer unsigned int to string
                    use_texture_height = true;
                }

                // now set the sampler to the correct texture unit
                _phongShader->setUniformInt((name + number).c_str(), i+1);

                // and finally bind the texture
                glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
            }

            _phongShader->setUniformBool("use_texture_kd", use_texture_kd);
            _phongShader->setUniformBool("use_texture_ks", use_texture_ks);
            _phongShader->setUniformBool("use_texture_normal", use_texture_normal);
            //_phongShader->setUniformBool("use_texture_height", use_texture_height);

            _phongShader->setUniformInt("shadowMap", 0);

            // draw mesh
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            // always good practice to set everything back to defaults once configured.
            glActiveTexture(GL_TEXTURE0);

            _phongShader->unuse();
        }

    //}
}

void Renderer::renderNormal(const AssimpModel& model)
{
    _normalShader->use();
    //_normalShader->setUniformMat4("model", model.transform.getLocalMatrix());

    for (const auto& mesh : model.meshes)
    {
        // draw mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    _normalShader->unuse();

}

void Renderer::renderBackground() 
{
    // floor
    //define material
    PhongMaterial material;
    material.ka = glm::vec3(1.0f, 1.0f, 1.0f);
    material.kd = glm::vec3(0.5f, 0.5f, 0.5f);
    material.ks = glm::vec3(0.0f, 0.0f, 0.0f);
    
    glm::mat4 model = glm::mat4(1.0f);
    _phongShader->use();
    _phongShader->setUniformMat4("model", model);

    _phongShader->setUniformVec3("material.ambient", material.ka);
    _phongShader->setUniformVec3("material.diffuse", material.kd);
    _phongShader->setUniformVec3("material.specular", material.ks);
    _phongShader->setUniformFloat("material.shininess", material.ns);

    _phongShader->setUniformBool("use_texture_kd", false);
    _phongShader->setUniformBool("use_texture_ks", false);
    _phongShader->setUniformBool("use_texture_normal", false);

    _phongShader->setUniformInt("shadowMap", 0);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    _phongShader->unuse();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}

void Renderer::genDepthMap(const PointLight& l, const std::vector<AssimpModel> &models, const Options &options)
{

    glEnable(GL_DEPTH_TEST);  
    glCullFace(GL_FRONT);
    //Generate frame buffer object
    glGenFramebuffers(1, &depthMapFBO);

    //depth map resolution
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    //Generate depth map
    glGenTextures(1, &depthMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH,
                 SHADOW_HEIGHT,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    //Bind depth map to frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //glDeleteFramebuffers(1, &depthMapFBO);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render depth of scene to texture (from light's perspective)
    glm::mat4 lightProjection, lightView;
    float     near_plane = 0.1f, far_plane = 30.0f;
    lightProjection  = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightView        = glm::lookAt(l.position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    // render scene from light's point of view
    _shadowShader->use();
    _shadowShader->setUniformMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    //glBindTexture(GL_TEXTURE_2D, woodTexture);
    // 
    //render scene from light perspective
    //render models
    if (options.showFacet)
    {
        for (const auto& model : models)
        {
            if (model.display == false)
                continue;
            //_shadowShader->setUniformMat4("model", model.transform.getLocalMatrix());
            for (const auto& mesh : model.meshes)
            {
                // draw mesh
                glBindVertexArray(mesh.VAO);
                glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
    }

    //render background
    _shadowShader->setUniformMat4("model", glm::mat4(1.0f));
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //reset view port
    glViewport(0, 0, screenWidth, screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::renderLight(const PointLight& pointlight)
{
    //render light
    if (1)
    {
        float* temp = new float[3];
        temp[0]     = pointlight.position.x;
        temp[1]     = pointlight.position.y;
        temp[2]     = pointlight.position.z;

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
        glPointSize(20.0f);
        glDrawArrays(GL_POINTS, 0, 1);
        _flatShader->unuse();

        //memory clear
        delete[] temp;
        glDeleteVertexArrays(1, &light_vao);
        glDeleteBuffers(1, &light_vbo);
    }
}