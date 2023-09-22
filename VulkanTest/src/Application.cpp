#include "Application.h"
#include "Platform/Vulkan/AppVulkanImpl.h"

Application::Application()
{
    m_AppImpl = std::unique_ptr<AppImpl>(AppImpl::Create());
}

Application::~Application()
{

}

void Application::Run() 
{
    {
        this->InitializeWindow();
        this->InitializeApp();
        this->MainLoop();
        this->Cleanup();
    }
}



void Application::InitializeWindow()
{
    m_AppImpl->InitializeWindow();
}

void Application::InitializeApp()
{
    m_AppImpl->InitializeApp();
}

void Application::MainLoop()
{
    m_AppImpl->MainLoop();
}
void Application::Cleanup()
{
    m_AppImpl->Cleanup();
}

