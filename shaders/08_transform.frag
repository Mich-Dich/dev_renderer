
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

const float PI      = 3.14159265359;
const float TWO_PI  = 6.28318530718;

mat2 rotate(float angle) {

    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

void main() {

    vec2 coord = gl_FragCoord.xy/u_resolution.xy;
    coord -= .5;
    coord *= 1.1;

    for (int x = 0; x < 6; x++) {

        coord *= 2.2;
        coord = rotate(u_time) * coord;
        coord = abs(coord);
        coord -= vec2(.5, .5);
    }

    float result = smoothstep(.41, .4, length(coord));
    gl_FragColor = vec4(vec3(result), 1.);
}
