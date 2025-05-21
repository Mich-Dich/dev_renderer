
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

void main() {

    vec2 coord = gl_FragCoord.xy/u_resolution.xy;

    vec2 square_pos = vec2(0.5, 0.5);
    vec2 square_size_half = vec2(0.1, 0.3);
    float square_edge_width_half = 0.01;

    float square = 
          step(square_pos.x - (square_size_half.x * 0.5), coord.x)
        * step(coord.x, square_pos.x + (square_size_half.x * 0.5))
        * step(square_pos.y - (square_size_half.y * 0.5), coord.y)
        * step(coord.y, square_pos.y + (square_size_half.y * 0.5));
    vec3 color = mix(vec3(0), vec3(0, 0.8, 0.2), square);

    vec3 glow_color = vec3(0, 0.3, 0.8);
    float glow_thickness = 0.1;
    float glow =
          smoothstep(square_pos.x - (square_size_half.x * 0.5) - glow_thickness, square_pos.x - (square_size_half.x * 0.5), coord.x)
        * (1.0 - smoothstep(square_pos.x + (square_size_half.x * 0.5), square_pos.x + (square_size_half.x * 0.5) + glow_thickness, coord.x))
        * smoothstep(square_pos.y - (square_size_half.y * 0.5) - glow_thickness, square_pos.y - (square_size_half.y * 0.5), coord.y)
        * (1.0 - smoothstep(square_pos.y + (square_size_half.y * 0.5), square_pos.y + (square_size_half.y * 0.5) + glow_thickness, coord.y))
        - square;
    glow = glow * (0.5 + sin(u_time * 3.) * 0.5);
    color = mix(color, glow_color, glow);

    float cursor = distance(coord, u_mouse/u_resolution.xy) * 10.;
    color.r = mix(color.r, 1.0, smoothstep(0.0, cursor, 0.3));
    color.g = mix(color.g, 1.0, smoothstep(0.0, cursor, 0.3));

    gl_FragColor = vec4(color, 1.0);
}
