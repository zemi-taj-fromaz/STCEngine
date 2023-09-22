#pragma once

#include "AppImpl.h"
#include <memory>

class Application
{
public:
    Application();
    ~Application();
    void Run();

private:
    void InitializeWindow();
    void InitializeApp();
    void MainLoop();
    void Cleanup();

private:
    std::unique_ptr<AppImpl> m_AppImpl;
};

