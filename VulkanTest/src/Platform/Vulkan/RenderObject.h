#pragma once

#include "Renderable.h"

class RenderObject : public Renderable
{
public:
   // RenderObject(){}

    RenderObject(MeshWrapper* meshHandle ) : Renderable(meshHandle)
    {

       // this->MeshHandle->object = std::shared_ptr<RenderObject>(this);
    }

    RenderObject(const RenderObject& renderObject);

    ~RenderObject(){}
};
