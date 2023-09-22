#pragma once

class AppImpl
{
public:
    AppImpl() {};
    virtual ~AppImpl() {};

public:
    static AppImpl* Create();

public:
    virtual void InitializeWindow() = 0;
    virtual void InitializeApp() = 0;
    virtual void MainLoop() = 0;
    virtual void Cleanup() = 0;
};

