#include "util/pch.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define MESHOPTIMIZER_EXPERIMENTAL
#include <meshoptimizer.h>

#include "util/timing/stopwatch.h"

#include "asset_importer.h"


namespace GLT::factory::geometry {

    void optimize_static_mesh(ref<GLT::geometry::static_mesh> mesh) {

        f32 time = 0;
        util::stopwatch loc_stopwatch = util::stopwatch(&time, duration_precision::microseconds);
        
        // generate vertex remap
        std::vector<unsigned int> remap(mesh->vertices.size());
        size_t vertex_count = meshopt_generateVertexRemap(
            remap.data(),
            mesh->indices.data(),
            mesh->indices.size(),
            mesh->vertices.data(),
            mesh->vertices.size(),
            sizeof(GLT::geometry::vertex)
        );

        // allocate remapped data
        std::vector<GLT::geometry::vertex> vertices(vertex_count);
        std::vector<unsigned int> indices(mesh->indices.size());
        
        // remap indices and vertices
        meshopt_remapIndexBuffer(indices.data(), mesh->indices.data(), mesh->indices.size(), remap.data());
        meshopt_remapVertexBuffer(
            vertices.data(),
            mesh->vertices.data(),
            mesh->vertices.size(),
            sizeof(GLT::geometry::vertex),
            remap.data()
        );

        // optimize vertex cache
        meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());

        // optimize overdraw
        meshopt_optimizeOverdraw(
            indices.data(), 
            indices.data(), 
            indices.size(),
            &vertices[0].position.x,
            vertices.size(),
            sizeof(GLT::geometry::vertex),
            1.05f
        );

        // optimize vertex fetch
        meshopt_optimizeVertexFetch(
            vertices.data(),
            indices.data(),
            indices.size(),
            vertices.data(),
            vertices.size(),
            sizeof(GLT::geometry::vertex)
        );

        loc_stopwatch.stop();
        LOG(Debug, "Function took: [" << time << "]")
    }


    bool load_static_mesh(const std::filesystem::path& file_path, ref<GLT::geometry::static_mesh> out_mesh) {
        f32 import_time = 0.f;
        util::stopwatch import_time_stopwatch = util::stopwatch(&import_time, duration_precision::microseconds);
        
        out_mesh->vertices.clear();
        out_mesh->indices.clear();

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(file_path.string(),
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_ImproveCacheLocality |
            aiProcess_SortByPType |
            aiProcess_GenUVCoords |
            aiProcess_FlipUVs);

        if (!scene || !scene->HasMeshes()) {
            std::cerr << "[Assimp] Failed to load mesh: " << importer.GetErrorString() << std::endl;
            return false;
        }

        // Loop through all meshes in the scene
        for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
            aiMesh* ai_mesh = scene->mMeshes[m];
            const uint32_t vertex_offset = static_cast<uint32_t>(out_mesh->vertices.size()); // Track offset for indices

            // Process vertices
            out_mesh->vertices.reserve(out_mesh->vertices.size() + ai_mesh->mNumVertices);
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

            // Process indices (adjust for vertex offset)
            for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
                const aiFace& face = ai_mesh->mFaces[i];
                if (face.mNumIndices == 3) {
                    out_mesh->indices.push_back(static_cast<uint32_t>(face.mIndices[0]) + vertex_offset);
                    out_mesh->indices.push_back(static_cast<uint32_t>(face.mIndices[1]) + vertex_offset);
                    out_mesh->indices.push_back(static_cast<uint32_t>(face.mIndices[2]) + vertex_offset);
                }
            }
        }

        optimize_static_mesh(out_mesh);
        import_time_stopwatch.stop();
        LOG(Debug, "Import time: [" << import_time << "]")
        
        return true;
    }

}