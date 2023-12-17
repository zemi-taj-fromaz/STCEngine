#pragma once

#include "AppImpl.h"
#include "LayerStack.h"

#include <memory>

class Application
{
public:
    Application();
    ~Application();
    void run();


private:
    void initialize_window();
    void initialize_app();
    void main_loop();
    void cleanup();

protected:
    void push_layer(std::shared_ptr<Layer>& layer);

private:

    std::unique_ptr<AppImpl> m_AppImpl;
    static Application* s_Application;
};

std::unique_ptr<Application> CreateApplication();


