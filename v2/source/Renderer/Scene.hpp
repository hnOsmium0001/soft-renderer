#pragma once

#include "Renderer/Primitive.hpp"

#include <glm/glm.hpp>
#include <vector>

class Camera {
public:
    glm::mat4 transformation;

public:
    glm::vec4 TransformAffine(const glm::vec4& pos) const;
    glm::vec3 TransformPos(glm::vec3 pos) const;
};

class SceneObject {
};

class Scene {
};
