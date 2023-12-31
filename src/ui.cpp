#include "ui.h"

UI::UI(GLFWwindow* window)
{
    // init imGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#if defined(__EMSCRIPTEN__)
    ImGui_ImplOpenGL3_Init("#version 100");
#elif defined(USE_GLES)
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init();
#endif
}

UI::~UI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool UI::wantCaptureMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

void UI::render(unique_ptr<PerspectiveCamera>& camera, UIOptions& options, Scene& scene)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    MenuGUI(options, scene);

    if (show_body)
        //PSBodyManagementGUI(scene.models);
    if (show_scene)
        SceneManagementGUI(scene, camera);
    if (show_option)
        SceneOptionGUI(scene, options);

    init_flag = false;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::SceneManagementGUI(Scene& scene, unique_ptr<PerspectiveCamera>& camera)
{
    if (init_flag)
    {
        ImGui::SetNextWindowPos(ImVec2(IG_LT_X, IG_LT_Y));
        ImGui::SetNextWindowSize(ImVec2(IG_LT_W, IG_LT_H));
    }

    ImGui::Begin("Scene management"); // Create a window called "Hello, world!" and append into it.

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Models"))
        {
            //show model property
            for (int i = 0; i < scene.models.size(); i++) {
                if (ImGui::TreeNode(("model " + std::to_string(i)).c_str()))
                {
                    ImVec4 temp;
                    glm::vec3* t = nullptr;
                    t = &scene.models[i].transform.position;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::SliderFloat3("Position", (float*)&temp, -30.0f, 30.0f);
                    UI::glmAssignment(temp, *t);

                    t = &scene.models[i].transform.scale;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::SliderFloat3("Scale", (float*)&temp, 0.01f, 10.0f);
                    UI::glmAssignment(temp, *t);

                    //todo : to adjust rotation in scene
                    //t = &scene.models[i].transform.rotation;
                    //UI::ImVec4Assignment(temp, *t);
                    //ImGui::SliderFloat3("Rotation", (float*)&temp, -1.0f, 1.0f);
                    //UI::glmAssignment(temp, *t);
                    ImGui::TreePop();
                }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Cameras"))
        {
            //show camera property
            ImVec4 temp;
            glm::vec3* t = nullptr;
            t = &camera->transform.position;
            UI::ImVec4Assignment(temp, *t);
            ImGui::SliderFloat3("Position", (float*)&temp, -30.0f, 30.0f);
            UI::glmAssignment(temp, *t);

            
            ImGui::SliderFloat("fovy", (float*)&camera->fovy, 0.1f, 45.0f);
            ImGui::SliderFloat("aspect", (float*)&camera->aspect, 0.1f, 45.0f);
            ImGui::SliderFloat("znear", (float*)&camera->znear, 0.1f, 10.0f);
            ImGui::SliderFloat("zfar", (float*)&camera->zfar, 0.1f, 1000.0f);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Material"))
        {
            for (int i = 0; i < scene.models.size(); i++)
            {
                if (ImGui::TreeNode(("model " + std::to_string(i)).c_str()))
                {
                    ImVec4 temp;
                    auto material =
                        static_cast<PhongMaterial*>(scene.models[i].facetMaterial.get());

                    glm::vec3* t = &material->ka;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::ColorEdit3("Ambient", (float*)&temp);
                    UI::glmAssignment(temp, *t);

                    t = &material->kd;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::ColorEdit3("Diffuse", (float*)&temp);
                    UI::glmAssignment(temp, *t);

                    t = &material->ks;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::ColorEdit3("Specular", (float*)&temp);
                    UI::glmAssignment(temp, *t);

                    ImGui::TreePop();
                }
            }
            //show material property
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Light"))
        {
            for (int i = 0; i < scene.pointLights.size(); i++)
            {
                std::string name = "PointLight" + std::to_string(i);
                ImVec4 temp;
                if (ImGui::TreeNode(name.c_str()))
                {
                    glm::vec3* t = &scene.pointLights[i].ambient;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::ColorEdit3("Ambient", (float*)&temp);
                    UI::glmAssignment(temp, *t);

                    t = &scene.pointLights[i].diffuse;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::ColorEdit3("Diffuse", (float*)&temp);
                    UI::glmAssignment(temp, *t);

                    t = &scene.pointLights[i].specular;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::ColorEdit3("Specular", (float*)&temp);
                    UI::glmAssignment(temp, *t);

                    t = &scene.pointLights[i].position;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::SliderFloat3("Position", (float*)&temp, -30.0f, 30.0f);
                    UI::glmAssignment(temp, *t);

                    ImGui::SliderFloat("Constant", (float*)&scene.pointLights[i].kc, 0.001f, 1.0f);
                    ImGui::SliderFloat("Linear", (float*)&scene.pointLights[i].kl, 0.001f, 1.0f);
                    ImGui::SliderFloat("Quadratic", (float*)&scene.pointLights[i].kq, 0.001f, 1.0f);
                    ImGui::Separator();

                    ImGui::TreePop();
                }
            }
            for (int i = 0; i < scene.directionalLights.size(); i++)
            {
                std::string name = "DirectionalLight" + std::to_string(i);
                ImVec4 temp;
                if (ImGui::TreeNode(name.c_str()))
                {
                    glm::vec3* t = &scene.directionalLights[i].color;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::ColorEdit3("Color", (float*)&temp);
                    UI::glmAssignment(temp, *t);

                    t = &scene.directionalLights[i].position;
                    UI::ImVec4Assignment(temp, *t);
                    ImGui::SliderFloat3("Position", (float*)&temp, -30.0f, 30.0f);
                    UI::glmAssignment(temp, *t);

                    ImGui::SliderFloat("Intensity", (float*)&scene.directionalLights[i].intensity, 0.001f, 1.0f);
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                    
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}


void UI::ShowMenuFile(UIOptions& options, Scene& scene)
{
    //ImGui::MenuItem("(demo menu)", NULL, false, false);
    if (ImGui::MenuItem("New"))
    { //create doc
    }
    if (ImGui::MenuItem("Open", "Ctrl+O"))
    {
    }
    if (ImGui::BeginMenu("Open Recent"))
    {
        //find recent files
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowMenuFile(options, scene);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S"))
    {
    }
    if (ImGui::MenuItem("Save As.."))
    {
    }
    if (ImGui::MenuItem("Import step"))
    {
        //ModelParameter p;
        //BrepModel model;
        //model = BrepModelConstructor::createBody(ModelType::File, p);
        //scene.models.push_back(std::move(model));
    }

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(
                p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true))
    {
    }
    if (ImGui::MenuItem("Quit", "Alt+F4"))
    {
    }
}

void UI::ShowAppMainMenuBar(UIOptions& options, Scene& scene)
{
    if (ImGui::BeginMainMenuBar())
    {
        //File operation
        if (ImGui::BeginMenu("File"))
        {
            ShowMenuFile(options, scene);
            ImGui::EndMenu();
        }
        //Edit operation
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z"))
            {
            }
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false))
            {
            } // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X"))
            {
            }
            if (ImGui::MenuItem("Copy", "CTRL+C"))
            {
            }
            if (ImGui::MenuItem("Paste", "CTRL+V"))
            {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene"))
        {
            if (ImGui::MenuItem("AddLight"))
            {
            }
            if (ImGui::MenuItem("Material"))
            {
            } // Disabled item
            ImGui::Separator();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("scene management"))
            {
                show_scene = !show_scene;
            }
            if (ImGui::MenuItem("rendering option GUI"))
            {
                show_option = !show_option;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Test cases"))
        {

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UI::MenuGUI(UIOptions& options, Scene& scene)
{
    ShowAppMainMenuBar(options, scene);
}

void UI::SceneOptionGUI(Scene& scene, UIOptions& options)
{
    if (init_flag)
    {
        ImGui::SetNextWindowPos(ImVec2(IG_RT_X, IG_RT_Y));
        ImGui::SetNextWindowSize(ImVec2(IG_RT_W, IG_RT_H));
    }

    if (ImGui::Begin("Rendering Option Manager"))
    {
        ImGui::Text("Display: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(IG_RT_W - 400);
        ImGui::Checkbox("facet", &options.displayFacet);
        ImGui::SameLine();
        ImGui::Checkbox("normal", &options.displayNormal);

        ImGui::Text("Wire Mode: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(IG_RT_W - 400);
        ImGui::Checkbox("On", &options.wire);

        ImGui::Text("Normal Map: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(IG_RT_W - 400);
        ImGui::Checkbox("Use", &options.useNormalMap);

        ImGui::Text("Renderer type: ");
        ImGui::SameLine();
        ImGui::SetCursorPosX(IG_RT_W - 400);
        if (ImGui::RadioButton("forward", options.renderType == RenderType::FORAWRD)) {
            options.renderType = RenderType::FORAWRD;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("deferred", options.renderType == RenderType::DEFERRED)) {
            options.renderType = RenderType::DEFERRED;
        }
        if (options.renderType == RenderType::FORAWRD)
        {
            ImGui::Text("Shader type: ");
            ImGui::SameLine();
            ImGui::SetCursorPosX(IG_RT_W - 400);
            if (ImGui::RadioButton("Phong", options.fShaderType == ForwardShaderType::Phong)) {
                options.fShaderType = ForwardShaderType::Phong;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("CSM", options.fShaderType == ForwardShaderType::CSM)) {
                options.fShaderType = ForwardShaderType::CSM;
            }
            ImGui::Text("Shadow: ");
            ImGui::SameLine();
            ImGui::SetCursorPosX(IG_RT_W - 400);
            ImGui::Checkbox("UseShadow", &options.useShadow);

            if (options.fShaderType == ForwardShaderType::CSM)
            {
                ImGui::Text("Advanced Shadow Options: ");
                ImGui::SameLine();
                ImGui::SetCursorPosX(IG_RT_W - 400);
                ImGui::Checkbox("CSMLayerVisulization", &options.CSMLayerVisulization);
                ImGui::SameLine();
                ImGui::Checkbox("CSMDebug", &options.CSMDebug);
            }
        }
        else if (options.renderType == RenderType::DEFERRED)
        {
            ImGui::Text("Deferred Shader Type: ");
            ImGui::SameLine();
            ImGui::SetCursorPosX(IG_RT_W - 400);
            if (ImGui::RadioButton("displayGbuffer", options.dShaderType == DeferredShaderType::GBufferDisplay)) {
                options.dShaderType = DeferredShaderType::GBufferDisplay;
            }

            if (options.dShaderType == DeferredShaderType::GBufferDisplay)
            {
                ImGui::Text("Display info: ");
                ImGui::SameLine();
                ImGui::SetCursorPosX(IG_RT_W - 400);
                if (ImGui::RadioButton("Gposition", options.gbufferDisplayType == GbufferDisplayType::POSITION)) {
                    options.gbufferDisplayType = GbufferDisplayType::POSITION;
                }
                ImGui::SameLine();

                if (ImGui::RadioButton("GNormal", options.gbufferDisplayType == GbufferDisplayType::NORMAL)) {
                    options.gbufferDisplayType = GbufferDisplayType::NORMAL;
                }
                ImGui::SameLine();

                if (ImGui::RadioButton("Gdiffuse", options.gbufferDisplayType == GbufferDisplayType::DIFFUSE)) {
                    options.gbufferDisplayType = GbufferDisplayType::DIFFUSE;
                }
                ImGui::SameLine();

                if (ImGui::RadioButton("Gspecular", options.gbufferDisplayType == GbufferDisplayType::TEXCOORDS)) {
                    options.gbufferDisplayType = GbufferDisplayType::TEXCOORDS;
                }
                ImGui::SameLine();
            }
        }
    }
    ImGui::End();
}
