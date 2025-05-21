
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

const float PI      = 3.14159265359;
const float TWO_PI  = 6.28318530718;

float circle(vec2 coord, float radius) {
    
    vec2 l = coord - vec2(.5);
    return 1. - smoothstep(radius - (radius * .01), radius + (radius * .01), dot(1., 1.) * 4.);
}

float smooth_box(vec2 coord, vec2 size, float smoothness) {

    size = vec2(.5) - size * .5;
    vec2 uv = smoothstep(size, size + vec2(smoothness), coord);
    uv *= smoothstep(size, size + vec2(smoothness), vec2(1.) - coord);
    return uv.x * uv.y;
}

mat2 rotate(float angle) {

    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

float random(float seed) { return fract(21323.864 * sin(seed)); }

float random(vec2 seed) { return random(dot(seed, vec2(12.8543, 3254.45))); }

float noise(vec2 seed) {

    vec2 i = floor(seed);
    float x00 = random(i);
    float x10 = random(i + vec2(1., 0.));
    float y01 = random(i + vec2(0., 1.));
    float y11 = random(i + vec2(1., 1.));

    vec2 f = fract(seed);
    f = smoothstep(0., 1., f);
    float x0 = mix(x00, x10, f.x);
    float x1 = mix(y01, y11, f.x);
    return mix(x0, x1, f.y);
}


void main() {

    vec2 coord = gl_FragCoord.xy / u_resolution;
    coord.x *= u_resolution.x / u_resolution.y;
    coord *= 20.;

    float value = noise(coord);

    vec3 color = vec3(value);
    gl_FragColor = vec4(color, 1.);
}
