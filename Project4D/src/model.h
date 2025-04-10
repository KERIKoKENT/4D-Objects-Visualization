#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

enum class ModelType {
    Triangle,
    Square,
    Cube
};

class Model {
public:
    Model(ModelType type, glm::vec3 position, glm::vec3 size, glm::vec3 color)
        : type(type), position(position), size(size), color(color) {
        LoadModelData();
    }

    ModelType GetType() const { return type; }
    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetSize() const { return size; }
    glm::vec3 GetColor() const { return color; }

    const std::vector<GLfloat>& GetVertices() const { return vertices; }
    const std::vector<GLuint>& GetIndices() const { return indices; }

    GLuint GetVAO() const { return VAO; }
    GLuint GetVBO() const { return VBO; }
    GLuint GetEBO() const { return EBO; }
    void SetVAO(GLuint vao) { VAO = vao; }
    void SetVBO(GLuint vbo) { VBO = vbo; }
    void SetEBO(GLuint ebo) { EBO = ebo; }

    virtual void InitBuffers();
    virtual void Rotate(float xy, float xz, float yz, double dt);
    virtual void Translate(glm::vec3 translation, glm::vec4 translation4D);

protected:
    ModelType type;
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    GLuint VAO, VBO, EBO;

private:
    void LoadModelData();
};

#endif