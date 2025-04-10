#ifndef LIGHT_H
#define LIGHT_H

class Light {
public:
    Light(glm::vec3 position, glm::vec3 color);
    glm::vec3 GetPosition();
    glm::vec3 GetColor();

private:
    glm::vec3 Position;
    glm::vec3 Color;
};

#endif