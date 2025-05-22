#version 430 core
#extension GL_ARB_shader_storage_buffer_object : require

out vec4 FragColor;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};

layout(std140, binding = 0) buffer VertexBuffer {
    Vertex vertices[];
};

layout(std140, binding = 1) buffer IndexBuffer {
    uint indices[];
};


uniform uint numIndices;
uniform vec3 cameraPos;
uniform mat4 invProjection;
uniform mat4 invView;
uniform mat4 model;
uniform vec2 screenSize;

bool rayIntersectsTriangle(vec3 rayOrigin, vec3 rayDir, vec3 v0, vec3 v1, vec3 v2, out float t, out vec3 normal) {
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 pvec = cross(rayDir, edge2);
    float det = dot(edge1, pvec);
    if (abs(det) < 1e-6)
        return false;
    float invDet = 1.0 / det;
    vec3 tvec = rayOrigin - v0;
    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0)
        return false;
    vec3 qvec = cross(tvec, edge1);
    float v = dot(rayDir, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0)
        return false;
    t = dot(edge2, qvec) * invDet;
    if (t <= 1e-6)
        return false;
    normal = normalize(cross(edge1, edge2));
    return true;
}

void main() {
    vec2 screenPos = gl_FragCoord.xy;
    vec2 ndc = (screenPos / screenSize) * 2.0 - 1.0;
    ndc.y *= -1.0; // Flip Y-axis to match OpenGL's coordinate system

    vec4 clipRay = vec4(ndc, 1.0, 1.0);
    vec4 viewRay = invProjection * clipRay;
    viewRay /= viewRay.w;
    vec3 worldRayDir = normalize((invView * vec4(viewRay.xyz, 0.0)).xyz);

    vec3 rayOrigin = cameraPos;
    vec3 rayDir = worldRayDir;

    float closestT = 1e10;
    vec3 hitNormal = vec3(0.0);
    bool hit = false;

    for (uint i = 0; i < numIndices; i += 3) {
        uint i0 = indices[i];
        uint i1 = indices[i+1];
        uint i2 = indices[i+2];

        // Apply model matrix to vertices
        vec3 v0 = (model * vec4(vertices[i0].position, 1.0)).xyz;
        vec3 v1 = (model * vec4(vertices[i1].position, 1.0)).xyz;
        vec3 v2 = (model * vec4(vertices[i2].position, 1.0)).xyz;

        float t;
        vec3 normal;
        if (rayIntersectsTriangle(rayOrigin, rayDir, v0, v1, v2, t, normal)) {
            if (t < closestT) {
                closestT = t;
                hitNormal = normal;
                hit = true;
            }
        }
    }

    if (hit) {
        FragColor = vec4(hitNormal * 0.5 + 0.5, 1.0); // use the normal for debug
    } else {
        FragColor = vec4(0., .5, .5, 1.);
    }
}