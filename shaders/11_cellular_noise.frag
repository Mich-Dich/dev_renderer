
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

vec2 hash(vec2 seed) {
    vec3 p3 = fract(seed.xyx * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx + p3.yz) * p3.zy);
}

float smin(float a, float b, float k) {

    float h = clamp(.5 + .5 * (b - a) / k, 0., 1.);
    return mix(b, a, h) - k * h * (1. -h);
}

void main() {

    vec2 coord = gl_FragCoord.xy / u_resolution;
    coord.x *= u_resolution.x / u_resolution.y;

    coord *= 20.;
    vec2 cell = floor(coord);
    vec2 cell_fract = fract(coord);

    float min_dist = 1e4;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {

            vec2 delta = vec2(x, y);
            vec2 point = hash(cell + delta);
            point = .5 +.5 * sin(u_time + (u_time + 10.) * point);
            min_dist = smin(min_dist, distance(cell_fract, point + delta), .21);
        }
    }

    vec3 color = vec3(min_dist);
    gl_FragColor = vec4(color, 1.);
}
