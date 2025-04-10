#include "model4D.h"
#include <iostream>
#include <array>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <glm/glm.hpp>
#include <GL/glew.h>



void Model4D::LoadModel4DData() {
    vertices.clear();
    indices.clear();
	adjacency.clear();
	intersectionMap.clear();

    switch (type) {
        case(Model4DType::Tesseract):
            defVertices = {
                // Позиции вершин тессеракта
                {-0.5f, -0.5f, -0.5f, -0.5f},
                {0.5f, -0.5f, -0.5f, -0.5f},
                {0.5f,  0.5f, -0.5f, -0.5f},
                {-0.5f,  0.5f, -0.5f, -0.5f},
                {-0.5f, -0.5f,  0.5f, -0.5f},
                {0.5f, -0.5f,  0.5f, -0.5f},
                {0.5f,  0.5f,  0.5f, -0.5f},
                {-0.5f,  0.5f,  0.5f, -0.5f},
                {-0.5f, -0.5f, -0.5f,  0.5f},
                {0.5f, -0.5f, -0.5f,  0.5f},
                {0.5f,  0.5f, -0.5f,  0.5f},
                {-0.5f,  0.5f, -0.5f,  0.5f},
                {-0.5f, -0.5f,  0.5f,  0.5f},
                {0.5f, -0.5f,  0.5f,  0.5f},
                {0.5f,  0.5f,  0.5f,  0.5f},
                {-0.5f,  0.5f,  0.5f,  0.5f}
            };

            edges = {
                {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7},
                {8, 9}, {9, 10}, {10, 11}, {11, 8}, {12, 13}, {13, 14}, {14, 15}, {15, 12}, {8, 12}, {9, 13}, {10, 14}, {11, 15},
                {0, 8}, {1, 9}, {2, 10}, {3, 11}, {4, 12}, {5, 13}, {6, 14}, {7, 15}  // рёбра
            };
       }
}

void Model4D::Rotate4D(float xw, float yw, float zw, float dt) {
    xw *= glm::radians(1.0f) * dt * 100;
    yw *= glm::radians(1.0f) * dt * 100;
    zw *= glm::radians(1.0f) * dt * 100;
    if (abs(xw) > 1 || abs(yw) > 1 || abs(zw) > 1) return;

    glm::mat4 rotMat = glm::mat4(
        glm::vec4(cos(xw), -sin(xw) * sin(yw), -sin(yw) * cos(yw) * sin(zw), -sin(xw) * cos(yw) * cos(zw)),
        glm::vec4(0, cos(yw), -sin(yw) * sin(zw), -sin(yw) * cos(zw)),
        glm::vec4(sin(xw), cos(xw) * sin(yw), cos(xw) * cos(yw) * sin(zw), cos(xw) * cos(yw) * cos(zw)),
        glm::vec4(0, 0, sin(zw), cos(zw))
    );


}

bool Model4D::ArePointsConnectedByEdge(const glm::vec3& point1, const glm::vec3& point2) {
    std::unordered_set<std::pair<glm::vec3, glm::vec3>, pair_hash> edgeSet;
    for (const auto& edge : edges) {
        edgeSet.insert({ glm::vec3(defVertices[edge[0]]), glm::vec3(defVertices[edge[1]]) });
        edgeSet.insert({ glm::vec3(defVertices[edge[1]]), glm::vec3(defVertices[edge[0]]) });
    }

    return edgeSet.find({ point1, point2 }) != edgeSet.end();
}

bool IntersectPlane(const glm::vec4& v1, const glm::vec4& v2, float wSlice, glm::vec3& intersection) {
    float w1 = v1.w, w2 = v2.w;

    // Избегание деления на 0
    if (w1 == w2) return false;

    float t = (wSlice - w1) / (w2 - w1);

    // Проверка, лежит ли точка на ребре
    if (t >= 0.0f && t <= 1.0f) {
        intersection.x = v1.x + t * (v2.x - v1.x);
        intersection.y = v1.y + t * (v2.y - v1.y);
        intersection.z = v1.z + t * (v2.z - v1.z);
        return true;
    }
    return false;
}

void Model4D::GenerateSlice(float wSlice) {

    vertices.clear();
    intersectionMap.clear();
    adjacency.clear();
    int indexCounter = 0;

    // 1. Определяем точки пересечения с 4D-гиперплоскостью
	for (auto& edge : edges) {



        glm::vec3 intersection;

        if (IntersectPlane(defVertices[edge[0]], defVertices[edge[1]], wSlice, intersection)) {

            if (intersectionMap.find(intersection) == intersectionMap.end()) {
                intersectionMap[intersection] = indexCounter++;
                vertices.push_back(intersection.x);
                vertices.push_back(intersection.y);
                vertices.push_back(intersection.z);
                vertices.push_back(1); //
                vertices.push_back(1); // нормали
                vertices.push_back(0); // 
            }

        }
    }


    for (auto& [point, index] : intersectionMap) {
		std::cout << "Point: " << point.x << ", " << point.y << ", " << point.z << "   " << index << std::endl;
        for (auto& [otherPoint, otherIndex] : intersectionMap) {
            if ( (point != otherPoint ) && ArePointsConnectedByEdge(point, otherPoint) ) {
                adjacency[index].insert(otherIndex);
                adjacency[otherIndex].insert(index);
            }
        }
    }

    for (auto& ad : adjacency) {
        std::cout << "Vertex " << ad.first << ": ";
        for (auto& neighbor : ad.second) {
            std::cout << neighbor << " ";
        }
        std::cout << std::endl;
    }

    GenerateIndices();
}

void Model4D::GenerateIndices() {
    indices.clear();
    faces.clear();
    
    std::unordered_set<int> visited;

    for (const auto& [index, neighbors] : adjacency) {
        for (int neighbor : neighbors) {
            for (int third : adjacency[neighbor]) {

                if (third != index && (adjacency[index].count(third) || (adjacency[neighbor].count(third)))) {

                    std::array<int, 3> face = { index, neighbor, third };
                    std::sort(face.begin(), face.end());

                    if (visited.count(face[0] * 10000 + face[1] * 100 + face[2]) == 0) { // спсоб сохранения проверенных в visited 012
                        indices.push_back(face[0]);
                        indices.push_back(face[1]);
                        indices.push_back(face[2]);

                        visited.insert(face[0] * 10000 + face[1] * 100 + face[2]);
                    }
                }
            }
        }
    }
}

void Model4D::Translate(glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4 translation4D = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) {

    for (auto& vertex : defVertices) {
        vertex += translation4D;
    }

    GenerateSlice(wSlice);

}

void Model4D::InitBuffers() {


    // Инициализация буферов OpenGL
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

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // Отладочное сообщение для проверки инициализации буферов
    /*std::cout << "VAO: " << VAO << ", VBO: " << VBO << ", EBO: " << EBO << std::endl;

    for (auto& vertex : vertices)
        std::cout << vertex << " ";

    std::cout << std::endl;

    for (auto& index : indices)
        std::cout << index << " ";*/
}