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

void UI::render(UIOptions& options, Scene& scene)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    MenuGUI(options, scene);

    if (show_body)
        //PSBodyManagementGUI(scene.models);
    if (show_scene)
        SceneManagementGUI(scene);
    if (show_option)
        SceneOptionGUI(scene, options);

    init_flag = false;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::SceneManagementGUI(Scene& scene)
{
    if (init_flag)
    {
        ImGui::SetNextWindowPos(ImVec2(IG_LT_X, IG_LT_Y));
        ImGui::SetNextWindowSize(ImVec2(IG_LT_W, IG_LT_H));
    }

    ImGui::Begin("Scene management"); // Create a window called "Hello, world!" and append into it.

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Cameras"))
        {
            //show camera property
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
        //Modeling operation
        if (ImGui::BeginMenu("Modeling"))
        {
            if (ImGui::BeginMenu("CreateEntity"))
            //create minimum entity
            {
                if (ImGui::MenuItem("Solid"))
                {
                }
                if (ImGui::MenuItem("Region"))
                {
                }
                if (ImGui::MenuItem("Shell"))
                {
                }
                if (ImGui::MenuItem("Face"))
                {
                }
                if (ImGui::MenuItem("Edge"))
                {
                }
                if (ImGui::MenuItem("Half_Edge"))
                {
                }
                if (ImGui::MenuItem("Vertex"))
                {
                }
                if (ImGui::MenuItem("Node"))
                {
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("CreateVoxel"))
            //create voxel entity
            {
                if (ImGui::MenuItem("Block"))
                {
                }
                if (ImGui::MenuItem("Cone"))
                {
                }
                if (ImGui::MenuItem("Prism"))
                {
                }
                if (ImGui::MenuItem("Cylinder"))
                {
                }
                if (ImGui::MenuItem("Sphere"))
                {
                }
                if (ImGui::MenuItem("Torus"))
                {
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("BoolOperation"))
            //bool op
            {
                if (ImGui::MenuItem("Union"))
                {
                }
                if (ImGui::MenuItem("Intersection"))
                {
                }
                if (ImGui::MenuItem("Subtraction"))
                {
                }
                if (ImGui::MenuItem("Chop"))
                {
                }
                if (ImGui::MenuItem("Nonreg_Union"))
                {
                }
                if (ImGui::MenuItem("Nonreg_Intersection"))
                {
                }
                if (ImGui::MenuItem("Nonreg_Substraction"))
                {
                }
                if (ImGui::MenuItem("Nonreg_Chop"))
                {
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Extrude"))
            //select entity
            {
            }
            if (ImGui::MenuItem("Sweep"))
            //select entity
            {
            }
            if (ImGui::MenuItem("Chamfer"))
            //select entity
            {
            }
            if (ImGui::MenuItem("Fillet"))
            //select entity
            {
            }
            if (ImGui::MenuItem("Select"))
            //select entity
            {
            }
            if (ImGui::MenuItem("Delete"))
            //Delete selected entity
            {
            }
            if (ImGui::MenuItem("Import"))
            //Delete selected entity
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
            if (ImGui::MenuItem("body management"))
            {
                show_body = !show_body;
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
        ImGui::SetNextWindowPos(ImVec2(IG_LM_X, IG_LM_Y));
        ImGui::SetNextWindowSize(ImVec2(IG_LM_W, IG_LM_H));
    }

    if (ImGui::Begin("Rendering Option Manager"))
    {
        ImGui::Text("Display: ");
        ImGui::Checkbox("facet", &options.displayFacet);
        ImGui::SameLine();
        ImGui::Checkbox("normal", &options.displayNormal);

        ImGui::Text("Wire mode: ");
        ImGui::SameLine();
        ImGui::Checkbox("wire", &options.wire);

        ImGui::Checkbox("useShadow", &options.useShadow);

        ImGui::Checkbox("useCSM", &options.useCSM);
        ImGui::SameLine();
        ImGui::Checkbox("CSMLayerVisulization", &options.CSMLayerVisulization);
        ImGui::SameLine();
        ImGui::Checkbox("CSMDebug", &options.CSMDebug);
        ImGui::End();
    }
}
