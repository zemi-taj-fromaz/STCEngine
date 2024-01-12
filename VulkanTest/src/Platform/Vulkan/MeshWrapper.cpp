#include "MeshWrapper.h"
#include "RenderObject.h"

void MeshWrapper::update_position(glm::vec3 position, glm::vec3 Front)
{
   object->update_position(position, Front);
}