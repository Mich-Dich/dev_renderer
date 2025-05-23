#version 430
#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 u_inv_proj;
uniform mat4 u_inv_view;
uniform vec3 u_cam_pos;

uniform vec3 u_mesh_center;
uniform float u_mesh_radius;

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

// ================================ get SSBOs ================================

layout(std430, binding = 0) buffer verticesBuffer {
    Vertex vertices[];
};

layout(std430, binding = 1) buffer indicesBuffer {
    uint indices[];
};

// ================================ functions ================================

ray create_camera_ray(vec2 pixel_coord) {
    // Convert pixel coordinates to normalized device coordinates (NDC)
    vec2 uv = (pixel_coord / u_resolution) * 2.0 - 1.0;
    
    // Create clip space coordinates (reverse Z for OpenGL NDC)
    vec4 ray_clip = vec4(uv.x, uv.y, -1.0, 1.0);
    
    // Transform to eye space using inverse projection
    vec4 ray_eye = u_inv_proj * ray_clip;
    ray_eye = vec4(ray_eye.xy, -1.0, 0.0);  // Forward direction
    
    // Transform to world space using inverse view
    vec3 ray_world = normalize((u_inv_view * ray_eye).xyz);
    
    return ray(u_cam_pos, ray_world);
}

#if 0

bool intersect_ray_sphere(ray r, vec3 center, float radius) {
    vec3 oc = r.origin - center;
    float a = dot(r.dir, r.dir);
    float b = 2.0 * dot(oc, r.dir);
    float c = dot(oc, oc) - radius * radius;
    float disc = b * b - 4.0 * a * c;
    
    if (disc < 0.0) return false;
    
    float sqrt_disc = sqrt(disc);
    float t0 = (-b - sqrt_disc) / (2.0 * a);
    float t1 = (-b + sqrt_disc) / (2.0 * a);
    
    float t = min(t0, t1);
    if (t < 0.0) t = max(t0, t1);
    
    return t > 0.0;
}
#else

bool intersect_ray_sphere(ray r, vec3 center, float radius) {
    vec3 oc = r.origin - center;
    float b = dot(oc, r.dir);
    float c = dot(oc, oc) - radius * radius;
    float disc = b * b - c;
    
    if (disc < 0.0) return false;
    float sqrt_disc = sqrt(disc);
    float t = -b - sqrt_disc;
    return t > 0.0;
}
#endif

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


// ================================ main ================================

void main() {
    ray cam_ray = create_camera_ray(gl_FragCoord.xy);
    vec3 light_source = normalize(u_cam_pos + vec3(1.0 + sin(u_time * 2.0), 1.0, -1.0));
    vec3 color = vec3(0.0);
    
    if (!intersect_ray_sphere(cam_ray, u_mesh_center, u_mesh_radius)) {
        color = mix(vec3(0.2, 0.2, 0.3), vec3(0.1, 0.4, 0.9), max(0.0, cam_ray.dir.y));
        FragColor = vec4(color, 1.0);
        return;
    }

    float t_min = 1e4;
    vec3 hit_normal = vec3(0.0);
    bool hit = false;
    
    uint triangle_count = indices.length() / 3;
    for (uint i = 0u; i < triangle_count; i++) {
        uint idx = i * 3;
        Vertex v0 = vertices[indices[idx]];
        Vertex v1 = vertices[indices[idx + 1]];
        Vertex v2 = vertices[indices[idx + 2]];
        
        float t, u, v_bary;
        if (intersect_ray_triangle(cam_ray, v0.position, v1.position, v2.position, t, u, v_bary)) {
            if (t < t_min && t > EPSILON) {
                t_min = t;
                hit_normal = normalize((1.0 - u - v_bary) * v0.normal + u * v1.normal + v_bary * v2.normal);
                hit = true;
            }
        }
    }
    
    if (hit) {
        float brightness = max(dot(light_source, hit_normal), 0.0);
        color = vec3(0.5, 0.5, 0.8) * brightness; // Use mesh color or material
    } else {
        color = mix(vec3(0.2, 0.2, 0.3), vec3(0.1, 0.4, 0.9), max(0.0, cam_ray.dir.y));
    }
    
    FragColor = vec4(color, 1.0);
}
