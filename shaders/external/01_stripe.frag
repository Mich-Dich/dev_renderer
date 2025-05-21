
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

void main() {

    vec2 coord = gl_FragCoord.xy/u_resolution.xy;

    float stripe_pos = 0.2;
    float stripe_width = 0.1;
    float stripe_edge_width = 0.02;
    float stripe = smoothstep(
            stripe_pos - (stripe_width * 0.5) - (stripe_edge_width * 0.5),
            stripe_pos - (stripe_width * 0.5) + (stripe_edge_width * 0.5),
            coord.y
        ) * 1.0 - step(stripe_pos + (stripe_width * 0.5), coord.y);

    vec3 color = mix(vec3(0), vec3(1), stripe);

    gl_FragColor = vec4(color, 1.0);
}
