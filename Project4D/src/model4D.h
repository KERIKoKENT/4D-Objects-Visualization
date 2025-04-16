#ifndef MODEL4D_H
#define MODEL4D_H

#include "model.h"
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

enum class Model4DType {
    Tesseract
};

// ѕользовательска€ хеш-функци€ дл€ векторов

struct Vec3Hash {
    std::size_t operator()(const glm::vec3& v) const {
        return std::hash<float>()(v.x) ^ std::hash<float>()(v.y) ^ std::hash<float>()(v.z);
    }
};

struct Vec4Hash {
    std::size_t operator()(const glm::vec4& v) const {
        return std::hash<float>()(v.x) ^ std::hash<float>()(v.y) ^ std::hash<float>()(v.z) ^ std::hash<float>()(v.w);
    }
};

// ѕользовательский оператор сравнени€ дл€ векторов

struct Vec3Equal {
    bool operator()(const glm::vec3& v1, const glm::vec3& v2) const {
        return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
    }
};

struct Vec4Equal {
    bool operator()(const glm::vec4& v1, const glm::vec4& v2) const {
        return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
    }
};


struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const {
        auto hash1 = Vec3Hash{}(pair.first);
        auto hash2 = Vec3Hash{}(pair.second);
        return hash1 ^ hash2;
    }
};

class Model4D : public Model {
public:
    Model4D(Model4DType type, glm::vec4 position, glm::vec4 size, glm::vec4 color, float wSlice)
        : Model(ModelType::Cube, glm::vec3(position), glm::vec3(size), glm::vec3(color)), type(type), position(position), size(size), color(color), wSlice(wSlice) {
        LoadModel4DData();
    }


    std::vector<glm::vec4> GetVertices() const { return defVertices; }
    void SetWSlice(float wSlice) { this->wSlice = wSlice; }
    float GetWSlice() const { return wSlice; }
    void GenerateSlice(float wSlice);
    void GenerateIndices();
    void InitBuffers() override;
    void Translate(glm::vec3 translation, glm::vec4 translation4D) override;
    void Rotate4D(float xw, float yw, float zw, float dt);

private:
    Model4DType type;

    std::vector<std::vector<GLuint>> edges;
    std::vector<std::vector<GLuint>> edges3D;
    std::vector<glm::vec4> defVertices;
    std::vector<std::vector<int>> faces;

    std::vector<std::vector<GLuint>> cells; // —писок 3D-€чеек (граней), кажда€ €чейка Ч набор индексов вершин
    std::map<std::pair<GLuint, GLuint>, std::set<std::pair<GLuint, GLuint>>> adjacentEdges;

    std::unordered_map<glm::vec3, int, Vec3Hash, Vec3Equal> intersectionMap;
    std::unordered_map<int, std::unordered_set<int>> adjacency;

    glm::vec4 position;
    glm::vec4 size;
    glm::vec4 color;
    float wSlice;
    
    void LoadModel4DData();
    bool ArePointsConnectedByEdge(const glm::vec3& point1, const glm::vec3& point2);
    std::vector<std::pair<GLuint, GLuint>> GetCellEdges(const std::vector<GLuint>& cell);
    void ComputeAdjacentEdges();
};

#endif