
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

float plot(float func, float y) {

    return (distance(func, y) < 0.01) ? 1. : 0.;
}

void main() {
    vec2 st = gl_FragCoord.xy/u_resolution.xy;
    st.x *= u_resolution.x/u_resolution.y;
    float x = st.x;

    vec3 color = vec3(0);
    color += mix(color, vec3(0, 0, 1), plot(x, st.y));
    color += mix(color, vec3(0, 1, 0), plot(sin(x), st.y));
    color += mix(color, vec3(1, 0, 0), plot(.5 + .5 * sin(10. * x), st.y));

    gl_FragColor = vec4(color, 1.);
}