#version 430
#ifdef GL_ES
precision mediump float;
#endif

uniform int u_bvh_viz_bounds_depth;
uniform int u_bvh_viz_triangle_depth;
uniform vec4 u_bvh_viz_color;

uniform mat4 u_inv_proj;
uniform mat4 u_inv_view;
uniform vec3 u_cam_pos;

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

out vec4 FragColor;

// ================================ define data ================================

const float EPSILON = 1e-4;

struct ray {
    vec3 origin;
    vec3 dir;
};

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
};

struct BVHNode {
    vec3 AABB_min;
    uint left_and_count;
    vec3 AABB_max;
    uint first_tri_index;
};

// ================================ get SSBOs ================================

layout(std430, binding = 0) buffer verticesBuffer {
    Vertex vertices[];
};

layout(std430, binding = 1) buffer indicesBuffer {
    uint indices[];
};

layout(std430, binding = 2) buffer bvhBuffer {
    BVHNode bvh_nodes[];
};

layout(std430, binding = 3) buffer triIdxBuffer {
    uint triIdx[];
};

// ================================ functions ================================

ray create_camera_ray(vec2 pixel_coord) {

    const vec2 uv = (pixel_coord / u_resolution) * 2.0 - 1.0;
    const vec4 ray_clip = vec4(uv.x, uv.y, -1.0, 1.0);
    vec4 ray_eye = u_inv_proj * ray_clip;
    ray_eye = vec4(ray_eye.xy, -1.0, 0.0);  // Forward direction
    const vec3 ray_world = normalize((u_inv_view * ray_eye).xyz);
    return ray(u_cam_pos, ray_world);
}

bool intersect_ray_sphere(ray r, vec3 center, float radius, out float t_out) {
    const vec3 oc = r.origin - center;
    const float b = dot(oc, r.dir);
    const float c = dot(oc, oc) - radius * radius;
    const float disc = b * b - c;
    if (disc < 0.0) return false;
    
    const float sqrt_disc = sqrt(disc);
    t_out = -b - sqrt_disc;
    return t_out > 0.0;
}

bool intersect_ray_triangle(ray r, vec3 v0, vec3 v1, vec3 v2, out float t, out float u, out float v) {
    const vec3 e1 = v1 - v0;
    const vec3 e2 = v2 - v0;
    const vec3 h = cross(r.dir, e2);
    const float a = dot(e1, h);
    
    if (abs(a) < EPSILON) return false;
    
    const float f = 1.0 / a;
    const vec3 s = r.origin - v0;
    u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;
    
    const vec3 q = cross(s, e1);
    v = f * dot(r.dir, q);
    if (v < 0.0 || u + v > 1.0) return false;
    
    t = f * dot(e2, q);
    return t > EPSILON;
}

bool intersectAABB(ray r, vec3 aabbMin, vec3 aabbMax, out float t_min, out float t_max) {
    vec3 invDir = 1.0 / r.dir;
    vec3 t0 = (aabbMin - r.origin) * invDir;
    vec3 t1 = (aabbMax - r.origin) * invDir;
    
    // Handle NaN when direction component is 0
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    
    t_min = max(max(tmin.x, tmin.y), tmin.z);
    t_max = min(min(tmax.x, tmax.y), tmax.z);
    
    return t_max >= max(t_min, 0.0);
}


struct HitInfo {
    float t;
    vec3 normal;
    bool hit;
    uint num_of_checked_bounds;
};

HitInfo traverseBVH(ray r) {

    HitInfo bestHit;
    bestHit.t = 1e30;
    bestHit.hit = false;
    bestHit.num_of_checked_bounds = 0;
    
    uint stack[32];
    int ptr = 0;
    stack[ptr++] = 0; // Start with root node
    while (ptr > 0) {
        
        // Check ray against AABB
        BVHNode node = bvh_nodes[stack[--ptr]];
        uint left_node = node.left_and_count & 0xFFFFu;
        uint tri_count = (node.left_and_count >> 16) & 0xFFFFu;

        bestHit.num_of_checked_bounds += 1;
        float t_min, t_max;
        if (!intersectAABB(r, node.AABB_min, node.AABB_max, t_min, t_max)) continue;
        if (t_min > bestHit.t) continue;

        if (tri_count > 0) { // Leaf node
            for (uint i = 0; i < tri_count; i++) {
                uint triIndex = triIdx[node.first_tri_index + i];
                uint idx0 = indices[triIndex * 3];
                uint idx1 = indices[triIndex * 3 + 1];
                uint idx2 = indices[triIndex * 3 + 2];
                
                Vertex v0 = vertices[idx0];
                Vertex v1 = vertices[idx1];
                Vertex v2 = vertices[idx2];
                
                float t, u, v;
                if (intersect_ray_triangle(r, v0.position, v1.position, v2.position, t, u, v)) {
                    if (t < bestHit.t && t > EPSILON) {
                        bestHit.t = t;
                        bestHit.normal = normalize((1.0 - u - v) * v0.normal + u * v1.normal + v * v2.normal);
                        bestHit.hit = true;
                    }
                }
            }
        } else { // Internal node
            stack[ptr++] = left_node + 1; // Right child
            stack[ptr++] = left_node;     // Left child
        }
    }
    return bestHit;
}

// ================================ main ================================

void main() {
    ray cam_ray = create_camera_ray(gl_FragCoord.xy);
    const vec3 light_source = normalize(u_cam_pos + vec3(1.0 + sin(u_time * 2.0), 1.0, -1.0));
    vec3 color = vec3(0.0);
    
    HitInfo hitInfo = traverseBVH(cam_ray);
    // vec3 viz_color = visualizeBVH(cam_ray, hitInfo.hit ? hitInfo.t : 1e30);
    
    if (hitInfo.hit) {
        // if (hitInfo.num_of_checked_bounds > 50)
        //     color = vec3(1.);
        // else
        //     color = vec3(float(hitInfo.num_of_checked_bounds) / float(50), .0, .0);
        float brightness = max(dot(light_source, hitInfo.normal), 0.0);
        color = vec3(0.5, 0.5, 0.8) * brightness; // Use mesh color or material
    } else {
        // Background color
        color = mix(vec3(0.2, 0.2, 0.3), vec3(0.1, 0.4, 0.9), max(0.0, cam_ray.dir.y));
    }
    
    // color = mix(color, viz_color, length(viz_color));
    FragColor = vec4(color, 1.0);
}
