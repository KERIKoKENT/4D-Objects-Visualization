#include <glm/glm.hpp>
#include "light.h"

Light::Light(glm::vec3 position, glm::vec3 color) {
    Position = position;
    Color = color;
}

glm::vec3 Light::GetPosition() {
    return Position;
}

glm::vec3 Light::GetColor() {
    return Color;
}