#pragma once

#include <glm/glm.hpp>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
    Vertex() = default;
    Vertex(const glm::vec3& p, const glm::vec3 n, const glm::vec2 texC, const glm::vec3 tan, const glm::vec3 bi)
        : Position(p), Normal(n), TexCoords(texC), Tangent(tan), Bitangent(bi) {}
    bool operator==(const Vertex& v) const {
        return (Position == v.Position) && (Normal == v.Normal) && (TexCoords == v.TexCoords) && (Tangent == v.Tangent) && (Bitangent == v.Bitangent);
    }
};

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(const Vertex& vertex) const {
        return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Normal) << 1)) >> 1)
               ^ (hash<glm::vec2>()(vertex.TexCoords) << 1);
    }
};
} // namespace std