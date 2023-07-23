#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "ui_options.h"
#include "model.h"
#include "scene.h"

#include <string>
//IMGUI window position
#define IG_LT_X 0
#define IG_LT_Y 20
#define IG_LT_W 350
#define IG_LT_H 400

#define IG_RT_X 1270
#define IG_RT_Y 20
#define IG_RT_W 650
#define IG_RT_H 400

#define IG_LM_X 0
#define IG_LM_Y 420
#define IG_LM_W 350
#define IG_LM_H 400


class UI {
public:
    UI(GLFWwindow* window);

    ~UI();

    bool init_flag = true;

    bool show_scene   = true;
    bool show_body    = true;
    bool show_option  = true;

    bool wantCaptureMouse() const;

    void SceneManagementGUI(Scene &scene);

    void MenuGUI(UIOptions& options, Scene& scene);

    void SceneOptionGUI(Scene& scene, UIOptions& options);

    void ShowMenuFile(UIOptions& options, Scene& scene);

    void ShowAppMainMenuBar(UIOptions& options, Scene& scene);

    void render(UIOptions& options, Scene &scene);

    static void ImVec4Assignment(ImVec4& v1, glm::vec3& v2)
    {
        v1.x = v2.x, v1.y = v2.y, v1.z = v2.z;
    }

    static void glmAssignment(ImVec4& v2, glm::vec3& v1)
    {
        v1.x = v2.x, v1.y = v2.y, v1.z = v2.z;
    }
};