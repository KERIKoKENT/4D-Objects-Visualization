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

            cells = {
                {0, 1, 2, 3, 4, 5, 6, 7},    // Ячейка 1: нижний куб
                {8, 9, 10, 11, 12, 13, 14, 15}, // Ячейка 2: верхний куб
                {0, 3, 4, 7, 8, 11, 12, 15},  // Ячейка 3: x = -1
                {1, 2, 5, 6, 9, 10, 13, 14},  // Ячейка 4: x = +1
                {0, 1, 4, 5, 8, 9, 12, 13},   // Ячейка 5: y = -1
                {2, 3, 6, 7, 10, 11, 14, 15}, // Ячейка 6: y = +1
                {0, 1, 2, 3, 8, 9, 10, 11},   // Ячейка 7: z = -1
                {4, 5, 6, 7, 12, 13, 14, 15}  // Ячейка 8: z = +1
            };
       }
}

void Model4D::Rotate4D(float xw, float yw, float zw, float dt) {
    xw *= glm::radians(1.0f) * dt * 100;
    yw *= glm::radians(1.0f) * dt * 100;
    zw *= glm::radians(1.0f) * dt * 100;
    if (abs(xw) > 1 || abs(yw) > 1 || abs(zw) > 1) return;

    glm::mat4 rotMat = glm::mat4(
        glm::vec4(cos(xw), -sin(xw) * sin(yw), -sin(xw) * cos(yw) * sin(zw), sin(xw) * cos(yw) * cos(zw)),
        glm::vec4(0, cos(yw), -sin(yw) * sin(zw), sin(yw) * cos(zw)),
        glm::vec4(0, 0, cos(zw), sin(zw)),
        glm::vec4(-sin(xw), -cos(xw) * sin(yw), cos(xw) * -cos(yw) * sin(zw), cos(xw) * cos(yw) * cos(zw))
    );

    for (auto& vertex : defVertices) {
        vertex = rotMat * vertex;

		//std::cout << "Rotated Vertex: " << vertex.x << ", " << vertex.y << ", " << vertex.z << ", " << vertex.w << std::endl;
    }

    GenerateSlice(wSlice);

}

bool Model4D::ArePointsConnectedByEdge(const glm::vec3& point1, const glm::vec3& point2) {
    std::unordered_set<std::pair<glm::vec3, glm::vec3>, pair_hash> edgeSet;
    for (const auto& edge : edges) {
        edgeSet.insert({ glm::vec3(defVertices[edge[0]]), glm::vec3(defVertices[edge[1]]) });
        edgeSet.insert({ glm::vec3(defVertices[edge[1]]), glm::vec3(defVertices[edge[0]]) });
    }

    return edgeSet.find({ point1, point2 }) != edgeSet.end();
}

std::vector<std::pair<GLuint, GLuint>> Model4D::GetCellEdges(const std::vector<GLuint>& cell) {
    std::vector<std::pair<GLuint, GLuint>> cellEdges;
    for (size_t i = 0; i < cell.size(); ++i) {
        for (size_t j = i + 1; j < cell.size(); ++j) {
            GLuint v1 = cell[i], v2 = cell[j];
            // Проверяем, существует ли ребро (v1, v2) в edges
            for (const auto& edge : edges) {
                if ((edge[0] == v1 && edge[1] == v2) || (edge[0] == v2 && edge[1] == v1)) {
                    cellEdges.emplace_back(std::min(v1, v2), std::max(v1, v2));
                    break;
                }
            }
        }
    }
    return cellEdges;
}

void Model4D::ComputeAdjacentEdges() {
    adjacentEdges.clear();
    for (const auto& cell : cells) {
        auto cellEdges = GetCellEdges(cell);
        for (size_t i = 0; i < cellEdges.size(); ++i) {
            for (size_t j = i + 1; j < cellEdges.size(); ++j) {
                auto edge1 = cellEdges[i];
                auto edge2 = cellEdges[j];
                adjacentEdges[edge1].insert(edge2);
                adjacentEdges[edge2].insert(edge1);
            }
        }
    }
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

    std::map<std::pair<GLuint, GLuint>, glm::vec3> edgeIntersections;

    
    for (auto& edge : edges) {

        /*for (auto& point : edge) {
            for (auto& otherPoint : edge) {
                if ((point != otherPoint)) {
                    std::vector<GLuint> edge3D = { point, otherPoint };
                    std::sort(begin(edge3D), end(edge3D));
                    if (std::find(edges3D.begin(), edges3D.end(), edge3D) == edges3D.end()) {
                        edges3D.push_back(edge3D);
                    }
                }
            }
        }

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

        }*/

        glm::vec3 intersection;
        if (IntersectPlane(defVertices[edge[0]], defVertices[edge[1]], wSlice, intersection)) {
            edgeIntersections[{edge[0], edge[1]}] = intersection;
            if (intersectionMap.find(intersection) == intersectionMap.end()) {
                intersectionMap[intersection] = indexCounter++;
                vertices.push_back(intersection.x);
                vertices.push_back(intersection.y);
                vertices.push_back(intersection.z);
                vertices.push_back(1);
                vertices.push_back(1); // нормали
                vertices.push_back(1);
            }
        }
     }


    ComputeAdjacentEdges(); // Рассчитываем смежные рёбра

    /*for (auto& ad : adjacentEdges) {
        std::cout << ad.first.first << " " << ad.first.second << ": ";
			for (auto& a : ad.second) {
				std::cout << "(" << a.first << ", " << a.second << ") ";
			}
            std::cout << std::endl;
    }*/

    for (const auto& [edge1, point1] : edgeIntersections) {
        for (const auto& [edge2, point2] : edgeIntersections) {
            if (edge1 == edge2 || point1 == point2) continue;
            auto e1 = std::make_pair(std::min(edge1.first, edge1.second), std::max(edge1.first, edge1.second));
            auto e2 = std::make_pair(std::min(edge2.first, edge2.second), std::max(edge2.first, edge2.second));
            if (adjacentEdges[e1].count(e2) > 0) {
                // Соединяем точки пересечения
                GLuint idx1 = intersectionMap[point1];
                GLuint idx2 = intersectionMap[point2];
                adjacency[idx1].insert(idx2);
                adjacency[idx2].insert(idx1);
                std::vector<GLuint> edge3D = { idx1, idx2 };
                std::sort(edge3D.begin(), edge3D.end());
                if (std::find(edges3D.begin(), edges3D.end(), edge3D) == edges3D.end()) {
                    edges3D.push_back(edge3D);
                }
            }
        }
    }

    for (auto& [point, index] : intersectionMap) {
        //std::cout << "Point: " << point.x << ", " << point.y << ", " << point.z << "   " << index << std::endl;
        for (auto& [otherPoint, otherIndex] : intersectionMap) {
            if ((point != otherPoint) && ArePointsConnectedByEdge(point, otherPoint)) {
                adjacency[index].insert(otherIndex);
                adjacency[otherIndex].insert(index);
            }
        }
    }

    /*for (auto& ad : adjacency) {
        std::cout << "Vertex " << ad.first << ": ";
        for (auto& neighbor : ad.second) {
            std::cout << neighbor << " ";
        }
        std::cout << std::endl;
    }

    for (const auto& [edge, adj] : adjacentEdges) {
        std::cout << "Edge (" << edge.first << ", " << edge.second << "): ";
        for (const auto& a : adj) {
            std::cout << "(" << a.first << ", " << a.second << ") ";
        }
        std::cout << std::endl;
    }*/

    GenerateIndices();
}

void Model4D::GenerateIndices() {
    indices.clear();
    faces.clear();
    
    std::unordered_set<int> visited;

    for (const auto& [index, neighbors] : adjacency) { // Триангуляция
        for (int neighbor : neighbors) {
            for (int third : adjacency[neighbor]) {

                if (third != index && (adjacency[index].count(third) || (adjacency[neighbor].count(third)))) {

                    std::array<int, 3> face = { index, neighbor, third };
                    std::sort(face.begin(), face.end());

                    if (visited.count(face[0] * 10000 + face[1] * 100 + face[2]) == 0) { // спсоб сохранения проверенных в visited 012
                        indices.push_back(face[0]);
                        indices.push_back(face[1]);
                        indices.push_back(face[2]); 
                        faces.push_back({face[0], face[1], face[2]});
                        visited.insert(face[0] * 10000 + face[1] * 100 + face[2]);
                    }
                }
            }
        }
    }

	/*for (auto& index : indices) {
		std::cout << index << " ";
	}*/

    // Код для расчёта нормалей

    /*std::vector<glm::vec3> vertexNormals(vertices.size() / 6, glm::vec3(0.0f));
    std::vector<int> faceCount(vertices.size() / 6, 0);

    for (const auto& face : faces) {
        if (face.size() < 3) continue;
        // Координаты вершин грани
        glm::vec3 p1 = glm::vec3(vertices[face[0] * 6], vertices[face[0] * 6 + 1], vertices[face[0] * 6 + 2]);
        glm::vec3 p2 = glm::vec3(vertices[face[1] * 6], vertices[face[1] * 6 + 1], vertices[face[1] * 6 + 2]);
        glm::vec3 p3 = glm::vec3(vertices[face[2] * 6], vertices[face[2] * 6 + 1], vertices[face[2] * 6 + 2]);

        // Рассчитываем нормаль грани
        glm::vec3 v1 = p2 - p1;
        glm::vec3 v2 = p3 - p1;
        glm::vec3 normal = glm::cross(v1, v2);
        if (glm::length(normal) > 1e-6) {
            normal = glm::normalize(normal);
        }
        else {
            normal = glm::vec3(0.0f, 0.0f, 1.0f); // Заглушка
        }

        // Добавляем нормаль к вершинам грани
        for (const auto& idx : face) {
            vertexNormals[idx] += normal;
            faceCount[idx]++;
        }
    }

    // Усредняем нормали
    for (size_t i = 0; i < vertexNormals.size(); ++i) {
        if (faceCount[i] > 0) {
            vertexNormals[i] /= static_cast<float>(faceCount[i]);
            if (glm::length(vertexNormals[i]) > 1e-6) {
                vertexNormals[i] = glm::normalize(vertexNormals[i]);
            }
            else {
                vertexNormals[i] = glm::vec3(0.0f, 0.0f, 1.0f); // Заглушка
            }
        }
        else {
            vertexNormals[i] = glm::vec3(0.0f, 0.0f, 1.0f); // Заглушка для изолированных вершин
        }
    }

    // Обновляем нормали в vertices
    for (size_t i = 0; i < vertexNormals.size(); ++i) {
        size_t offset = i * 6; // [x, y, z, nx, ny, nz]
        vertices[offset + 3] = vertexNormals[i].x;
        vertices[offset + 4] = vertexNormals[i].y;
        vertices[offset + 5] = vertexNormals[i].z;
    }*/

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