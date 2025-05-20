
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

vec2 rotate_uv(vec2 uv, vec2 pivot, float angle) {

    float c = cos(angle);
    float s = sin(angle);
    vec2 p = uv - pivot;
    return vec2(
        p.x * c - p.y * s,
        p.x * s + p.y * c
    ) + pivot;
}

void main() {

    vec2 UV         = gl_FragCoord.xy / u_resolution;
    float aspect    = u_resolution.x / u_resolution.y;
    vec2 pos        = (UV - .5) * vec2(aspect, 1.);

    vec2 center     = vec2(0.3, 0.0);
    vec2 halfSize   = vec2(0.1 * aspect, 0.3);
    float glowWidth = 0.05;
    float roation_speed = 2.;
    float angle = u_time * roation_speed;
    vec2 pivot_below = vec2(0,  0.7);
    vec2 pivot_above = vec2(0, -0.7);
    vec2 rotation_pivot = (cos(angle) > 0.) ? pivot_above : pivot_below;
    pos = rotate_uv(pos, rotation_pivot, (cos(angle) > 0.) ? angle : angle + 3.1415);

    vec2 d = abs(pos) - halfSize;
    float dist = length(max(d, .0)) + min(max(d.x, d.y), .0);
    float glow = smoothstep(glowWidth, 0.0, dist);

    vec3 coreColor = vec3(0.0, 0.8, 0.2);
    vec3 glowColor = vec3(0.0, 0.3, 0.8);
    float inside = smoothstep(0.0, -0.001, dist);

    glow = glow * (0.5 + sin(u_time * 3.) * 0.5);
    vec3 color = mix(glowColor * glow, coreColor, inside);

    gl_FragColor = vec4(color, 1.0);
}
