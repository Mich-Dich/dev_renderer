#include "util/pch.h"

#include "util/timing/stopwatch.h"
#include "static_mesh.h"


namespace GLT::geometry {

    struct TriAABB {
        glm::vec3 min, max;
    };
    std::vector<TriAABB> triAABBs; // Cache triangle bounds


    void static_mesh::build_BVH(const u32 target_tri_count) {

        util::stopwatch loc_stopwatch = util::stopwatch(&BVH_build_time, duration_precision::microseconds);
        
        // Precompute triangle AABBs and centroids
        const u32 triCount = static_cast<u32>(indices.size() / 3);
        triAABBs.resize(triCount);
        std::vector<glm::vec3> centroids(triCount);
        
        for (u32 i = 0; i < triCount; ++i) {
            const u32 idx = i * 3;
            const auto& v0 = vertices[indices[idx]];
            const auto& v1 = vertices[indices[idx + 1]];
            const auto& v2 = vertices[indices[idx + 2]];
            
            triAABBs[i].min = glm::min(v0.position, glm::min(v1.position, v2.position));
            triAABBs[i].max = glm::max(v0.position, glm::max(v1.position, v2.position));
            centroids[i] = (v0.position + v1.position + v2.position) / 3.0f;
        }

        // Initialize BVH root node
        BVH_nodes.clear();
        BVH_nodes.emplace_back();
        BVH_node& root = BVH_nodes.back();
        root.first_tri_index = 0;
        root.tri_count = static_cast<u32>(indices.size() / 3);
        triIdx.resize(root.tri_count);
        for (u32 i = 0; i < root.tri_count; ++i) {
            triIdx[i] = i;
        }
        update_node_bounds(0);
        subdivide(0, target_tri_count, centroids);
        loc_stopwatch.stop();

    #ifdef DEBUG
        compute_bvh_stats();
    #endif
    }


    float static_mesh::evaluateSAH(const BVH_node& node, int axis, float pos, const std::vector<glm::vec3>& centroids) {

        glm::vec3 leftMin(FLT_MAX), leftMax(-FLT_MAX);
        glm::vec3 rightMin(FLT_MAX), rightMax(-FLT_MAX);
        int leftCount = 0, rightCount = 0;

        for (u32 i = 0; i < node.tri_count; ++i) {
            const u32 triIdxGlobal = triIdx[node.first_tri_index + i];
            const bool left = centroids[triIdxGlobal][axis] < pos;
            
            if (left) {
                leftMin = glm::min(leftMin, triAABBs[triIdxGlobal].min);
                leftMax = glm::max(leftMax, triAABBs[triIdxGlobal].max);
                leftCount++;
            } else {
                rightMin = glm::min(rightMin, triAABBs[triIdxGlobal].min);
                rightMax = glm::max(rightMax, triAABBs[triIdxGlobal].max);
                rightCount++;
            }
        }

        if (leftCount == 0 || rightCount == 0)
            return FLT_MAX;

        const glm::vec3 leftExtent = leftMax - leftMin;
        const float leftArea = leftExtent.x * leftExtent.y + leftExtent.y * leftExtent.z + leftExtent.z * leftExtent.x;
        
        const glm::vec3 rightExtent = rightMax - rightMin;
        const float rightArea = rightExtent.x * rightExtent.y + rightExtent.y * rightExtent.z + rightExtent.z * leftExtent.x;

        return leftCount * leftArea + rightCount * rightArea;
    }


    static float CalculateAABBArea(const glm::vec3& min, const glm::vec3& max) {
        glm::vec3 extent = max - min;
        return extent.x * extent.y + extent.y * extent.z + extent.z * extent.x;
    }


    void static_mesh::subdivide(u32 nodeIdx, const u32 target_tri_count, const std::vector<glm::vec3>& centroids) {
        
        BVH_node& node = BVH_nodes[nodeIdx];
        if (node.tri_count <= target_tri_count)
            return;

        // Determine parent cost (cost of not splitting)
        glm::vec3 e = node.AABB_max - node.AABB_min;
        float parentArea = e.x * e.y + e.y * e.z + e.z * e.x;
        float parentCost = node.tri_count * parentArea;

        // Binned SAH evaluation
        constexpr int BINS = 8;
        struct Bin {
            glm::vec3 min{FLT_MAX}, max{-FLT_MAX};
            int count = 0;
        };
        
        float bestCost = FLT_MAX;
        int bestAxis = -1;
        float bestSplit = 0;
        glm::vec3 nodeSize = node.AABB_max - node.AABB_min;

        for (int axis = 0; axis < 3; ++axis) {
            if (nodeSize[axis] < 1e-5f) continue; // Skip degenerate axis
            
            // Initialize bins
            std::vector<Bin> bins(BINS);
            const float scale = BINS / nodeSize[axis];
            
            // Populate bins
            for (u32 i = 0; i < node.tri_count; ++i) {
                const u32 triIdxGlobal = triIdx[node.first_tri_index + i];
                const float centroid = centroids[triIdxGlobal][axis];
                const int binIdx = glm::clamp<int>((centroid - node.AABB_min[axis]) * scale, 0, BINS-1);
                
                Bin& bin = bins[binIdx];
                bin.min = glm::min(bin.min, triAABBs[triIdxGlobal].min);
                bin.max = glm::max(bin.max, triAABBs[triIdxGlobal].max);
                bin.count++;
            }

            // Evaluate splits between bins
            for (int split = 1; split < BINS; ++split) {
                glm::vec3 leftMin(FLT_MAX), leftMax(-FLT_MAX);
                glm::vec3 rightMin(FLT_MAX), rightMax(-FLT_MAX);
                int leftCount = 0, rightCount = 0;
                
                for (int i = 0; i < split; ++i) {
                    if (bins[i].count == 0) continue;
                    leftMin = glm::min(leftMin, bins[i].min);
                    leftMax = glm::max(leftMax, bins[i].max);
                    leftCount += bins[i].count;
                }
                
                for (int i = split; i < BINS; ++i) {
                    if (bins[i].count == 0) continue;
                    rightMin = glm::min(rightMin, bins[i].min);
                    rightMax = glm::max(rightMax, bins[i].max);
                    rightCount += bins[i].count;
                }
                
                if (leftCount == 0 || rightCount == 0) continue;
                
                // Calculate SAH cost
                const float leftArea = CalculateAABBArea(leftMin, leftMax);
                const float rightArea = CalculateAABBArea(rightMin, rightMax);
                const float cost = leftCount * leftArea + rightCount * rightArea;
                
                if (cost < bestCost) {
                    bestCost = cost;
                    bestAxis = axis;
                    bestSplit = node.AABB_min[axis] + split * (nodeSize[axis] / BINS);
                }
            }
        }

        // Fallback to median split if SAH failed
        if (bestAxis == -1) {
            bestAxis = nodeSize.x > nodeSize.y ? 
                    (nodeSize.x > nodeSize.z ? 0 : 2) : 
                    (nodeSize.y > nodeSize.z ? 1 : 2);
            std::nth_element(
                triIdx.begin() + node.first_tri_index,
                triIdx.begin() + node.first_tri_index + node.tri_count/2,
                triIdx.begin() + node.first_tri_index + node.tri_count,
                [&](uint a, uint b) { return centroids[a][bestAxis] < centroids[b][bestAxis]; }
            );
            bestSplit = centroids[triIdx[node.first_tri_index + node.tri_count/2]][bestAxis];
        }

        // Partition the triangles based on the best split
        u16 first = node.first_tri_index;
        u16 splitIndex = first;
        for (u32 i = first; i < first + node.tri_count; ++i) {
            u32 triIdxGlobal = triIdx[i];
            if (centroids[triIdxGlobal][bestAxis] < bestSplit) {
                std::swap(triIdx[i], triIdx[splitIndex]);
                splitIndex++;
            }
        }

        u32 leftCount = splitIndex - first;
        u32 rightCount = node.tri_count - leftCount;
        if (leftCount == 0 || rightCount == 0)                                      // Can't split, force leaf
            return;
        
        const u32 MIN_TRIS_PER_CHILD = 2;
        if (leftCount < MIN_TRIS_PER_CHILD || rightCount < MIN_TRIS_PER_CHILD)      // Prevent creating nodes with very small triangle counts
            return;

        // Create child nodes
        u16 leftChildIdx = static_cast<u32>(BVH_nodes.size());
        node.left_node = leftChildIdx;
        node.tri_count = 0; // Internal node
        BVH_nodes.resize(BVH_nodes.size() + 2);

        BVH_node& left = BVH_nodes[leftChildIdx];
        BVH_node& right = BVH_nodes[leftChildIdx + 1];

        left.first_tri_index = first;
        left.tri_count = leftCount;
        right.first_tri_index = splitIndex;
        right.tri_count = rightCount;

        update_node_bounds(leftChildIdx);
        update_node_bounds(leftChildIdx + 1);

        // Only recurse if children are worth splitting
        if (leftCount > target_tri_count) subdivide(leftChildIdx, target_tri_count, centroids);
        if (rightCount > target_tri_count) subdivide(leftChildIdx + 1, target_tri_count, centroids);
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
        std::function<int(u64)> compute_depth = [&](u64 nodeIdx) -> int {
            if (nodeIdx >= BVH_nodes.size())
                return 0;
            const BVH_node& node = BVH_nodes[nodeIdx];
            return node.is_leaf() ? 0 : 1 + std::max(
                compute_depth(static_cast<u64>(node.left_node)),
                compute_depth(static_cast<u64>(node.left_node + 1))
            );
        };
        bvh_max_depth = compute_depth(0);
    }

        
    void static_mesh::update_node_bounds(u32 nodeIdx) {
        BVH_node& node = BVH_nodes[nodeIdx];
        node.AABB_min = glm::vec3(FLT_MAX);
        node.AABB_max = glm::vec3(-FLT_MAX);

        for (u32 i = 0; i < node.tri_count; ++i) {
            u32 triIdxGlobal = triIdx[node.first_tri_index + i];
            const auto& v0 = vertices[indices[triIdxGlobal * 3]];
            const auto& v1 = vertices[indices[triIdxGlobal * 3 + 1]];
            const auto& v2 = vertices[indices[triIdxGlobal * 3 + 2]];

            node.AABB_min = glm::min(node.AABB_min, v0.position);
            node.AABB_min = glm::min(node.AABB_min, v1.position);
            node.AABB_min = glm::min(node.AABB_min, v2.position);

            node.AABB_max = glm::max(node.AABB_max, v0.position);
            node.AABB_max = glm::max(node.AABB_max, v1.position);
            node.AABB_max = glm::max(node.AABB_max, v2.position);
        }
    }

}
