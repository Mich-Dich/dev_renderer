
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

const float EPSILON = 1e-4;
const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

// ------------- RAY TRACING -------------

const int SPHERE_COUNT = 5;
const int MAX_BOUNCES  = 4;

struct ray {
    vec3 origin;
    vec3 dir;
};

struct sphere {
    vec3 center;
    vec3 color;
    float radius;
    float metallic;
};

sphere spheres[SPHERE_COUNT];

struct ray_intersection {
    int hit_index;
    float t_min;
};

ray create_camera_ray(vec2 pixel_coord) {

    float fx = tan(radians(90.) / 2.) / u_resolution.x;
    vec2 d = (2. * pixel_coord - u_resolution) * fx;
    return ray(vec3(0.), normalize(vec3(d, 1.)));
}

float intersect_ray_sphere(int index, ray cam_ray) {

    vec3 oc = cam_ray.origin - spheres[index].center;
    float dot_dir_OC = dot(oc, cam_ray.dir);
    float root = dot_dir_OC * dot_dir_OC - (dot(oc, oc) - spheres[index].radius * spheres[index].radius);
    if (root < EPSILON)
        return -1.;

    float p = -dot_dir_OC;
    float q = sqrt(root);
    return (p-q) > .0 ? p-q : p+q;
}

vec3 sphere_normal(int index, vec3 point) {
    
    return normalize(point - spheres[index].center);
}

ray_intersection trace_diffuse(ray ray) {
    
    float t_min = 1e4;
    int hit_index  = -1;
    for (int i = 0; i < SPHERE_COUNT; i++) {
        float t = intersect_ray_sphere(i, ray);
        if (t > EPSILON && t < t_min) {
            t_min = t;
            hit_index  = i;
        }
    }
    if (hit_index < 0)
        return ray_intersection(-1, 1e4);

    vec3 P = ray.origin + ray.dir * t_min;
    vec3 N = sphere_normal(hit_index, P);
    vec3 L = normalize(vec3(1.0, 1.0, -1.0));
    float diff = max(dot(L, N), 0.0);
    return ray_intersection(hit_index, t_min);
}

// ------------- NORMAL STUFF -------------

vec3 palette(float t) {

    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(0.5, 0.1, 0.5);
    vec3 d = vec3(0.5, 0.5, 0.9);
    return a + b * cos(TWO_PI * (c * t + d));
}

// -------------  -------------

void main() {

    spheres[0] = sphere(vec3( 2.0, 1.0 + 0.5 * sin(u_time * 2.0), 5.0),                             vec3(1.0, 0.2, 0.2),    1.0,    .8);
    spheres[1] = sphere(vec3( 1.5 * cos(u_time * 1.5) + 1.5, 0.5, 4.0 + 0.3 * sin(u_time * 1.5)),   palette(u_time),        0.8,    .0);
    spheres[2] = sphere(vec3(-1.0, 0.7 + 0.7 * cos(u_time), 6.0),                                   palette(u_time + 1.),   1.2,    .1);
    spheres[3] = sphere(vec3( 0.0, -1001.0, 5.0),                                                   vec3(0.8, 0.8, 0.2),    1000.0, .2);
    spheres[4] = sphere(vec3(-1.5 + 0.5 * sin(u_time * 0.8), 1.5, 3.5 + 0.5 * cos(u_time * 0.8)),   vec3(1.0, 0.8, 0.2),    0.5,    .9);

    ray  current_ray = create_camera_ray(gl_FragCoord.xy);
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0);

    for (int x = 0; x < MAX_BOUNCES; x++) {
        ray_intersection hit = trace_diffuse(current_ray);

        if (hit.hit_index < 0) {
            radiance += throughput * mix(vec3(.2,.2,.3), vec3(.1,.4,.9), max(current_ray.dir.y, 0.0));
            break;
        }

        vec3 P = current_ray.origin + current_ray.dir * hit.t_min;
        vec3 N = sphere_normal(hit.hit_index, P);
        vec3 L    = normalize(vec3(1.+sin(u_time*2.), 1, -1));
        float dif = max(dot(L, N), 0.0);
        vec3 diff_color = spheres[hit.hit_index].color * dif;
        radiance   += throughput * diff_color;
        throughput *= spheres[hit.hit_index].metallic;

        vec3 R     = reflect(current_ray.dir, N);
        current_ray.origin   = P + N * EPSILON;
        current_ray.dir      = normalize(R);

        if (max(max(throughput.r, throughput.g), throughput.b) < 1e-3)          // if throughput is negligible, stop
            break;
    }

    gl_FragColor = vec4(radiance, 1.0);
}
