#include "util/pch.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "asset_importer.h"


namespace GLT::factory::mesh {

    bool load_mesh(const std::filesystem::path& file_path, ref<GLT::mesh::static_mesh> out_mesh) {
    
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(file_path.string(),
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality |
            aiProcess_SortByPType |
            aiProcess_RemoveRedundantMaterials |
            aiProcess_OptimizeMeshes |
            aiProcess_OptimizeGraph |
            aiProcess_ValidateDataStructure);

        if (!scene || !scene->HasMeshes()) {
            std::cerr << "[Assimp] Failed to load mesh: " << importer.GetErrorString() << std::endl;
            return false;
        }

        aiMesh* ai_mesh = scene->mMeshes[0]; // Only load the first mesh

        out_mesh->vertices.reserve(ai_mesh->mNumVertices);
        for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i) {

            GLT::mesh::vertex v{};
            v.position[0] = ai_mesh->mVertices[i].x;
            v.position[1] = ai_mesh->mVertices[i].y;
            v.position[2] = ai_mesh->mVertices[i].z;

            if (ai_mesh->HasNormals()) {
                v.normal[0] = ai_mesh->mNormals[i].x;
                v.normal[1] = ai_mesh->mNormals[i].y;
                v.normal[2] = ai_mesh->mNormals[i].z;
            }

            if (ai_mesh->HasTextureCoords(0)) {
                v.texcoord[0] = ai_mesh->mTextureCoords[0][i].x;
                v.texcoord[1] = ai_mesh->mTextureCoords[0][i].y;
            } else {
                v.texcoord[0] = v.texcoord[1] = 0.0f;
            }

            out_mesh->vertices.push_back(v);
        }

        for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
            const aiFace& face = ai_mesh->mFaces[i];
            if (face.mNumIndices == 3) { // Ensure it's a triangle
                out_mesh->indices.push_back(face.mIndices[0]);
                out_mesh->indices.push_back(face.mIndices[1]);
                out_mesh->indices.push_back(face.mIndices[2]);
            }
        }

        return true;
    }

}
