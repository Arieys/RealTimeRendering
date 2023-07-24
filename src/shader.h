#pragma once
#include "base/glsl_program.h"
#include "renderer_options.h"
#include "model.h"
#include "base/camera.h"]
#include "base/light.h"
class Shader
{
public:
	std::unique_ptr<GLSLProgram> _shader;
	std::unique_ptr<GLSLProgram> _shadowShader;
	int screenWidth;
	int screenHeight;
	//plane render
	GLuint planeVAO;
	GLuint planeVBO;
	Shader(int width, int height, const std::string& shaderBasePath, const std::string vs_path, const std::string fs_path, const std::string gs_path = std::string(""))
	{
		screenWidth = width;
		screenHeight = height;
		std::cout << shaderBasePath << std::endl;
		const std::string VertShaderRelPath = shaderBasePath + vs_path;
		const std::string FragShaderRelPath = shaderBasePath + fs_path;
		const std::string GeomShaderRelPath = shaderBasePath + gs_path;

		_shader.reset(new GLSLProgram);

		_shader->attachVertexShaderFromFile(VertShaderRelPath);
		_shader->attachFragmentShaderFromFile(FragShaderRelPath);
		if(!gs_path.empty()) _shader->attachGeometryShaderFromFile(GeomShaderRelPath);
		_shader->link();
	}

	void setBackgroundVAOVBO(GLuint vao, GLuint vbo)
	{
		this->planeVAO = vao, this->planeVBO = vbo;
	}

	void setScreenSize(int width, int height)
	{
		screenHeight = height, screenWidth = width;
	}

	virtual void updateLight(const DirectionalLight& light)
	{
		const std::string lightStr = "dLight";
		_shader->use();
		_shader->setUniformVec3(lightStr + ".direction", light.direction);
		_shader->setUniformVec3(lightStr + ".ambient", light.color);
		_shader->setUniformVec3(lightStr + ".diffuse", light.color);
		_shader->setUniformVec3(lightStr + ".specular", light.color);
		_shader->setUniformFloat(lightStr + ".intensity", light.intensity);
		_shader->unuse();
	}
	virtual void updateCamera(unique_ptr<PerspectiveCamera>& camera)
	{
		_shader->use();
		_shader->setUniformMat4("projection", camera->getProjectionMatrix());
		_shader->setUniformMat4("view", camera->getViewMatrix());
		_shader->setUniformVec3("viewPos", camera->transform.position);
		_shader->unuse();
	}
	virtual void renderFacet(const AssimpModel& model, const RendererOptions& options) = 0;
	virtual void renderBackground() = 0;
	virtual void genDepthMap(const DirectionalLight& l, std::unique_ptr<PerspectiveCamera>& _camera, const std::vector<AssimpModel>& models, const RendererOptions& options){} //default gen no shadow map
	virtual void deleteBuffer(){}
};

class PhongShader : public Shader
{
public:
	PhongShader(int width, int height, const std::string& shaderBasePath, const std::string vs_path, const std::string fs_path, const std::string gs_path = std::string("")):
		Shader(width, height, shaderBasePath,vs_path,fs_path,gs_path)
	{
		initShadowShader(shaderBasePath);
	}
	virtual void renderFacet(const AssimpModel& model, const RendererOptions& options);
	virtual void renderBackground();
	~PhongShader()
	{
	}

	void initShadowShader(const std::string& shaderBasePath)
	{
		const std::string shadowVertShaderRelPath = shaderBasePath + "/shadow/shadowvShader.vert";
		const std::string shadowFragShaderRelPath = shaderBasePath + "/shadow/shadowfShader.frag";

		_shadowShader.reset(new GLSLProgram);

		_shadowShader->attachVertexShaderFromFile(shadowVertShaderRelPath);
		_shadowShader->attachFragmentShaderFromFile(shadowFragShaderRelPath);
		_shadowShader->link();
	}

	//depth map resolution
	const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

	//for shadow map
	GLuint depthMapFBO = 0;
	GLuint depthMap = 0;
	glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
	void genDepthMap(const DirectionalLight& l, std::unique_ptr<PerspectiveCamera>& _camera, const std::vector<AssimpModel>& models, const RendererOptions& options);
	virtual void deleteBuffer()
	{
		//delete shadow related frameBuffer
		if (depthMap != 0) glDeleteTextures(1, &depthMap);
		if (depthMapFBO != 0) glDeleteFramebuffers(1, &depthMapFBO);
	}
};

class CSMShader : public Shader
{
public:
	//debug shader
	std::unique_ptr<GLSLProgram> _debugShader;

	//for shadow map
	GLuint depthMapFBO = 0;
	GLuint depthMap = 0;

	//depth map resolution
	const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

	//for cascade shadow map
	std::vector<float> shadowCascadeLevels;
	std::vector<glm::mat4> lightspace_matrics;
	void genDepthMap (const DirectionalLight& l, std::unique_ptr<PerspectiveCamera>& _camera, const std::vector<AssimpModel>& models, const RendererOptions& options);
	std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
	glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane, std::unique_ptr<PerspectiveCamera>& _camera, const DirectionalLight& l);
	std::vector<glm::mat4> getLightSpaceMatrices(std::unique_ptr<PerspectiveCamera>& _camera, const DirectionalLight& l);
	void renderDubugInfo();

	void initShadowShader(const std::string& shaderBasePath)
	{
		const std::string shadowVertShaderRelPath = shaderBasePath + "/shadow/shadowvShader.vert";
		const std::string shadowFragShaderRelPath = shaderBasePath + "/shadow/shadowfShader.frag";

		_shadowShader.reset(new GLSLProgram);

		_shadowShader->attachVertexShaderFromFile(shadowVertShaderRelPath);
		_shadowShader->attachFragmentShaderFromFile(shadowFragShaderRelPath);
		_shadowShader->link();
	}

	void initOtherShader(const std::string& shaderBasePath)
	{
		const std::string debugVertShaderRelPath = shaderBasePath + "/csm/quadDebug.vert";
		const std::string debugFragShaderRelPath = shaderBasePath + "/csm/quadDebug.frag";

		_debugShader.reset(new GLSLProgram);

		_debugShader->attachVertexShaderFromFile(debugVertShaderRelPath);
		_debugShader->attachFragmentShaderFromFile(debugFragShaderRelPath);
		_debugShader->link();
	}


	CSMShader(int width, int height, const std::string& shaderBasePath, const std::string vs_path, const std::string fs_path, const std::string gs_path = std::string("")) :
		Shader(width, height, shaderBasePath, vs_path, fs_path, gs_path) 
	{
		initShadowShader(shaderBasePath);
		initOtherShader(shaderBasePath);
	}
	virtual void renderFacet(const AssimpModel& model, const RendererOptions& options);
	virtual void renderBackground();
	virtual void deleteBuffer()
	{
		//delete shadow related frameBuffer
		if (depthMap != 0) glDeleteTextures(1, &depthMap);
		if (depthMapFBO != 0) glDeleteFramebuffers(1, &depthMapFBO);
	}

	~CSMShader()
	{
	}
};