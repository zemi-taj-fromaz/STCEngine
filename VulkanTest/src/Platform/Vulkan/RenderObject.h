#pragma once

#include "Renderable.h"

class RenderObject : public Renderable
{
public:
   // RenderObject(){}

    RenderObject(MeshWrapper* meshHandle, bool isSkybox) : Renderable(meshHandle, isSkybox)
    {}

    RenderObject(MeshWrapper* meshHandle) : RenderObject(meshHandle, false)
    {}
    RenderObject(const RenderObject& renderObject);

    ~RenderObject(){}
};
