
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

float noise(float seed) {
    
    const float factor = 32345.12345;
    return fract(factor * sin(seed));
}

float noise(vec2 seed) { return fract(32143.654 * sin(dot(seed, vec2(15.358, 354.35)))); }


void main() {

    vec2 coord = gl_FragCoord.xy / u_resolution;
    coord.x *= u_resolution.x / u_resolution.y;


    // coord *= 5.;
    coord *= vec2(5., 10.);
    float row = step(1. ,mod(coord.y, 2.));
    coord.x += row * .5;

    vec2 index = floor(coord);
    coord = fract(coord);
    vec2 match = 1. - abs(index - vec2(2. ,3.));
    float is_target = step(.5, min(match.x, match.y));

    coord = mix(coord, rotate(u_time) * (coord - .5) + .5, is_target);
    
    float selected_brick = float(all(equal(index, vec2(2., 4.))));

    // coord *= step(rotate(.5) * coord, vec2(selected_brick));
    // float brick = step(coord.x, 2.) * step(4., coord.x);

    float box = smooth_box(coord, vec2(.95, .9), .01);
    vec3 color = vec3(0.632, 0.648, 0.636) * noise(gl_FragCoord.xy / u_resolution);
    gl_FragColor = vec4(color * box, 1.);
}
