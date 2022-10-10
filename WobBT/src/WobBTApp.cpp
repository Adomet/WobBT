#include "WobBTApp.h"

class ExampleLayer : public Walnut::Layer
{
public:
    virtual void OnUIRender() override
    {
        ImGui::Begin("Hello");
        ImGui::Button("Button");
        ImGui::End();

        ImGui::ShowDemoWindow();
    }
};

WobBTApp::WobBTApp(int argc,char** argv)
{
    m_App = Walnut::CreateApplication(argc, argv);
}

WobBTApp::~WobBTApp()
{

}

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
    Walnut::ApplicationSpecification spec;
    spec.Name = "Walnut Example";

    Walnut::Application* app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();
    app->SetMenubarCallback([app]()
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit"))
                {
                    app->Close();
                }
                ImGui::EndMenu();
            }
        });
    return app;
}

void WobBTApp::Run()
{
    m_App->m_Running=true;

    while (m_App->m_Running)
    {
        m_App->Run();
        delete m_App;
    }
}

