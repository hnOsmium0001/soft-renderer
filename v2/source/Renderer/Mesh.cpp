#include "Mesh.hpp"

#include "Color.hpp"

#include <fstream>
#include <unordered_map>
#include <vector>

void Mesh::ReadObj(std::istream& data) {
    char ctrash;
    std::vector<glm::vec3> posBuf;
    std::vector<glm::vec3> normalBuf;
    std::vector<glm::vec2> uvBuf;
    std::unordered_map<Vertex, uint32_t> knownVerts;
    uint32_t nextID = 0;

    std::string line;
    while (!data.eof()) {
        std::string start;
        data >> start;
        if (start == "#") {
            continue;
        } else if (start == "v") {
            glm::vec3 pos;
            for (int i = 0; i < 3; ++i) {
                data >> pos[i];
            }
            posBuf.push_back(pos);
        } else if (start == "vt") {
            glm::vec3 normal;
            for (int i = 0; i < 3; ++i) {
                data >> normal[i];
            }
            normalBuf.push_back(normal);
        } else if (start == "vn") {
            glm::vec2 uv;
            for (int i = 0; i < 2; ++i) {
                data >> uv[i];
            }
            uvBuf.push_back(uv);
        } else if (start == "f") {
            uint32_t tri[3] = {};
            // We only support triangular faces (with 3 vertices)
            // Format: f pos/uv/normal pos/uv/normal pos/uv/normal
            for (int i = 0; i < 3; ++i) {
                uint32_t iv, it, in;
                data >> iv; // A vertex must have at least the position
                data >> ctrash;
                if (!uvBuf.empty()) data >> it;
                data >> ctrash;
                if (!normalBuf.empty()) data >> in;

                // .obj file index starts at 1
                --iv;
                --it;
                --in;
                auto candidate = Vertex{
                    .pos = posBuf[iv],
                    .normal = normalBuf.empty() ? glm::vec3{} : normalBuf[in],
                    .uv = uvBuf.empty() ? glm::vec2{} : uvBuf[it],
                    .color = RgbaColor(255, 255, 255), // TODO
                };

                auto iter = knownVerts.find(candidate);
                if (iter != knownVerts.end()) {
                    // The vertex already exists, reuse the known version
                    tri[i] = iter->second;
                } else {
                    // The vertex does not exist, create it
                    knownVerts.insert({ std::move(candidate), nextID });
                    tri[i] = nextID;
                    ++nextID;
                }
            }

            for (int i = 0; i < std::size(tri); ++i) {
                indices.push_back(tri[i]);
            }
        } else if (start == "g") {
            // TODO
        }
    }

    vertices.reserve(knownVerts.size());
    for (const auto& [vert, idx] : knownVerts) {
        vertices.push_back(vert);
    }
}

void Mesh::ReadObjAt(const char* path) {
    std::ifstream ifs(path);
    if (!ifs) return;
    ReadObj(ifs);
}
