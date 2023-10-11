#pragma once

class AppImpl
{
public:
    AppImpl() {};
    virtual ~AppImpl() {};

public:
    static AppImpl* create();

public:
    virtual void initialize_window() = 0;
    virtual void initialize_app() = 0;
    virtual void main_loop() = 0;
    virtual void cleanup() = 0;
};

