
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

const int MAX_BOUNCES  = 4;

struct ray {
    vec3 origin;
    vec3 dir;
};

struct ray_intersection {
    int hit_index;
    float t_min;
};

// ------------- RAY TRACING (generic shape) -------------

struct shape {
    int type;
    vec3 position;
    vec2 size;              // sphere: only use X as radius     cone: X is angle, Y is heigth
    vec3 color;
    float metallic;
};

const int SHAPE_COUNT = 6;
shape shapes[SHAPE_COUNT];

// ------------- RAY TRACING (sphere) -------------

ray create_camera_ray(vec2 pixel_coord) {

    float fx = tan(radians(90.) / 2.) / u_resolution.x;
    vec2 d = (2. * pixel_coord - u_resolution) * fx;
    return ray(vec3(0.), normalize(vec3(d, 1.)));
}

float intersect_ray_sphere(int index, ray cam_ray) {

    vec3 oc = cam_ray.origin - shapes[index].position;
    float dot_dir_OC = dot(oc, cam_ray.dir);
    float root = dot_dir_OC * dot_dir_OC - (dot(oc, oc) - shapes[index].size.x * shapes[index].size.x);
    if (root < EPSILON)
        return -1.;

    float p = -dot_dir_OC;
    float q = sqrt(root);
    return (p-q) > .0 ? p-q : p+q;
}

vec3 sphere_normal(int index, vec3 point) {
    
    return normalize(point - shapes[index].position);
}

// ------------- RAY TRACING (cone) -------------

// returns t or -1.0 if no hit
float intersect_ray_cone(int index, ray R) {
    // fetch parameters
    vec3  apex    = shapes[index].position;
    float h_angle = shapes[index].size.x;   // half‐angle
    float H       = shapes[index].size.y;   // height
    float m       = tan(h_angle);
    float m2      = m*m;
    // vector from apex to ray origin
    vec3 co      = R.origin - apex;

    // ----- 1) side surface intersection -----
    float a = dot(R.dir.xz, R.dir.xz) - m2 * R.dir.y*R.dir.y;
    float b = 2.0 * (dot(co.xz, R.dir.xz) - m2 * co.y * R.dir.y);
    float c = dot(co.xz, co.xz)          - m2 * co.y * co.y;

    float disc = b*b - 4.0*a*c;
    float t_side = -1.0;
    if (disc > EPSILON) {
        float sd    = sqrt(disc);
        float inv2a = 0.5 / a;
        float t0    = (-b - sd) * inv2a;
        float t1    = (-b + sd) * inv2a;
        // pick nearest positive
        t_side = (t0 > EPSILON) ? t0 : (t1 > EPSILON) ? t1 : -1.0;
        if (t_side > 0.0) {
            // check y‐coordinate of intersection
            float y = co.y + R.dir.y * t_side;
            if (y < 0.0 || y > H) {
                t_side = -1.0;
            }
        }
    }

    // ----- 2) base cap intersection (horizontal disc) -----
    float t_cap = -1.0;
    if (H > 0.0) {
        // plane at y = H
        float denom = R.dir.y;
        if (abs(denom) > EPSILON) {
            float t_plane = (H - co.y) / denom;
            if (t_plane > EPSILON) {
                // check within base radius
                float r_base = H * m;
                vec3  P      = co + R.dir * t_plane; 
                if (dot(P.xz, P.xz) <= r_base * r_base) {
                    t_cap = t_plane;
                }
            }
        }
    }

    // choose the closest valid hit
    if (t_side > 0.0 && (t_cap < 0.0 || t_side < t_cap)) {
        return t_side;
    } else if (t_cap > 0.0) {
        return t_cap;
    } else {
        return -1.0;
    }
}

vec3 cone_normal(int index, vec3 P_world) {
    vec3 apex = shapes[index].position;
    float h_angle = shapes[index].size.x;
    float H = shapes[index].size.y;
    float m = tan(h_angle);
    float m2 = m*m;

    vec3 P = P_world - apex;
    if (abs(P.y - H) < EPSILON)
        return vec3(0.0, 1.0, 0.0);

    vec3 n = vec3(P.x, -m2 * P.y, P.z);
    return normalize(n);
}

// ------------- RAY TRACING - MAIN -------------

ray_intersection trace_diffuse(ray ray) {
    
    float t_min = 1e4;
    int hit_index  = -1;
    for (int i = 0; i < SHAPE_COUNT; i++) {

        float t = 0.;
        if (shapes[i].type == 0)
            t = intersect_ray_sphere(i, ray);
        else
            t = intersect_ray_cone(i, ray);

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

    shapes[0] = shape(0, vec3( 2.0, 1.0 + 0.5 * sin(u_time * 2.0), 5.0),                            vec2(1.0,       0.),    vec3(1.0, 0.2, 0.2),    .8);
    shapes[1] = shape(0, vec3( 1.5 * cos(u_time * 1.5) + 1.5, 0.5, 4.0 + 0.3 * sin(u_time * 1.5)),  vec2(0.8,       0.),    palette(u_time),        .0);
    shapes[2] = shape(0, vec3(-1.0, 0.7 + 0.7 * cos(u_time), 6.0),                                  vec2(1.2,       0.),    palette(u_time + 1.),   .1);
    shapes[3] = shape(0, vec3( 0.0, -2001.0, 5.0),                                                  vec2(2000.0,    0.),    vec3(0.1),              .2);
    shapes[4] = shape(0, vec3(-1.5 + 0.5 * sin(u_time * 0.8), 1.5, 3.5 + 0.5 * cos(u_time * 0.8)),  vec2(0.5,       0.),    vec3(1.0, 0.8, 0.2),    .9);
    shapes[4] = shape(1, vec3(-1.5 + 0.5 * sin(u_time * 0.8), .0, 2.5 + 0.5 * cos(u_time * 0.8)),   vec2(0.5,       1.),    vec3(1.0, 0.8, 0.2),    .9);

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
        vec3 diff_color = shapes[hit.hit_index].color * dif;
        radiance   += throughput * diff_color;
        throughput *= shapes[hit.hit_index].metallic;

        vec3 R     = reflect(current_ray.dir, N);
        current_ray.origin   = P + N * EPSILON;
        current_ray.dir      = normalize(R);

        if (max(max(throughput.r, throughput.g), throughput.b) < 1e-3)          // if throughput is negligible, stop
            break;
    }

    gl_FragColor = vec4(radiance, 1.0);
}
