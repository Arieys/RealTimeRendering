#include "viewer.h"

Viewer::Viewer(const Options& options)
    : Application(options)
{   

    // camera
    _camera.reset(new PerspectiveCamera(
        glm::radians(45.0f), 1.0f * _windowWidth / _windowHeight, 0.1f, 1000.0f));
    _camera->transform.position = {0.0f, 0.0f, 5.0f};

    _cameraController.reset(
        new CameraController(*_camera, glm::vec3(0.0f), _windowWidth, _windowHeight));

    // ui
    _ui.reset(new UI(_window));
    _uiOptions.reset(new UIOptions);

    // scene
    _scene.reset(new Scene);

    // scene.lights
    //_scene->pointLights.push_back(PointLight());
    _scene->directionalLights.push_back(DirectionalLight());

    // renderer
    _renderer.reset(new Renderer(getAssetFullPath("shader")));

    // load asset
    AssimpModel model(getAssetFullPath("model/cyborg/cyborg.obj"));
    _scene->addToScene(model);

}

Viewer::~Viewer()
{
}

void Viewer::handleInput()
{
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE)
    {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    if (!_ui->wantCaptureMouse())
    {
        _cameraController->update(_input, _deltaTime);
    }

    //handle wire mode
    if (_uiOptions->wire == false)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    _input.forwardState();

}

void Viewer::renderFrame()
{
    showFpsInWindowTitle();
    clearScreen();

    Renderer::Options options{
        _uiOptions->displayFacet,
        _uiOptions->wire,
        _uiOptions->useShadow,
        _uiOptions->useCSM,
        _uiOptions->CSMDebug,
        _uiOptions->displayNormal
    };

    _renderer->setScreenSize(this->_windowHeight, this->_windowWidth);
    _renderer->render(_camera, *_scene, options);

    _ui->render(*_uiOptions, *_scene);
}

void Viewer::clearScreen()
{
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}