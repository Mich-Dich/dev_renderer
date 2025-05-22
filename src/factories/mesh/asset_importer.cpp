#include "util/pch.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "asset_importer.h"


namespace GLT::factory::geometry {

    bool load_mesh(const std::filesystem::path& file_path, ref<GLT::geometry::static_mesh> out_mesh) {
    
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(file_path.string(),
            aiProcess_Triangulate |
            aiProcess_GenNormals |          // Keep this to generate normals if missing
            aiProcess_CalcTangentSpace |    // Optional: Only if you need tangents
            aiProcess_ImproveCacheLocality | // Keep for optimization
            aiProcess_SortByPType |         // Keep to separate primitive types
            aiProcess_GenUVCoords |         // Add to ensure proper UV handling
            aiProcess_FlipUVs);             // Add if textures appear flipped

        if (!scene || !scene->HasMeshes()) {
            std::cerr << "[Assimp] Failed to load mesh: " << importer.GetErrorString() << std::endl;
            return false;
        }

        aiMesh* ai_mesh = scene->mMeshes[0]; // Only load the first mesh

        out_mesh->vertices.reserve(ai_mesh->mNumVertices);
        for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i) {

            GLT::geometry::vertex v{};
            v.position[0] = ai_mesh->mVertices[i].x;
            v.position[1] = ai_mesh->mVertices[i].y;
            v.position[2] = ai_mesh->mVertices[i].z;

            if (ai_mesh->HasNormals()) {
                v.normal[0] = ai_mesh->mNormals[i].x;
                v.normal[1] = ai_mesh->mNormals[i].y;
                v.normal[2] = ai_mesh->mNormals[i].z;
            }

            if (ai_mesh->HasTextureCoords(0)) {
                v.uv_x = ai_mesh->mTextureCoords[0][i].x;
                v.uv_y = ai_mesh->mTextureCoords[0][i].y;
            } else {
                v.uv_x = v.uv_y = 0.0f;
            }

            out_mesh->vertices.push_back(v);
        }

        for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
            const aiFace& face = ai_mesh->mFaces[i];
            if (face.mNumIndices == 3) { // Ensure it's a triangle
                out_mesh->indices.push_back(static_cast<uint32_t>(face.mIndices[0]));
                out_mesh->indices.push_back(static_cast<uint32_t>(face.mIndices[1]));
                out_mesh->indices.push_back(static_cast<uint32_t>(face.mIndices[2]));
            }
        }

        return true;
    }

}