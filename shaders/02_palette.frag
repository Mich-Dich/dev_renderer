
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

const float TWO_PI = 6.28318530718;


vec3 palette(float t) {

    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(0.5, 0.1, 0.5);
    vec3 d = vec3(0.5, 0.5, 0.9);
    return a + b * cos(TWO_PI * (c * t + d));
}

void main() {

    vec2 coord = gl_FragCoord.xy/u_resolution.xy;

    gl_FragColor = vec4(palette(coord.x), 1.0);
}
