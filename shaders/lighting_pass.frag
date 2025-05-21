#version 330 core
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

in vec2 vTexCoord;

void main() {
    // Simply output green for testing
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}