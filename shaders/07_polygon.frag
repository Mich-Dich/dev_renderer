#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

float poligon(float N, float theta, float r, float smoothness) {
    float fact = TWO_PI / N;
    float result = cos(theta - floor(.5 + theta / fact) * fact) * r;
    return smoothstep(0.4, 0.4 + smoothness, result + 0.2);
}

vec3 shaded_poligon(vec2 offset, float size, vec2 uv, vec3 display_color) {

    vec2 m = (u_mouse / u_resolution) - 0.5;
    m.x *= u_resolution.x / u_resolution.y;
    float mouse_dist = length(m - offset);
    
    float poli_count = floor(mix(7.0, 3.0, smoothstep(0.0, 0.5, mouse_dist)));
    
    vec2 mouse_dir = normalize(m - offset + 0.0001);
    const float MAX_BEND = 0.15;
    float bend = pow(mouse_dist, 1.5) * MAX_BEND;
    
    vec2 loc_uv = uv - offset;
    loc_uv += mouse_dir * bend * length(uv);
    float r = length(loc_uv) * size;
    float theta = atan(loc_uv.y, loc_uv.x) + u_time;
    theta = mod(theta + TWO_PI, TWO_PI);
    float shadow_result = poligon(poli_count, theta, r, 0.015);
    
    loc_uv = uv - offset;
    r = length(loc_uv) * size;
    theta = atan(loc_uv.y, loc_uv.x) + u_time;
    theta = mod(theta + TWO_PI, TWO_PI);
    float poli_result = poligon(poli_count, theta, r, 0.005);
    
    vec3 loc_color = mix(vec3(0.0), vec3(0.13), shadow_result);
    return mix(display_color, loc_color, poli_result);
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution.xy - 0.5;
    uv.x *= u_resolution.x / u_resolution.y;
    
    vec3 color = shaded_poligon(vec2(0.4, 0.0), 3.2, uv, vec3(0.1, 0.3, 0.1));
    color += shaded_poligon(vec2(0.1, 0.2), 3.2, uv, vec3(0.7, 0.6, 0.2));
    color += shaded_poligon(vec2(-0.4, -0.3), 3.2, uv, vec3(0.1, 0.3, 0.8));
    color /= 2.0;
    
    gl_FragColor = vec4(color, 1.0);
}