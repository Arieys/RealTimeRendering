#pragma once

#include <map>
#include <memory>
#include <vector>

#include "base/application.h"
#include "base/camera.h"
#include "base/light.h"

#include "camera_controller.h"
#include "renderer.h"
#include "scene.h"
#include "ui.h"
#include "ui_options.h"


class Viewer : public Application
{
public:
    Viewer(const Options& options);

    ~Viewer();

private:
    // ui
    std::unique_ptr<UI>        _ui;
    std::unique_ptr<UIOptions> _uiOptions;

    // camera
    std::unique_ptr<PerspectiveCamera> _camera;
    std::unique_ptr<CameraController>  _cameraController;

    // scene
    std::unique_ptr<Scene> _scene;

    // renderer
    std::unique_ptr<Renderer> _renderer;


private:
    void handleInput() override;

    void renderFrame() override;

    void clearScreen();
};