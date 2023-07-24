#include "shader.h"

void PhongShader::renderFacet(const AssimpModel& model, const RendererOptions& options)
{
    for (const auto& mesh : model.meshes) {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        const PhongMaterial* material = static_cast<PhongMaterial*>(model.facetMaterial.get());
        _shader->use();
        _shader->setUniformVec3("material.ambient", material->ka);
        _shader->setUniformVec3("material.diffuse", material->kd);
        _shader->setUniformVec3("material.specular", material->ks);
        _shader->setUniformFloat("material.shininess", material->ns);

        _shader->setUniformMat4("model", glm::mat4(1.0f));
        _shader->setUniformMat4("lightSpaceMatrix", lightSpaceMatrix);

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
            _shader->setUniformInt((name + number).c_str(), i + 1);

            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }

        _shader->setUniformBool("use_texture_kd", use_texture_kd);
        _shader->setUniformBool("use_texture_ks", use_texture_ks);
        _shader->setUniformBool("use_texture_normal", use_texture_normal);

        _shader->setUniformInt("shadowMap", 0);

        // draw mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);

        _shader->unuse();
    }
}
void PhongShader::renderBackground()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //always filled mode 

    PhongMaterial material;
    material.ka = glm::vec3(0.1f, 0.1f, 0.1f);
    material.kd = glm::vec3(0.5f, 0.5f, 0.5f);
    material.ks = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::mat4 model = glm::mat4(1.0f);
    _shader->use();
    _shader->setUniformMat4("model", model);

    _shader->setUniformVec3("material.ambient", material.ka);
    _shader->setUniformVec3("material.diffuse", material.kd);
    _shader->setUniformVec3("material.specular", material.ks);
    _shader->setUniformFloat("material.shininess", material.ns);

    _shader->setUniformBool("use_texture_kd", false);
    _shader->setUniformBool("use_texture_ks", false);
    _shader->setUniformBool("use_texture_normal", false);

    _shader->setUniformInt("shadowMap", 0);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    _shader->unuse();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void PhongShader::genDepthMap(const DirectionalLight& l, std::unique_ptr<PerspectiveCamera>& _camera, const std::vector<AssimpModel>& models, const RendererOptions& options)
{
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    //Generate frame buffer object
    glGenFramebuffers(1, &depthMapFBO);

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
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
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
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightView = glm::lookAt(l.position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;


    // render scene from light's point of view
    _shadowShader->use();
    _shadowShader->setUniformMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

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

void CSMShader::renderFacet(const AssimpModel& model, const RendererOptions& options)
{
    for (const auto& mesh : model.meshes) {
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        const PhongMaterial* material = static_cast<PhongMaterial*>(model.facetMaterial.get());
        _shader->use();
        _shader->setUniformVec3("material.ambient", material->ka);
        _shader->setUniformVec3("material.diffuse", material->kd);
        _shader->setUniformVec3("material.specular", material->ks);
        _shader->setUniformFloat("material.shininess", material->ns);
        _shader->setUniformBool("LayerVisulization", options.CSMLayerVis);
        _shader->setUniformMat4("model", glm::mat4(1.0f));

        for (int i = 0; i < lightspace_matrics.size(); i++) {
            _shader->setUniformMat4("lightSpaceMatrices[" + std::to_string(i) + "]", lightspace_matrics[i]);
            if (i < shadowCascadeLevels.size()) _shader->setUniformFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
        }
        _shader->setUniformInt("cascadeCount", shadowCascadeLevels.size());

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
            _shader->setUniformInt((name + number).c_str(), i + 1);

            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }

        _shader->setUniformBool("use_texture_kd", use_texture_kd);
        _shader->setUniformBool("use_texture_ks", use_texture_ks);
        _shader->setUniformBool("use_texture_normal", use_texture_normal);

        _shader->setUniformInt("shadowMap", 0);

        // draw mesh
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);

        _shader->unuse();
    }
}
void CSMShader::renderBackground()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //always filled mode 

    PhongMaterial material;
    material.ka = glm::vec3(0.1f, 0.1f, 0.1f);
    material.kd = glm::vec3(0.5f, 0.5f, 0.5f);
    material.ks = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::mat4 model = glm::mat4(1.0f);
    _shader->use();
    _shader->setUniformMat4("model", model);

    _shader->setUniformVec3("material.ambient", material.ka);
    _shader->setUniformVec3("material.diffuse", material.kd);
    _shader->setUniformVec3("material.specular", material.ks);
    _shader->setUniformFloat("material.shininess", material.ns);

    _shader->setUniformBool("use_texture_kd", false);
    _shader->setUniformBool("use_texture_ks", false);
    _shader->setUniformBool("use_texture_normal", false);

    _shader->setUniformInt("shadowMap", 0);

    for (int i = 0; i < lightspace_matrics.size(); i++) {
        _shader->setUniformMat4("lightSpaceMatrices[" + std::to_string(i) + "]", lightspace_matrics[i]);
        if (i < shadowCascadeLevels.size()) _shader->setUniformFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
    }
    _shader->setUniformInt("cascadeCount", shadowCascadeLevels.size());

    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    _shader->unuse();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void CSMShader::genDepthMap(const DirectionalLight& l, std::unique_ptr<PerspectiveCamera>& _camera, const std::vector<AssimpModel>& models, const RendererOptions& options)
{
    lightspace_matrics = getLightSpaceMatrices(_camera, l);

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    //Generate frame buffer object
    glGenFramebuffers(1, &depthMapFBO);

    //Generate depth map
    glGenTextures(1, &depthMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);
    glTexImage3D(GL_TEXTURE_2D_ARRAY,
        0,
        GL_DEPTH_COMPONENT,
        SHADOW_WIDTH,
        SHADOW_HEIGHT,
        lightspace_matrics.size(),
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < lightspace_matrics.size(); i++) {
        //Bind depth map to frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0, i);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //render depth of scene to texture (from light's perspective)

        glm::mat4 lightSpaceMatrix = lightspace_matrics[i];

        // render scene from light's point of view
        _shadowShader->use();
        _shadowShader->setUniformMat4("lightSpaceMatrix", lightSpaceMatrix);
        _shadowShader->setUniformMat4("model", glm::mat4(1.0f));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, i);
        glClear(GL_DEPTH_BUFFER_BIT);

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

        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        _shadowShader->unuse();


        //reset view port
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

}

glm::mat4 CSMShader::getLightSpaceMatrix(const float nearPlane, const float farPlane, std::unique_ptr<PerspectiveCamera>& _camera, const DirectionalLight& l)
{
    const auto proj = glm::perspective(_camera->fovy, _camera->aspect, nearPlane, farPlane);
    const auto corners = getFrustumCornersWorldSpace(proj, _camera->getViewMatrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    const auto lightView = glm::lookAt(center + l.direction, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    constexpr float zMult = 10.0f;
    if (minZ < 0)
    {
        minZ *= zMult;
    }
    else
    {
        minZ /= zMult;
    }
    if (maxZ < 0)
    {
        maxZ /= zMult;
    }
    else
    {
        maxZ *= zMult;
    }

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
}

std::vector<glm::mat4> CSMShader::getLightSpaceMatrices(std::unique_ptr<PerspectiveCamera>& _camera, const DirectionalLight& l)
{
    shadowCascadeLevels = std::vector{ _camera->zfar / 50.0f, _camera->zfar / 25.0f, _camera->zfar / 10.0f, _camera->zfar / 2.0f };
    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(_camera->znear, shadowCascadeLevels[i], _camera, l));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i], _camera, l));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], _camera->zfar, _camera, l));
        }
    }
    return ret;
}


std::vector<glm::vec4> CSMShader::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt =
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

void CSMShader::renderDubugInfo()
{
    _debugShader->use();
    _debugShader->setUniformInt("layer", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMap);

    {
        unsigned int quadVAO, quadVBO;
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
}