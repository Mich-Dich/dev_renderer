
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

const float PI      = 3.14159265359;
const float TWO_PI  = 6.28318530718;

float time_0_1(float scale) { return scale * ((.5 * cos(u_time)) + .5); }

vec3 palette(float t) {
    vec3 a = vec3(0.5);
    vec3 b = vec3(0.5);
    vec3 c = vec3(1.0);
    vec3 d = vec3(0.75, 0.25, 0.50);
    return a + b * cos( TWO_PI * (c * t + d) );
}

void main() {

    vec2 uv = gl_FragCoord.xy/u_resolution.xy;
    uv -= .5;

    float cycle_count   = floor(u_time / TWO_PI);
    float num_of_petals = 20.;

    float r = length(uv);
    float theta = atan(uv.x, uv.y);
    theta += theta < 0.0 ? TWO_PI : 0.;

    float result = cos(20. * theta);
    result = 0.05 * smoothstep(-1., 1., result) + 0.4;
    result = step(result, r);

    vec3 display_color = vec3(0.1, .5, .2);
    vec3 color = mix(display_color, vec3(0.), result);
    gl_FragColor = vec4(color, 1.0);
}


// vec2 coord = gl_FragCoord.xy/u_resolution.xy;
// coord -= 0.5;
// float r = length(coord) * (1. / abs((sin(u_time) * .3)));
// float theta = atan(coord.x, coord.y) + (u_time - cos(u_time));

// float num_of_petals = 4.;

// float value = cos(num_of_petals * theta);
// float result = smoothstep(value, value + 0.1, r);
// vec3 color = mix(vec3(0, 0.4, 0.6), vec3(0), result);
// gl_FragColor = vec4(color, 1.0);
