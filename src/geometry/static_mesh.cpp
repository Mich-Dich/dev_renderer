#include "util/pch.h"

#include "util/timing/stopwatch.h"
#include "static_mesh.h"


namespace GLT::geometry {

    void static_mesh::build_BVH() {

        util::stopwatch loc_stopwatch = util::stopwatch(&BVH_build_time, duration_precision::microseconds);
        
        BVH_nodes.clear();
        triIdx.clear();

        const u32 numTriangles = indices.size() / 3;
        if (numTriangles == 0)
            return;

        std::vector<glm::vec3> centroids(numTriangles);
        for (u32 i = 0; i < numTriangles; ++i) {
            const u32 idx0 = indices[i * 3];
            const u32 idx1 = indices[i * 3 + 1];
            const u32 idx2 = indices[i * 3 + 2];

            const auto& v0 = vertices[idx0];
            const auto& v1 = vertices[idx1];
            const auto& v2 = vertices[idx2];

            centroids[i] = (v0.position + v1.position + v2.position) / 3.0f;
        }

        triIdx.resize(numTriangles);
        for (u32 i = 0; i < numTriangles; ++i)
            triIdx[i] = i;

        // Create root node
        BVH_node root;
        root.first_tri_index = 0;
        root.tri_count = numTriangles;
        root.left_node = 0; // No children initially
        BVH_nodes.push_back(root);
        update_node_bounds(0);

        BVH_nodes.reserve(2 * numTriangles - 1);
        subdivide(0, centroids);

        loc_stopwatch.stop();
        compute_bvh_stats();
    }


    void static_mesh::subdivide(u32 nodeIdx, const std::vector<glm::vec3>& centroids) {
            
        BVH_node& node = BVH_nodes[nodeIdx];
        if (node.tri_count <= 2) return;

        // Determine split axis (longest axis)
        glm::vec3 extent = node.AABB_max - node.AABB_min;
        int axis = 0;
        if (extent.y > extent.x) axis = 1;
        if (extent.z > extent[axis]) axis = 2;

        // Split position at center of the longest axis
        float splitPos = node.AABB_min[axis] + extent[axis] * 0.5f;

        // Partition triangles into left and right
        int i = static_cast<int>(node.first_tri_index); // Cast to int
        int j = i + static_cast<int>(node.tri_count) - 1; // Cast to int

        while (i <= j) {
            if (centroids[triIdx[i]][axis] < splitPos)
                ++i;
            else
                std::swap(triIdx[i], triIdx[j--]);
        }

        // Check for invalid splits
        int leftCount = i - static_cast<int>(node.first_tri_index); // Cast to int
        if (leftCount == 0 || leftCount == static_cast<int>(node.tri_count)) return;

        // Create child nodes
        u32 leftChildIdx = BVH_nodes.size();
        BVH_nodes.emplace_back(); // Left child
        BVH_nodes.emplace_back(); // Right child

        // Initialize children
        BVH_node& leftChild = BVH_nodes[leftChildIdx];
        BVH_node& rightChild = BVH_nodes[leftChildIdx + 1];

        leftChild.first_tri_index = node.first_tri_index;
        leftChild.tri_count = static_cast<u32>(leftCount); // Cast to u32
        leftChild.left_node = 0;

        rightChild.first_tri_index = static_cast<u32>(i); // Cast to u32
        rightChild.tri_count = node.tri_count - static_cast<u32>(leftCount); // Cast
        rightChild.left_node = 0;

        // Update parent node
        node.left_node = leftChildIdx;
        node.tri_count = 0; // Mark as internal node

        // Update child bounds
        update_node_bounds(leftChildIdx);
        update_node_bounds(leftChildIdx + 1);

        // Recurse
        subdivide(leftChildIdx, centroids);
        subdivide(leftChildIdx + 1, centroids);
    }

    
    void static_mesh::compute_bvh_stats() {
        
        u32 sum_triangles = 0;
        u32 current_max = 0;
        bvh_leaf_count = 0;

        // Calculate leaf count, sum of triangles, and max triangles in a leaf
        for (const auto& node : BVH_nodes) {
            if (node.is_leaf()) {
                bvh_leaf_count++;
                sum_triangles += node.tri_count;
                if (node.tri_count > current_max) {
                    current_max = node.tri_count;
                }
            }
        }

        max_triangles_count = current_max;
        average_triangles_count = (bvh_leaf_count == 0) ? 0 : (sum_triangles / bvh_leaf_count);

        // Calculate max depth of the BVH
        std::function<int(int)> compute_depth = [&](int nodeIdx) -> int {
            if (nodeIdx >= BVH_nodes.size())
                return 0;
            const BVH_node& node = BVH_nodes[nodeIdx];
            return node.is_leaf() ? 0 : 1 + std::max(
                compute_depth(node.left_node),
                compute_depth(node.left_node + 1)
            );
        };
        bvh_max_depth = compute_depth(0);
    }


    void static_mesh::update_node_bounds(u32 nodeIdx) {

        BVH_node& node = BVH_nodes[nodeIdx];
        node.AABB_min = glm::vec3(FLT_MAX);
        node.AABB_max = glm::vec3(-FLT_MAX);

        for (u32 i = 0; i < node.tri_count; ++i) {
            const u32 triIdxEntry = triIdx[node.first_tri_index + i];
            const u32 idx0 = indices[triIdxEntry * 3];
            const u32 idx1 = indices[triIdxEntry * 3 + 1];
            const u32 idx2 = indices[triIdxEntry * 3 + 2];

            const auto& v0 = vertices[idx0];
            const auto& v1 = vertices[idx1];
            const auto& v2 = vertices[idx2];

            // Expand AABB using each vertex
            node.AABB_min = glm::min(node.AABB_min, v0.position);
            node.AABB_min = glm::min(node.AABB_min, v1.position);
            node.AABB_min = glm::min(node.AABB_min, v2.position);

            node.AABB_max = glm::max(node.AABB_max, v0.position);
            node.AABB_max = glm::max(node.AABB_max, v1.position);
            node.AABB_max = glm::max(node.AABB_max, v2.position);
        }
    }

}
