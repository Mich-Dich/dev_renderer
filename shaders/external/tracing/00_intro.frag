
#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

// ------------- RAY TRACING -------------

const float EPSILON = 1e-4;

struct ray {
    vec3 origin;
    vec3 dir;
};

struct sphere {
    vec3 center;
    vec3 color;
    float radius;
};

const int SHERE_COUNT = 5;
sphere spheres[SHERE_COUNT];

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


// ------------- NORMAL STUFF -------------

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

vec3 palette(float t) {

    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(0.5, 0.1, 0.5);
    vec3 d = vec3(0.5, 0.5, 0.9);
    return a + b * cos(TWO_PI * (c * t + d));
}



void main() {

    ray cam_ray = create_camera_ray(gl_FragCoord.xy);

    vec3 light_source = normalize(vec3(1.0 + sin(u_time * 2.0), 1., -1.));
    vec3 color = vec3(1., 0., 0.);

    spheres[0] = sphere(vec3( 2.0, 1.0 + 0.5 * sin(u_time * 2.0), 5.0),                             vec3(1.0, 0.2, 0.2),    1.0 );
    spheres[1] = sphere(vec3( 1.5 * cos(u_time * 1.5) + 1.5, 0.5, 4.0 + 0.3 * sin(u_time * 1.5)),   palette(u_time),        0.8 );
    spheres[2] = sphere(vec3(-1.0, 0.7 + 0.7 * cos(u_time), 6.0),                                   palette(u_time + 1.),   1.2 );
    spheres[3] = sphere(vec3( 0.0, -1001.0, 5.0),                                                   vec3(0.8, 0.8, 0.2),    1000.0 );
    spheres[4] = sphere(vec3(-1.5 + 0.5 * sin(u_time * 0.8), 1.5, 3.5 + 0.5 * cos(u_time * 0.8)),   vec3(1.0, 0.8, 0.2),    0.5 );

    float t_min = 1e4;
    int hit_index = -1;
    for (int x = 0; x < SHERE_COUNT; x++) {

        float loc_t = intersect_ray_sphere(x, cam_ray);
        if (loc_t > EPSILON && loc_t < t_min) {
            t_min = loc_t;
            hit_index = x;
        }
    }

    if (hit_index != -1) {

        vec3 point = cam_ray.origin + (cam_ray.dir * t_min);
        vec3 normal = sphere_normal(hit_index, point);
        float brightness = dot(light_source, normal);
        color = spheres[hit_index].color * brightness;

    } else
        color = mix(vec3(.2, .2, .3), vec3(.1, .4, .9), max(0., cam_ray.dir.y));

    gl_FragColor = vec4(color, 1.);
}
