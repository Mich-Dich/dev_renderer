#version 430
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

out vec4 FragColor;

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

// SSBOs for mesh data
layout(std430, binding = 0) buffer verticesBuffer {
    Vertex vertices[];
};

layout(std430, binding = 1) buffer indicesBuffer {
    uint indices[];
};

ray create_camera_ray(vec2 pixel_coord) {
    float aspect = u_resolution.x / u_resolution.y;
    vec2 uv = (2.0 * pixel_coord - u_resolution) / u_resolution.y;
    return ray(vec3(0., 0., -5.), normalize(vec3(uv, 1.0)));
}

bool intersect_ray_triangle(ray r, vec3 v0, vec3 v1, vec3 v2, out float t, out float u, out float v) {
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    vec3 h = cross(r.dir, e2);
    float a = dot(e1, h);
    
    if (abs(a) < EPSILON) return false;
    
    float f = 1.0 / a;
    vec3 s = r.origin - v0;
    u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;
    
    vec3 q = cross(s, e1);
    v = f * dot(r.dir, q);
    if (v < 0.0 || u + v > 1.0) return false;
    
    t = f * dot(e2, q);
    return t > EPSILON;
}

void main() {
    ray cam_ray = create_camera_ray(gl_FragCoord.xy);
    vec3 light_source = normalize(vec3(1.0 + sin(u_time * 2.0), 1.0, -1.0));
    vec3 color = vec3(0.0);
    
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
