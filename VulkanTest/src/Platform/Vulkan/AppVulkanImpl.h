#pragma once

#include "../../AppImpl.h"


class AppVulkanImpl : public AppImpl
{
public:
    AppVulkanImpl();
    ~AppVulkanImpl();
    virtual void InitializeWindow() override;
    virtual void InitializeApp() override;
    virtual void MainLoop() override;
    virtual void Cleanup() override;
};
