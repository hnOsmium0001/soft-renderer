#include "Scene.hpp"

glm::vec4 Camera::TransformAffine(const glm::vec4& pos) const {
    return transformation * pos;
}

glm::vec3 Camera::TransformPos(glm::vec3 pos) const {
    auto affine = TransformAffine(glm::vec4(pos, 1.0f));
    return glm::vec3(affine.x / affine.w, affine.y / affine.w, affine.z / affine.w);
}
