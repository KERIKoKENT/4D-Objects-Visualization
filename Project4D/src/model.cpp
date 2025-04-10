#include "model.h"
#include <iostream>

void Model::LoadModelData() {
    if (type == ModelType::Cube) {
        vertices = {
            // Позиции вершин куба и нормали
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
        };

        indices = {
            // Задняя грань
            0, 1, 2, 2, 3, 0,
            // Передняя грань
            4, 5, 6, 6, 7, 4,
            // Левая грань
            8, 9, 10, 10, 11, 8,
            // Правая грань
            12, 13, 14, 14, 15, 12,
            // Нижняя грань
            16, 17, 18, 18, 19, 16,
            // Верхняя грань
            20, 21, 22, 22, 23, 20
        };
    }
}


void Model::InitBuffers() {

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Позиции вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Нормали
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    SetVAO(VAO);
    SetVBO(VBO);
    SetEBO(EBO);
}

void Model::Rotate(float xy, float xz, float yz, double dt) {

    xy *= glm::radians(1.0f) * dt * 100;
    xz *= glm::radians(1.0f) * dt * 100;
    yz *= glm::radians(1.0f) * dt * 100;
    if (abs(xy) > 1 || abs(xz) > 1 || abs(yz) > 1) return;

    glm::mat3 rotMat = glm::mat3(
        glm::vec3(cos(xy) * cos(xz), -sin(xy) * cos(yz) + cos(xy) * sin(xz) * sin(yz), sin(xy) * sin(yz) + cos(xy) * sin(xz) * cos(yz)),
        glm::vec3(sin(xy) * cos(xz), cos(xy) * cos(yz) + sin(xy) * sin(xz) * sin(yz), -cos(xy) * sin(yz) + sin(xy) * sin(xz) * cos(yz)),
        glm::vec3(-sin(xz), cos(xz) * sin(yz), cos(xz) * cos(yz))
    );

    for (int i = 0;i < vertices.size() - 5;) {
        glm::vec3 rotated = rotMat * glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
        vertices[i] = rotated.x;
        vertices[i + 1] = rotated.y;
        vertices[i + 2] = rotated.z;
        i += 6;
    }


    InitBuffers();
}

void Model::Translate(glm::vec3 translation, glm::vec4 translation4D = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) ) {

    for (int i = 0;i < vertices.size() - 5; i += 6) {
        vertices[i] += translation.x;
        vertices[i + 1] += translation.y;
        vertices[i + 2] += translation.z;
    }


    InitBuffers();
}