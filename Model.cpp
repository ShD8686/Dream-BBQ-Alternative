#include "Model.h"
#include <iostream>
#include <map>
#include <cfloat>
#include <algorithm>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

bool Model::LoadTexture(const std::string& path, bool pixelated, Texture& out)
{
    auto it = textureCache.find(path);
    if (it != textureCache.end())
    {
        out = it->second;
        return true;
    }
    Texture t;
    if (!t.Load(path, pixelated)) return false;
    textureCache[path] = t;
    out = t;
    return true;
}

bool Model::Load(const std::string& path, float scale, bool pixelated)
{
    std::string dir = path.substr(0, path.find_last_of("/\\") + 1);

    tinyobj::ObjReaderConfig cfg;
    cfg.triangulate = true;
    cfg.mtl_search_path = dir;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path, cfg))
    {
        std::cerr << "[Model] Ошибка загрузки " << path << ": " << reader.Error() << std::endl;
        return false;
    }
    if (!reader.Warning().empty())
        std::cout << "[Model] " << reader.Warning() << std::endl;

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

    auto getPos = [&](int vi) {
        return glm::vec3(attrib.vertices[3 * vi + 0],
            attrib.vertices[3 * vi + 1],
            attrib.vertices[3 * vi + 2]) * scale;
        };

    for (const auto& shape : shapes)
    {
        // --- объекты "col_*" = коллизия ---
        if (shape.name.rfind("col_", 0) == 0)
        {
            AABB box{ glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };
            for (const auto& idx : shape.mesh.indices)
            {
                glm::vec3 p = getPos(idx.vertex_index);
                box.min = glm::min(box.min, p);
                box.max = glm::max(box.max, p);
            }
            colliders.push_back(box);
            continue;
        }

        // --- обычная геометрия: делим по материалам ---
        struct Bucket { std::vector<float> v; std::vector<GLuint> i; };
        std::map<int, Bucket> buckets;

        size_t offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
        {
            int fv = shape.mesh.num_face_vertices[f]; // 3 после triangulate
            int matId = shape.mesh.material_ids[f];
            Bucket& b = buckets[matId];

            glm::vec3 p[3];
            for (int k = 0; k < 3; k++)
                p[k] = getPos(shape.mesh.indices[offset + k].vertex_index);

            glm::vec3 fn = glm::cross(p[1] - p[0], p[2] - p[0]);
            fn = (glm::length(fn) > 1e-8f) ? glm::normalize(fn) : glm::vec3(0, 1, 0);

            for (int k = 0; k < 3; k++)
            {
                const tinyobj::index_t& idx = shape.mesh.indices[offset + k];

                glm::vec3 n = fn;
                if (idx.normal_index >= 0)
                    n = glm::vec3(attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]);

                float u = 0, v = 0;
                if (idx.texcoord_index >= 0)
                {
                    u = attrib.texcoords[2 * idx.texcoord_index + 0];
                    v = attrib.texcoords[2 * idx.texcoord_index + 1];
                }

                b.i.push_back((GLuint)(b.v.size() / 8));
                b.v.insert(b.v.end(), { p[k].x, p[k].y, p[k].z, n.x, n.y, n.z, u, v });
            }
            offset += fv;
        }

        for (auto& kv : buckets)
        {
            int matId = kv.first;
            Bucket& b = kv.second;

            glm::vec3 color(0.8f);
            Texture tex;
            bool hasTex = false;

            if (matId >= 0 && matId < (int)materials.size())
            {
                const tinyobj::material_t& m = materials[matId];
                color = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);

                if (!m.diffuse_texname.empty())
                {
                    std::string texPath = m.diffuse_texname;
                    std::replace(texPath.begin(), texPath.end(), '\\', '/');
                    hasTex = LoadTexture(dir + texPath, pixelated, tex);
                }
            }

            Mesh mesh(b.v, b.i, color);
            if (hasTex) mesh.SetTexture(tex);
            meshes.push_back(mesh);
        }
    }

    std::cout << "[Model] " << path << ": мешей " << meshes.size()
        << ", коллайдеров " << colliders.size() << std::endl;
    return true;
}

void Model::Draw(Shader& shader, const glm::mat4& model)
{
    for (auto& m : meshes) m.Draw(shader, model);
}

void Model::Delete()
{
    for (auto& m : meshes) m.Delete(false);
    for (auto& kv : textureCache) kv.second.Delete();
    textureCache.clear();
    meshes.clear();
}
