#pragma once

#include "LayerStack.h"

class AppImpl
{
public:
    AppImpl() {};
    virtual ~AppImpl() {};

public:
    static AppImpl* create();

protected:
    LayerStack m_LayerStack;

public:
    virtual void initialize_window() = 0;
    virtual void initialize_app() = 0;
    virtual void main_loop() = 0;
    virtual void cleanup() = 0;
    virtual void push_layer(std::shared_ptr<Layer>& layer);
};

