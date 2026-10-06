/* nukanim.h - animation for Nuklear. v0.1.0, MIT, see the end of the file.
 *
 * A C99 port of ImAnim by Soufiane KHIAT, with ImAnimate's nka_animate() by
 * K. Bieniek (Raidcore.GG). Include nuklear.h first; in one C file, also
 *
 *     #define NUKANIM_IMPLEMENTATION
 *     #include "nukanim.h"
 *
 * The math comes from <math.h>. To use your own float functions instead,
 * name them with NKA_MATH: #define NKA_MATH(fn) my_##fn uses my_sin, my_pow...
 */
#ifndef NUKANIM_H
#define NUKANIM_H

#define NKA_VERSION "0.1.0"

#ifndef NKA_API
#ifdef NKA_PRIVATE
#define NKA_API static inline
#else
#define NKA_API extern
#endif
#endif

enum nka_ease_type {
    NKA_EASE_LINEAR,
    NKA_EASE_IN_QUAD, NKA_EASE_OUT_QUAD, NKA_EASE_IN_OUT_QUAD,
    NKA_EASE_IN_CUBIC, NKA_EASE_OUT_CUBIC, NKA_EASE_IN_OUT_CUBIC,
    NKA_EASE_IN_QUART, NKA_EASE_OUT_QUART, NKA_EASE_IN_OUT_QUART,
    NKA_EASE_IN_QUINT, NKA_EASE_OUT_QUINT, NKA_EASE_IN_OUT_QUINT,
    NKA_EASE_IN_SINE, NKA_EASE_OUT_SINE, NKA_EASE_IN_OUT_SINE,
    NKA_EASE_IN_EXPO, NKA_EASE_OUT_EXPO, NKA_EASE_IN_OUT_EXPO,
    NKA_EASE_IN_CIRC, NKA_EASE_OUT_CIRC, NKA_EASE_IN_OUT_CIRC,
    NKA_EASE_IN_BACK, NKA_EASE_OUT_BACK, NKA_EASE_IN_OUT_BACK,          /* p0 overshoot */
    NKA_EASE_IN_ELASTIC, NKA_EASE_OUT_ELASTIC, NKA_EASE_IN_OUT_ELASTIC, /* p0 amplitude, p1 period */
    NKA_EASE_IN_BOUNCE, NKA_EASE_OUT_BOUNCE, NKA_EASE_IN_OUT_BOUNCE,
    NKA_EASE_STEPS,        /* p0 steps; p1 jumps at 0 the end, 1 the start, 2 both */
    NKA_EASE_CUBIC_BEZIER, /* p0 x1, p1 y1, p2 x2, p3 y2 */
    NKA_EASE_SPRING,       /* p0 mass, p1 stiffness, p2 damping, p3 velocity */
    NKA_EASE_CUSTOM,       /* p0 slot given to nka_register_ease */
    NKA_EASE_COUNT
};

enum nka_policy {
    NKA_POLICY_CROSSFADE, /* ease from where it is to the new target */
    NKA_POLICY_CUT,       /* jump to the new target */
    NKA_POLICY_QUEUE,     /* finish, then head for the latest target */
    NKA_POLICY_ADDITIVE,  /* a new target is added to the value */
    NKA_POLICY_MULTIPLY   /* a new target multiplies the value */
};

enum nka_color_space {
    NKA_COL_SRGB, NKA_COL_SRGB_LINEAR, NKA_COL_HSV, NKA_COL_OKLAB, NKA_COL_OKLCH
};

enum nka_anchor_space {
    NKA_ANCHOR_WINDOW_CONTENT, NKA_ANCHOR_WINDOW, NKA_ANCHOR_VIEWPORT,
    NKA_ANCHOR_LAST_ITEM, NKA_ANCHOR_COUNT
};

struct nka_context;
struct nka_vec4 { float x, y, z, w; };
struct nka_ease { int type; float p0, p1, p2, p3; };
struct nka_ease_axes { struct nka_ease x, y, z, w; };

typedef float (*nka_ease_fn)(float t);
typedef float (*nka_float_resolver)(void *user);
typedef struct nk_vec2 (*nka_vec2_resolver)(void *user);
typedef struct nka_vec4 (*nka_vec4_resolver)(void *user);
typedef struct nk_colorf (*nka_color_resolver)(void *user);
typedef int (*nka_int_resolver)(void *user);

/* A NULL allocator means malloc and free, under NK_INCLUDE_DEFAULT_ALLOCATOR. */
NKA_API struct nka_context *nka_create(const struct nk_allocator *alloc);
NKA_API void nka_destroy(struct nka_context *a);

/* Once a frame, before anything animates: advances time by dt seconds. */
NKA_API void nka_update(struct nka_context *a, float dt);
/* Nonzero when something sampled this frame is still moving. */
NKA_API int nka_busy(const struct nka_context *a);
NKA_API double nka_time(const struct nka_context *a);
/* Forgets whatever was not sampled in the last max_age frames. */
NKA_API void nka_gc(struct nka_context *a, unsigned max_age);
NKA_API void nka_clear(struct nka_context *a);
NKA_API void nka_reserve(struct nka_context *a, int tweens);
NKA_API void nka_set_time_scale(struct nka_context *a, float scale);
NKA_API float nka_time_scale(const struct nka_context *a);
/* On by default: a tween asked for its init value is not stored. */
NKA_API void nka_set_lazy_init(struct nka_context *a, int on);
NKA_API int nka_lazy_init(const struct nka_context *a);
/* The sizes that relative targets are fractions of. */
NKA_API void nka_set_anchor(struct nka_context *a, int space, struct nk_vec2 size);
NKA_API struct nk_vec2 nka_anchor(const struct nka_context *a, int space);

NKA_API nk_hash nka_id(const char *name);
NKA_API nk_hash nka_id_mix(nk_hash a, nk_hash b);

NKA_API struct nka_ease nka_ease(int type);
NKA_API struct nka_ease nka_ease_bezier(float x1, float y1, float x2, float y2);
NKA_API struct nka_ease nka_ease_steps(int steps, int mode);
NKA_API struct nka_ease nka_ease_back(float overshoot);
NKA_API struct nka_ease nka_ease_elastic(float amplitude, float period);
NKA_API struct nka_ease nka_ease_spring(float mass, float stiffness, float damping, float velocity);
NKA_API struct nka_ease nka_ease_custom(int slot);
/* Slots 0 to 15. */
NKA_API void nka_register_ease(struct nka_context *a, int slot, nka_ease_fn fn);
NKA_API nka_ease_fn nka_custom_ease(const struct nka_context *a, int slot);
NKA_API float nka_eval_preset(int type, float t);
NKA_API float nka_eval(const struct nka_context *a, struct nka_ease ease, float t);

/* ImAnimate: moves *value from `from` toward `to` over ms milliseconds,
 * from wherever *value stands. The pointer is the key. */
NKA_API void nka_animate(struct nka_context *a, float from, float to, float ms, float *value, int curve);

/* A tween is keyed by id and channel. init is where it starts. */
NKA_API float nka_tween_float(struct nka_context *a, nk_hash id, nk_hash ch, float target, float dur, struct nka_ease ease, int policy, float init);
NKA_API struct nk_vec2 nka_tween_vec2(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_vec2 target, float dur, struct nka_ease ease, int policy, struct nk_vec2 init);
NKA_API struct nka_vec4 nka_tween_vec4(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_vec4 target, float dur, struct nka_ease ease, int policy, struct nka_vec4 init);
NKA_API int nka_tween_int(struct nka_context *a, nk_hash id, nk_hash ch, int target, float dur, struct nka_ease ease, int policy, int init);
NKA_API struct nk_colorf nka_tween_color(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_colorf target, float dur, struct nka_ease ease, int policy, int space, struct nk_colorf init);

NKA_API struct nk_vec2 nka_tween_vec2_per_axis(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_vec2 target, float dur, struct nka_ease_axes ease, int policy);
NKA_API struct nka_vec4 nka_tween_vec4_per_axis(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_vec4 target, float dur, struct nka_ease_axes ease, int policy);
NKA_API struct nk_colorf nka_tween_color_per_axis(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_colorf target, float dur, struct nka_ease_axes ease, int policy, int space);

/* Targets that are a fraction of an anchor plus pixels. Axis 0 is x, 1 is y;
 * a vec4 takes z and w as they are. */
NKA_API float nka_tween_float_rel(struct nka_context *a, nk_hash id, nk_hash ch, float percent, float px_bias, float dur, struct nka_ease ease, int policy, int anchor, int axis);
NKA_API struct nk_vec2 nka_tween_vec2_rel(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_vec2 percent, struct nk_vec2 px_bias, float dur, struct nka_ease ease, int policy, int anchor);
NKA_API struct nka_vec4 nka_tween_vec4_rel(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_vec4 percent, struct nka_vec4 px_bias, float dur, struct nka_ease ease, int policy, int anchor);

/* Targets that a callback works out on every call. */
NKA_API float nka_tween_float_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_float_resolver fn, void *user, float dur, struct nka_ease ease, int policy);
NKA_API struct nk_vec2 nka_tween_vec2_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_vec2_resolver fn, void *user, float dur, struct nka_ease ease, int policy);
NKA_API struct nka_vec4 nka_tween_vec4_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_vec4_resolver fn, void *user, float dur, struct nka_ease ease, int policy);
NKA_API struct nk_colorf nka_tween_color_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_color_resolver fn, void *user, float dur, struct nka_ease ease, int policy, int space);
NKA_API int nka_tween_int_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_int_resolver fn, void *user, float dur, struct nka_ease ease, int policy);

/* How far a running tween has got, from 0 to 1, or -1 when it is not running. */
NKA_API float nka_tween_progress(const struct nka_context *a, nk_hash id, nk_hash ch);

/* Points a running tween somewhere else in the time it has left. */
NKA_API void nka_rebase_float(struct nka_context *a, nk_hash id, nk_hash ch, float target);
NKA_API void nka_rebase_vec2(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_vec2 target);
NKA_API void nka_rebase_vec4(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_vec4 target);
NKA_API void nka_rebase_color(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_colorf target);
NKA_API void nka_rebase_int(struct nka_context *a, nk_hash id, nk_hash ch, int target);

NKA_API struct nk_colorf nka_color_blend(struct nk_colorf a, struct nk_colorf b, float t, int space);

/* Motion paths */

enum nka_path_segment_type {
    NKA_SEG_LINE, NKA_SEG_QUADRATIC_BEZIER, NKA_SEG_CUBIC_BEZIER, NKA_SEG_CATMULL_ROM
};

/* The defaults are 64 samples and both flags on. Both ends always match, so
 * match_endpoints changes nothing, as in ImAnim. */
struct nka_morph_opts { int samples, match_endpoints, use_arc_length; };

NKA_API struct nk_vec2 nka_bezier_quadratic(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, float t);
NKA_API struct nk_vec2 nka_bezier_cubic(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, struct nk_vec2 p3, float t);
/* Through p1 at t 0 and p2 at t 1. Tension 0 is Catmull-Rom proper; ImAnim's default is 0.5. */
NKA_API struct nk_vec2 nka_catmull_rom(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, struct nk_vec2 p3, float t, float tension);
NKA_API struct nk_vec2 nka_bezier_quadratic_deriv(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, float t);
NKA_API struct nk_vec2 nka_bezier_cubic_deriv(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, struct nk_vec2 p3, float t);
NKA_API struct nk_vec2 nka_catmull_rom_deriv(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, struct nk_vec2 p3, float t, float tension);

/* nka_path_end puts the path in place of any other with its id. */
NKA_API void nka_path_begin(struct nka_context *a, nk_hash path_id, struct nk_vec2 start);
NKA_API void nka_path_line_to(struct nka_context *a, struct nk_vec2 end);
NKA_API void nka_path_quadratic_to(struct nka_context *a, struct nk_vec2 ctrl, struct nk_vec2 end);
NKA_API void nka_path_cubic_to(struct nka_context *a, struct nk_vec2 ctrl1, struct nk_vec2 ctrl2, struct nk_vec2 end);
NKA_API void nka_path_catmull_to(struct nka_context *a, struct nk_vec2 end, float tension);
/* A line back to the start, unless the path is there already. */
NKA_API void nka_path_close(struct nka_context *a);
NKA_API void nka_path_end(struct nka_context *a);

NKA_API int nka_path_exists(const struct nka_context *a, nk_hash path_id);
/* Approximate: the arc-length table's measure once there is one. */
NKA_API float nka_path_length(const struct nka_context *a, nk_hash path_id);
NKA_API struct nk_vec2 nka_path_evaluate(const struct nka_context *a, nk_hash path_id, float t);
NKA_API struct nk_vec2 nka_path_tangent(const struct nka_context *a, nk_hash path_id, float t);
NKA_API float nka_path_angle(const struct nka_context *a, nk_hash path_id, float t);

/* From the start of the path to its end once, at constant speed when the path has an
 * arc-length table. As in ImAnim, policy changes nothing. */
NKA_API struct nk_vec2 nka_tween_path(struct nka_context *a, nk_hash id, nk_hash ch, nk_hash path_id, float dur, struct nka_ease ease, int policy);
NKA_API float nka_tween_path_angle(struct nka_context *a, nk_hash id, nk_hash ch, nk_hash path_id, float dur, struct nka_ease ease, int policy);

/* Subdivisions below 1 mean 64. Rebuilding the path drops its table. */
NKA_API void nka_path_build_arc_lut(struct nka_context *a, nk_hash path_id, int subdivisions);
NKA_API int nka_path_has_arc_lut(const struct nka_context *a, nk_hash path_id);
NKA_API float nka_path_distance_to_t(const struct nka_context *a, nk_hash path_id, float distance);
NKA_API struct nk_vec2 nka_path_evaluate_at_distance(const struct nka_context *a, nk_hash path_id, float distance);
NKA_API float nka_path_angle_at_distance(const struct nka_context *a, nk_hash path_id, float distance);
NKA_API struct nk_vec2 nka_path_tangent_at_distance(const struct nka_context *a, nk_hash path_id, float distance);

NKA_API struct nka_morph_opts nka_morph_opts_default(void);
/* path_a at blend 0, path_b at 1; NULL opts are the defaults. With use_arc_length, t is a
 * fraction of each path's length, and a path without an arc-length table gets one of 64. */
NKA_API struct nk_vec2 nka_path_morph(struct nka_context *a, nk_hash path_a, nk_hash path_b, float t, float blend, const struct nka_morph_opts *opts);
NKA_API struct nk_vec2 nka_path_morph_tangent(struct nka_context *a, nk_hash path_a, nk_hash path_b, float t, float blend, const struct nka_morph_opts *opts);
NKA_API float nka_path_morph_angle(struct nka_context *a, nk_hash path_a, nk_hash path_b, float t, float blend, const struct nka_morph_opts *opts);
/* Along the morph once while the blend eases to target_blend. NKA_POLICY_CUT makes the
 * blend jump; every other policy crossfades it. */
NKA_API struct nk_vec2 nka_tween_path_morph(struct nka_context *a, nk_hash id, nk_hash ch, nk_hash path_a, nk_hash path_b, float target_blend, float dur, struct nka_ease path_ease, struct nka_ease morph_ease, int policy, const struct nka_morph_opts *opts);
NKA_API float nka_get_morph_blend(const struct nka_context *a, nk_hash id, nk_hash ch);

/* Turns the four points about center, then moves them by translation. */
NKA_API void nka_transform_quad(struct nk_vec2 *quad, struct nk_vec2 center, float angle, struct nk_vec2 translation);
/* Bottom-left, bottom-right, top-right, top-left of a glyph standing on pos, its baseline
 * raised by baseline_offset, turned to angle. */
NKA_API void nka_make_glyph_quad(struct nk_vec2 *quad, struct nk_vec2 pos, float angle, float glyph_width, float glyph_height, float baseline_offset);

/* Clips */

enum nka_direction { NKA_DIR_NORMAL, NKA_DIR_REVERSE, NKA_DIR_ALTERNATE };

enum nka_channel_type {
    NKA_CHAN_FLOAT, NKA_CHAN_VEC2, NKA_CHAN_VEC4, NKA_CHAN_INT, NKA_CHAN_COLOR,
    NKA_CHAN_FLOAT_REL, NKA_CHAN_VEC2_REL, NKA_CHAN_VEC4_REL, NKA_CHAN_COLOR_REL
};

enum nka_result { NKA_OK, NKA_ERR_NOT_FOUND, NKA_ERR_BAD_ARG, NKA_ERR_NO_MEM };

/* How a keyed value changes from one loop to the next. */
enum nka_variation_mode {
    NKA_VAR_NONE, NKA_VAR_INCREMENT, NKA_VAR_DECREMENT, NKA_VAR_MULTIPLY,
    NKA_VAR_RANDOM,     /* within -amount and amount */
    NKA_VAR_RANDOM_ABS, /* within 0 and amount */
    NKA_VAR_PINGPONG,   /* 0, +amount, 0, -amount, ... */
    NKA_VAR_CALLBACK
};

enum nka_stagger_from { NKA_STAGGER_FIRST, NKA_STAGGER_LAST, NKA_STAGGER_CENTER, NKA_STAGGER_INDEX };
enum nka_stagger_axis { NKA_STAGGER_BOTH, NKA_STAGGER_X, NKA_STAGGER_Y };

struct nka_spring_params { float mass, stiffness, damping, initial_velocity; };

/* Called while values are worked out, with the loop index: they must not call nka_ functions. */
typedef float (*nka_variation_float_fn)(int loop, void *user);
typedef int (*nka_variation_int_fn)(int loop, void *user);
typedef struct nk_vec2 (*nka_variation_vec2_fn)(int loop, void *user);
typedef struct nka_vec4 (*nka_variation_vec4_fn)(int loop, void *user);

/* A seed of 0 draws from the instance; any other seed draws the same numbers every time. */
struct nka_variation_float {
    int mode;
    float amount, min_clamp, max_clamp;
    nk_uint seed;
    nka_variation_float_fn callback;
    void *user;
};

struct nka_variation_int {
    int mode, amount, min_clamp, max_clamp;
    nk_uint seed;
    nka_variation_int_fn callback;
    void *user;
};

/* With mode NKA_VAR_NONE, each axis varies on its own. */
struct nka_variation_vec2 {
    int mode;
    struct nk_vec2 amount, min_clamp, max_clamp;
    nk_uint seed;
    nka_variation_vec2_fn callback;
    void *user;
    struct nka_variation_float x, y;
};

struct nka_variation_vec4 {
    int mode;
    struct nka_vec4 amount, min_clamp, max_clamp;
    nk_uint seed;
    nka_variation_vec4_fn callback;
    void *user;
    struct nka_variation_float x, y, z, w;
};

/* Varies the components of space and clamps there; the result, or the callback's, is sRGB. */
struct nka_variation_color {
    int mode;
    struct nka_vec4 amount, min_clamp, max_clamp;
    int space;
    nk_uint seed;
    nka_variation_vec4_fn callback;
    void *user;
    struct nka_variation_float r, g, b, a;
};

typedef void (*nka_clip_callback)(struct nka_context *a, nk_hash inst, void *user);
typedef void (*nka_loop_callback)(struct nka_context *a, nk_hash inst, int loop, void *user);
typedef void (*nka_marker_callback)(struct nka_context *a, nk_hash inst, nk_hash marker, float time, void *user);

struct nka_stagger_grid_opts {
    int cols, rows;
    int from, from_index; /* nka_stagger_from; from_index with NKA_STAGGER_INDEX */
    int axis;             /* nka_stagger_axis */
    float delay;          /* the farthest element waits delay times the count less one */
    int ease;             /* shapes the spread */
    float start_delay;
};

NKA_API struct nka_variation_float nka_varf_none(void);
NKA_API struct nka_variation_float nka_varf_inc(float amount);
NKA_API struct nka_variation_float nka_varf_dec(float amount);
NKA_API struct nka_variation_float nka_varf_mul(float factor);
NKA_API struct nka_variation_float nka_varf_rand(float range);
NKA_API struct nka_variation_float nka_varf_rand_abs(float range);
NKA_API struct nka_variation_float nka_varf_pingpong(float amount);
NKA_API struct nka_variation_float nka_varf_fn(nka_variation_float_fn fn, void *user);
NKA_API struct nka_variation_float nka_varf_clamp(struct nka_variation_float v, float lo, float hi);
NKA_API struct nka_variation_float nka_varf_seed(struct nka_variation_float v, nk_uint seed);

NKA_API struct nka_variation_int nka_vari_none(void);
NKA_API struct nka_variation_int nka_vari_inc(int amount);
NKA_API struct nka_variation_int nka_vari_dec(int amount);
NKA_API struct nka_variation_int nka_vari_rand(int range);
NKA_API struct nka_variation_int nka_vari_fn(nka_variation_int_fn fn, void *user);
NKA_API struct nka_variation_int nka_vari_clamp(struct nka_variation_int v, int lo, int hi);
NKA_API struct nka_variation_int nka_vari_seed(struct nka_variation_int v, nk_uint seed);

NKA_API struct nka_variation_vec2 nka_varv2_none(void);
NKA_API struct nka_variation_vec2 nka_varv2_inc(float x, float y);
NKA_API struct nka_variation_vec2 nka_varv2_dec(float x, float y);
NKA_API struct nka_variation_vec2 nka_varv2_mul(float factor);
NKA_API struct nka_variation_vec2 nka_varv2_rand(float x, float y);
NKA_API struct nka_variation_vec2 nka_varv2_fn(nka_variation_vec2_fn fn, void *user);
NKA_API struct nka_variation_vec2 nka_varv2_axis(struct nka_variation_float x, struct nka_variation_float y);
NKA_API struct nka_variation_vec2 nka_varv2_clamp(struct nka_variation_vec2 v, struct nk_vec2 lo, struct nk_vec2 hi);
NKA_API struct nka_variation_vec2 nka_varv2_seed(struct nka_variation_vec2 v, nk_uint seed);

NKA_API struct nka_variation_vec4 nka_varv4_none(void);
NKA_API struct nka_variation_vec4 nka_varv4_inc(float x, float y, float z, float w);
NKA_API struct nka_variation_vec4 nka_varv4_dec(float x, float y, float z, float w);
NKA_API struct nka_variation_vec4 nka_varv4_mul(float factor);
NKA_API struct nka_variation_vec4 nka_varv4_rand(float x, float y, float z, float w);
NKA_API struct nka_variation_vec4 nka_varv4_fn(nka_variation_vec4_fn fn, void *user);
NKA_API struct nka_variation_vec4 nka_varv4_axis(struct nka_variation_float x, struct nka_variation_float y,
                                                 struct nka_variation_float z, struct nka_variation_float w);
NKA_API struct nka_variation_vec4 nka_varv4_clamp(struct nka_variation_vec4 v, struct nka_vec4 lo, struct nka_vec4 hi);
NKA_API struct nka_variation_vec4 nka_varv4_seed(struct nka_variation_vec4 v, nk_uint seed);

/* In NKA_COL_OKLAB until nka_varc_space says otherwise. */
NKA_API struct nka_variation_color nka_varc_none(void);
NKA_API struct nka_variation_color nka_varc_inc(float r, float g, float b, float alpha);
NKA_API struct nka_variation_color nka_varc_dec(float r, float g, float b, float alpha);
NKA_API struct nka_variation_color nka_varc_mul(float factor);
NKA_API struct nka_variation_color nka_varc_rand(float r, float g, float b, float alpha);
NKA_API struct nka_variation_color nka_varc_fn(nka_variation_vec4_fn fn, void *user);
NKA_API struct nka_variation_color nka_varc_channel(struct nka_variation_float r, struct nka_variation_float g,
                                                    struct nka_variation_float b, struct nka_variation_float alpha);
NKA_API struct nka_variation_color nka_varc_space(struct nka_variation_color v, int space);
NKA_API struct nka_variation_color nka_varc_clamp(struct nka_variation_color v, struct nka_vec4 lo, struct nka_vec4 hi);
NKA_API struct nka_variation_color nka_varc_seed(struct nka_variation_color v, nk_uint seed);

/* One clip is built at a time; a begin drops a build that never ended. */
NKA_API void nka_clip_begin(struct nka_context *a, nk_hash clip);
/* A key's ease shapes the way to the next key; bezier4, x1 y1 x2 y2, only with NKA_EASE_CUBIC_BEZIER. */
NKA_API void nka_clip_key_float(struct nka_context *a, nk_hash ch, float time, float value, int ease, const float *bezier4);
NKA_API void nka_clip_key_vec2(struct nka_context *a, nk_hash ch, float time, struct nk_vec2 value, int ease, const float *bezier4);
NKA_API void nka_clip_key_vec4(struct nka_context *a, nk_hash ch, float time, struct nka_vec4 value, int ease, const float *bezier4);
NKA_API void nka_clip_key_int(struct nka_context *a, nk_hash ch, float time, int value, int ease);
NKA_API void nka_clip_key_color(struct nka_context *a, nk_hash ch, float time, struct nk_colorf value, int space, int ease, const float *bezier4);
NKA_API void nka_clip_key_float_var(struct nka_context *a, nk_hash ch, float time, float value, struct nka_variation_float var,
                                   int ease, const float *bezier4);
NKA_API void nka_clip_key_vec2_var(struct nka_context *a, nk_hash ch, float time, struct nk_vec2 value, struct nka_variation_vec2 var,
                                  int ease, const float *bezier4);
NKA_API void nka_clip_key_vec4_var(struct nka_context *a, nk_hash ch, float time, struct nka_vec4 value, struct nka_variation_vec4 var,
                                  int ease, const float *bezier4);
NKA_API void nka_clip_key_int_var(struct nka_context *a, nk_hash ch, float time, int value, struct nka_variation_int var, int ease);
NKA_API void nka_clip_key_color_var(struct nka_context *a, nk_hash ch, float time, struct nk_colorf value, struct nka_variation_color var,
                                   int space, int ease, const float *bezier4);
NKA_API void nka_clip_key_float_spring(struct nka_context *a, nk_hash ch, float time, float target, struct nka_spring_params spring);
/* Fractions of an anchor plus pixels, worked out when read. Axis 0 is x, 1 is y; a vec4 takes
 * z and w as they are; a color scales r and b by the anchor's x, g and a by its y. */
NKA_API void nka_clip_key_float_rel(struct nka_context *a, nk_hash ch, float time, float percent, float px_bias, int anchor, int axis,
                                   int ease, const float *bezier4);
NKA_API void nka_clip_key_vec2_rel(struct nka_context *a, nk_hash ch, float time, struct nk_vec2 percent, struct nk_vec2 px_bias,
                                  int anchor, int ease, const float *bezier4);
NKA_API void nka_clip_key_vec4_rel(struct nka_context *a, nk_hash ch, float time, struct nka_vec4 percent, struct nka_vec4 px_bias,
                                  int anchor, int ease, const float *bezier4);
NKA_API void nka_clip_key_color_rel(struct nka_context *a, nk_hash ch, float time, struct nka_vec4 percent, struct nka_vec4 px_bias,
                                   int space, int anchor, int ease, const float *bezier4);
/* Keys in a group take their times from where it began; after its end, from its last key. */
NKA_API void nka_clip_seq_begin(struct nka_context *a);
NKA_API void nka_clip_seq_end(struct nka_context *a);
NKA_API void nka_clip_par_begin(struct nka_context *a);
NKA_API void nka_clip_par_end(struct nka_context *a);
/* Returns the marker's id, made up when id is 0. */
NKA_API nk_hash nka_clip_marker(struct nka_context *a, float time, nk_hash id, nka_marker_callback cb, void *user);
/* A count of -1 loops forever, n repeats n times. */
NKA_API void nka_clip_set_loop(struct nka_context *a, int loop, int direction, int count);
NKA_API void nka_clip_set_loop_delay(struct nka_context *a, float seconds);
NKA_API void nka_clip_set_delay(struct nka_context *a, float seconds);
/* center_bias from 0, a wave from the first element, to 1, from the middle out. */
NKA_API void nka_clip_set_stagger(struct nka_context *a, int count, float each_delay, float center_bias);
NKA_API void nka_clip_set_stagger_ease(struct nka_context *a, int ease);
/* Each loop's length, the pause before it, and its speed. */
NKA_API void nka_clip_set_duration_var(struct nka_context *a, struct nka_variation_float var);
NKA_API void nka_clip_set_delay_var(struct nka_context *a, struct nka_variation_float var);
NKA_API void nka_clip_set_timescale_var(struct nka_context *a, struct nka_variation_float var);
NKA_API void nka_clip_on_begin(struct nka_context *a, nka_clip_callback cb, void *user);
NKA_API void nka_clip_on_update(struct nka_context *a, nka_clip_callback cb, void *user);
NKA_API void nka_clip_on_complete(struct nka_context *a, nka_clip_callback cb, void *user);
NKA_API void nka_clip_on_loop(struct nka_context *a, nka_loop_callback cb, void *user);
NKA_API void nka_clip_on_pause(struct nka_context *a, nka_clip_callback cb, void *user);
/* NKA_OK, or an error with any clip of that id left as it was. */
NKA_API int nka_clip_end(struct nka_context *a);

NKA_API float nka_clip_duration(const struct nka_context *a, nk_hash clip);
NKA_API int nka_clip_exists(const struct nka_context *a, nk_hash clip);
NKA_API void nka_clip_reserve(struct nka_context *a, int clips, int instances);

/* Starts clip on inst, made or restarted; returns inst, or 0 for an unknown clip or inst 0. */
NKA_API nk_hash nka_play(struct nka_context *a, nk_hash clip, nk_hash inst);
/* inst when it exists, else 0. */
NKA_API nk_hash nka_get_instance(struct nka_context *a, nk_hash inst);
NKA_API int nka_instance_valid(struct nka_context *a, nk_hash inst);
NKA_API void nka_instance_pause(struct nka_context *a, nk_hash inst);
NKA_API void nka_instance_resume(struct nka_context *a, nk_hash inst);
/* Stops at time 0 and keeps the values. */
NKA_API void nka_instance_stop(struct nka_context *a, nk_hash inst);
NKA_API void nka_instance_restart(struct nka_context *a, nk_hash inst);
/* Goes back to the start of the pass and stops there. */
NKA_API void nka_instance_reset(struct nka_context *a, nk_hash inst);
NKA_API void nka_instance_refresh(struct nka_context *a, nk_hash inst);
NKA_API void nka_instance_destroy(struct nka_context *a, nk_hash inst);
NKA_API void nka_instance_seek(struct nka_context *a, nk_hash inst, float time);
/* A scale of zero or below plays at normal speed. */
NKA_API void nka_instance_set_time_scale(struct nka_context *a, nk_hash inst, float scale);
/* Multiplies the weight nka_layer_add gives the instance. */
NKA_API void nka_instance_set_weight(struct nka_context *a, nk_hash inst, float weight);
/* When inst completes, next_clip plays on next_inst, made up when 0. Returns next_inst. */
NKA_API nk_hash nka_instance_then(struct nka_context *a, nk_hash inst, nk_hash next_clip, nk_hash next_inst);
NKA_API void nka_instance_then_delay(struct nka_context *a, nk_hash inst, float delay);
NKA_API float nka_instance_time(struct nka_context *a, nk_hash inst);
NKA_API float nka_instance_duration(struct nka_context *a, nk_hash inst);
NKA_API int nka_instance_is_playing(struct nka_context *a, nk_hash inst);
NKA_API int nka_instance_is_paused(struct nka_context *a, nk_hash inst);
/* Zero, and *out zero (a color opaque black), when the clip has no such channel. */
NKA_API int nka_instance_get_float(struct nka_context *a, nk_hash inst, nk_hash ch, float *out);
NKA_API int nka_instance_get_vec2(struct nka_context *a, nk_hash inst, nk_hash ch, struct nk_vec2 *out);
NKA_API int nka_instance_get_vec4(struct nka_context *a, nk_hash inst, nk_hash ch, struct nka_vec4 *out);
NKA_API int nka_instance_get_int(struct nka_context *a, nk_hash inst, nk_hash ch, int *out);
NKA_API int nka_instance_get_color(struct nka_context *a, nk_hash inst, nk_hash ch, struct nk_colorf *out);

NKA_API float nka_stagger_delay(const struct nka_context *a, nk_hash clip, int index);
/* Plays with the clip's delay plus its stagger delay for index. */
NKA_API nk_hash nka_play_stagger(struct nka_context *a, nk_hash clip, nk_hash inst, int index);
NKA_API struct nka_stagger_grid_opts nka_stagger_grid_opts_default(void);
/* NULL opts are the defaults. */
NKA_API float nka_stagger_grid_delay(const struct nka_context *a, int col, int row, const struct nka_stagger_grid_opts *opts);
NKA_API float nka_stagger_grid_delay_index(const struct nka_context *a, int index, const struct nka_stagger_grid_opts *opts);
/* Plays with delay in place of the clip's. */
NKA_API nk_hash nka_play_with_delay(struct nka_context *a, nk_hash clip, nk_hash inst, float delay);

/* Weighted averages of instances' values per channel, kept under target; colors are left out. */
NKA_API void nka_layer_begin(struct nka_context *a, nk_hash target);
NKA_API void nka_layer_add(struct nka_context *a, nk_hash inst, float weight);
NKA_API void nka_layer_end(struct nka_context *a, nk_hash target);
NKA_API int nka_get_blended_float(struct nka_context *a, nk_hash target, nk_hash ch, float *out);
NKA_API int nka_get_blended_vec2(struct nka_context *a, nk_hash target, nk_hash ch, struct nk_vec2 *out);
NKA_API int nka_get_blended_vec4(struct nka_context *a, nk_hash target, nk_hash ch, struct nka_vec4 *out);
NKA_API int nka_get_blended_int(struct nka_context *a, nk_hash target, nk_hash ch, int *out);

/* Returns the size of clip saved, 0 for no such clip, and writes it to buf when cap allows. Callbacks are not saved. */
NKA_API int nka_clip_save(const struct nka_context *a, nk_hash clip, void *buf, int cap);
/* Replaces the clip saved in buf, keeping the callbacks of the one it replaces, markers' by id. */
NKA_API int nka_clip_load(struct nka_context *a, const void *buf, int len, nk_hash *out_id);

/* Procedural */

enum nka_wave_type { NKA_WAVE_SINE, NKA_WAVE_TRIANGLE, NKA_WAVE_SAWTOOTH, NKA_WAVE_SQUARE };
enum nka_noise_type { NKA_NOISE_PERLIN, NKA_NOISE_SIMPLEX, NKA_NOISE_VALUE, NKA_NOISE_WORLEY };
/* CW only adds to the angle and CCW only takes from it; DIRECT lerps the two numbers. */
enum nka_rotation_mode {
    NKA_ROTATION_SHORTEST, NKA_ROTATION_LONGEST, NKA_ROTATION_CW, NKA_ROTATION_CCW, NKA_ROTATION_DIRECT
};

#ifndef NKA_GRADIENT_MAX
#define NKA_GRADIENT_MAX 16
#endif

/* Stops sorted by position. Zeroed, it is empty and samples white. */
struct nka_gradient {
    int count;
    float positions[NKA_GRADIENT_MAX];
    struct nk_colorf colors[NKA_GRADIENT_MAX];
};

/* Scales, then rotates by radians, then moves by position. */
struct nka_transform { struct nk_vec2 position, scale; float rotation; };

struct nka_noise_opts {
    int type, octaves;
    float persistence, lacunarity; /* each octave's amplitude and frequency factors */
    int seed;
};

struct nka_drag_opts {
    struct nk_vec2 snap_grid;          /* 0 on an axis leaves it free */
    const struct nk_vec2 *snap_points; /* the nearest one wins over the grid */
    int snap_points_count;
    float snap_duration;
    float overshoot;                   /* 1 overshoots as much as NKA_EASE_OUT_BACK */
    int ease_type;
};

struct nka_drag_feedback {
    struct nk_vec2 position, offset, velocity;
    int is_dragging, is_snapping;
    float snap_progress;
};

/* Between -amplitude and amplitude; phase is in cycles. */
NKA_API float nka_oscillate(struct nka_context *a, nk_hash id, float amplitude, float frequency, int wave, float phase);
NKA_API int nka_oscillate_int(struct nka_context *a, nk_hash id, int amplitude, float frequency, int wave, float phase);
NKA_API struct nk_vec2 nka_oscillate_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 amplitude, struct nk_vec2 frequency, int wave, struct nk_vec2 phase);
NKA_API struct nka_vec4 nka_oscillate_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 amplitude, struct nka_vec4 frequency, int wave, struct nka_vec4 phase);
/* The amplitude moves the channels of the space, as hue, saturation, value and alpha in HSV. */
NKA_API struct nk_colorf nka_oscillate_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 amplitude, float frequency, int wave, float phase, int space);

/* A shake starts here and dies out over decay_time seconds. */
NKA_API void nka_trigger_shake(struct nka_context *a, nk_hash id);
NKA_API float nka_shake(struct nka_context *a, nk_hash id, float intensity, float frequency, float decay_time);
NKA_API int nka_shake_int(struct nka_context *a, nk_hash id, int intensity, float frequency, float decay_time);
NKA_API struct nk_vec2 nka_shake_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 intensity, float frequency, float decay_time);
NKA_API struct nka_vec4 nka_shake_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 intensity, float frequency, float decay_time);
NKA_API struct nk_colorf nka_shake_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 intensity, float frequency, float decay_time, int space);
NKA_API float nka_wiggle(struct nka_context *a, nk_hash id, float amplitude, float frequency);
NKA_API int nka_wiggle_int(struct nka_context *a, nk_hash id, int amplitude, float frequency);
NKA_API struct nk_vec2 nka_wiggle_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 amplitude, float frequency);
NKA_API struct nka_vec4 nka_wiggle_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 amplitude, float frequency);
NKA_API struct nk_colorf nka_wiggle_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 amplitude, float frequency, int space);

/* Perlin, 4 octaves, persistence 0.5, lacunarity 2, seed 0; a NULL opts means these. */
NKA_API struct nka_noise_opts nka_noise_opts_default(void);
/* Between -1 and 1. */
NKA_API float nka_noise_2d(float x, float y, const struct nka_noise_opts *opts);
NKA_API float nka_noise_3d(float x, float y, float z, const struct nka_noise_opts *opts);
NKA_API float nka_noise_channel_float(struct nka_context *a, nk_hash id, float frequency, float amplitude, const struct nka_noise_opts *opts);
NKA_API struct nk_vec2 nka_noise_channel_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 frequency, struct nk_vec2 amplitude, const struct nka_noise_opts *opts);
NKA_API struct nka_vec4 nka_noise_channel_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 frequency, struct nka_vec4 amplitude, const struct nka_noise_opts *opts);
NKA_API struct nk_colorf nka_noise_channel_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 amplitude, float frequency, const struct nka_noise_opts *opts, int space);
/* Two octaves of Perlin noise, speed being its frequency. */
NKA_API float nka_smooth_noise_float(struct nka_context *a, nk_hash id, float amplitude, float speed);
NKA_API struct nk_vec2 nka_smooth_noise_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 amplitude, float speed);
NKA_API struct nka_vec4 nka_smooth_noise_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 amplitude, float speed);
NKA_API struct nk_colorf nka_smooth_noise_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 amplitude, float speed, int space);

/* 0, adding nothing, when the gradient is full. */
NKA_API int nka_gradient_add(struct nka_gradient *g, float position, struct nk_colorf color);
NKA_API int nka_gradient_add_rgba(struct nka_gradient *g, float position, struct nk_color color);
NKA_API struct nk_colorf nka_gradient_sample(const struct nka_gradient *g, float t, int space);
NKA_API struct nka_gradient nka_gradient_solid(struct nk_colorf color);
NKA_API struct nka_gradient nka_gradient_two_color(struct nk_colorf start, struct nk_colorf end);
NKA_API struct nka_gradient nka_gradient_three_color(struct nk_colorf start, struct nk_colorf mid, struct nk_colorf end);
/* Has a stop wherever either has one; past NKA_GRADIENT_MAX, the least visible go. */
NKA_API struct nka_gradient nka_gradient_lerp(const struct nka_gradient *a, const struct nka_gradient *b, float t, int space);
/* NKA_POLICY_CUT jumps, every other policy crossfades. */
NKA_API struct nka_gradient nka_tween_gradient(struct nka_context *a, nk_hash id, nk_hash ch, const struct nka_gradient *target, float dur, struct nka_ease ease, int policy, int space);

NKA_API struct nka_transform nka_transform(struct nk_vec2 position, float rotation, struct nk_vec2 scale);
NKA_API struct nka_transform nka_transform_identity(void);
/* a after b: applying the result applies b, then a. */
NKA_API struct nka_transform nka_transform_compose(struct nka_transform a, struct nka_transform b);
NKA_API struct nk_vec2 nka_transform_apply(struct nka_transform t, struct nk_vec2 point);
/* Exact under a uniform scale; compose(inverse(t), t) is the identity under any. */
NKA_API struct nka_transform nka_transform_inverse(struct nka_transform t);
NKA_API struct nka_transform nka_transform_lerp(struct nka_transform a, struct nka_transform b, float t, int rotation_mode);
/* NKA_POLICY_CUT jumps, every other policy crossfades. */
NKA_API struct nka_transform nka_tween_transform(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_transform target, float dur, struct nka_ease ease, int policy, int rotation_mode);
/* Row-major 3x2; out holds m00 m01 tx m10 m11 ty. */
NKA_API struct nka_transform nka_transform_from_matrix(float m00, float m01, float m10, float m11, float tx, float ty);
NKA_API void nka_transform_to_matrix(struct nka_transform t, float *out);

/* No snapping, 0.2 seconds, no overshoot, NKA_EASE_OUT_CUBIC; a NULL opts means these. */
NKA_API struct nka_drag_opts nka_drag_opts_default(void);
NKA_API struct nka_drag_feedback nka_drag_begin(struct nka_context *a, nk_hash id, struct nk_vec2 pos);
NKA_API struct nka_drag_feedback nka_drag_update(struct nka_context *a, nk_hash id, struct nk_vec2 pos);
/* Once a frame until it stops snapping: the first call picks the target, the rest move there. */
NKA_API struct nka_drag_feedback nka_drag_release(struct nka_context *a, nk_hash id, struct nk_vec2 pos, const struct nka_drag_opts *opts);
NKA_API void nka_drag_cancel(struct nka_context *a, nk_hash id);

/* Nuklear */

/* Anchors from the current window: its content region and its size. The
 * viewport and the last item are yours to set. */
NKA_API void nka_anchor_update(struct nka_context *a, struct nk_context *ctx);

/* In a window or group that scrolls this way, call nka_scroll last thing
 * before nk_end or nk_group_end, every frame; the nka_scroll_to calls go
 * anywhere inside it. Scrolling by hand stops a running one. */
NKA_API void nka_scroll(struct nka_context *a, struct nk_context *ctx);
NKA_API void nka_scroll_to_x(struct nka_context *a, struct nk_context *ctx, float x, float dur, struct nka_ease ease);
NKA_API void nka_scroll_to_y(struct nka_context *a, struct nk_context *ctx, float y, float dur, struct nka_ease ease);
NKA_API void nka_scroll_to_top(struct nka_context *a, struct nk_context *ctx, float dur, struct nka_ease ease);
NKA_API void nka_scroll_to_bottom(struct nka_context *a, struct nk_context *ctx, float dur, struct nka_ease ease);

/* Styles blend every color, size and color style item; the rest comes from
 * whichever style is nearer. */
NKA_API void nka_style_register(struct nka_context *a, nk_hash style_id, const struct nk_style *style);
NKA_API void nka_style_register_current(struct nka_context *a, const struct nk_context *ctx, nk_hash style_id);
/* The style nk_style_from_table makes from NK_COLOR_COUNT colors. */
NKA_API void nka_style_register_table(struct nka_context *a, struct nk_context *ctx, nk_hash style_id, const struct nk_color *table);
NKA_API int nka_style_exists(const struct nka_context *a, nk_hash style_id);
NKA_API void nka_style_unregister(struct nka_context *a, nk_hash style_id);
NKA_API void nka_style_blend_to(struct nka_context *a, nk_hash style_a, nk_hash style_b, float t, struct nk_style *out, int space);
NKA_API void nka_style_blend(struct nka_context *a, struct nk_context *ctx, nk_hash style_a, nk_hash style_b, float t, int space);
/* Eases ctx->style from what it is to a registered style. */
NKA_API void nka_style_tween(struct nka_context *a, struct nk_context *ctx, nk_hash id, nk_hash target_style, float dur, struct nka_ease ease, int space);

enum nka_text_stagger_effect {
    NKA_TEXT_FX_NONE, NKA_TEXT_FX_FADE, NKA_TEXT_FX_SCALE,
    NKA_TEXT_FX_SLIDE_UP, NKA_TEXT_FX_SLIDE_DOWN, NKA_TEXT_FX_SLIDE_LEFT, NKA_TEXT_FX_SLIDE_RIGHT,
    NKA_TEXT_FX_ROTATE, NKA_TEXT_FX_BOUNCE, NKA_TEXT_FX_WAVE, NKA_TEXT_FX_TYPEWRITER
};

struct nka_text_stagger_opts {
    struct nk_vec2 pos;
    int effect;
    float char_delay, char_duration;
    float effect_intensity; /* pixels for slides and waves */
    struct nka_ease ease;
    struct nk_color color;
    const struct nk_user_font *font; /* NULL is the style's */
    float font_scale;
    float letter_spacing;
};

NKA_API struct nka_text_stagger_opts nka_text_stagger_opts_default(void);
/* Draws text into the current window, glyph by glyph as progress goes from
 * 0 to 1. Scaling shows on backends that draw Nuklear's vertex output, from
 * font copies kept until the next nka_update; Nuklear cannot turn text, so
 * the rotate effect fades. */
NKA_API void nka_text_stagger(struct nka_context *a, struct nk_context *ctx, const char *text, float progress, const struct nka_text_stagger_opts *opts);
NKA_API float nka_text_stagger_width(const struct nk_context *ctx, const char *text, const struct nka_text_stagger_opts *opts);
NKA_API float nka_text_stagger_duration(const char *text, const struct nka_text_stagger_opts *opts);

enum nka_text_path_align { NKA_TEXT_ALIGN_START, NKA_TEXT_ALIGN_CENTER, NKA_TEXT_ALIGN_END };

struct nka_text_path_opts {
    struct nk_vec2 origin; /* added to the path's points */
    float offset;          /* along the path, in pixels */
    float letter_spacing;
    int align;
    int flip_y;            /* glyphs hang below the path instead of standing on it */
    struct nk_color color;
    const struct nk_user_font *font; /* NULL is the style's */
    float font_scale;
};

NKA_API struct nka_text_path_opts nka_text_path_opts_default(void);
/* Draws text along a path into the current window. Each glyph is centered on
 * the path upright, as Nuklear's text cannot turn. The path gets an
 * arc-length table if it has none. */
NKA_API void nka_text_path(struct nka_context *a, struct nk_context *ctx, nk_hash path_id, const char *text, const struct nka_text_path_opts *opts);
/* The same, glyph by glyph as progress goes from 0 to 1. */
NKA_API void nka_text_path_animated(struct nka_context *a, struct nk_context *ctx, nk_hash path_id, const char *text, float progress, const struct nka_text_path_opts *opts);
NKA_API float nka_text_path_width(const struct nk_context *ctx, const char *text, const struct nka_text_path_opts *opts);

/* The profiler times named sections with the clock you give it, in seconds. */
NKA_API void nka_profiler_set_clock(struct nka_context *a, double (*now)(void *user), void *user);
NKA_API void nka_profiler_enable(struct nka_context *a, int on);
NKA_API int nka_profiler_is_enabled(const struct nka_context *a);
NKA_API void nka_profiler_begin_frame(struct nka_context *a);
NKA_API void nka_profiler_end_frame(struct nka_context *a);
NKA_API void nka_profiler_begin(struct nka_context *a, const char *name);
NKA_API void nka_profiler_end(struct nka_context *a);

/* A window with the time scale, what is animating, and the profiler. It
 * closes by clearing *open; a NULL open keeps it open. */
NKA_API void nka_show_unified_inspector(struct nka_context *a, struct nk_context *ctx, int *open);
/* A clip instance's tracks, keys and playhead, laid out in the current window. */
NKA_API void nka_show_debug_timeline(struct nka_context *a, struct nk_context *ctx, nk_hash instance_id);

#endif /* NUKANIM_H */

#ifdef NUKANIM_IMPLEMENTATION
#ifndef NKA__IMPLEMENTED
#define NKA__IMPLEMENTED

#ifndef NKA_MATH
#include <math.h>
#define NKA_MATH(fn) fn##f
#endif
#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
#include <stdlib.h>
#endif

#define nka__sin NKA_MATH(sin)
#define nka__cos NKA_MATH(cos)
#define nka__tan NKA_MATH(tan)
#define nka__asin NKA_MATH(asin)
#define nka__acos NKA_MATH(acos)
#define nka__atan2 NKA_MATH(atan2)
#define nka__pow NKA_MATH(pow)
#define nka__exp NKA_MATH(exp)
#define nka__log NKA_MATH(log)
#define nka__sqrt NKA_MATH(sqrt)
#define nka__cbrt NKA_MATH(cbrt)
#define nka__fabs NKA_MATH(fabs)
#define nka__fmod NKA_MATH(fmod)
#define nka__floor NKA_MATH(floor)
#define nka__ceil NKA_MATH(ceil)

#define NKA__PI 3.14159265358979f
#define NKA__EPS 1e-6f
#define NKA__MIN_DUR 1e-6f

/* Open addressing over power-of-two tables. A pointer into one lasts until
 * the next put or del on that map. */
struct nka__map {
    nk_hash *keys;
    char *vals;
    int cap, len;
    nk_size size;
};

enum { NKA__FLOAT, NKA__VEC2, NKA__VEC4, NKA__INT, NKA__COLOR };

/* Whatever nka__sweep collects starts with the frame it was last seen. */
struct nka__chan {
    nk_uint seen;
    unsigned char kind, n, policy, space, sleeping, pending;
    nk_hash id, ch;
    float cur[4], from[4], to[4], req[4], pend[4];
    float dur;
    double start;
    struct nka_ease ease;
};

struct nka__anim {
    nk_uint seen;
    int running;
    float from, to;
    double start;
};

struct nka__path_seg {
    int type;
    float tension, length;
    struct nk_vec2 p[4];
};

/* seg_len, which maps t to a segment, stays the segments' sum; length becomes the table's measure. */
struct nka__path_data {
    struct nka__path_seg *segs;
    float *lut;
    int n, cap, lut_n, lut_cap;
    float seg_len, length;
    struct nk_vec2 start;
};

struct nka__path_pair {
    const struct nka__path_data *a, *b;
    int n, arc;
};

#define NKA__PATH_KEY 0x50415448u
#define NKA__PATH_POS 0x50504f53u
#define NKA__PATH_ANGLE 0x50414e47u
#define NKA__PATH_MORPH 0x504d5048u
#define NKA__PATH_BLEND 0x50424c44u

#define NKA__CLIP_MAX_DT 1.0f
#define NKA__CLIP_MAX_WRAPS 1000
#define NKA__CLIP_MIN_PASS 0.001f
#define NKA__CLIP_FLT_MAX 3.402823466e+38f
#define NKA__CLIP_INT_MAX ((int)(~0u >> 1))
#define NKA__CLIP_MAGIC 0x43414b4eu
#define NKA__CLIP_VERSION 1u
#define NKA__CLIP_BEZIER 1
#define NKA__CLIP_SPRING 2
#define NKA__CLIP_VARIED 4
#define NKA__CLIP_SALT_DUR 0xd1000001u
#define NKA__CLIP_SALT_DELAY 0xd1000002u
#define NKA__CLIP_SALT_SCALE 0xd1000003u
#define NKA__CLIP_SALT_INST 0x494e5354u
#define NKA__CLIP_SALT_MARK 0x4d41524bu

union nka__clip_slot { float f; int i; nk_uint u; };

union nka__clip_var {
    struct nka_variation_float f;
    struct nka_variation_int i;
    struct nka_variation_vec2 v2;
    struct nka_variation_vec4 v4;
    struct nka_variation_color c;
};

/* An int key or value lives in i, never read through f. */
struct nka__clip_key {
    nk_hash ch;
    int type, space, anchor, axis;
    float time;
    int ease, flags, var;
    float bezier[4], spring[4], ext[4];
    union nka__clip_slot v[4];
};

struct nka__clip_track {
    nk_hash ch;
    int type, space, anchor, axis, first, count;
};

struct nka__clip_marker {
    float time;
    nk_hash id;
    nka_marker_callback fn;
    void *user;
};

struct nka__clip_clip {
    nk_hash id;
    nk_uint gen;
    float duration, delay, loop_delay, stagger_delay, stagger_bias;
    int loop_count, direction, stagger_count, stagger_ease;
    int has_dur_var, has_delay_var, has_scale_var;
    struct nka_variation_float dur_var, delay_var, scale_var;
    nka_clip_callback on_begin, on_update, on_complete, on_pause;
    nka_loop_callback on_loop;
    void *begin_user, *update_user, *complete_user, *pause_user, *loop_user;
    struct nka__clip_track *tracks;
    struct nka__clip_key *keys;
    union nka__clip_var *vars;
    struct nka__clip_marker *markers;
    int ntracks, nkeys, nvars, nmarkers;
};

struct nka__clip_group { float max; int seq; };

struct nka__clip_build {
    struct nka__clip_clip clip;
    struct nka__clip_key *keys;
    union nka__clip_var *vars;
    struct nka__clip_marker *markers;
    struct nka__clip_group *groups;
    int nkeys, nvars, nmarkers, ngroups, ckeys, cvars, cmarkers, cgroups;
    float offset;
    int open, failed;
};

/* vals holds 8 slots a track, then a fired flag a marker, sized for the clip of gen. */
struct nka__clip_inst {
    nk_uint seen, stepped, gen, epoch, rng;
    nk_hash id, clip, chain_clip, chain_inst;
    float time, scale, weight, delay, chain_delay;
    int playing, paused, begun, dir, loops, loop;
    union nka__clip_slot *vals;
    unsigned char *fired;
};

struct nka__clip_blend { nk_hash ch; int kind; float v[4]; };
struct nka__clip_layer { nk_uint seen; nk_hash id; int n; struct nka__clip_blend *e; };
struct nka__clip_acc { nk_hash ch; int kind; float w, sum[4]; };

/* Off the lattice: along a whole-number row, gradient noise loses most of its range. */
#define NKA__FX_ROW0 0.37f
#define NKA__FX_ROW1 100.71f
#define NKA__FX_ROW2 200.19f
#define NKA__FX_ROW3 300.53f

enum { NKA__FX_OSC = 1, NKA__FX_SHAKE, NKA__FX_WIGGLE, NKA__FX_NOISE };

/* Its time passes only on frames where its id is sampled. */
struct nka__fx_clock {
    nk_uint seen, frame;
    int triggered;
    double time, since;
};

struct nka__fx_grad {
    nk_uint seen;
    unsigned char sleeping, policy, space;
    float dur;
    double start;
    struct nka_ease ease;
    struct nka_gradient from, to;
};

struct nka__fx_xform {
    nk_uint seen;
    unsigned char sleeping, policy, mode;
    float dur;
    double start;
    struct nka_ease ease;
    struct nka_transform from, to;
};

struct nka__fx_drag {
    nk_uint seen, frame;
    int dragging, snapping, ease;
    float dur, overshoot, progress;
    double t0, t_prev, t_cur;
    struct nk_vec2 start, cur, prev, vel, from, to;
};

struct nka__nk_scroll {
    nk_uint seen;
    int active[2];
    float from[2], to[2], max[2], dur[2];
    nk_uint wrote[2];
    double start[2];
    struct nka_ease ease[2];
};

struct nka__nk_style {
    struct nk_style *style;
};

struct nka__nk_style_tween {
    nk_uint seen;
    int active, space;
    nk_hash target;
    struct nk_style *from;
    double start;
    float dur;
    struct nka_ease ease;
};

#define NKA__NK_FONTS 32
struct nka__nk_fonts {
    struct nka__nk_fonts *next;
    int used;
    struct nk_user_font font[NKA__NK_FONTS];
};

#define NKA__NK_SECTIONS 64
#define NKA__NK_STACK 16
#define NKA__NK_HISTORY 120
struct nka__nk_section {
    char name[32];
    double start, ms;
    int calls;
    float history[NKA__NK_HISTORY];
};

struct nka__nk_profiler {
    double (*now)(void *user);
    void *user;
    int enabled, count, depth, at;
    double frame_start, frame_ms;
    float frame[NKA__NK_HISTORY];
    int stack[NKA__NK_STACK];
    struct nka__nk_section sections[NKA__NK_SECTIONS];
};

struct nka_context {
    struct nk_allocator alloc;
    double time;
    float dt, scale;
    nk_uint frame;
    int lazy, busy;
    nka_ease_fn custom[16];
    struct nk_vec2 anchors[NKA_ANCHOR_COUNT];
    struct nka__map chans, anims;
    struct nka__map path_map;
    struct nka__path_data path_new;
    struct nk_vec2 path_cur, path_prev;
    nk_hash path_id;
    int path_building, path_failed;
    struct nka__map clip_clips, clip_insts, clip_layers, clip_acc;
    struct nka__clip_build clip_build;
    nk_uint clip_gen, clip_epoch, clip_auto, clip_mod;
    nk_hash clip_layer;
    float clip_layer_weight;
    int clip_updating;
    struct nka__map fx_clocks, fx_grads, fx_xforms, fx_drags;
    struct nka__map nk_scrolls, nk_styles, nk_style_tweens;
    struct nka__nk_fonts *nk_fonts;
    struct nka__nk_profiler *nk_profiler;
};

#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
static void *nka__malloc(nk_handle h, void *old, nk_size n) { (void)h; (void)old; return malloc(n); }
static void nka__mfree(nk_handle h, void *p) { (void)h; free(p); }
#endif

static void *nka__alloc(struct nka_context *a, nk_size n) { return a->alloc.alloc(a->alloc.userdata, 0, n); }
static void nka__free(struct nka_context *a, void *p) { if (p) a->alloc.free(a->alloc.userdata, p); }

static void nka__zero(void *p, nk_size n)
{
    unsigned char *b = (unsigned char *)p;
    while (n--) *b++ = 0;
}

static void nka__copy(void *dst, const void *src, nk_size n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
}

static nk_hash nka__fmix(nk_hash h)
{
    h ^= h >> 16; h *= 0x85ebca6bu;
    h ^= h >> 13; h *= 0xc2b2ae35u;
    return h ^ (h >> 16);
}

NKA_API nk_hash nka_id(const char *name)
{
    nk_hash h = 2166136261u;
    while (name && *name) { h ^= (unsigned char)*name++; h *= 16777619u; }
    return h;
}

NKA_API nk_hash nka_id_mix(nk_hash a, nk_hash b)
{
    return nka__fmix(a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2)));
}

static nk_hash nka__key(nk_hash id, nk_hash ch, int kind)
{
    return nka_id_mix(nka_id_mix(id, ch), (nk_hash)kind);
}

static nk_hash nka__ptr_key(const void *p)
{
    nk_ptr v = (nk_ptr)p;
    return nka_id_mix((nk_hash)v, (nk_hash)((v >> 16) >> 16));
}

static void *nka__at(const struct nka__map *m, int i) { return m->vals + (nk_size)i * m->size; }

/* Keys are stored scrambled, so raw ids like 0 and 1 stay apart; 0 marks a free slot. */
static nk_hash nka__mkey(nk_hash key)
{
    key = nka__fmix(key);
    return key ? key : 1u;
}

static int nka__home(const struct nka__map *m, nk_hash stored) { return (int)(stored & (nk_hash)(m->cap - 1)); }

static int nka__slot(const struct nka__map *m, nk_hash key)
{
    int i;
    if (!m->cap) return -1;
    key = nka__mkey(key);
    for (i = nka__home(m, key); m->keys[i]; i = (i + 1) & (m->cap - 1))
        if (m->keys[i] == key) return i;
    return -1;
}

static void *nka__get(const struct nka__map *m, nk_hash key)
{
    int i = nka__slot(m, key);
    return i < 0 ? 0 : nka__at(m, i);
}

static int nka__grow(struct nka_context *a, struct nka__map *m, int cap)
{
    struct nka__map n = *m;
    int i;
    n.keys = (nk_hash *)nka__alloc(a, (nk_size)cap * (sizeof(nk_hash) + m->size));
    if (!n.keys) return 0;
    n.vals = (char *)(n.keys + cap);
    n.cap = cap;
    nka__zero(n.keys, (nk_size)cap * sizeof(nk_hash));
    for (i = 0; i < m->cap; ++i) {
        int j;
        if (!m->keys[i]) continue;
        j = nka__home(&n, m->keys[i]);
        while (n.keys[j]) j = (j + 1) & (cap - 1);
        n.keys[j] = m->keys[i];
        nka__copy(nka__at(&n, j), nka__at(m, i), m->size);
    }
    nka__free(a, m->keys);
    *m = n;
    return 1;
}

/* Zeroed when new. NULL when out of memory. */
static void *nka__put(struct nka_context *a, struct nka__map *m, nk_hash key, int *fresh)
{
    int i;
    if (fresh) *fresh = 0;
    i = nka__slot(m, key);
    if (i >= 0) return nka__at(m, i);
    if ((m->len + 1) * 2 > m->cap && !nka__grow(a, m, m->cap ? m->cap * 2 : 16)) return 0;
    key = nka__mkey(key);
    i = nka__home(m, key);
    while (m->keys[i]) i = (i + 1) & (m->cap - 1);
    m->keys[i] = key;
    m->len++;
    nka__zero(nka__at(m, i), m->size);
    if (fresh) *fresh = 1;
    return nka__at(m, i);
}

/* Shifts the rest of the cluster back, so slot i may hold another entry after. */
static void nka__del_at(struct nka__map *m, int i)
{
    int mask = m->cap - 1, j = i;
    for (;;) {
        m->keys[i] = 0;
        for (;;) {
            int h;
            j = (j + 1) & mask;
            if (!m->keys[j]) { m->len--; return; }
            h = nka__home(m, m->keys[j]);
            if (!(i <= j ? (i < h && h <= j) : (i < h || h <= j))) break;
        }
        m->keys[i] = m->keys[j];
        nka__copy(nka__at(m, i), nka__at(m, j), m->size);
        i = j;
    }
}

static void nka__del(struct nka__map *m, nk_hash key)
{
    int i = nka__slot(m, key);
    if (i >= 0) nka__del_at(m, i);
}

static void nka__sweep(struct nka__map *m, nk_uint frame, unsigned max_age)
{
    int i = 0;
    while (i < m->cap) {
        if (m->keys[i] && frame - *(const nk_uint *)nka__at(m, i) > max_age) nka__del_at(m, i);
        else ++i;
    }
}

static void nka__map_free(struct nka_context *a, struct nka__map *m)
{
    nka__free(a, m->keys);
    m->keys = 0;
    m->vals = 0;
    m->cap = m->len = 0;
}

static float nka__clamp01(float t) { return t < 0 ? 0 : (t > 1 ? 1 : t); }
static float nka__lerp(float a, float b, float t) { return a + (b - a) * t; }

static struct nk_vec2 nka__vec2(float x, float y)
{
    struct nk_vec2 v;
    v.x = x; v.y = y;
    return v;
}

static struct nka_vec4 nka__vec4(float x, float y, float z, float w)
{
    struct nka_vec4 v;
    v.x = x; v.y = y; v.z = z; v.w = w;
    return v;
}

static struct nka_vec4 nka__cv(struct nk_colorf c) { return nka__vec4(c.r, c.g, c.b, c.a); }

static struct nk_colorf nka__vc(struct nka_vec4 v)
{
    struct nk_colorf c;
    c.r = v.x; c.g = v.y; c.b = v.z; c.a = v.w;
    return c;
}

/* Easing */

NKA_API struct nka_ease nka_ease(int type)
{
    struct nka_ease e;
    e.type = type;
    e.p0 = e.p1 = e.p2 = e.p3 = 0;
    return e;
}

NKA_API struct nka_ease nka_ease_bezier(float x1, float y1, float x2, float y2)
{
    struct nka_ease e = nka_ease(NKA_EASE_CUBIC_BEZIER);
    e.p0 = x1; e.p1 = y1; e.p2 = x2; e.p3 = y2;
    return e;
}

NKA_API struct nka_ease nka_ease_steps(int steps, int mode)
{
    struct nka_ease e = nka_ease(NKA_EASE_STEPS);
    e.p0 = (float)steps; e.p1 = (float)mode;
    return e;
}

NKA_API struct nka_ease nka_ease_back(float overshoot)
{
    struct nka_ease e = nka_ease(NKA_EASE_OUT_BACK);
    e.p0 = overshoot;
    return e;
}

NKA_API struct nka_ease nka_ease_elastic(float amplitude, float period)
{
    struct nka_ease e = nka_ease(NKA_EASE_OUT_ELASTIC);
    e.p0 = amplitude; e.p1 = period;
    return e;
}

NKA_API struct nka_ease nka_ease_spring(float mass, float stiffness, float damping, float velocity)
{
    struct nka_ease e = nka_ease(NKA_EASE_SPRING);
    e.p0 = mass; e.p1 = stiffness; e.p2 = damping; e.p3 = velocity;
    return e;
}

NKA_API struct nka_ease nka_ease_custom(int slot)
{
    struct nka_ease e = nka_ease(NKA_EASE_CUSTOM);
    e.p0 = (float)slot;
    return e;
}

static float nka__bounce(float t)
{
    const float n = 7.5625f, d = 2.75f;
    if (t < 1 / d) return n * t * t;
    if (t < 2 / d) { t -= 1.5f / d; return n * t * t + 0.75f; }
    if (t < 2.5f / d) { t -= 2.25f / d; return n * t * t + 0.9375f; }
    t -= 2.625f / d;
    return n * t * t + 0.984375f;
}

static float nka__in(int family, float t)
{
    const float c1 = 1.70158f, c3 = c1 + 1, c4 = 2 * NKA__PI / 3;
    switch (family) {
    case 0: return t * t;
    case 1: return t * t * t;
    case 2: return t * t * t * t;
    case 3: return t * t * t * t * t;
    case 4: return 1 - nka__cos(t * NKA__PI / 2);
    case 5: return t == 0 ? 0 : nka__pow(2, 10 * t - 10);
    case 6: return 1 - nka__sqrt(1 - t * t);
    case 7: return c3 * t * t * t - c1 * t * t;
    case 8: return t == 0 || t == 1 ? t : -nka__pow(2, 10 * t - 10) * nka__sin((t * 10 - 10.75f) * c4);
    case 9: return 1 - nka__bounce(1 - t);
    default: return t;
    }
}

NKA_API float nka_eval_preset(int type, float t)
{
    int family = (type - 1) / 3;
    t = nka__clamp01(t);
    if (type <= NKA_EASE_LINEAR || type > NKA_EASE_IN_OUT_BOUNCE) return t;
    switch ((type - 1) % 3) {
    case 0: return nka__in(family, t);
    case 1: return 1 - nka__in(family, 1 - t);
    default: return t < 0.5f ? nka__in(family, 2 * t) / 2 : 1 - nka__in(family, 2 - 2 * t) / 2;
    }
}

static float nka__back(float t, float s) { return t * t * ((s + 1) * t - s); }

static float nka__elastic(float t, float amp, float period)
{
    float s;
    if (t <= 0 || t >= 1) return t;
    if (amp < 1) { amp = 1; s = period / 4; }
    else s = period / (2 * NKA__PI) * nka__asin(1 / amp);
    return -(amp * nka__pow(2, 10 * (t - 1)) * nka__sin((t - 1 - s) * 2 * NKA__PI / period));
}

static float nka__steps(float t, int n, int mode)
{
    float k, y;
    if (n < 1) n = 1;
    k = nka__floor(t * (float)n + NKA__EPS);
    if (mode == 1) y = (k + 1) / (float)n;
    else if (mode == 2) y = (k + 1) / (float)(n + 1);
    else y = k / (float)n;
    return y > 1 ? 1 : y;
}

static float nka__bezier(float x, float x1, float y1, float x2, float y2)
{
    float t = x, m;
    int i;
    for (i = 0; i < 8; ++i) {
        float u = 1 - t;
        float bx = 3 * u * u * t * x1 + 3 * u * t * t * x2 + t * t * t;
        float dx = 3 * u * u * x1 + 6 * u * t * (x2 - x1) + 3 * t * t * (1 - x2);
        if (dx != 0) t = nka__clamp01(t - (bx - x) / dx);
    }
    m = 1 - t;
    return 3 * m * m * t * y1 + 3 * m * t * t * y2 + t * t * t;
}

/* A unit step of a damped spring, t in seconds, v0 toward the target. */
static float nka__spring(float t, float mass, float k, float c, float v0)
{
    float wn = nka__sqrt(k / mass), zeta = c / (2 * nka__sqrt(k * mass));
    if (zeta < 1) {
        float wd = wn * nka__sqrt(1 - zeta * zeta);
        float b = (zeta * wn - v0) / wd;
        return 1 - nka__exp(-zeta * wn * t) * (nka__cos(wd * t) + b * nka__sin(wd * t));
    }
    if (zeta == 1) return 1 - nka__exp(-wn * t) * (1 + (wn - v0) * t);
    {
        float wd = wn * nka__sqrt(zeta * zeta - 1);
        float r1 = -zeta * wn + wd, r2 = -zeta * wn - wd;
        float c1 = (-v0 - r2) / (r1 - r2);
        return 1 - c1 * nka__exp(r1 * t) - (1 - c1) * nka__exp(r2 * t);
    }
}

NKA_API float nka_eval(const struct nka_context *a, struct nka_ease e, float t)
{
    t = nka__clamp01(t);
    switch (e.type) {
    case NKA_EASE_IN_BACK: case NKA_EASE_OUT_BACK: case NKA_EASE_IN_OUT_BACK:
        if (e.p0 == 0) break;
        if (e.type == NKA_EASE_IN_BACK) return nka__back(t, e.p0);
        if (e.type == NKA_EASE_OUT_BACK) return 1 - nka__back(1 - t, e.p0);
        return t < 0.5f ? nka__back(2 * t, e.p0) / 2 : 1 - nka__back(2 - 2 * t, e.p0) / 2;
    case NKA_EASE_IN_ELASTIC: case NKA_EASE_OUT_ELASTIC: case NKA_EASE_IN_OUT_ELASTIC: {
        float amp = e.p0 > 0 ? e.p0 : 1;
        float period = e.p1 > 0 ? e.p1 : (e.type == NKA_EASE_IN_OUT_ELASTIC ? 0.45f : 0.3f);
        if (e.p0 <= 0 && e.p1 <= 0) break;
        if (e.type == NKA_EASE_IN_ELASTIC) return nka__elastic(t, amp, period);
        if (e.type == NKA_EASE_OUT_ELASTIC) return 1 - nka__elastic(1 - t, amp, period);
        return t < 0.5f ? nka__elastic(2 * t, amp, period) / 2 : 1 - nka__elastic(2 - 2 * t, amp, period) / 2;
    }
    case NKA_EASE_STEPS: return nka__steps(t, (int)e.p0, (int)e.p1);
    case NKA_EASE_CUBIC_BEZIER: return nka__bezier(t, e.p0, e.p1, e.p2, e.p3);
    case NKA_EASE_SPRING:
        return nka__spring(t, e.p0 > 0 ? e.p0 : 1, e.p1 > 0 ? e.p1 : 120, e.p2 > 0 ? e.p2 : 20, e.p3);
    case NKA_EASE_CUSTOM: {
        int slot = (int)e.p0;
        if (a && slot >= 0 && slot < 16 && a->custom[slot]) return a->custom[slot](t);
        return t;
    }
    default: break;
    }
    return nka_eval_preset(e.type, t);
}

NKA_API void nka_register_ease(struct nka_context *a, int slot, nka_ease_fn fn)
{
    if (a && slot >= 0 && slot < 16) a->custom[slot] = fn;
}

NKA_API nka_ease_fn nka_custom_ease(const struct nka_context *a, int slot)
{
    return a && slot >= 0 && slot < 16 ? a->custom[slot] : 0;
}

/* Color spaces. Everything outside them is sRGB, 0 to 1. */

static float nka__to_linear(float c) { return c <= 0.04045f ? c / 12.92f : nka__pow((c + 0.055f) / 1.055f, 2.4f); }
static float nka__to_srgb(float c) { return c <= 0.0031308f ? 12.92f * c : 1.055f * nka__pow(c, 1 / 2.4f) - 0.055f; }

static struct nka_vec4 nka__hsv_to_srgb(struct nka_vec4 c)
{
    float h, f, p, q, t;
    int i;
    if (c.y <= 0) return nka__vec4(c.z, c.z, c.z, c.w);
    h = nka__fmod(c.x, 1);
    if (h < 0) h += 1;
    h *= 6;
    i = (int)nka__floor(h);
    f = h - (float)i;
    p = c.z * (1 - c.y);
    q = c.z * (1 - c.y * f);
    t = c.z * (1 - c.y * (1 - f));
    switch (i % 6) {
    case 0: return nka__vec4(c.z, t, p, c.w);
    case 1: return nka__vec4(q, c.z, p, c.w);
    case 2: return nka__vec4(p, c.z, t, c.w);
    case 3: return nka__vec4(p, q, c.z, c.w);
    case 4: return nka__vec4(t, p, c.z, c.w);
    default: return nka__vec4(c.z, p, q, c.w);
    }
}

static struct nka_vec4 nka__srgb_to_hsv(struct nka_vec4 c)
{
    float mx = c.x > c.y ? c.x : c.y, mn = c.x < c.y ? c.x : c.y, d, h = 0;
    if (c.z > mx) mx = c.z;
    if (c.z < mn) mn = c.z;
    d = mx - mn;
    if (d != 0) {
        if (mx == c.x) h = nka__fmod((c.y - c.z) / d, 6);
        else if (mx == c.y) h = (c.z - c.x) / d + 2;
        else h = (c.x - c.y) / d + 4;
        h /= 6;
        if (h < 0) h += 1;
    }
    return nka__vec4(h, mx == 0 ? 0 : d / mx, mx, c.w);
}

static struct nka_vec4 nka__srgb_to_oklab(struct nka_vec4 c)
{
    float r = nka__to_linear(c.x), g = nka__to_linear(c.y), b = nka__to_linear(c.z);
    float l = nka__cbrt(0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b);
    float m = nka__cbrt(0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b);
    float s = nka__cbrt(0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b);
    return nka__vec4(0.2104542553f * l + 0.7936177850f * m - 0.0040720468f * s,
                     1.9779984951f * l - 2.4285922050f * m + 0.4505937099f * s,
                     0.0259040371f * l + 0.7827717662f * m - 0.8086757660f * s, c.w);
}

static struct nka_vec4 nka__oklab_to_srgb(struct nka_vec4 c)
{
    float l = c.x + 0.3963377774f * c.y + 0.2158037573f * c.z;
    float m = c.x - 0.1055613458f * c.y - 0.0638541728f * c.z;
    float s = c.x - 0.0894841775f * c.y - 1.2914855480f * c.z;
    l = l * l * l; m = m * m * m; s = s * s * s;
    return nka__vec4(nka__to_srgb(nka__clamp01(4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s)),
                     nka__to_srgb(nka__clamp01(-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s)),
                     nka__to_srgb(nka__clamp01(-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s)), c.w);
}

static struct nka_vec4 nka__to_space(struct nka_vec4 c, int space)
{
    struct nka_vec4 v;
    switch (space) {
    case NKA_COL_SRGB_LINEAR: return nka__vec4(nka__to_linear(c.x), nka__to_linear(c.y), nka__to_linear(c.z), c.w);
    case NKA_COL_HSV: return nka__srgb_to_hsv(c);
    case NKA_COL_OKLAB: return nka__srgb_to_oklab(c);
    case NKA_COL_OKLCH:
        v = nka__srgb_to_oklab(c);
        c.x = v.x;
        c.y = nka__sqrt(v.y * v.y + v.z * v.z);
        c.z = nka__atan2(v.z, v.y) / (2 * NKA__PI);
        if (c.z < 0) c.z += 1;
        return c;
    default: return c;
    }
}

static struct nka_vec4 nka__from_space(struct nka_vec4 c, int space)
{
    switch (space) {
    case NKA_COL_SRGB_LINEAR: return nka__vec4(nka__to_srgb(c.x), nka__to_srgb(c.y), nka__to_srgb(c.z), c.w);
    case NKA_COL_HSV: return nka__hsv_to_srgb(c);
    case NKA_COL_OKLAB: return nka__oklab_to_srgb(c);
    case NKA_COL_OKLCH:
        return nka__oklab_to_srgb(nka__vec4(c.x, c.y * nka__cos(c.z * 2 * NKA__PI), c.y * nka__sin(c.z * 2 * NKA__PI), c.w));
    default: return c;
    }
}

static float nka__hue_lerp(float a, float b, float t)
{
    float h = a + (nka__fmod(b - a + 1.5f, 1) - 0.5f) * t;
    return h < 0 ? h + 1 : (h > 1 ? h - 1 : h);
}

static struct nka_vec4 nka__lerp4(struct nka_vec4 a, struct nka_vec4 b, float t)
{
    return nka__vec4(nka__lerp(a.x, b.x, t), nka__lerp(a.y, b.y, t), nka__lerp(a.z, b.z, t), nka__lerp(a.w, b.w, t));
}

static struct nka_vec4 nka__lerp_color(struct nka_vec4 a, struct nka_vec4 b, float t, int space)
{
    struct nka_vec4 x, y, r;
    if (t == 0) return a;
    if (t == 1) return b;
    x = nka__to_space(a, space);
    y = nka__to_space(b, space);
    r = nka__lerp4(x, y, t);
    if (space == NKA_COL_HSV) r.x = nka__hue_lerp(x.x, y.x, t);
    if (space == NKA_COL_OKLCH) r.z = nka__hue_lerp(x.z, y.z, t);
    return nka__from_space(r, space);
}

NKA_API struct nk_colorf nka_color_blend(struct nk_colorf a, struct nk_colorf b, float t, int space)
{
    return nka__vc(nka__lerp_color(nka__cv(a), nka__cv(b), t, space));
}

/* Tweens */

static void nka__chan_eval(struct nka_context *a, struct nka__chan *c)
{
    float t, k;
    int i;
    if (c->sleeping) return;
    t = (float)((a->time - c->start) / c->dur);
    if (t >= 1) {
        for (i = 0; i < 4; ++i) c->cur[i] = c->to[i];
        c->sleeping = 1;
        return;
    }
    k = nka_eval(a, c->ease, t);
    if (c->kind == NKA__COLOR) {
        struct nka_vec4 v = nka__lerp_color(nka__vec4(c->from[0], c->from[1], c->from[2], c->from[3]),
                                            nka__vec4(c->to[0], c->to[1], c->to[2], c->to[3]), k, c->space);
        c->cur[0] = v.x; c->cur[1] = v.y; c->cur[2] = v.z; c->cur[3] = v.w;
    } else {
        for (i = 0; i < c->n; ++i) {
            c->cur[i] = nka__lerp(c->from[i], c->to[i], k);
            if (c->kind == NKA__INT) c->cur[i] = nka__floor(c->cur[i] + 0.5f);
        }
    }
    a->busy = 1;
}

static void nka__chan_set(struct nka_context *a, struct nka__chan *c, const float *to, float dur,
                          struct nka_ease e, int policy, int space)
{
    int i;
    for (i = 0; i < 4; ++i) { c->from[i] = c->cur[i]; c->to[i] = to[i]; }
    c->dur = dur > NKA__MIN_DUR ? dur : NKA__MIN_DUR;
    c->start = a->time;
    c->ease = e;
    c->policy = (unsigned char)policy;
    c->space = (unsigned char)space;
    c->sleeping = 0;
}

static int nka__near(const float *x, const float *y, int n)
{
    float d = 0;
    int i;
    for (i = 0; i < n; ++i) d += nka__fabs(x[i] - y[i]);
    return d <= NKA__EPS;
}

static int nka__ease_eq(struct nka_ease x, struct nka_ease y)
{
    return x.type == y.type && x.p0 == y.p0 && x.p1 == y.p1 && x.p2 == y.p2 && x.p3 == y.p3;
}

/* Every array is four floats; a kind reads the first one, two or four. */
static void nka__tween(struct nka_context *a, nk_hash id, nk_hash ch, int kind, const float *target, float dur,
                       struct nka_ease e, int policy, int space, const float *init, float *out)
{
    static const unsigned char width[] = { 1, 2, 4, 1, 4 };
    int n = width[kind], i, same, cfg, done;
    nk_hash key = nka__key(id, ch, kind);
    struct nka__chan *c = (struct nka__chan *)nka__get(&a->chans, key);
    if (!c) {
        if (a->lazy && policy < NKA_POLICY_ADDITIVE && nka__near(target, init, n)) c = 0;
        else c = (struct nka__chan *)nka__put(a, &a->chans, key, 0);
        if (!c) {
            for (i = 0; i < 4; ++i) out[i] = target[i];
            return;
        }
        for (i = 0; i < 4; ++i) c->cur[i] = c->from[i] = c->to[i] = c->req[i] = init[i];
        c->id = id;
        c->ch = ch;
        c->kind = (unsigned char)kind;
        c->n = (unsigned char)n;
        c->policy = (unsigned char)policy;
        c->space = (unsigned char)space;
        c->ease = e;
        c->dur = NKA__MIN_DUR;
        c->sleeping = 1;
    }
    c->seen = a->frame;
    same = nka__near(c->req, target, n);
    cfg = c->policy == policy && c->space == space && nka__ease_eq(c->ease, e);
    done = c->sleeping || a->time - c->start >= c->dur;
    if (c->pending && done) {
        c->pending = 0;
        nka__chan_eval(a, c);
        for (i = 0; i < 4; ++i) c->req[i] = c->pend[i];
        nka__chan_set(a, c, c->pend, dur, e, policy, space);
    } else if (policy == NKA_POLICY_QUEUE && !done) {
        c->pending = (unsigned char)!same;
        for (i = 0; i < 4; ++i) c->pend[i] = target[i];
    } else if (!same || (!cfg && policy < NKA_POLICY_ADDITIVE && !c->sleeping)) {
        float to[4];
        nka__chan_eval(a, c);
        for (i = 0; i < 4; ++i) {
            c->req[i] = target[i];
            to[i] = policy == NKA_POLICY_ADDITIVE ? c->cur[i] + target[i]
                  : policy == NKA_POLICY_MULTIPLY ? c->cur[i] * target[i] : target[i];
        }
        nka__chan_set(a, c, to, dur, e, policy, space);
        if (policy == NKA_POLICY_CUT) {
            for (i = 0; i < 4; ++i) c->cur[i] = c->from[i] = to[i];
            c->sleeping = 1;
        }
    }
    nka__chan_eval(a, c);
    for (i = 0; i < 4; ++i) out[i] = c->cur[i];
}

NKA_API float nka_tween_float(struct nka_context *a, nk_hash id, nk_hash ch, float target, float dur,
                              struct nka_ease ease, int policy, float init)
{
    float t[4] = { 0 }, s[4] = { 0 }, o[4];
    if (!a) return target;
    t[0] = target;
    s[0] = init;
    nka__tween(a, id, ch, NKA__FLOAT, t, dur, ease, policy, 0, s, o);
    return o[0];
}

NKA_API struct nk_vec2 nka_tween_vec2(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_vec2 target,
                                      float dur, struct nka_ease ease, int policy, struct nk_vec2 init)
{
    float t[4] = { 0 }, s[4] = { 0 }, o[4];
    if (!a) return target;
    t[0] = target.x; t[1] = target.y;
    s[0] = init.x; s[1] = init.y;
    nka__tween(a, id, ch, NKA__VEC2, t, dur, ease, policy, 0, s, o);
    return nka__vec2(o[0], o[1]);
}

NKA_API struct nka_vec4 nka_tween_vec4(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_vec4 target,
                                       float dur, struct nka_ease ease, int policy, struct nka_vec4 init)
{
    float t[4], s[4], o[4];
    if (!a) return target;
    t[0] = target.x; t[1] = target.y; t[2] = target.z; t[3] = target.w;
    s[0] = init.x; s[1] = init.y; s[2] = init.z; s[3] = init.w;
    nka__tween(a, id, ch, NKA__VEC4, t, dur, ease, policy, 0, s, o);
    return nka__vec4(o[0], o[1], o[2], o[3]);
}

NKA_API int nka_tween_int(struct nka_context *a, nk_hash id, nk_hash ch, int target, float dur,
                          struct nka_ease ease, int policy, int init)
{
    float t[4] = { 0 }, s[4] = { 0 }, o[4];
    if (!a) return target;
    t[0] = (float)target;
    s[0] = (float)init;
    nka__tween(a, id, ch, NKA__INT, t, dur, ease, policy, 0, s, o);
    return (int)nka__floor(o[0] + 0.5f);
}

NKA_API struct nk_colorf nka_tween_color(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_colorf target,
                                         float dur, struct nka_ease ease, int policy, int space, struct nk_colorf init)
{
    float t[4], s[4], o[4];
    if (!a) return target;
    t[0] = target.r; t[1] = target.g; t[2] = target.b; t[3] = target.a;
    s[0] = init.r; s[1] = init.g; s[2] = init.b; s[3] = init.a;
    nka__tween(a, id, ch, NKA__COLOR, t, dur, ease, policy, space, s, o);
    return nka__vc(nka__vec4(o[0], o[1], o[2], o[3]));
}

static nk_hash nka__axis(nk_hash ch, int axis) { return nka_id_mix(ch, 0x41584953u + (nk_hash)axis); }

NKA_API struct nk_vec2 nka_tween_vec2_per_axis(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_vec2 target,
                                               float dur, struct nka_ease_axes ease, int policy)
{
    struct nk_vec2 v;
    v.x = nka_tween_float(a, id, nka__axis(ch, 0), target.x, dur, ease.x, policy, 0);
    v.y = nka_tween_float(a, id, nka__axis(ch, 1), target.y, dur, ease.y, policy, 0);
    return v;
}

NKA_API struct nka_vec4 nka_tween_vec4_per_axis(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_vec4 target,
                                                float dur, struct nka_ease_axes ease, int policy)
{
    struct nka_vec4 v;
    v.x = nka_tween_float(a, id, nka__axis(ch, 0), target.x, dur, ease.x, policy, 0);
    v.y = nka_tween_float(a, id, nka__axis(ch, 1), target.y, dur, ease.y, policy, 0);
    v.z = nka_tween_float(a, id, nka__axis(ch, 2), target.z, dur, ease.z, policy, 0);
    v.w = nka_tween_float(a, id, nka__axis(ch, 3), target.w, dur, ease.w, policy, 0);
    return v;
}

/* Each component eases on its own, in the given space. */
NKA_API struct nk_colorf nka_tween_color_per_axis(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_colorf target,
                                                  float dur, struct nka_ease_axes ease, int policy, int space)
{
    struct nka_vec4 w = nka__to_space(nka__cv(target), space), v;
    v.x = nka_tween_float(a, id, nka__axis(ch, 4), w.x, dur, ease.x, policy, 0);
    v.y = nka_tween_float(a, id, nka__axis(ch, 5), w.y, dur, ease.y, policy, 0);
    v.z = nka_tween_float(a, id, nka__axis(ch, 6), w.z, dur, ease.z, policy, 0);
    v.w = nka_tween_float(a, id, nka__axis(ch, 7), w.w, dur, ease.w, policy, 0);
    return nka__vc(nka__from_space(v, space));
}

NKA_API void nka_set_anchor(struct nka_context *a, int space, struct nk_vec2 size)
{
    if (a && space >= 0 && space < NKA_ANCHOR_COUNT) a->anchors[space] = size;
}

NKA_API struct nk_vec2 nka_anchor(const struct nka_context *a, int space)
{
    return a && space >= 0 && space < NKA_ANCHOR_COUNT ? a->anchors[space] : nka__vec2(0, 0);
}

NKA_API float nka_tween_float_rel(struct nka_context *a, nk_hash id, nk_hash ch, float percent, float px_bias,
                                  float dur, struct nka_ease ease, int policy, int anchor, int axis)
{
    struct nk_vec2 s = nka_anchor(a, anchor);
    return nka_tween_float(a, id, ch, (axis ? s.y : s.x) * percent + px_bias, dur, ease, policy, 0);
}

NKA_API struct nk_vec2 nka_tween_vec2_rel(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_vec2 percent,
                                          struct nk_vec2 px_bias, float dur, struct nka_ease ease, int policy, int anchor)
{
    struct nk_vec2 s = nka_anchor(a, anchor);
    return nka_tween_vec2(a, id, ch, nka__vec2(s.x * percent.x + px_bias.x, s.y * percent.y + px_bias.y),
                          dur, ease, policy, nka__vec2(0, 0));
}

NKA_API struct nka_vec4 nka_tween_vec4_rel(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_vec4 percent,
                                           struct nka_vec4 px_bias, float dur, struct nka_ease ease, int policy, int anchor)
{
    struct nk_vec2 s = nka_anchor(a, anchor);
    return nka_tween_vec4(a, id, ch, nka__vec4(s.x * percent.x + px_bias.x, s.y * percent.y + px_bias.y,
                                               percent.z + px_bias.z, percent.w + px_bias.w),
                          dur, ease, policy, nka__vec4(0, 0, 0, 0));
}

NKA_API float nka_tween_float_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_float_resolver fn,
                                       void *user, float dur, struct nka_ease ease, int policy)
{
    return nka_tween_float(a, id, ch, fn ? fn(user) : 0, dur, ease, policy, 0);
}

NKA_API struct nk_vec2 nka_tween_vec2_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_vec2_resolver fn,
                                               void *user, float dur, struct nka_ease ease, int policy)
{
    return nka_tween_vec2(a, id, ch, fn ? fn(user) : nka__vec2(0, 0), dur, ease, policy, nka__vec2(0, 0));
}

NKA_API struct nka_vec4 nka_tween_vec4_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_vec4_resolver fn,
                                                void *user, float dur, struct nka_ease ease, int policy)
{
    struct nka_vec4 zero = nka__vec4(0, 0, 0, 0);
    return nka_tween_vec4(a, id, ch, fn ? fn(user) : zero, dur, ease, policy, zero);
}

NKA_API struct nk_colorf nka_tween_color_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_color_resolver fn,
                                                  void *user, float dur, struct nka_ease ease, int policy, int space)
{
    struct nk_colorf white = nka__vc(nka__vec4(1, 1, 1, 1));
    return nka_tween_color(a, id, ch, fn ? fn(user) : white, dur, ease, policy, space, white);
}

NKA_API int nka_tween_int_resolved(struct nka_context *a, nk_hash id, nk_hash ch, nka_int_resolver fn,
                                   void *user, float dur, struct nka_ease ease, int policy)
{
    return nka_tween_int(a, id, ch, fn ? fn(user) : 0, dur, ease, policy, 0);
}

NKA_API float nka_tween_progress(const struct nka_context *a, nk_hash id, nk_hash ch)
{
    int kind;
    if (!a) return -1;
    for (kind = NKA__FLOAT; kind <= NKA__COLOR; ++kind) {
        const struct nka__chan *c = (const struct nka__chan *)nka__get(&a->chans, nka__key(id, ch, kind));
        if (c && !c->sleeping) return nka__clamp01((float)((a->time - c->start) / c->dur));
    }
    return -1;
}

static void nka__rebase(struct nka_context *a, nk_hash id, nk_hash ch, int kind, const float *target)
{
    struct nka__chan *c;
    float t, left;
    int i;
    if (!a) return;
    c = (struct nka__chan *)nka__get(&a->chans, nka__key(id, ch, kind));
    if (!c) return;
    t = c->sleeping ? 1 : nka__clamp01((float)((a->time - c->start) / c->dur));
    left = (1 - t) * c->dur;
    nka__chan_eval(a, c);
    for (i = 0; i < 4; ++i) {
        c->from[i] = left > NKA__MIN_DUR ? c->cur[i] : target[i];
        c->cur[i] = c->from[i];
        c->to[i] = c->req[i] = target[i];
    }
    c->start = a->time;
    c->dur = left > NKA__MIN_DUR ? left : NKA__MIN_DUR;
    c->sleeping = (unsigned char)(left <= NKA__MIN_DUR);
}

NKA_API void nka_rebase_float(struct nka_context *a, nk_hash id, nk_hash ch, float target)
{
    float t[4] = { 0 };
    t[0] = target;
    nka__rebase(a, id, ch, NKA__FLOAT, t);
}

NKA_API void nka_rebase_vec2(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_vec2 target)
{
    float t[4] = { 0 };
    t[0] = target.x; t[1] = target.y;
    nka__rebase(a, id, ch, NKA__VEC2, t);
}

NKA_API void nka_rebase_vec4(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_vec4 target)
{
    float t[4];
    t[0] = target.x; t[1] = target.y; t[2] = target.z; t[3] = target.w;
    nka__rebase(a, id, ch, NKA__VEC4, t);
}

NKA_API void nka_rebase_color(struct nka_context *a, nk_hash id, nk_hash ch, struct nk_colorf target)
{
    float t[4];
    t[0] = target.r; t[1] = target.g; t[2] = target.b; t[3] = target.a;
    nka__rebase(a, id, ch, NKA__COLOR, t);
}

NKA_API void nka_rebase_int(struct nka_context *a, nk_hash id, nk_hash ch, int target)
{
    float t[4] = { 0 };
    t[0] = (float)target;
    nka__rebase(a, id, ch, NKA__INT, t);
}

/* ImAnimate. The state lives in the value: a change of direction or range
 * picks up at the time where the curve passes through it. */

static float nka__unease(const struct nka_context *a, struct nka_ease e, float y)
{
    float lo = 0, hi = 1;
    int i;
    for (i = 0; i < 24; ++i) {
        float mid = (lo + hi) / 2;
        if (nka_eval(a, e, mid) < y) lo = mid;
        else hi = mid;
    }
    return (lo + hi) / 2;
}

NKA_API void nka_animate(struct nka_context *a, float from, float to, float ms, float *value, int curve)
{
    struct nka__anim *s;
    struct nka_ease e = nka_ease(curve);
    float lo = from < to ? from : to, hi = from < to ? to : from, v, p;
    double now;
    if (!a || !value) return;
    v = *value;
    s = (struct nka__anim *)nka__put(a, &a->anims, nka__ptr_key(value), 0);
    if (!s) { *value = to; return; }
    s->seen = a->frame;
    if (v < lo || v > hi) { s->running = 0; *value = v < lo ? lo : hi; return; }
    if (v == to || ms <= 0) { s->running = 0; *value = to; return; }
    now = a->time * 1000;
    if (!s->running || s->from != from || s->to != to) {
        s->start = now - ms * nka__unease(a, e, (v - from) / (to - from));
        s->from = from;
        s->to = to;
        s->running = 1;
    }
    p = (float)((now - s->start) / ms);
    if (p >= 1) { s->running = 0; *value = to; return; }
    v = from + (to - from) * nka_eval(a, e, p);
    *value = v < lo ? lo : (v > hi ? hi : v);
    a->busy = 1;
}

/* Motion paths */

NKA_API struct nk_vec2 nka_bezier_quadratic(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, float t)
{
    float u = 1 - t, uu = u * u, ut2 = 2 * u * t, tt = t * t;
    return nka__vec2(uu * p0.x + ut2 * p1.x + tt * p2.x, uu * p0.y + ut2 * p1.y + tt * p2.y);
}

NKA_API struct nk_vec2 nka_bezier_quadratic_deriv(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, float t)
{
    float u = 1 - t;
    return nka__vec2(2 * u * (p1.x - p0.x) + 2 * t * (p2.x - p1.x), 2 * u * (p1.y - p0.y) + 2 * t * (p2.y - p1.y));
}

NKA_API struct nk_vec2 nka_bezier_cubic(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, struct nk_vec2 p3, float t)
{
    float u = 1 - t, uu = u * u, tt = t * t;
    float k0 = uu * u, k1 = 3 * uu * t, k2 = 3 * u * tt, k3 = tt * t;
    return nka__vec2(k0 * p0.x + k1 * p1.x + k2 * p2.x + k3 * p3.x, k0 * p0.y + k1 * p1.y + k2 * p2.y + k3 * p3.y);
}

NKA_API struct nk_vec2 nka_bezier_cubic_deriv(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, struct nk_vec2 p3, float t)
{
    float u = 1 - t, uu = u * u, tt = t * t;
    float k0 = 3 * uu, k1 = 6 * u * t, k2 = 3 * tt;
    return nka__vec2(k0 * (p1.x - p0.x) + k1 * (p2.x - p1.x) + k2 * (p3.x - p2.x),
                     k0 * (p1.y - p0.y) + k1 * (p2.y - p1.y) + k2 * (p3.y - p2.y));
}

NKA_API struct nk_vec2 nka_catmull_rom(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, struct nk_vec2 p3,
                                       float t, float tension)
{
    float t2 = t * t, t3 = t2 * t, s = (1 - tension) / 2;
    float h1 = -s * t3 + 2 * s * t2 - s * t;
    float h2 = (2 - s) * t3 + (s - 3) * t2 + 1;
    float h3 = (s - 2) * t3 + (3 - 2 * s) * t2 + s * t;
    float h4 = s * t3 - s * t2;
    return nka__vec2(h1 * p0.x + h2 * p1.x + h3 * p2.x + h4 * p3.x, h1 * p0.y + h2 * p1.y + h3 * p2.y + h4 * p3.y);
}

NKA_API struct nk_vec2 nka_catmull_rom_deriv(struct nk_vec2 p0, struct nk_vec2 p1, struct nk_vec2 p2, struct nk_vec2 p3,
                                             float t, float tension)
{
    float t2 = t * t, s = (1 - tension) / 2;
    float d1 = -3 * s * t2 + 4 * s * t - s;
    float d2 = 3 * (2 - s) * t2 + 2 * (s - 3) * t;
    float d3 = 3 * (s - 2) * t2 + 2 * (3 - 2 * s) * t + s;
    float d4 = 3 * s * t2 - 2 * s * t;
    return nka__vec2(d1 * p0.x + d2 * p1.x + d3 * p2.x + d4 * p3.x, d1 * p0.y + d2 * p1.y + d3 * p2.y + d4 * p3.y);
}

/* A bijection, so ids 0 and 1 stay apart in a map that reads key 0 as 1. */
static nk_hash nka__path_key(nk_hash id) { return nka__fmix(id ^ NKA__PATH_KEY); }

static struct nka__path_data *nka__path_get(const struct nka_context *a, nk_hash id)
{
    return a ? (struct nka__path_data *)nka__get(&a->path_map, nka__path_key(id)) : 0;
}

static struct nk_vec2 nka__path_seg_at(const struct nka__path_seg *s, float t)
{
    switch (s->type) {
    case NKA_SEG_LINE: return nka__vec2(nka__lerp(s->p[0].x, s->p[1].x, t), nka__lerp(s->p[0].y, s->p[1].y, t));
    case NKA_SEG_QUADRATIC_BEZIER: return nka_bezier_quadratic(s->p[0], s->p[1], s->p[2], t);
    case NKA_SEG_CUBIC_BEZIER: return nka_bezier_cubic(s->p[0], s->p[1], s->p[2], s->p[3], t);
    default: return nka_catmull_rom(s->p[0], s->p[1], s->p[2], s->p[3], t, s->tension);
    }
}

static struct nk_vec2 nka__path_seg_deriv(const struct nka__path_seg *s, float t)
{
    switch (s->type) {
    case NKA_SEG_LINE: return nka__vec2(s->p[1].x - s->p[0].x, s->p[1].y - s->p[0].y);
    case NKA_SEG_QUADRATIC_BEZIER: return nka_bezier_quadratic_deriv(s->p[0], s->p[1], s->p[2], t);
    case NKA_SEG_CUBIC_BEZIER: return nka_bezier_cubic_deriv(s->p[0], s->p[1], s->p[2], s->p[3], t);
    default: return nka_catmull_rom_deriv(s->p[0], s->p[1], s->p[2], s->p[3], t, s->tension);
    }
}

static float nka__path_dist(struct nk_vec2 p, struct nk_vec2 q)
{
    float dx = q.x - p.x, dy = q.y - p.y;
    return nka__sqrt(dx * dx + dy * dy);
}

static float nka__path_seg_len(const struct nka__path_seg *s)
{
    struct nk_vec2 prev = nka__path_seg_at(s, 0), cur;
    float len = 0;
    int i;
    for (i = 1; i <= 16; ++i) {
        cur = nka__path_seg_at(s, (float)i / 16);
        len += nka__path_dist(prev, cur);
        prev = cur;
    }
    return len;
}

static int nka__path_find(const struct nka__path_data *p, float t, float *local)
{
    float target = t * p->seg_len, acc = 0;
    int i;
    *local = 0;
    if (!p->n) return -1;
    if (t <= 0) return 0;
    if (t < 1) {
        for (i = 0; i < p->n; ++i) {
            float len = p->segs[i].length;
            if (acc + len >= target) {
                if (len > 0) *local = (target - acc) / len;
                return i;
            }
            acc += len;
        }
    }
    *local = 1;
    return p->n - 1;
}

static struct nk_vec2 nka__path_at(const struct nka__path_data *p, float t)
{
    float lt;
    int i = nka__path_find(p, t, &lt);
    return i < 0 ? p->start : nka__path_seg_at(&p->segs[i], lt);
}

/* Where the derivative vanishes, as at a control point lying on its end point, the
 * direction is the derivative's just inside the segment. */
static struct nk_vec2 nka__path_dir(const struct nka__path_data *p, float t)
{
    struct nk_vec2 d = nka__vec2(1, 0);
    float lt, h, len = 1;
    int i = nka__path_find(p, t, &lt);
    if (i >= 0) {
        d = nka__path_seg_deriv(&p->segs[i], lt);
        len = nka__sqrt(d.x * d.x + d.y * d.y);
        for (h = 1e-3f; len <= 1e-6f && h < 0.5f; h *= 10) {
            d = nka__path_seg_deriv(&p->segs[i], lt < 0.5f ? lt + h : lt - h);
            len = nka__sqrt(d.x * d.x + d.y * d.y);
        }
    }
    return len > 1e-6f ? nka__vec2(d.x / len, d.y / len) : nka__vec2(1, 0);
}

static void nka__path_lut(struct nka_context *a, struct nka__path_data *p, int sub)
{
    struct nk_vec2 prev, cur;
    float acc = 0;
    int i;
    if (sub < 1) sub = 64;
    if (sub > 1 << 24) sub = 1 << 24;
    p->lut_n = 0;
    p->length = p->seg_len;
    if (!p->n || p->seg_len <= 0) return;
    if (sub + 1 > p->lut_cap) {
        float *lut = (float *)nka__alloc(a, (nk_size)(sub + 1) * sizeof(float));
        if (!lut) return;
        nka__free(a, p->lut);
        p->lut = lut;
        p->lut_cap = sub + 1;
    }
    p->lut[0] = 0;
    prev = nka__path_at(p, 0);
    for (i = 1; i <= sub; ++i) {
        cur = nka__path_at(p, (float)i / (float)sub);
        acc += nka__path_dist(prev, cur);
        p->lut[i] = acc;
        prev = cur;
    }
    if (acc <= 0) return;
    p->lut_n = sub + 1;
    p->length = acc;
}

static float nka__path_d2t(const struct nka__path_data *p, float d)
{
    int lo = 0, hi = p->lut_n - 1;
    float d0, d1, t0, t1;
    if (p->lut_n < 2) return p->length > 0 ? nka__clamp01(d / p->length) : 0;
    if (d <= 0) return 0;
    if (d >= p->length) return 1;
    while (lo < hi - 1) {
        int mid = (lo + hi) / 2;
        if (p->lut[mid] < d) lo = mid;
        else hi = mid;
    }
    d0 = p->lut[lo];
    d1 = p->lut[hi];
    t0 = (float)lo / (float)(p->lut_n - 1);
    t1 = (float)hi / (float)(p->lut_n - 1);
    if (d1 - d0 <= 0) return t0;
    return t0 + (t1 - t0) * ((d - d0) / (d1 - d0));
}

static struct nka__path_seg *nka__path_push(struct nka_context *a, int type, struct nk_vec2 end)
{
    struct nka__path_data *b;
    struct nka__path_seg *s;
    if (!a || !a->path_building || a->path_failed) return 0;
    b = &a->path_new;
    if (b->n == b->cap) {
        int cap = b->cap ? b->cap * 2 : 8;
        s = (struct nka__path_seg *)nka__alloc(a, (nk_size)cap * sizeof *s);
        if (!s) {
            a->path_failed = 1;
            return 0;
        }
        if (b->n) nka__copy(s, b->segs, (nk_size)b->n * sizeof *s);
        nka__free(a, b->segs);
        b->segs = s;
        b->cap = cap;
    }
    s = &b->segs[b->n++];
    nka__zero(s, sizeof *s);
    s->type = type;
    s->p[0] = a->path_cur;
    a->path_prev = a->path_cur;
    a->path_cur = end;
    return s;
}

NKA_API void nka_path_begin(struct nka_context *a, nk_hash path_id, struct nk_vec2 start)
{
    if (!a) return;
    a->path_building = 1;
    a->path_failed = 0;
    a->path_id = path_id;
    a->path_new.n = 0;
    a->path_new.start = start;
    a->path_cur = a->path_prev = start;
}

NKA_API void nka_path_line_to(struct nka_context *a, struct nk_vec2 end)
{
    struct nka__path_seg *s = nka__path_push(a, NKA_SEG_LINE, end);
    if (s) s->p[1] = end;
}

NKA_API void nka_path_quadratic_to(struct nka_context *a, struct nk_vec2 ctrl, struct nk_vec2 end)
{
    struct nka__path_seg *s = nka__path_push(a, NKA_SEG_QUADRATIC_BEZIER, end);
    if (!s) return;
    s->p[1] = ctrl;
    s->p[2] = end;
}

NKA_API void nka_path_cubic_to(struct nka_context *a, struct nk_vec2 ctrl1, struct nk_vec2 ctrl2, struct nk_vec2 end)
{
    struct nka__path_seg *s = nka__path_push(a, NKA_SEG_CUBIC_BEZIER, end);
    if (!s) return;
    s->p[1] = ctrl1;
    s->p[2] = ctrl2;
    s->p[3] = end;
}

NKA_API void nka_path_catmull_to(struct nka_context *a, struct nk_vec2 end, float tension)
{
    struct nka__path_seg *s;
    struct nk_vec2 before;
    if (!a) return;
    before = a->path_new.n ? a->path_prev : a->path_cur;
    s = nka__path_push(a, NKA_SEG_CATMULL_ROM, end);
    if (!s) return;
    s->p[1] = s->p[0];
    s->p[0] = before;
    s->p[2] = s->p[3] = end;
    s->tension = tension;
    if (a->path_new.n >= 2 && a->path_new.segs[a->path_new.n - 2].type == NKA_SEG_CATMULL_ROM)
        a->path_new.segs[a->path_new.n - 2].p[3] = end;
}

NKA_API void nka_path_close(struct nka_context *a)
{
    if (!a || !a->path_building) return;
    if (a->path_cur.x != a->path_new.start.x || a->path_cur.y != a->path_new.start.y)
        nka_path_line_to(a, a->path_new.start);
}

/* The path takes the builder's array and leaves its old one for the next build. */
NKA_API void nka_path_end(struct nka_context *a)
{
    struct nka__path_data *b, *p;
    struct nka__path_seg *segs;
    int i, cap;
    if (!a || !a->path_building) return;
    a->path_building = 0;
    b = &a->path_new;
    p = a->path_failed ? 0 : (struct nka__path_data *)nka__put(a, &a->path_map, nka__path_key(a->path_id), 0);
    if (!p) {
        b->n = 0;
        return;
    }
    b->seg_len = 0;
    for (i = 0; i < b->n; ++i) {
        b->segs[i].length = nka__path_seg_len(&b->segs[i]);
        b->seg_len += b->segs[i].length;
    }
    segs = p->segs;
    cap = p->cap;
    p->segs = b->segs;
    p->cap = b->cap;
    p->n = b->n;
    p->start = b->start;
    p->seg_len = p->length = b->seg_len;
    p->lut_n = 0;
    b->segs = segs;
    b->cap = cap;
    b->n = 0;
}

NKA_API int nka_path_exists(const struct nka_context *a, nk_hash path_id) { return nka__path_get(a, path_id) != 0; }

NKA_API float nka_path_length(const struct nka_context *a, nk_hash path_id)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    return p ? p->length : 0;
}

NKA_API struct nk_vec2 nka_path_evaluate(const struct nka_context *a, nk_hash path_id, float t)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    return p ? nka__path_at(p, t) : nka__vec2(0, 0);
}

NKA_API struct nk_vec2 nka_path_tangent(const struct nka_context *a, nk_hash path_id, float t)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    return p ? nka__path_dir(p, t) : nka__vec2(1, 0);
}

NKA_API float nka_path_angle(const struct nka_context *a, nk_hash path_id, float t)
{
    struct nk_vec2 d = nka_path_tangent(a, path_id, t);
    return nka__atan2(d.y, d.x);
}

NKA_API void nka_path_build_arc_lut(struct nka_context *a, nk_hash path_id, int subdivisions)
{
    struct nka__path_data *p = nka__path_get(a, path_id);
    if (p) nka__path_lut(a, p, subdivisions);
}

NKA_API int nka_path_has_arc_lut(const struct nka_context *a, nk_hash path_id)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    return p && p->lut_n >= 2;
}

NKA_API float nka_path_distance_to_t(const struct nka_context *a, nk_hash path_id, float distance)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    return p ? nka__path_d2t(p, distance) : 0;
}

NKA_API struct nk_vec2 nka_path_evaluate_at_distance(const struct nka_context *a, nk_hash path_id, float distance)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    return p ? nka__path_at(p, nka__path_d2t(p, distance)) : nka__vec2(0, 0);
}

NKA_API struct nk_vec2 nka_path_tangent_at_distance(const struct nka_context *a, nk_hash path_id, float distance)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    return p ? nka__path_dir(p, nka__path_d2t(p, distance)) : nka__vec2(1, 0);
}

NKA_API float nka_path_angle_at_distance(const struct nka_context *a, nk_hash path_id, float distance)
{
    struct nk_vec2 d = nka_path_tangent_at_distance(a, path_id, distance);
    return nka__atan2(d.y, d.x);
}

static nk_hash nka__path_ch(nk_hash ch, nk_hash salt) { return nka_id_mix(ch, salt); }

/* Additive from 0 adds 1 a single time and, unlike a crossfade, does not start over
 * when the ease changes: ImAnim's progress along a path. */
static float nka__path_progress(struct nka_context *a, nk_hash id, nk_hash ch, float dur, struct nka_ease ease)
{
    return nka_tween_float(a, id, ch, 1, dur, ease, NKA_POLICY_ADDITIVE, 0);
}

static float nka__path_arc_t(const struct nka__path_data *p, float t, int arc)
{
    return arc && p->lut_n >= 2 ? nka__path_d2t(p, t * p->length) : t;
}

NKA_API struct nk_vec2 nka_tween_path(struct nka_context *a, nk_hash id, nk_hash ch, nk_hash path_id, float dur,
                                      struct nka_ease ease, int policy)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    float k;
    (void)policy;
    if (!p || !p->n) return nka__vec2(0, 0);
    k = nka__path_progress(a, id, nka__path_ch(ch, NKA__PATH_POS), dur, ease);
    return nka__path_at(p, nka__path_arc_t(p, k, 1));
}

NKA_API float nka_tween_path_angle(struct nka_context *a, nk_hash id, nk_hash ch, nk_hash path_id, float dur,
                                   struct nka_ease ease, int policy)
{
    const struct nka__path_data *p = nka__path_get(a, path_id);
    struct nk_vec2 d;
    float k;
    (void)policy;
    if (!p || !p->n) return 0;
    k = nka__path_progress(a, id, nka__path_ch(ch, NKA__PATH_ANGLE), dur, ease);
    d = nka__path_dir(p, nka__path_arc_t(p, k, 1));
    return nka__atan2(d.y, d.x);
}

/* Morphing */

NKA_API struct nka_morph_opts nka_morph_opts_default(void)
{
    struct nka_morph_opts o;
    o.samples = 64;
    o.match_endpoints = 1;
    o.use_arc_length = 1;
    return o;
}

/* 0 to 1, and 0 for NaN, which reaches a float to int conversion below. */
static float nka__path_sat(float v) { return v > 0 ? (v < 1 ? v : 1) : 0; }

static struct nk_vec2 nka__path_mix(struct nk_vec2 p, struct nk_vec2 q, float k)
{
    return nka__vec2(p.x + (q.x - p.x) * k, p.y + (q.y - p.y) * k);
}

static struct nka__path_data *nka__path_morph_get(struct nka_context *a, nk_hash id, int arc)
{
    struct nka__path_data *p = nka__path_get(a, id);
    if (p && arc && p->lut_n < 2) nka__path_lut(a, p, 64);
    return p;
}

/* Point i of the n that ImAnim resamples a path to; a missing or empty path is the origin. */
static struct nk_vec2 nka__path_sample(const struct nka__path_data *p, int i, int n, int arc)
{
    if (!p || !p->n) return nka__vec2(0, 0);
    return nka__path_at(p, nka__path_arc_t(p, (float)i / (float)(n - 1), arc));
}

/* Only the paths the blend shows. Alone at blend 0 or 1, a path is still timed by arc
 * length when asked, or the morph would jump as the blend leaves either end. */
static void nka__path_pair_of(struct nka_context *a, nk_hash path_a, nk_hash path_b, float blend,
                              const struct nka_morph_opts *opts, struct nka__path_pair *m)
{
    struct nka_morph_opts o = opts ? *opts : nka_morph_opts_default();
    m->n = o.samples > 2 ? o.samples : 2;
    m->arc = o.use_arc_length != 0;
    m->a = blend < 1 ? nka__path_morph_get(a, path_a, m->arc) : 0;
    m->b = blend > 0 ? nka__path_morph_get(a, path_b, m->arc) : 0;
}

static struct nk_vec2 nka__path_morph_at(const struct nka__path_pair *m, float t, float blend)
{
    float s, f;
    int i, n = m->n;
    if (t <= 0) return nka__path_mix(nka__path_sample(m->a, 0, n, m->arc), nka__path_sample(m->b, 0, n, m->arc), blend);
    if (t >= 1)
        return nka__path_mix(nka__path_sample(m->a, n - 1, n, m->arc), nka__path_sample(m->b, n - 1, n, m->arc), blend);
    s = t * (float)(n - 1);
    i = (int)s;
    f = s - (float)i;
    if (i >= n - 1) {
        i = n - 2;
        f = 1;
    }
    return nka__path_mix(nka__path_mix(nka__path_sample(m->a, i, n, m->arc), nka__path_sample(m->a, i + 1, n, m->arc), f),
                         nka__path_mix(nka__path_sample(m->b, i, n, m->arc), nka__path_sample(m->b, i + 1, n, m->arc), f),
                         blend);
}

NKA_API struct nk_vec2 nka_path_morph(struct nka_context *a, nk_hash path_a, nk_hash path_b, float t, float blend,
                                      const struct nka_morph_opts *opts)
{
    struct nka__path_pair m;
    const struct nka__path_data *p;
    t = nka__path_sat(t);
    blend = nka__path_sat(blend);
    nka__path_pair_of(a, path_a, path_b, blend, opts, &m);
    if (blend > 0 && blend < 1) return nka__path_morph_at(&m, t, blend);
    p = blend <= 0 ? m.a : m.b;
    return p ? nka__path_at(p, nka__path_arc_t(p, t, m.arc)) : nka__vec2(0, 0);
}

NKA_API struct nk_vec2 nka_path_morph_tangent(struct nka_context *a, nk_hash path_a, nk_hash path_b, float t, float blend,
                                              const struct nka_morph_opts *opts)
{
    struct nka__path_pair m;
    const struct nka__path_data *p;
    struct nk_vec2 p0, p1, d;
    float len;
    t = nka__path_sat(t);
    blend = nka__path_sat(blend);
    nka__path_pair_of(a, path_a, path_b, blend, opts, &m);
    if (blend <= 0 || blend >= 1) {
        p = blend <= 0 ? m.a : m.b;
        return p ? nka__path_dir(p, nka__path_arc_t(p, t, m.arc)) : nka__vec2(1, 0);
    }
    p0 = nka__path_morph_at(&m, t - 0.001f > 0 ? t - 0.001f : 0, blend);
    p1 = nka__path_morph_at(&m, t + 0.001f < 1 ? t + 0.001f : 1, blend);
    d = nka__vec2(p1.x - p0.x, p1.y - p0.y);
    len = nka__sqrt(d.x * d.x + d.y * d.y);
    return len > 1e-6f ? nka__vec2(d.x / len, d.y / len) : nka__vec2(1, 0);
}

NKA_API float nka_path_morph_angle(struct nka_context *a, nk_hash path_a, nk_hash path_b, float t, float blend,
                                   const struct nka_morph_opts *opts)
{
    struct nk_vec2 d = nka_path_morph_tangent(a, path_a, path_b, t, blend, opts);
    return nka__atan2(d.y, d.x);
}

NKA_API struct nk_vec2 nka_tween_path_morph(struct nka_context *a, nk_hash id, nk_hash ch, nk_hash path_a, nk_hash path_b,
                                            float target_blend, float dur, struct nka_ease path_ease,
                                            struct nka_ease morph_ease, int policy, const struct nka_morph_opts *opts)
{
    float k, blend;
    if (!a) return nka__vec2(0, 0);
    k = nka__path_progress(a, id, nka__path_ch(ch, NKA__PATH_MORPH), dur, path_ease);
    blend = nka_tween_float(a, id, nka__path_ch(ch, NKA__PATH_BLEND), target_blend, dur, morph_ease,
                            policy == NKA_POLICY_CUT ? NKA_POLICY_CUT : NKA_POLICY_CROSSFADE, 0);
    return nka_path_morph(a, path_a, path_b, k, blend, opts);
}

NKA_API float nka_get_morph_blend(const struct nka_context *a, nk_hash id, nk_hash ch)
{
    const struct nka__chan *c;
    if (!a) return 0;
    c = (const struct nka__chan *)nka__get(&a->chans, nka__key(id, nka__path_ch(ch, NKA__PATH_BLEND), NKA__FLOAT));
    return c ? c->cur[0] : 0;
}

NKA_API void nka_transform_quad(struct nk_vec2 *quad, struct nk_vec2 center, float angle, struct nk_vec2 translation)
{
    float c = nka__cos(angle), s = nka__sin(angle);
    int i;
    if (!quad) return;
    for (i = 0; i < 4; ++i) {
        float x = quad[i].x - center.x, y = quad[i].y - center.y;
        quad[i].x = x * c - y * s + center.x + translation.x;
        quad[i].y = x * s + y * c + center.y + translation.y;
    }
}

/* Up, away from the path, is (sin, -cos) on a y-down screen. */
NKA_API void nka_make_glyph_quad(struct nk_vec2 *quad, struct nk_vec2 pos, float angle, float glyph_width,
                                 float glyph_height, float baseline_offset)
{
    float c = nka__cos(angle), s = nka__sin(angle), hw = glyph_width * 0.5f;
    float ox = s * baseline_offset, oy = -c * baseline_offset, ux = s * glyph_height, uy = -c * glyph_height;
    if (!quad) return;
    quad[0] = nka__vec2(pos.x - c * hw + ox, pos.y - s * hw + oy);
    quad[1] = nka__vec2(pos.x + c * hw + ox, pos.y + s * hw + oy);
    quad[2] = nka__vec2(pos.x + c * hw + ox + ux, pos.y + s * hw + oy + uy);
    quad[3] = nka__vec2(pos.x - c * hw + ox + ux, pos.y - s * hw + oy + uy);
}

static void nka__path_free(struct nka_context *a)
{
    int i;
    for (i = 0; i < a->path_map.cap; ++i) {
        struct nka__path_data *p = (struct nka__path_data *)nka__at(&a->path_map, i);
        if (!a->path_map.keys[i]) continue;
        nka__free(a, p->segs);
        nka__free(a, p->lut);
    }
    nka__map_free(a, &a->path_map);
    nka__free(a, a->path_new.segs);
}

/* Clips */

static float nka__clip_clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static int nka__clip_toint(float f)
{
    if (f != f) return 0;
    if (f >= 2147483520.0f) return 2147483520;
    if (f <= -2147483648.0f) return -NKA__CLIP_INT_MAX - 1;
    return (int)f;
}

static int nka__clip_s32(nk_uint u) { return u <= 0x7fffffffu ? (int)u : -(int)~u - 1; }

/* x to y rounded half up, in doubles so ints of any size land exactly on their keys. */
static int nka__clip_lerpi(int x, int y, float w)
{
    double d = ((double)y - (double)x) * (double)w + 0.5;
    long long v;
    if (!(d > -1e17 && d < 1e17)) return w > 0.5f ? y : x;
    v = (long long)d;
    if ((double)v > d) --v;
    v += x;
    return (int)(v > NKA__CLIP_INT_MAX ? NKA__CLIP_INT_MAX : (v < -NKA__CLIP_INT_MAX - 1 ? -NKA__CLIP_INT_MAX - 1 : v));
}

NKA_API struct nka_variation_float nka_varf_none(void)
{
    struct nka_variation_float v;
    v.mode = NKA_VAR_NONE;
    v.amount = 0;
    v.min_clamp = -NKA__CLIP_FLT_MAX;
    v.max_clamp = NKA__CLIP_FLT_MAX;
    v.seed = 0;
    v.callback = 0;
    v.user = 0;
    return v;
}

static struct nka_variation_float nka__clip_varf_of(int mode, float amount)
{
    struct nka_variation_float v = nka_varf_none();
    v.mode = mode;
    v.amount = amount;
    return v;
}

NKA_API struct nka_variation_float nka_varf_inc(float amount) { return nka__clip_varf_of(NKA_VAR_INCREMENT, amount); }
NKA_API struct nka_variation_float nka_varf_dec(float amount) { return nka__clip_varf_of(NKA_VAR_DECREMENT, amount); }
NKA_API struct nka_variation_float nka_varf_mul(float factor) { return nka__clip_varf_of(NKA_VAR_MULTIPLY, factor); }
NKA_API struct nka_variation_float nka_varf_rand(float range) { return nka__clip_varf_of(NKA_VAR_RANDOM, range); }
NKA_API struct nka_variation_float nka_varf_rand_abs(float range) { return nka__clip_varf_of(NKA_VAR_RANDOM_ABS, range); }
NKA_API struct nka_variation_float nka_varf_pingpong(float amount) { return nka__clip_varf_of(NKA_VAR_PINGPONG, amount); }

NKA_API struct nka_variation_float nka_varf_fn(nka_variation_float_fn fn, void *user)
{
    struct nka_variation_float v = nka__clip_varf_of(NKA_VAR_CALLBACK, 0);
    v.callback = fn;
    v.user = user;
    return v;
}

NKA_API struct nka_variation_float nka_varf_clamp(struct nka_variation_float v, float lo, float hi)
{
    v.min_clamp = lo;
    v.max_clamp = hi;
    return v;
}

NKA_API struct nka_variation_float nka_varf_seed(struct nka_variation_float v, nk_uint seed) { v.seed = seed; return v; }

NKA_API struct nka_variation_int nka_vari_none(void)
{
    struct nka_variation_int v;
    v.mode = NKA_VAR_NONE;
    v.amount = 0;
    v.min_clamp = -NKA__CLIP_INT_MAX - 1;
    v.max_clamp = NKA__CLIP_INT_MAX;
    v.seed = 0;
    v.callback = 0;
    v.user = 0;
    return v;
}

static struct nka_variation_int nka__clip_vari_of(int mode, int amount)
{
    struct nka_variation_int v = nka_vari_none();
    v.mode = mode;
    v.amount = amount;
    return v;
}

NKA_API struct nka_variation_int nka_vari_inc(int amount) { return nka__clip_vari_of(NKA_VAR_INCREMENT, amount); }
NKA_API struct nka_variation_int nka_vari_dec(int amount) { return nka__clip_vari_of(NKA_VAR_DECREMENT, amount); }
NKA_API struct nka_variation_int nka_vari_rand(int range) { return nka__clip_vari_of(NKA_VAR_RANDOM, range); }

NKA_API struct nka_variation_int nka_vari_fn(nka_variation_int_fn fn, void *user)
{
    struct nka_variation_int v = nka__clip_vari_of(NKA_VAR_CALLBACK, 0);
    v.callback = fn;
    v.user = user;
    return v;
}

NKA_API struct nka_variation_int nka_vari_clamp(struct nka_variation_int v, int lo, int hi)
{
    v.min_clamp = lo;
    v.max_clamp = hi;
    return v;
}

NKA_API struct nka_variation_int nka_vari_seed(struct nka_variation_int v, nk_uint seed) { v.seed = seed; return v; }

NKA_API struct nka_variation_vec2 nka_varv2_none(void)
{
    struct nka_variation_vec2 v;
    v.mode = NKA_VAR_NONE;
    v.amount = nka__vec2(0, 0);
    v.min_clamp = nka__vec2(-NKA__CLIP_FLT_MAX, -NKA__CLIP_FLT_MAX);
    v.max_clamp = nka__vec2(NKA__CLIP_FLT_MAX, NKA__CLIP_FLT_MAX);
    v.seed = 0;
    v.callback = 0;
    v.user = 0;
    v.x = v.y = nka_varf_none();
    return v;
}

static struct nka_variation_vec2 nka__clip_varv2_of(int mode, float x, float y)
{
    struct nka_variation_vec2 v = nka_varv2_none();
    v.mode = mode;
    v.amount = nka__vec2(x, y);
    return v;
}

NKA_API struct nka_variation_vec2 nka_varv2_inc(float x, float y) { return nka__clip_varv2_of(NKA_VAR_INCREMENT, x, y); }
NKA_API struct nka_variation_vec2 nka_varv2_dec(float x, float y) { return nka__clip_varv2_of(NKA_VAR_DECREMENT, x, y); }
NKA_API struct nka_variation_vec2 nka_varv2_mul(float factor) { return nka__clip_varv2_of(NKA_VAR_MULTIPLY, factor, factor); }
NKA_API struct nka_variation_vec2 nka_varv2_rand(float x, float y) { return nka__clip_varv2_of(NKA_VAR_RANDOM, x, y); }

NKA_API struct nka_variation_vec2 nka_varv2_fn(nka_variation_vec2_fn fn, void *user)
{
    struct nka_variation_vec2 v = nka__clip_varv2_of(NKA_VAR_CALLBACK, 0, 0);
    v.callback = fn;
    v.user = user;
    return v;
}

NKA_API struct nka_variation_vec2 nka_varv2_axis(struct nka_variation_float x, struct nka_variation_float y)
{
    struct nka_variation_vec2 v = nka_varv2_none();
    v.x = x;
    v.y = y;
    return v;
}

NKA_API struct nka_variation_vec2 nka_varv2_clamp(struct nka_variation_vec2 v, struct nk_vec2 lo, struct nk_vec2 hi)
{
    v.min_clamp = lo;
    v.max_clamp = hi;
    return v;
}

NKA_API struct nka_variation_vec2 nka_varv2_seed(struct nka_variation_vec2 v, nk_uint seed) { v.seed = seed; return v; }

NKA_API struct nka_variation_vec4 nka_varv4_none(void)
{
    struct nka_variation_vec4 v;
    v.mode = NKA_VAR_NONE;
    v.amount = nka__vec4(0, 0, 0, 0);
    v.min_clamp = nka__vec4(-NKA__CLIP_FLT_MAX, -NKA__CLIP_FLT_MAX, -NKA__CLIP_FLT_MAX, -NKA__CLIP_FLT_MAX);
    v.max_clamp = nka__vec4(NKA__CLIP_FLT_MAX, NKA__CLIP_FLT_MAX, NKA__CLIP_FLT_MAX, NKA__CLIP_FLT_MAX);
    v.seed = 0;
    v.callback = 0;
    v.user = 0;
    v.x = v.y = v.z = v.w = nka_varf_none();
    return v;
}

static struct nka_variation_vec4 nka__clip_varv4_of(int mode, struct nka_vec4 amount)
{
    struct nka_variation_vec4 v = nka_varv4_none();
    v.mode = mode;
    v.amount = amount;
    return v;
}

NKA_API struct nka_variation_vec4 nka_varv4_inc(float x, float y, float z, float w)
{
    return nka__clip_varv4_of(NKA_VAR_INCREMENT, nka__vec4(x, y, z, w));
}

NKA_API struct nka_variation_vec4 nka_varv4_dec(float x, float y, float z, float w)
{
    return nka__clip_varv4_of(NKA_VAR_DECREMENT, nka__vec4(x, y, z, w));
}

NKA_API struct nka_variation_vec4 nka_varv4_mul(float factor)
{
    return nka__clip_varv4_of(NKA_VAR_MULTIPLY, nka__vec4(factor, factor, factor, factor));
}

NKA_API struct nka_variation_vec4 nka_varv4_rand(float x, float y, float z, float w)
{
    return nka__clip_varv4_of(NKA_VAR_RANDOM, nka__vec4(x, y, z, w));
}

NKA_API struct nka_variation_vec4 nka_varv4_fn(nka_variation_vec4_fn fn, void *user)
{
    struct nka_variation_vec4 v = nka__clip_varv4_of(NKA_VAR_CALLBACK, nka__vec4(0, 0, 0, 0));
    v.callback = fn;
    v.user = user;
    return v;
}

NKA_API struct nka_variation_vec4 nka_varv4_axis(struct nka_variation_float x, struct nka_variation_float y,
                                                 struct nka_variation_float z, struct nka_variation_float w)
{
    struct nka_variation_vec4 v = nka_varv4_none();
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

NKA_API struct nka_variation_vec4 nka_varv4_clamp(struct nka_variation_vec4 v, struct nka_vec4 lo, struct nka_vec4 hi)
{
    v.min_clamp = lo;
    v.max_clamp = hi;
    return v;
}

NKA_API struct nka_variation_vec4 nka_varv4_seed(struct nka_variation_vec4 v, nk_uint seed) { v.seed = seed; return v; }

NKA_API struct nka_variation_color nka_varc_none(void)
{
    struct nka_variation_color v;
    v.mode = NKA_VAR_NONE;
    v.amount = nka__vec4(0, 0, 0, 0);
    v.min_clamp = nka__vec4(-NKA__CLIP_FLT_MAX, -NKA__CLIP_FLT_MAX, -NKA__CLIP_FLT_MAX, -NKA__CLIP_FLT_MAX);
    v.max_clamp = nka__vec4(NKA__CLIP_FLT_MAX, NKA__CLIP_FLT_MAX, NKA__CLIP_FLT_MAX, NKA__CLIP_FLT_MAX);
    v.space = NKA_COL_OKLAB;
    v.seed = 0;
    v.callback = 0;
    v.user = 0;
    v.r = v.g = v.b = v.a = nka_varf_none();
    return v;
}

static struct nka_variation_color nka__clip_varc_of(int mode, struct nka_vec4 amount)
{
    struct nka_variation_color v = nka_varc_none();
    v.mode = mode;
    v.amount = amount;
    return v;
}

NKA_API struct nka_variation_color nka_varc_inc(float r, float g, float b, float alpha)
{
    return nka__clip_varc_of(NKA_VAR_INCREMENT, nka__vec4(r, g, b, alpha));
}

NKA_API struct nka_variation_color nka_varc_dec(float r, float g, float b, float alpha)
{
    return nka__clip_varc_of(NKA_VAR_DECREMENT, nka__vec4(r, g, b, alpha));
}

NKA_API struct nka_variation_color nka_varc_mul(float factor)
{
    return nka__clip_varc_of(NKA_VAR_MULTIPLY, nka__vec4(factor, factor, factor, 1));
}

NKA_API struct nka_variation_color nka_varc_rand(float r, float g, float b, float alpha)
{
    return nka__clip_varc_of(NKA_VAR_RANDOM, nka__vec4(r, g, b, alpha));
}

NKA_API struct nka_variation_color nka_varc_fn(nka_variation_vec4_fn fn, void *user)
{
    struct nka_variation_color v = nka__clip_varc_of(NKA_VAR_CALLBACK, nka__vec4(0, 0, 0, 0));
    v.callback = fn;
    v.user = user;
    return v;
}

NKA_API struct nka_variation_color nka_varc_channel(struct nka_variation_float r, struct nka_variation_float g,
                                                    struct nka_variation_float b, struct nka_variation_float alpha)
{
    struct nka_variation_color v = nka_varc_none();
    v.r = r;
    v.g = g;
    v.b = b;
    v.a = alpha;
    return v;
}

NKA_API struct nka_variation_color nka_varc_space(struct nka_variation_color v, int space) { v.space = space; return v; }

NKA_API struct nka_variation_color nka_varc_clamp(struct nka_variation_color v, struct nka_vec4 lo, struct nka_vec4 hi)
{
    v.min_clamp = lo;
    v.max_clamp = hi;
    return v;
}

NKA_API struct nka_variation_color nka_varc_seed(struct nka_variation_color v, nk_uint seed) { v.seed = seed; return v; }

/* Variations. A draw depends on the instance, the loop and the key, so it holds for a loop. */

static nk_uint nka__clip_xorshift(nk_uint *s)
{
    nk_uint x = *s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *s = x;
    return x;
}

static float nka__clip_unit(nk_uint *s) { return (float)(nka__clip_xorshift(s) & 0x7fffffffu) / (float)0x7fffffff; }
static float nka__clip_signed(nk_uint *s) { return nka__clip_unit(s) * 2 - 1; }

static nk_uint nka__clip_rng(nk_uint seed, int loop, nk_uint inst, nk_uint salt)
{
    return seed ? seed + (nk_uint)loop * 1664525u : nka_id_mix(nka_id_mix(inst, (nk_hash)loop), salt);
}

static float nka__clip_varf(const struct nka_variation_float *v, float base, int loop, nk_uint inst, nk_uint salt)
{
    nk_uint s;
    float d = 0;
    if (v->mode == NKA_VAR_NONE) return base;
    if (v->mode == NKA_VAR_CALLBACK && v->callback)
        return nka__clip_clampf(v->callback(loop, v->user), v->min_clamp, v->max_clamp);
    if (v->mode == NKA_VAR_MULTIPLY)
        return nka__clip_clampf(base * nka__pow(v->amount, (float)loop), v->min_clamp, v->max_clamp);
    s = nka__clip_rng(v->seed, loop, inst, salt);
    switch (v->mode) {
    case NKA_VAR_INCREMENT: d = v->amount * (float)loop; break;
    case NKA_VAR_DECREMENT: d = -v->amount * (float)loop; break;
    case NKA_VAR_RANDOM: d = nka__clip_signed(&s) * v->amount; break;
    case NKA_VAR_RANDOM_ABS: d = nka__clip_unit(&s) * v->amount; break;
    case NKA_VAR_PINGPONG:
        d = loop % 2 == 0 ? 0 : v->amount;
        if (loop % 4 >= 2) d = -d;
        break;
    default: break;
    }
    return nka__clip_clampf(base + d, v->min_clamp, v->max_clamp);
}

static int nka__clip_vari(const struct nka_variation_int *v, int base, int loop, nk_uint inst, nk_uint salt)
{
    nk_uint s;
    long long r = base;
    if (v->mode == NKA_VAR_NONE) return base;
    if (v->mode == NKA_VAR_CALLBACK && v->callback) r = v->callback(loop, v->user);
    else {
        s = nka__clip_rng(v->seed, loop, inst, salt);
        switch (v->mode) {
        case NKA_VAR_INCREMENT: r += (long long)v->amount * loop; break;
        case NKA_VAR_DECREMENT: r -= (long long)v->amount * loop; break;
        case NKA_VAR_MULTIPLY: r = nka__clip_toint((float)base * nka__pow((float)v->amount, (float)loop)); break;
        case NKA_VAR_RANDOM: r += nka__clip_toint(nka__clip_signed(&s) * (float)v->amount); break;
        case NKA_VAR_RANDOM_ABS: r += nka__clip_toint(nka__clip_unit(&s) * (float)v->amount); break;
        case NKA_VAR_PINGPONG:
            if (loop % 2) r += loop % 4 >= 2 ? -(long long)v->amount : v->amount;
            break;
        default: return base;
        }
    }
    return (int)(r < v->min_clamp ? v->min_clamp : (r > v->max_clamp ? v->max_clamp : r));
}

/* The modes shared by vec2, vec4 and color, on n components. */
static void nka__clip_varn(int mode, const float *amount, nk_uint seed, int loop, nk_uint inst, nk_uint salt, float *r, int n)
{
    nk_uint s = nka__clip_rng(seed, loop, inst, salt);
    float k = loop % 2 == 0 ? 0.0f : (loop % 4 >= 2 ? -1.0f : 1.0f);
    int i;
    for (i = 0; i < n; ++i) {
        switch (mode) {
        case NKA_VAR_INCREMENT: r[i] += amount[i] * (float)loop; break;
        case NKA_VAR_DECREMENT: r[i] -= amount[i] * (float)loop; break;
        case NKA_VAR_MULTIPLY: r[i] *= nka__pow(amount[i], (float)loop); break;
        case NKA_VAR_RANDOM: r[i] += nka__clip_signed(&s) * amount[i]; break;
        case NKA_VAR_RANDOM_ABS: r[i] += nka__clip_unit(&s) * amount[i]; break;
        case NKA_VAR_PINGPONG: r[i] += amount[i] * k; break;
        default: break;
        }
    }
}

static struct nk_vec2 nka__clip_varv2(const struct nka_variation_vec2 *v, struct nk_vec2 base, int loop, nk_uint inst, nk_uint salt)
{
    float r[2], amount[2];
    if (v->mode == NKA_VAR_NONE) {
        if (v->x.mode == NKA_VAR_NONE && v->y.mode == NKA_VAR_NONE) return base;
        r[0] = nka__clip_varf(&v->x, base.x, loop, inst, nka_id_mix(salt, 1u));
        r[1] = nka__clip_varf(&v->y, base.y, loop, inst, nka_id_mix(salt, 2u));
        return nka__vec2(r[0], r[1]);
    }
    if (v->mode == NKA_VAR_CALLBACK && v->callback) base = v->callback(loop, v->user);
    else {
        r[0] = base.x;
        r[1] = base.y;
        amount[0] = v->amount.x;
        amount[1] = v->amount.y;
        nka__clip_varn(v->mode, amount, v->seed, loop, inst, salt, r, 2);
        base = nka__vec2(r[0], r[1]);
    }
    return nka__vec2(nka__clip_clampf(base.x, v->min_clamp.x, v->max_clamp.x),
                     nka__clip_clampf(base.y, v->min_clamp.y, v->max_clamp.y));
}

static void nka__clip_v4f(struct nka_vec4 v, float *f) { f[0] = v.x; f[1] = v.y; f[2] = v.z; f[3] = v.w; }

static struct nka_vec4 nka__clip_clamp4(struct nka_vec4 v, struct nka_vec4 lo, struct nka_vec4 hi)
{
    return nka__vec4(nka__clip_clampf(v.x, lo.x, hi.x), nka__clip_clampf(v.y, lo.y, hi.y),
                     nka__clip_clampf(v.z, lo.z, hi.z), nka__clip_clampf(v.w, lo.w, hi.w));
}

static struct nka_vec4 nka__clip_axes(const struct nka_variation_float *axes, struct nka_vec4 v, int loop, nk_uint inst, nk_uint salt)
{
    v.x = nka__clip_varf(&axes[0], v.x, loop, inst, nka_id_mix(salt, 1u));
    v.y = nka__clip_varf(&axes[1], v.y, loop, inst, nka_id_mix(salt, 2u));
    v.z = nka__clip_varf(&axes[2], v.z, loop, inst, nka_id_mix(salt, 3u));
    v.w = nka__clip_varf(&axes[3], v.w, loop, inst, nka_id_mix(salt, 4u));
    return v;
}

static int nka__clip_still(const struct nka_variation_float *a, const struct nka_variation_float *b,
                           const struct nka_variation_float *c, const struct nka_variation_float *d)
{
    return a->mode == NKA_VAR_NONE && b->mode == NKA_VAR_NONE && c->mode == NKA_VAR_NONE && d->mode == NKA_VAR_NONE;
}

static struct nka_vec4 nka__clip_varv4(const struct nka_variation_vec4 *v, struct nka_vec4 base, int loop, nk_uint inst, nk_uint salt)
{
    struct nka_variation_float axes[4];
    float r[4], amount[4];
    if (v->mode == NKA_VAR_NONE) {
        if (nka__clip_still(&v->x, &v->y, &v->z, &v->w)) return base;
        axes[0] = v->x;
        axes[1] = v->y;
        axes[2] = v->z;
        axes[3] = v->w;
        return nka__clip_axes(axes, base, loop, inst, salt);
    }
    if (v->mode == NKA_VAR_CALLBACK && v->callback) base = v->callback(loop, v->user);
    else {
        nka__clip_v4f(base, r);
        nka__clip_v4f(v->amount, amount);
        nka__clip_varn(v->mode, amount, v->seed, loop, inst, salt, r, 4);
        base = nka__vec4(r[0], r[1], r[2], r[3]);
    }
    return nka__clip_clamp4(base, v->min_clamp, v->max_clamp);
}

static struct nka_vec4 nka__clip_varc(const struct nka_variation_color *v, struct nka_vec4 base, int loop, nk_uint inst, nk_uint salt)
{
    struct nka_variation_float axes[4];
    float r[4], amount[4];
    if (v->mode == NKA_VAR_NONE && nka__clip_still(&v->r, &v->g, &v->b, &v->a)) return base;
    if (v->mode == NKA_VAR_CALLBACK && v->callback) base = nka__clip_clamp4(v->callback(loop, v->user), v->min_clamp, v->max_clamp);
    else if (v->mode == NKA_VAR_NONE) {
        axes[0] = v->r;
        axes[1] = v->g;
        axes[2] = v->b;
        axes[3] = v->a;
        base = nka__from_space(nka__clip_axes(axes, nka__to_space(base, v->space), loop, inst, salt), v->space);
    } else {
        nka__clip_v4f(nka__to_space(base, v->space), r);
        nka__clip_v4f(v->amount, amount);
        nka__clip_varn(v->mode, amount, v->seed, loop, inst, salt, r, 4);
        base = nka__from_space(nka__clip_clamp4(nka__vec4(r[0], r[1], r[2], r[3]), v->min_clamp, v->max_clamp), v->space);
    }
    return nka__vec4(nka__clamp01(base.x), nka__clamp01(base.y), nka__clamp01(base.z), nka__clamp01(base.w));
}

/* Authoring */

static struct nka__clip_clip *nka__clip_get(const struct nka_context *a, nk_hash id)
{
    return a ? (struct nka__clip_clip *)nka__get(&a->clip_clips, id) : 0;
}

static struct nka__clip_build *nka__clip_building(struct nka_context *a)
{
    return a && a->clip_build.open ? &a->clip_build : 0;
}

/* Room for one more of size bytes after n: the array, moved if it grew, or NULL. */
static void *nka__clip_room(struct nka_context *a, void *p, int n, int *cap, nk_size size)
{
    void *q;
    int c;
    if (n < *cap) return p;
    c = *cap ? *cap * 2 : 8;
    q = nka__alloc(a, (nk_size)c * size);
    if (!q) return 0;
    if (n) nka__copy(q, p, (nk_size)n * size);
    nka__free(a, p);
    *cap = c;
    return q;
}

NKA_API void nka_clip_begin(struct nka_context *a, nk_hash clip)
{
    struct nka__clip_build *b;
    if (!a) return;
    b = &a->clip_build;
    b->nkeys = b->nvars = b->nmarkers = b->ngroups = 0;
    b->offset = 0;
    b->failed = 0;
    b->open = 1;
    nka__zero(&b->clip, sizeof b->clip);
    b->clip.id = clip;
}

static struct nka__clip_key *nka__clip_key(struct nka_context *a, nk_hash ch, float time, int type, int ease, const float *bezier4)
{
    struct nka__clip_build *b = nka__clip_building(a);
    struct nka__clip_key *k;
    void *q;
    int i;
    if (!b) return 0;
    q = nka__clip_room(a, b->keys, b->nkeys, &b->ckeys, sizeof *k);
    if (!q) {
        b->failed = 1;
        return 0;
    }
    b->keys = (struct nka__clip_key *)q;
    k = &b->keys[b->nkeys++];
    nka__zero(k, sizeof *k);
    k->ch = ch;
    k->type = type;
    k->ease = ease;
    k->var = -1;
    k->time = time + b->offset;
    if (b->ngroups && k->time > b->groups[b->ngroups - 1].max) b->groups[b->ngroups - 1].max = k->time;
    if (k->time > b->clip.duration) b->clip.duration = k->time;
    if (bezier4) {
        k->flags |= NKA__CLIP_BEZIER;
        for (i = 0; i < 4; ++i) k->bezier[i] = bezier4[i];
    }
    return k;
}

static void nka__clip_key_var(struct nka_context *a, struct nka__clip_key *k, const union nka__clip_var *v)
{
    struct nka__clip_build *b = &a->clip_build;
    void *q = nka__clip_room(a, b->vars, b->nvars, &b->cvars, sizeof *v);
    if (!q) {
        b->failed = 1;
        return;
    }
    b->vars = (union nka__clip_var *)q;
    b->vars[b->nvars] = *v;
    k->var = b->nvars++;
}

static void nka__clip_set4(struct nka__clip_key *k, struct nka_vec4 v)
{
    k->v[0].f = v.x;
    k->v[1].f = v.y;
    k->v[2].f = v.z;
    k->v[3].f = v.w;
}

NKA_API void nka_clip_key_float(struct nka_context *a, nk_hash ch, float time, float value, int ease, const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_FLOAT, ease, bezier4);
    if (k) k->v[0].f = value;
}

NKA_API void nka_clip_key_vec2(struct nka_context *a, nk_hash ch, float time, struct nk_vec2 value, int ease, const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_VEC2, ease, bezier4);
    if (!k) return;
    k->v[0].f = value.x;
    k->v[1].f = value.y;
}

NKA_API void nka_clip_key_vec4(struct nka_context *a, nk_hash ch, float time, struct nka_vec4 value, int ease, const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_VEC4, ease, bezier4);
    if (k) nka__clip_set4(k, value);
}

NKA_API void nka_clip_key_int(struct nka_context *a, nk_hash ch, float time, int value, int ease)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_INT, ease, 0);
    if (k) k->v[0].i = value;
}

NKA_API void nka_clip_key_color(struct nka_context *a, nk_hash ch, float time, struct nk_colorf value, int space, int ease,
                                const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_COLOR, ease, bezier4);
    if (!k) return;
    k->space = space;
    nka__clip_set4(k, nka__cv(value));
}

NKA_API void nka_clip_key_float_var(struct nka_context *a, nk_hash ch, float time, float value, struct nka_variation_float var,
                                    int ease, const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_FLOAT, ease, bezier4);
    union nka__clip_var v;
    if (!k) return;
    k->v[0].f = value;
    nka__zero(&v, sizeof v);
    v.f = var;
    nka__clip_key_var(a, k, &v);
}

NKA_API void nka_clip_key_vec2_var(struct nka_context *a, nk_hash ch, float time, struct nk_vec2 value, struct nka_variation_vec2 var,
                                   int ease, const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_VEC2, ease, bezier4);
    union nka__clip_var v;
    if (!k) return;
    k->v[0].f = value.x;
    k->v[1].f = value.y;
    nka__zero(&v, sizeof v);
    v.v2 = var;
    nka__clip_key_var(a, k, &v);
}

NKA_API void nka_clip_key_vec4_var(struct nka_context *a, nk_hash ch, float time, struct nka_vec4 value, struct nka_variation_vec4 var,
                                   int ease, const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_VEC4, ease, bezier4);
    union nka__clip_var v;
    if (!k) return;
    nka__clip_set4(k, value);
    nka__zero(&v, sizeof v);
    v.v4 = var;
    nka__clip_key_var(a, k, &v);
}

NKA_API void nka_clip_key_int_var(struct nka_context *a, nk_hash ch, float time, int value, struct nka_variation_int var, int ease)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_INT, ease, 0);
    union nka__clip_var v;
    if (!k) return;
    k->v[0].i = value;
    nka__zero(&v, sizeof v);
    v.i = var;
    nka__clip_key_var(a, k, &v);
}

NKA_API void nka_clip_key_color_var(struct nka_context *a, nk_hash ch, float time, struct nk_colorf value, struct nka_variation_color var,
                                    int space, int ease, const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_COLOR, ease, bezier4);
    union nka__clip_var v;
    if (!k) return;
    k->space = space;
    nka__clip_set4(k, nka__cv(value));
    nka__zero(&v, sizeof v);
    v.c = var;
    nka__clip_key_var(a, k, &v);
}

NKA_API void nka_clip_key_float_spring(struct nka_context *a, nk_hash ch, float time, float target, struct nka_spring_params spring)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, NKA_CHAN_FLOAT, NKA_EASE_SPRING, 0);
    if (!k) return;
    k->flags |= NKA__CLIP_SPRING;
    k->spring[0] = spring.mass;
    k->spring[1] = spring.stiffness;
    k->spring[2] = spring.damping;
    k->spring[3] = spring.initial_velocity;
    k->v[0].f = target;
}

static void nka__clip_key_rel(struct nka_context *a, nk_hash ch, float time, int type, struct nka_vec4 percent, struct nka_vec4 bias,
                              int space, int anchor, int axis, int ease, const float *bezier4)
{
    struct nka__clip_key *k = nka__clip_key(a, ch, time, type, ease, bezier4);
    if (!k) return;
    k->space = space;
    k->anchor = anchor;
    k->axis = axis;
    nka__clip_set4(k, percent);
    nka__clip_v4f(bias, k->ext);
}

NKA_API void nka_clip_key_float_rel(struct nka_context *a, nk_hash ch, float time, float percent, float px_bias, int anchor, int axis,
                                    int ease, const float *bezier4)
{
    nka__clip_key_rel(a, ch, time, NKA_CHAN_FLOAT_REL, nka__vec4(percent, 0, 0, 0), nka__vec4(px_bias, 0, 0, 0),
                      0, anchor, axis, ease, bezier4);
}

NKA_API void nka_clip_key_vec2_rel(struct nka_context *a, nk_hash ch, float time, struct nk_vec2 percent, struct nk_vec2 px_bias,
                                   int anchor, int ease, const float *bezier4)
{
    nka__clip_key_rel(a, ch, time, NKA_CHAN_VEC2_REL, nka__vec4(percent.x, percent.y, 0, 0), nka__vec4(px_bias.x, px_bias.y, 0, 0),
                      0, anchor, 0, ease, bezier4);
}

NKA_API void nka_clip_key_vec4_rel(struct nka_context *a, nk_hash ch, float time, struct nka_vec4 percent, struct nka_vec4 px_bias,
                                   int anchor, int ease, const float *bezier4)
{
    nka__clip_key_rel(a, ch, time, NKA_CHAN_VEC4_REL, percent, px_bias, 0, anchor, 0, ease, bezier4);
}

NKA_API void nka_clip_key_color_rel(struct nka_context *a, nk_hash ch, float time, struct nka_vec4 percent, struct nka_vec4 px_bias,
                                    int space, int anchor, int ease, const float *bezier4)
{
    nka__clip_key_rel(a, ch, time, NKA_CHAN_COLOR_REL, percent, px_bias, space, anchor, 0, ease, bezier4);
}

static void nka__clip_group(struct nka_context *a, int seq)
{
    struct nka__clip_build *b = nka__clip_building(a);
    void *q;
    if (!b) return;
    q = nka__clip_room(a, b->groups, b->ngroups, &b->cgroups, sizeof *b->groups);
    if (!q) {
        b->failed = 1;
        return;
    }
    b->groups = (struct nka__clip_group *)q;
    b->groups[b->ngroups].max = b->offset;
    b->groups[b->ngroups].seq = seq;
    b->ngroups++;
}

static void nka__clip_group_end(struct nka_context *a, int seq)
{
    struct nka__clip_build *b = nka__clip_building(a);
    struct nka__clip_group g;
    if (!b || !b->ngroups) return;
    g = b->groups[--b->ngroups];
    if (!seq || g.seq) b->offset = g.max;
    if (b->ngroups && g.max > b->groups[b->ngroups - 1].max) b->groups[b->ngroups - 1].max = g.max;
}

NKA_API void nka_clip_seq_begin(struct nka_context *a) { nka__clip_group(a, 1); }
NKA_API void nka_clip_seq_end(struct nka_context *a) { nka__clip_group_end(a, 1); }
NKA_API void nka_clip_par_begin(struct nka_context *a) { nka__clip_group(a, 0); }
NKA_API void nka_clip_par_end(struct nka_context *a) { nka__clip_group_end(a, 0); }

static nk_hash nka__clip_auto(struct nka_context *a, nk_hash salt, int instance)
{
    nk_hash id;
    do {
        id = nka_id_mix(++a->clip_auto, salt);
    } while (!id || (instance && nka__get(&a->clip_insts, id)));
    return id;
}

NKA_API nk_hash nka_clip_marker(struct nka_context *a, float time, nk_hash id, nka_marker_callback cb, void *user)
{
    struct nka__clip_build *b = nka__clip_building(a);
    struct nka__clip_marker *m;
    void *q;
    if (!b) return 0;
    q = nka__clip_room(a, b->markers, b->nmarkers, &b->cmarkers, sizeof *m);
    if (!q) {
        b->failed = 1;
        return 0;
    }
    b->markers = (struct nka__clip_marker *)q;
    if (!id) id = nka__clip_auto(a, NKA__CLIP_SALT_MARK, 0);
    m = &b->markers[b->nmarkers++];
    m->time = time + b->offset;
    m->id = id;
    m->fn = cb;
    m->user = user;
    if (m->time > b->clip.duration) b->clip.duration = m->time;
    return id;
}

NKA_API void nka_clip_set_loop(struct nka_context *a, int loop, int direction, int count)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.direction = direction;
    b->clip.loop_count = loop ? count : 0;
}

NKA_API void nka_clip_set_loop_delay(struct nka_context *a, float seconds)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (b) b->clip.loop_delay = seconds;
}

NKA_API void nka_clip_set_delay(struct nka_context *a, float seconds)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (b) b->clip.delay = seconds;
}

NKA_API void nka_clip_set_stagger(struct nka_context *a, int count, float each_delay, float center_bias)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.stagger_count = count > 0 ? count : 1;
    b->clip.stagger_delay = each_delay;
    b->clip.stagger_bias = nka__clamp01(center_bias);
}

NKA_API void nka_clip_set_stagger_ease(struct nka_context *a, int ease)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (b) b->clip.stagger_ease = ease;
}

NKA_API void nka_clip_set_duration_var(struct nka_context *a, struct nka_variation_float var)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.has_dur_var = 1;
    b->clip.dur_var = var;
}

NKA_API void nka_clip_set_delay_var(struct nka_context *a, struct nka_variation_float var)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.has_delay_var = 1;
    b->clip.delay_var = var;
}

NKA_API void nka_clip_set_timescale_var(struct nka_context *a, struct nka_variation_float var)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.has_scale_var = 1;
    b->clip.scale_var = var;
}

NKA_API void nka_clip_on_begin(struct nka_context *a, nka_clip_callback cb, void *user)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.on_begin = cb;
    b->clip.begin_user = user;
}

NKA_API void nka_clip_on_update(struct nka_context *a, nka_clip_callback cb, void *user)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.on_update = cb;
    b->clip.update_user = user;
}

NKA_API void nka_clip_on_complete(struct nka_context *a, nka_clip_callback cb, void *user)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.on_complete = cb;
    b->clip.complete_user = user;
}

NKA_API void nka_clip_on_loop(struct nka_context *a, nka_loop_callback cb, void *user)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.on_loop = cb;
    b->clip.loop_user = user;
}

NKA_API void nka_clip_on_pause(struct nka_context *a, nka_clip_callback cb, void *user)
{
    struct nka__clip_build *b = nka__clip_building(a);
    if (!b) return;
    b->clip.on_pause = cb;
    b->clip.pause_user = user;
}

static void nka__clip_release(struct nka_context *a, struct nka__clip_clip *c)
{
    nka__free(a, c->tracks);
    nka__free(a, c->keys);
    nka__free(a, c->vars);
    nka__free(a, c->markers);
    c->tracks = 0;
    c->keys = 0;
    c->vars = 0;
    c->markers = 0;
    c->ntracks = c->nkeys = c->nvars = c->nmarkers = 0;
}

static void nka__clip_keep_callbacks(struct nka__clip_clip *c, const struct nka__clip_clip *old)
{
    int i, j;
    c->on_begin = old->on_begin;
    c->on_update = old->on_update;
    c->on_complete = old->on_complete;
    c->on_pause = old->on_pause;
    c->on_loop = old->on_loop;
    c->begin_user = old->begin_user;
    c->update_user = old->update_user;
    c->complete_user = old->complete_user;
    c->pause_user = old->pause_user;
    c->loop_user = old->loop_user;
    for (i = 0; i < c->nmarkers; ++i)
        for (j = 0; j < old->nmarkers; ++j)
            if (old->markers[j].id == c->markers[i].id) {
                c->markers[i].fn = old->markers[j].fn;
                c->markers[i].user = old->markers[j].user;
                break;
            }
}

/* Sorts keys into tracks and markers by time, stably, and puts c in place of the clip with its id. */
static int nka__clip_install(struct nka_context *a, struct nka__clip_clip *c, const struct nka__clip_key *keys, int nk,
                             const union nka__clip_var *vars, int nv, const struct nka__clip_marker *marks, int nm, int loaded)
{
    struct nka__clip_clip *old;
    int *tmp = 0, *order, *tix, *head, i, j, nt = 0;
    c->tracks = 0;
    c->keys = nk ? (struct nka__clip_key *)nka__alloc(a, (nk_size)nk * sizeof *keys) : 0;
    c->vars = nv ? (union nka__clip_var *)nka__alloc(a, (nk_size)nv * sizeof *vars) : 0;
    c->markers = nm ? (struct nka__clip_marker *)nka__alloc(a, (nk_size)nm * sizeof *marks) : 0;
    c->ntracks = 0;
    c->nkeys = nk;
    c->nvars = nv;
    c->nmarkers = nm;
    if (nk) tmp = (int *)nka__alloc(a, (nk_size)nk * 3 * sizeof(int));
    if ((nk && (!c->keys || !tmp)) || (nv && !c->vars) || (nm && !c->markers)) {
        nka__free(a, tmp);
        nka__clip_release(a, c);
        return NKA_ERR_NO_MEM;
    }
    order = tix = head = tmp;
    if (tmp) {
        tix = tmp + nk;
        head = tmp + 2 * nk;
    }
    for (i = 0; i < nk; ++i) {
        for (j = i; j > 0 && keys[order[j - 1]].time > keys[i].time; --j) order[j] = order[j - 1];
        order[j] = i;
    }
    for (i = 0; i < nk; ++i) {
        const struct nka__clip_key *k = &keys[order[i]];
        for (j = 0; j < nt; ++j)
            if (keys[head[j]].ch == k->ch && keys[head[j]].type == k->type) break;
        if (j == nt) head[nt++] = order[i];
        tix[i] = j;
    }
    if (nt) {
        c->tracks = (struct nka__clip_track *)nka__alloc(a, (nk_size)nt * sizeof *c->tracks);
        if (!c->tracks) {
            nka__free(a, tmp);
            nka__clip_release(a, c);
            return NKA_ERR_NO_MEM;
        }
        c->ntracks = nt;
    }
    for (j = 0; j < nt; ++j) {
        const struct nka__clip_key *k = &keys[head[j]];
        struct nka__clip_track *tr = &c->tracks[j];
        tr->ch = k->ch;
        tr->type = k->type;
        tr->space = k->space;
        tr->anchor = k->anchor;
        tr->axis = k->axis;
        tr->first = tr->count = 0;
    }
    for (i = 0; i < nk; ++i) c->tracks[tix[i]].count++;
    for (j = 1; j < nt; ++j) c->tracks[j].first = c->tracks[j - 1].first + c->tracks[j - 1].count;
    for (j = 0; j < nt; ++j) head[j] = 0;
    for (i = 0; i < nk; ++i) {
        const struct nka__clip_track *tr = &c->tracks[tix[i]];
        struct nka__clip_key *k = &c->keys[tr->first + head[tix[i]]];
        head[tix[i]]++;
        *k = keys[order[i]];
        k->space = tr->space;
        k->anchor = tr->anchor;
        k->axis = tr->axis;
    }
    nka__free(a, tmp);
    for (i = 0; i < nv; ++i) c->vars[i] = vars[i];
    for (i = 0; i < nm; ++i) {
        struct nka__clip_marker m = marks[i];
        for (j = i; j > 0 && c->markers[j - 1].time > m.time; --j) c->markers[j] = c->markers[j - 1];
        c->markers[j] = m;
    }
    c->gen = ++a->clip_gen;
    old = nka__clip_get(a, c->id);
    if (old) {
        if (loaded) nka__clip_keep_callbacks(c, old);
        nka__clip_release(a, old);
        *old = *c;
        return NKA_OK;
    }
    old = (struct nka__clip_clip *)nka__put(a, &a->clip_clips, c->id, 0);
    if (!old) {
        nka__clip_release(a, c);
        return NKA_ERR_NO_MEM;
    }
    *old = *c;
    return NKA_OK;
}

NKA_API int nka_clip_end(struct nka_context *a)
{
    struct nka__clip_build *b = nka__clip_building(a);
    struct nka__clip_clip c;
    if (!b) return NKA_ERR_BAD_ARG;
    b->open = 0;
    if (b->failed) return NKA_ERR_NO_MEM;
    c = b->clip;
    return nka__clip_install(a, &c, b->keys, b->nkeys, b->vars, b->nvars, b->markers, b->nmarkers, 0);
}

NKA_API float nka_clip_duration(const struct nka_context *a, nk_hash clip)
{
    const struct nka__clip_clip *c = nka__clip_get(a, clip);
    return c ? c->duration : 0;
}

NKA_API int nka_clip_exists(const struct nka_context *a, nk_hash clip) { return nka__clip_get(a, clip) != 0; }

NKA_API void nka_clip_reserve(struct nka_context *a, int clips, int instances)
{
    int cap = 16;
    if (!a) return;
    while (cap < clips * 2) cap *= 2;
    if (cap > a->clip_clips.cap) nka__grow(a, &a->clip_clips, cap);
    cap = 16;
    while (cap < instances * 2) cap *= 2;
    if (cap > a->clip_insts.cap && nka__grow(a, &a->clip_insts, cap)) a->clip_mod++;
}

/* Instances */

static struct nka__clip_inst *nka__clip_inst(struct nka_context *a, nk_hash id)
{
    struct nka__clip_inst *in;
    if (!a || !id) return 0;
    in = (struct nka__clip_inst *)nka__get(&a->clip_insts, id);
    if (in) in->seen = a->frame;
    return in;
}

static void nka__clip_find(const struct nka__clip_key *k, int n, float t, int *i0, int *i1)
{
    int lo = 1, hi = n - 1;
    if (n == 1 || t <= k[0].time) {
        *i0 = *i1 = 0;
        return;
    }
    if (t >= k[n - 1].time) {
        *i0 = *i1 = n - 1;
        return;
    }
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (k[mid].time >= t) hi = mid;
        else lo = mid + 1;
    }
    *i0 = lo - 1;
    *i1 = lo;
}

static float nka__clip_weight(const struct nka_context *a, const struct nka__clip_key *k, int type, float u)
{
    if ((k->flags & NKA__CLIP_SPRING) && type == NKA_CHAN_FLOAT)
        return nka_eval(a, nka_ease_spring(k->spring[0], k->spring[1], k->spring[2], k->spring[3]), u);
    if (k->ease == NKA_EASE_CUBIC_BEZIER) {
        if (!(k->flags & NKA__CLIP_BEZIER)) return nka__clamp01(u);
        return nka_eval(a, nka_ease_bezier(k->bezier[0], k->bezier[1], k->bezier[2], k->bezier[3]), u);
    }
    return nka_eval(a, nka_ease(k->ease), u);
}

static struct nka_vec4 nka__clip_key4(const struct nka__clip_key *k)
{
    return nka__vec4(k->v[0].f, k->v[1].f, k->v[2].f, k->v[3].f);
}

static void nka__clip_eval_track(struct nka_context *a, const struct nka__clip_clip *c, int ti, struct nka__clip_inst *in)
{
    const struct nka__clip_track *tr = &c->tracks[ti];
    const struct nka__clip_key *k0, *k1;
    union nka__clip_slot *out = in->vals + ti * 8;
    float t = in->time, u, w;
    nk_uint s0, s1;
    int i0, i1, i, n;
    nka__clip_find(c->keys + tr->first, tr->count, t, &i0, &i1);
    k0 = c->keys + tr->first + i0;
    k1 = c->keys + tr->first + i1;
    s0 = (nk_uint)(tr->first + i0);
    s1 = (nk_uint)(tr->first + i1);
    u = k1->time == k0->time ? 1 : (t - k0->time) / (k1->time - k0->time);
    w = nka__clip_weight(a, k0, tr->type, u);
    switch (tr->type) {
    case NKA_CHAN_FLOAT: {
        float x = k0->v[0].f, y = k1->v[0].f;
        if (k0->var >= 0) x = nka__clip_varf(&c->vars[k0->var].f, x, in->loop, in->rng, s0);
        if (k1->var >= 0) y = nka__clip_varf(&c->vars[k1->var].f, y, in->loop, in->rng, s1);
        out[0].f = nka__lerp(x, y, w);
        break;
    }
    case NKA_CHAN_VEC2: {
        struct nk_vec2 x = nka__vec2(k0->v[0].f, k0->v[1].f), y = nka__vec2(k1->v[0].f, k1->v[1].f);
        if (k0->var >= 0) x = nka__clip_varv2(&c->vars[k0->var].v2, x, in->loop, in->rng, s0);
        if (k1->var >= 0) y = nka__clip_varv2(&c->vars[k1->var].v2, y, in->loop, in->rng, s1);
        out[0].f = nka__lerp(x.x, y.x, w);
        out[1].f = nka__lerp(x.y, y.y, w);
        break;
    }
    case NKA_CHAN_VEC4: case NKA_CHAN_COLOR: {
        struct nka_vec4 x = nka__clip_key4(k0), y = nka__clip_key4(k1);
        if (tr->type == NKA_CHAN_VEC4) {
            if (k0->var >= 0) x = nka__clip_varv4(&c->vars[k0->var].v4, x, in->loop, in->rng, s0);
            if (k1->var >= 0) y = nka__clip_varv4(&c->vars[k1->var].v4, y, in->loop, in->rng, s1);
            x = nka__lerp4(x, y, w);
        } else {
            if (k0->var >= 0) x = nka__clip_varc(&c->vars[k0->var].c, x, in->loop, in->rng, s0);
            if (k1->var >= 0) y = nka__clip_varc(&c->vars[k1->var].c, y, in->loop, in->rng, s1);
            x = nka__lerp_color(x, y, w, tr->space);
        }
        out[0].f = x.x;
        out[1].f = x.y;
        out[2].f = x.z;
        out[3].f = x.w;
        break;
    }
    case NKA_CHAN_INT: {
        int x = k0->v[0].i, y = k1->v[0].i;
        if (k0->var >= 0) x = nka__clip_vari(&c->vars[k0->var].i, x, in->loop, in->rng, s0);
        if (k1->var >= 0) y = nka__clip_vari(&c->vars[k1->var].i, y, in->loop, in->rng, s1);
        out[0].i = nka__clip_lerpi(x, y, w);
        break;
    }
    default:
        n = tr->type == NKA_CHAN_FLOAT_REL ? 1 : (tr->type == NKA_CHAN_VEC2_REL ? 2 : 4);
        for (i = 0; i < n; ++i) {
            out[i].f = nka__lerp(k0->v[i].f, k1->v[i].f, w);
            out[4 + i].f = nka__lerp(k0->ext[i], k1->ext[i], w);
        }
        break;
    }
}

static void nka__clip_eval(struct nka_context *a, const struct nka__clip_clip *c, struct nka__clip_inst *in)
{
    int i;
    if (in->gen != c->gen) return;
    for (i = 0; i < c->ntracks; ++i) nka__clip_eval_track(a, c, i, in);
}

/* Markers ahead of the time, in the way the instance plays, may fire again. */
static void nka__clip_arm(struct nka__clip_inst *in, const struct nka__clip_clip *c)
{
    int m;
    for (m = 0; m < c->nmarkers; ++m)
        in->fired[m] = (unsigned char)(in->dir > 0 ? c->markers[m].time < in->time : c->markers[m].time > in->time);
}

/* Fits the instance to its clip, which may have been authored again; force re-arms and re-evaluates. */
static int nka__clip_sync(struct nka_context *a, struct nka__clip_inst *in, const struct nka__clip_clip *c, int force)
{
    if (in->gen != c->gen) {
        nk_size size = (nk_size)c->ntracks * 8 * sizeof(union nka__clip_slot) + (nk_size)c->nmarkers;
        union nka__clip_slot *p = 0;
        if (size) {
            p = (union nka__clip_slot *)nka__alloc(a, size);
            if (!p) return 0;
            nka__zero(p, size);
        }
        nka__free(a, in->vals);
        in->vals = p;
        in->fired = p ? (unsigned char *)(p + c->ntracks * 8) : 0;
        in->gen = c->gen;
        in->time = nka__clip_clampf(in->time, 0, c->duration);
        force = 1;
    }
    if (force) {
        nka__clip_arm(in, c);
        nka__clip_eval(a, c, in);
    }
    return 1;
}

static void nka__clip_rewind(struct nka_context *a, struct nka__clip_inst *in, const struct nka__clip_clip *c)
{
    in->delay = c->delay;
    in->playing = 1;
    in->paused = 0;
    in->begun = 0;
    in->dir = c->direction == NKA_DIR_REVERSE ? -1 : 1;
    in->loops = c->loop_count;
    in->loop = 0;
    in->rng = 12345u + in->id;
    in->time = in->dir > 0 ? 0 : c->duration;
    in->chain_clip = in->chain_inst = 0;
    in->chain_delay = 0;
    in->seen = in->stepped = a->frame;
    in->epoch = ++a->clip_epoch;
    nka__clip_sync(a, in, c, 1);
    a->busy = 1;
}

/* An instance being stepped, with what user code may change under it. */
struct nka__clip_run {
    struct nka__clip_inst *in;
    struct nka__clip_clip *c;
    nk_hash id;
    nk_uint epoch, gen;
};

/* After user code: refetches both, and says whether the step may go on. */
static int nka__clip_ok(struct nka_context *a, struct nka__clip_run *r)
{
    r->in = (struct nka__clip_inst *)nka__get(&a->clip_insts, r->id);
    if (!r->in || r->in->epoch != r->epoch || r->in->gen != r->gen) return 0;
    r->c = nka__clip_get(a, r->in->clip);
    return r->c && r->c->gen == r->gen;
}

/* Fires the markers from from to to that have not fired this pass, in the order passed. */
static int nka__clip_fire(struct nka_context *a, struct nka__clip_run *r, float from, float to)
{
    float lo = from < to ? from : to, hi = from < to ? to : from;
    int n = r->c->nmarkers, i;
    for (i = 0; i < n; ++i) {
        int m = to < from ? n - 1 - i : i;
        const struct nka__clip_marker *mk = &r->c->markers[m];
        if (r->in->fired[m] || mk->time < lo || mk->time > hi) continue;
        r->in->fired[m] = 1;
        if (!mk->fn) continue;
        mk->fn(a, r->id, mk->id, mk->time, mk->user);
        if (!nka__clip_ok(a, r)) return 0;
    }
    return 1;
}

static void nka__clip_complete(struct nka_context *a, struct nka__clip_run *r)
{
    nk_hash next, inst;
    float delay;
    r->in->playing = 0;
    nka__clip_eval(a, r->c, r->in);
    if (r->c->on_complete) {
        r->c->on_complete(a, r->id, r->c->complete_user);
        r->in = (struct nka__clip_inst *)nka__get(&a->clip_insts, r->id);
        if (!r->in || r->in->epoch != r->epoch) return;
    }
    next = r->in->chain_clip;
    inst = r->in->chain_inst;
    delay = r->in->chain_delay;
    r->in->chain_clip = r->in->chain_inst = 0;
    r->in->chain_delay = 0;
    if (next && nka_play(a, next, inst) && delay > 0) {
        struct nka__clip_inst *n = (struct nka__clip_inst *)nka__get(&a->clip_insts, inst);
        if (n) n->delay += delay;
    }
}

static void nka__clip_step(struct nka_context *a, struct nka__clip_inst *in, float dt)
{
    struct nka__clip_run r;
    float left = dt;
    int wraps = 0, m;
    r.id = in->id;
    r.in = in;
    r.c = nka__clip_get(a, in->clip);
    if (!in->playing || in->paused || !r.c || !nka__clip_sync(a, in, r.c, 0)) return;
    in->seen = a->frame;
    r.epoch = in->epoch;
    r.gen = in->gen;
    if (in->delay > 0) {
        in->delay -= left;
        if (in->delay > 0) {
            a->busy = 1;
            return;
        }
        left = -in->delay;
    }
    in->delay = 0;
    if (!in->begun) {
        in->begun = 1;
        if (r.c->on_begin) {
            r.c->on_begin(a, r.id, r.c->begin_user);
            if (!nka__clip_ok(a, &r)) return;
        }
    }
    if (r.c->duration <= 0) {
        if (nka__clip_fire(a, &r, r.in->time, r.in->time)) nka__clip_complete(a, &r);
        return;
    }
    for (;;) {
        float dur = r.c->duration, from = r.in->time, rate = 1, speed = r.in->scale > 0 ? r.in->scale : 1, step, room, hold;
        if (r.c->has_dur_var) {
            float v = nka__clip_varf(&r.c->dur_var, dur, r.in->loop, r.in->rng, NKA__CLIP_SALT_DUR);
            rate = dur / (v > NKA__CLIP_MIN_PASS ? v : NKA__CLIP_MIN_PASS);
        }
        step = left * speed * rate;
        room = r.in->dir > 0 ? dur - from : from;
        if (room < 0) room = 0;
        if (step <= room || wraps >= NKA__CLIP_MAX_WRAPS) {
            if (step > room) step = room;
            r.in->time = nka__clip_clampf(r.in->dir > 0 ? from + step : from - step, 0, dur);
            if (!nka__clip_fire(a, &r, from, r.in->time)) return;
            break;
        }
        left -= room / (speed * rate);
        if (left < 0) left = 0;
        r.in->time = r.in->dir > 0 ? dur : 0;
        if (!nka__clip_fire(a, &r, from, r.in->time)) return;
        if (r.in->loops == 0) {
            nka__clip_complete(a, &r);
            return;
        }
        if (r.in->loops > 0) r.in->loops--;
        r.in->loop++;
        wraps++;
        if (r.c->direction == NKA_DIR_ALTERNATE) r.in->dir = -r.in->dir;
        else r.in->time = r.in->dir > 0 ? 0 : dur;
        for (m = 0; m < r.c->nmarkers; ++m)
            r.in->fired[m] = (unsigned char)(r.c->direction == NKA_DIR_ALTERNATE && r.c->markers[m].time == r.in->time);
        if (r.c->on_loop) {
            r.c->on_loop(a, r.id, r.in->loop, r.c->loop_user);
            if (!nka__clip_ok(a, &r)) return;
        }
        hold = r.c->loop_delay > 0 ? r.c->loop_delay : 0;
        if (r.c->has_scale_var) {
            float v = nka__clip_varf(&r.c->scale_var, 1, r.in->loop, r.in->rng, NKA__CLIP_SALT_SCALE);
            r.in->scale = v > 0 ? v : 1;
        }
        if (r.c->has_delay_var) {
            float v = nka__clip_varf(&r.c->delay_var, 0, r.in->loop, r.in->rng, NKA__CLIP_SALT_DELAY);
            if (v > 0) hold += v;
        }
        if (hold > left) {
            r.in->delay = hold - left;
            break;
        }
        left -= hold;
    }
    nka__clip_eval(a, r.c, r.in);
    if (r.in->playing && !r.in->paused) a->busy = 1;
    if (r.c->on_update) r.c->on_update(a, r.id, r.c->update_user);
}

static void nka__clip_update(struct nka_context *a)
{
    float dt = a->dt < NKA__CLIP_MAX_DT ? a->dt : NKA__CLIP_MAX_DT;
    int i = 0;
    if (a->clip_updating) return;
    a->clip_updating = 1;
    while (i < a->clip_insts.cap) {
        struct nka__clip_inst *in;
        nk_uint mod = a->clip_mod;
        if (!a->clip_insts.keys[i]) {
            ++i;
            continue;
        }
        in = (struct nka__clip_inst *)nka__at(&a->clip_insts, i);
        if (in->stepped == a->frame) {
            ++i;
            continue;
        }
        in->stepped = a->frame;
        nka__clip_step(a, in, dt);
        if (a->clip_mod != mod) i = 0;
        else ++i;
    }
    a->clip_updating = 0;
}

static void nka__clip_gc(struct nka_context *a, unsigned max_age)
{
    struct nka__map *m = &a->clip_insts;
    int i = 0;
    while (i < m->cap) {
        if (m->keys[i] && a->frame - ((const struct nka__clip_inst *)nka__at(m, i))->seen > max_age) {
            nka__free(a, ((struct nka__clip_inst *)nka__at(m, i))->vals);
            nka__del_at(m, i);
            a->clip_mod++;
        } else ++i;
    }
    m = &a->clip_layers;
    i = 0;
    while (i < m->cap) {
        if (m->keys[i] && a->frame - ((const struct nka__clip_layer *)nka__at(m, i))->seen > max_age) {
            nka__free(a, ((struct nka__clip_layer *)nka__at(m, i))->e);
            nka__del_at(m, i);
        } else ++i;
    }
}

static void nka__clip_clear(struct nka_context *a)
{
    int i;
    for (i = 0; i < a->clip_insts.cap; ++i)
        if (a->clip_insts.keys[i]) nka__free(a, ((struct nka__clip_inst *)nka__at(&a->clip_insts, i))->vals);
    for (i = 0; i < a->clip_layers.cap; ++i)
        if (a->clip_layers.keys[i]) nka__free(a, ((struct nka__clip_layer *)nka__at(&a->clip_layers, i))->e);
    nka__map_free(a, &a->clip_insts);
    nka__map_free(a, &a->clip_layers);
    nka__map_free(a, &a->clip_acc);
    a->clip_layer = 0;
    a->clip_mod++;
}

static void nka__clip_destroy(struct nka_context *a)
{
    struct nka__clip_build *b = &a->clip_build;
    int i;
    nka__clip_clear(a);
    for (i = 0; i < a->clip_clips.cap; ++i)
        if (a->clip_clips.keys[i]) nka__clip_release(a, (struct nka__clip_clip *)nka__at(&a->clip_clips, i));
    nka__map_free(a, &a->clip_clips);
    nka__free(a, b->keys);
    nka__free(a, b->vars);
    nka__free(a, b->markers);
    nka__free(a, b->groups);
}

NKA_API nk_hash nka_play(struct nka_context *a, nk_hash clip, nk_hash inst)
{
    const struct nka__clip_clip *c = nka__clip_get(a, clip);
    struct nka__clip_inst *in;
    int fresh;
    if (!c || !inst) return 0;
    in = (struct nka__clip_inst *)nka__put(a, &a->clip_insts, inst, &fresh);
    if (!in) return 0;
    if (fresh) {
        in->id = inst;
        a->clip_mod++;
    }
    in->clip = clip;
    in->scale = 1;
    in->weight = 1;
    nka__clip_rewind(a, in, c);
    return inst;
}

NKA_API nk_hash nka_get_instance(struct nka_context *a, nk_hash inst) { return nka__clip_inst(a, inst) ? inst : 0; }
NKA_API int nka_instance_valid(struct nka_context *a, nk_hash inst) { return nka__clip_inst(a, inst) != 0; }

NKA_API void nka_instance_pause(struct nka_context *a, nk_hash inst)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    const struct nka__clip_clip *c;
    if (!in || in->paused) return;
    in->paused = 1;
    c = nka__clip_get(a, in->clip);
    if (c && c->on_pause) c->on_pause(a, inst, c->pause_user);
}

NKA_API void nka_instance_resume(struct nka_context *a, nk_hash inst)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    if (!in) return;
    in->paused = 0;
    if (in->playing) a->busy = 1;
}

NKA_API void nka_instance_stop(struct nka_context *a, nk_hash inst)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    if (!in) return;
    in->playing = 0;
    in->time = 0;
    in->epoch = ++a->clip_epoch;
}

NKA_API void nka_instance_restart(struct nka_context *a, nk_hash inst)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    const struct nka__clip_clip *c = in ? nka__clip_get(a, in->clip) : 0;
    if (c) nka__clip_rewind(a, in, c);
}

NKA_API void nka_instance_reset(struct nka_context *a, nk_hash inst)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    const struct nka__clip_clip *c = in ? nka__clip_get(a, in->clip) : 0;
    if (!c) return;
    in->playing = in->paused = 0;
    in->delay = 0;
    in->epoch = ++a->clip_epoch;
    if (!nka__clip_sync(a, in, c, 0)) return;
    in->time = in->dir > 0 ? 0 : c->duration;
    nka__clip_eval(a, c, in);
}

NKA_API void nka_instance_refresh(struct nka_context *a, nk_hash inst)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    const struct nka__clip_clip *c = in ? nka__clip_get(a, in->clip) : 0;
    if (c && nka__clip_sync(a, in, c, 0)) nka__clip_eval(a, c, in);
}

NKA_API void nka_instance_destroy(struct nka_context *a, nk_hash inst)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    if (!in) return;
    nka__free(a, in->vals);
    nka__del(&a->clip_insts, inst);
    a->clip_mod++;
}

NKA_API void nka_instance_seek(struct nka_context *a, nk_hash inst, float time)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    const struct nka__clip_clip *c = in ? nka__clip_get(a, in->clip) : 0;
    if (!c) return;
    in->time = nka__clip_clampf(time, 0, c->duration);
    in->epoch = ++a->clip_epoch;
    nka__clip_sync(a, in, c, 1);
}

NKA_API void nka_instance_set_time_scale(struct nka_context *a, nk_hash inst, float scale)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    if (in) in->scale = scale;
}

NKA_API void nka_instance_set_weight(struct nka_context *a, nk_hash inst, float weight)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    if (in) in->weight = weight;
}

NKA_API nk_hash nka_instance_then(struct nka_context *a, nk_hash inst, nk_hash next_clip, nk_hash next_inst)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    if (!in) return 0;
    if (!next_inst) next_inst = nka__clip_auto(a, NKA__CLIP_SALT_INST, 1);
    in->chain_clip = next_clip;
    in->chain_inst = next_inst;
    return next_inst;
}

NKA_API void nka_instance_then_delay(struct nka_context *a, nk_hash inst, float delay)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    if (in) in->chain_delay = delay;
}

NKA_API float nka_instance_time(struct nka_context *a, nk_hash inst)
{
    const struct nka__clip_inst *in = nka__clip_inst(a, inst);
    return in ? in->time : 0;
}

NKA_API float nka_instance_duration(struct nka_context *a, nk_hash inst)
{
    const struct nka__clip_inst *in = nka__clip_inst(a, inst);
    const struct nka__clip_clip *c = in ? nka__clip_get(a, in->clip) : 0;
    return c ? c->duration : 0;
}

NKA_API int nka_instance_is_playing(struct nka_context *a, nk_hash inst)
{
    const struct nka__clip_inst *in = nka__clip_inst(a, inst);
    return in ? in->playing : 0;
}

NKA_API int nka_instance_is_paused(struct nka_context *a, nk_hash inst)
{
    const struct nka__clip_inst *in = nka__clip_inst(a, inst);
    return in ? in->paused : 0;
}

static int nka__clip_track_of(const struct nka__clip_clip *c, nk_hash ch, int type)
{
    int i;
    for (i = 0; i < c->ntracks; ++i)
        if (c->tracks[i].ch == ch && c->tracks[i].type == type) return i;
    return -1;
}

/* The values of the track a getter reads for ch: its relative kind first. */
static const union nka__clip_slot *nka__clip_lookup(struct nka_context *a, nk_hash inst, nk_hash ch, int type, int rel,
                                                    const struct nka__clip_track **tr)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    const struct nka__clip_clip *c = in ? nka__clip_get(a, in->clip) : 0;
    int t;
    if (!c || !nka__clip_sync(a, in, c, 0)) return 0;
    t = rel >= 0 ? nka__clip_track_of(c, ch, rel) : -1;
    if (t < 0) t = nka__clip_track_of(c, ch, type);
    if (t < 0) return 0;
    *tr = &c->tracks[t];
    return in->vals + t * 8;
}

/* A float track's values, relative ones worked out against the anchors as they are now. */
static void nka__clip_resolve(const struct nka_context *a, const struct nka__clip_track *tr, const union nka__clip_slot *v, float *out)
{
    struct nk_vec2 s = nka_anchor(a, tr->anchor);
    int i;
    for (i = 0; i < 4; ++i) out[i] = v[i].f;
    switch (tr->type) {
    case NKA_CHAN_FLOAT_REL:
        out[0] = (tr->axis ? s.y : s.x) * v[0].f + v[4].f;
        break;
    case NKA_CHAN_VEC2_REL: case NKA_CHAN_VEC4_REL: case NKA_CHAN_COLOR_REL:
        out[0] = s.x * v[0].f + v[4].f;
        out[1] = s.y * v[1].f + v[5].f;
        out[2] = (tr->type == NKA_CHAN_COLOR_REL ? s.x * v[2].f : v[2].f) + v[6].f;
        out[3] = (tr->type == NKA_CHAN_COLOR_REL ? s.y * v[3].f : v[3].f) + v[7].f;
        break;
    default: break;
    }
}

static int nka__clip_read4(struct nka_context *a, nk_hash inst, nk_hash ch, int type, int rel, float *out)
{
    const struct nka__clip_track *tr = 0;
    const union nka__clip_slot *v = nka__clip_lookup(a, inst, ch, type, rel, &tr);
    if (!v) return 0;
    nka__clip_resolve(a, tr, v, out);
    return 1;
}

NKA_API int nka_instance_get_float(struct nka_context *a, nk_hash inst, nk_hash ch, float *out)
{
    float v[4] = { 0 };
    int ok = out && nka__clip_read4(a, inst, ch, NKA_CHAN_FLOAT, NKA_CHAN_FLOAT_REL, v);
    if (out) *out = v[0];
    return ok;
}

NKA_API int nka_instance_get_vec2(struct nka_context *a, nk_hash inst, nk_hash ch, struct nk_vec2 *out)
{
    float v[4] = { 0 };
    int ok = out && nka__clip_read4(a, inst, ch, NKA_CHAN_VEC2, NKA_CHAN_VEC2_REL, v);
    if (out) *out = nka__vec2(v[0], v[1]);
    return ok;
}

NKA_API int nka_instance_get_vec4(struct nka_context *a, nk_hash inst, nk_hash ch, struct nka_vec4 *out)
{
    float v[4] = { 0 };
    int ok = out && nka__clip_read4(a, inst, ch, NKA_CHAN_VEC4, NKA_CHAN_VEC4_REL, v);
    if (out) *out = nka__vec4(v[0], v[1], v[2], v[3]);
    return ok;
}

NKA_API int nka_instance_get_int(struct nka_context *a, nk_hash inst, nk_hash ch, int *out)
{
    const struct nka__clip_track *tr = 0;
    const union nka__clip_slot *v = out ? nka__clip_lookup(a, inst, ch, NKA_CHAN_INT, -1, &tr) : 0;
    if (out) *out = v ? v[0].i : 0;
    return v != 0;
}

NKA_API int nka_instance_get_color(struct nka_context *a, nk_hash inst, nk_hash ch, struct nk_colorf *out)
{
    float v[4] = { 0, 0, 0, 1 };
    int ok = out && nka__clip_read4(a, inst, ch, NKA_CHAN_COLOR, NKA_CHAN_COLOR_REL, v);
    if (out) *out = nka__vc(nka__vec4(v[0], v[1], v[2], v[3]));
    return ok;
}

/* Stagger */

NKA_API float nka_stagger_delay(const struct nka_context *a, nk_hash clip, int index)
{
    const struct nka__clip_clip *c = nka__clip_get(a, clip);
    float delay, bias, raw = 0;
    int count;
    if (!c || c->stagger_count <= 1) return 0;
    count = c->stagger_count;
    delay = c->stagger_delay;
    bias = c->stagger_bias;
    if (bias <= 0) raw = (float)index * delay;
    else {
        float center = (float)(count - 1) * 0.5f;
        if (center > 0) {
            float lin = (float)index * delay;
            float mid = nka__fabs((float)index - center) * delay * 2 / (float)count * (float)(count - 1);
            raw = lin * (1 - bias) + mid * bias;
        }
    }
    if (c->stagger_ease != NKA_EASE_LINEAR) {
        float most = (float)(count - 1) * delay;
        if (most > 0) raw = nka_eval(a, nka_ease(c->stagger_ease), raw / most) * most;
    }
    return raw;
}

NKA_API nk_hash nka_play_stagger(struct nka_context *a, nk_hash clip, nk_hash inst, int index)
{
    struct nka__clip_inst *in;
    const struct nka__clip_clip *c;
    if (!nka_play(a, clip, inst)) return 0;
    c = nka__clip_get(a, clip);
    in = nka__clip_inst(a, inst);
    in->delay = c->delay + nka_stagger_delay(a, clip, index);
    return inst;
}

NKA_API struct nka_stagger_grid_opts nka_stagger_grid_opts_default(void)
{
    struct nka_stagger_grid_opts o;
    o.cols = o.rows = 1;
    o.from = NKA_STAGGER_FIRST;
    o.from_index = 0;
    o.axis = NKA_STAGGER_BOTH;
    o.delay = 0.05f;
    o.ease = NKA_EASE_LINEAR;
    o.start_delay = 0;
    return o;
}

static float nka__clip_grid_dist(int axis, float dx, float dy)
{
    if (axis == NKA_STAGGER_X) return nka__fabs(dx);
    if (axis == NKA_STAGGER_Y) return nka__fabs(dy);
    return nka__sqrt(dx * dx + dy * dy);
}

NKA_API float nka_stagger_grid_delay(const struct nka_context *a, int col, int row, const struct nka_stagger_grid_opts *opts)
{
    struct nka_stagger_grid_opts o = opts ? *opts : nka_stagger_grid_opts_default();
    int cols = o.cols > 0 ? o.cols : 1, rows = o.rows > 0 ? o.rows : 1, total = cols * rows, i;
    float ox = 0, oy = 0, most = 0, t;
    if (total <= 1) return o.start_delay;
    if (o.from == NKA_STAGGER_LAST) {
        ox = (float)(cols - 1);
        oy = (float)(rows - 1);
    } else if (o.from == NKA_STAGGER_CENTER) {
        ox = (float)(cols - 1) * 0.5f;
        oy = (float)(rows - 1) * 0.5f;
    } else if (o.from == NKA_STAGGER_INDEX) {
        int at = o.from_index < 0 ? 0 : (o.from_index > total - 1 ? total - 1 : o.from_index);
        ox = (float)(at % cols);
        oy = (float)(at / cols);
    }
    for (i = 0; i < 4; ++i) {
        float d = nka__clip_grid_dist(o.axis, (float)(i & 1 ? cols - 1 : 0) - ox, (float)(i & 2 ? rows - 1 : 0) - oy);
        if (d > most) most = d;
    }
    t = most > 0 ? nka__clip_grid_dist(o.axis, (float)col - ox, (float)row - oy) / most : 0;
    if (o.ease != NKA_EASE_LINEAR) t = nka_eval(a, nka_ease(o.ease), t);
    return o.start_delay + t * o.delay * (float)(total - 1);
}

NKA_API float nka_stagger_grid_delay_index(const struct nka_context *a, int index, const struct nka_stagger_grid_opts *opts)
{
    int cols = opts && opts->cols > 0 ? opts->cols : 1;
    return nka_stagger_grid_delay(a, index % cols, index / cols, opts);
}

NKA_API nk_hash nka_play_with_delay(struct nka_context *a, nk_hash clip, nk_hash inst, float delay)
{
    struct nka__clip_inst *in;
    if (!nka_play(a, clip, inst)) return 0;
    in = nka__clip_inst(a, inst);
    in->delay = delay;
    return inst;
}

/* Layers */

static int nka__clip_kind(int type)
{
    switch (type) {
    case NKA_CHAN_FLOAT: case NKA_CHAN_FLOAT_REL: return NKA_CHAN_FLOAT;
    case NKA_CHAN_VEC2: case NKA_CHAN_VEC2_REL: return NKA_CHAN_VEC2;
    case NKA_CHAN_VEC4: case NKA_CHAN_VEC4_REL: return NKA_CHAN_VEC4;
    case NKA_CHAN_INT: return NKA_CHAN_INT;
    default: return -1;
    }
}

static void nka__clip_map_reset(struct nka__map *m)
{
    if (m->keys) nka__zero(m->keys, (nk_size)m->cap * sizeof(nk_hash));
    m->len = 0;
}

NKA_API void nka_layer_begin(struct nka_context *a, nk_hash target)
{
    if (!a) return;
    nka__clip_map_reset(&a->clip_acc);
    a->clip_layer = target;
    a->clip_layer_weight = 0;
}

NKA_API void nka_layer_add(struct nka_context *a, nk_hash inst, float weight)
{
    struct nka__clip_inst *in = nka__clip_inst(a, inst);
    const struct nka__clip_clip *c = in ? nka__clip_get(a, in->clip) : 0;
    int t, i;
    if (!c || !a->clip_layer || !nka__clip_sync(a, in, c, 0)) return;
    weight *= in->weight;
    if (!(weight > 0)) return;
    a->clip_layer_weight += weight;
    for (t = 0; t < c->ntracks; ++t) {
        const struct nka__clip_track *tr = &c->tracks[t];
        int kind = nka__clip_kind(tr->type);
        struct nka__clip_acc *acc;
        float v[4];
        if (kind < 0 || (tr->type <= NKA_CHAN_VEC4 && nka__clip_track_of(c, tr->ch, tr->type + NKA_CHAN_FLOAT_REL) >= 0)) continue;
        nka__clip_resolve(a, tr, in->vals + t * 8, v);
        if (kind == NKA_CHAN_INT) v[0] = (float)in->vals[t * 8].i;
        acc = (struct nka__clip_acc *)nka__put(a, &a->clip_acc, nka_id_mix(tr->ch, (nk_hash)kind), 0);
        if (!acc) return;
        acc->ch = tr->ch;
        acc->kind = kind;
        acc->w += weight;
        for (i = 0; i < 4; ++i) acc->sum[i] += v[i] * weight;
    }
}

NKA_API void nka_layer_end(struct nka_context *a, nk_hash target)
{
    struct nka__clip_layer *ly;
    int i, k, n = 0, len;
    if (!a || !target || target != a->clip_layer) return;
    a->clip_layer = 0;
    if (!(a->clip_layer_weight > 0)) return;
    ly = (struct nka__clip_layer *)nka__put(a, &a->clip_layers, target, 0);
    if (!ly) return;
    ly->id = target;
    ly->seen = a->frame;
    len = a->clip_acc.len;
    if (ly->n != len) {
        struct nka__clip_blend *e = 0;
        if (len) {
            e = (struct nka__clip_blend *)nka__alloc(a, (nk_size)len * sizeof *e);
            if (!e) return;
        }
        nka__free(a, ly->e);
        ly->e = e;
        ly->n = len;
    }
    for (i = 0; i < a->clip_acc.cap; ++i) {
        const struct nka__clip_acc *acc;
        struct nka__clip_blend *b;
        if (!a->clip_acc.keys[i]) continue;
        acc = (const struct nka__clip_acc *)nka__at(&a->clip_acc, i);
        b = &ly->e[n++];
        b->ch = acc->ch;
        b->kind = acc->kind;
        for (k = 0; k < 4; ++k) b->v[k] = acc->sum[k] / acc->w;
        if (b->kind == NKA_CHAN_INT) b->v[0] = nka__floor(b->v[0] + 0.5f);
    }
}

static const float *nka__clip_blended(struct nka_context *a, nk_hash target, nk_hash ch, int kind)
{
    struct nka__clip_layer *ly;
    int i;
    if (!a || !target) return 0;
    ly = (struct nka__clip_layer *)nka__get(&a->clip_layers, target);
    if (!ly) return 0;
    ly->seen = a->frame;
    for (i = 0; i < ly->n; ++i)
        if (ly->e[i].ch == ch && ly->e[i].kind == kind) return ly->e[i].v;
    return 0;
}

NKA_API int nka_get_blended_float(struct nka_context *a, nk_hash target, nk_hash ch, float *out)
{
    const float *v = nka__clip_blended(a, target, ch, NKA_CHAN_FLOAT);
    if (!v || !out) return 0;
    *out = v[0];
    return 1;
}

NKA_API int nka_get_blended_vec2(struct nka_context *a, nk_hash target, nk_hash ch, struct nk_vec2 *out)
{
    const float *v = nka__clip_blended(a, target, ch, NKA_CHAN_VEC2);
    if (!v || !out) return 0;
    *out = nka__vec2(v[0], v[1]);
    return 1;
}

NKA_API int nka_get_blended_vec4(struct nka_context *a, nk_hash target, nk_hash ch, struct nka_vec4 *out)
{
    const float *v = nka__clip_blended(a, target, ch, NKA_CHAN_VEC4);
    if (!v || !out) return 0;
    *out = nka__vec4(v[0], v[1], v[2], v[3]);
    return 1;
}

NKA_API int nka_get_blended_int(struct nka_context *a, nk_hash target, nk_hash ch, int *out)
{
    const float *v = nka__clip_blended(a, target, ch, NKA_CHAN_INT);
    if (!v || !out) return 0;
    *out = nka__clip_toint(v[0]);
    return 1;
}

/* Saving. Words of four bytes, low byte first: "NKAC", version, size, clip id, the clip,
 * its tracks and keys, markers, and an FNV-1a sum of everything before it. */

struct nka__clip_out { unsigned char *p; int n, cap; };
struct nka__clip_in { const unsigned char *p; int n, at, bad; };

static nk_uint nka__clip_fnv(const unsigned char *p, int n)
{
    nk_uint h = 2166136261u;
    int i;
    for (i = 0; i < n; ++i) {
        h ^= p[i];
        h *= 16777619u;
    }
    return h;
}

static void nka__clip_w(struct nka__clip_out *o, nk_uint v)
{
    if (o->p && o->n + 4 <= o->cap) {
        o->p[o->n] = (unsigned char)(v & 0xffu);
        o->p[o->n + 1] = (unsigned char)((v >> 8) & 0xffu);
        o->p[o->n + 2] = (unsigned char)((v >> 16) & 0xffu);
        o->p[o->n + 3] = (unsigned char)(v >> 24);
    }
    o->n += 4;
}

static void nka__clip_wi(struct nka__clip_out *o, int v) { nka__clip_w(o, (nk_uint)v); }

static void nka__clip_wf(struct nka__clip_out *o, float v)
{
    union nka__clip_slot s;
    s.f = v;
    nka__clip_w(o, s.u);
}

static void nka__clip_wf4(struct nka__clip_out *o, const float *v, int n)
{
    int i;
    for (i = 0; i < n; ++i) nka__clip_wf(o, v[i]);
}

static nk_uint nka__clip_r(struct nka__clip_in *r)
{
    const unsigned char *p = r->p + r->at;
    if (r->bad || r->n - r->at < 4) {
        r->bad = 1;
        return 0;
    }
    r->at += 4;
    return (nk_uint)p[0] | ((nk_uint)p[1] << 8) | ((nk_uint)p[2] << 16) | ((nk_uint)p[3] << 24);
}

static int nka__clip_ri(struct nka__clip_in *r) { return nka__clip_s32(nka__clip_r(r)); }

static float nka__clip_rf(struct nka__clip_in *r)
{
    union nka__clip_slot s;
    s.u = nka__clip_r(r);
    return s.f;
}

/* A float that steers time: NaN means the buffer is corrupt. */
static float nka__clip_rt(struct nka__clip_in *r)
{
    float f = nka__clip_rf(r);
    if (f != f) r->bad = 1;
    return f;
}

static void nka__clip_rf4(struct nka__clip_in *r, float *v, int n)
{
    int i;
    for (i = 0; i < n; ++i) v[i] = nka__clip_rf(r);
}

static void nka__clip_wvarf(struct nka__clip_out *o, const struct nka_variation_float *v)
{
    nka__clip_wi(o, v->mode);
    nka__clip_wf(o, v->amount);
    nka__clip_wf(o, v->min_clamp);
    nka__clip_wf(o, v->max_clamp);
    nka__clip_w(o, v->seed);
}

static void nka__clip_rvarf(struct nka__clip_in *r, struct nka_variation_float *v)
{
    v->mode = nka__clip_ri(r);
    v->amount = nka__clip_rf(r);
    v->min_clamp = nka__clip_rf(r);
    v->max_clamp = nka__clip_rf(r);
    v->seed = nka__clip_r(r);
    v->callback = 0;
    v->user = 0;
}

/* A vec4 or color variation's numbers: mode, amount, clamps, then space for colors, seed. */
static void nka__clip_wvar4(struct nka__clip_out *o, int mode, struct nka_vec4 amount, struct nka_vec4 lo, struct nka_vec4 hi,
                            const int *space, nk_uint seed, const struct nka_variation_float *axes)
{
    float f[12];
    int i;
    nka__clip_v4f(amount, f);
    nka__clip_v4f(lo, f + 4);
    nka__clip_v4f(hi, f + 8);
    nka__clip_wi(o, mode);
    nka__clip_wf4(o, f, 12);
    if (space) nka__clip_wi(o, *space);
    nka__clip_w(o, seed);
    for (i = 0; i < 4; ++i) nka__clip_wvarf(o, &axes[i]);
}

static void nka__clip_rvar4(struct nka__clip_in *r, int *mode, struct nka_vec4 *amount, struct nka_vec4 *lo, struct nka_vec4 *hi,
                            int *space, nk_uint *seed, struct nka_variation_float *axes)
{
    float f[12];
    int i;
    *mode = nka__clip_ri(r);
    nka__clip_rf4(r, f, 12);
    *amount = nka__vec4(f[0], f[1], f[2], f[3]);
    *lo = nka__vec4(f[4], f[5], f[6], f[7]);
    *hi = nka__vec4(f[8], f[9], f[10], f[11]);
    if (space) *space = nka__clip_ri(r);
    *seed = nka__clip_r(r);
    for (i = 0; i < 4; ++i) nka__clip_rvarf(r, &axes[i]);
}

static void nka__clip_wvar(struct nka__clip_out *o, int type, const union nka__clip_var *v)
{
    struct nka_variation_float axes[4];
    float f[6];
    switch (type) {
    case NKA_CHAN_FLOAT:
        nka__clip_wvarf(o, &v->f);
        break;
    case NKA_CHAN_INT:
        nka__clip_wi(o, v->i.mode);
        nka__clip_wi(o, v->i.amount);
        nka__clip_wi(o, v->i.min_clamp);
        nka__clip_wi(o, v->i.max_clamp);
        nka__clip_w(o, v->i.seed);
        break;
    case NKA_CHAN_VEC2:
        f[0] = v->v2.amount.x;
        f[1] = v->v2.amount.y;
        f[2] = v->v2.min_clamp.x;
        f[3] = v->v2.min_clamp.y;
        f[4] = v->v2.max_clamp.x;
        f[5] = v->v2.max_clamp.y;
        nka__clip_wi(o, v->v2.mode);
        nka__clip_wf4(o, f, 6);
        nka__clip_w(o, v->v2.seed);
        nka__clip_wvarf(o, &v->v2.x);
        nka__clip_wvarf(o, &v->v2.y);
        break;
    case NKA_CHAN_VEC4:
        axes[0] = v->v4.x;
        axes[1] = v->v4.y;
        axes[2] = v->v4.z;
        axes[3] = v->v4.w;
        nka__clip_wvar4(o, v->v4.mode, v->v4.amount, v->v4.min_clamp, v->v4.max_clamp, 0, v->v4.seed, axes);
        break;
    default:
        axes[0] = v->c.r;
        axes[1] = v->c.g;
        axes[2] = v->c.b;
        axes[3] = v->c.a;
        nka__clip_wvar4(o, v->c.mode, v->c.amount, v->c.min_clamp, v->c.max_clamp, &v->c.space, v->c.seed, axes);
        break;
    }
}

static void nka__clip_rvar(struct nka__clip_in *r, int type, union nka__clip_var *v)
{
    struct nka_variation_float axes[4];
    float f[6];
    nka__zero(v, sizeof *v);
    switch (type) {
    case NKA_CHAN_FLOAT:
        nka__clip_rvarf(r, &v->f);
        break;
    case NKA_CHAN_INT:
        v->i.mode = nka__clip_ri(r);
        v->i.amount = nka__clip_ri(r);
        v->i.min_clamp = nka__clip_ri(r);
        v->i.max_clamp = nka__clip_ri(r);
        v->i.seed = nka__clip_r(r);
        break;
    case NKA_CHAN_VEC2:
        v->v2.mode = nka__clip_ri(r);
        nka__clip_rf4(r, f, 6);
        v->v2.amount = nka__vec2(f[0], f[1]);
        v->v2.min_clamp = nka__vec2(f[2], f[3]);
        v->v2.max_clamp = nka__vec2(f[4], f[5]);
        v->v2.seed = nka__clip_r(r);
        nka__clip_rvarf(r, &v->v2.x);
        nka__clip_rvarf(r, &v->v2.y);
        break;
    case NKA_CHAN_VEC4:
        nka__clip_rvar4(r, &v->v4.mode, &v->v4.amount, &v->v4.min_clamp, &v->v4.max_clamp, 0, &v->v4.seed, axes);
        v->v4.x = axes[0];
        v->v4.y = axes[1];
        v->v4.z = axes[2];
        v->v4.w = axes[3];
        break;
    case NKA_CHAN_COLOR:
        nka__clip_rvar4(r, &v->c.mode, &v->c.amount, &v->c.min_clamp, &v->c.max_clamp, &v->c.space, &v->c.seed, axes);
        v->c.r = axes[0];
        v->c.g = axes[1];
        v->c.b = axes[2];
        v->c.a = axes[3];
        break;
    default:
        r->bad = 1;
        break;
    }
}

static void nka__clip_write(struct nka__clip_out *o, const struct nka__clip_clip *c, int size)
{
    int t, k, i;
    nka__clip_w(o, NKA__CLIP_MAGIC);
    nka__clip_w(o, NKA__CLIP_VERSION);
    nka__clip_wi(o, size);
    nka__clip_w(o, c->id);
    nka__clip_wf(o, c->duration);
    nka__clip_wf(o, c->delay);
    nka__clip_wf(o, c->loop_delay);
    nka__clip_wi(o, c->loop_count);
    nka__clip_wi(o, c->direction);
    nka__clip_wi(o, c->stagger_count);
    nka__clip_wf(o, c->stagger_delay);
    nka__clip_wf(o, c->stagger_bias);
    nka__clip_wi(o, c->stagger_ease);
    nka__clip_wi(o, (c->has_dur_var ? 1 : 0) | (c->has_delay_var ? 2 : 0) | (c->has_scale_var ? 4 : 0));
    nka__clip_wvarf(o, &c->dur_var);
    nka__clip_wvarf(o, &c->delay_var);
    nka__clip_wvarf(o, &c->scale_var);
    nka__clip_wi(o, c->ntracks);
    for (t = 0; t < c->ntracks; ++t) {
        const struct nka__clip_track *tr = &c->tracks[t];
        nka__clip_w(o, tr->ch);
        nka__clip_wi(o, tr->type);
        nka__clip_wi(o, tr->space);
        nka__clip_wi(o, tr->anchor);
        nka__clip_wi(o, tr->axis);
        nka__clip_wi(o, tr->count);
        for (k = tr->first; k < tr->first + tr->count; ++k) {
            const struct nka__clip_key *key = &c->keys[k];
            nka__clip_wf(o, key->time);
            nka__clip_wi(o, key->ease);
            nka__clip_wi(o, key->flags | (key->var >= 0 ? NKA__CLIP_VARIED : 0));
            nka__clip_wf4(o, key->bezier, 4);
            nka__clip_wf4(o, key->spring, 4);
            for (i = 0; i < 4; ++i) nka__clip_w(o, key->v[i].u);
            nka__clip_wf4(o, key->ext, 4);
            if (key->var >= 0) nka__clip_wvar(o, tr->type, &c->vars[key->var]);
        }
    }
    nka__clip_wi(o, c->nmarkers);
    for (i = 0; i < c->nmarkers; ++i) {
        nka__clip_wf(o, c->markers[i].time);
        nka__clip_w(o, c->markers[i].id);
    }
    nka__clip_w(o, 0);
}

NKA_API int nka_clip_save(const struct nka_context *a, nk_hash clip, void *buf, int cap)
{
    const struct nka__clip_clip *c = nka__clip_get(a, clip);
    struct nka__clip_out o;
    int size;
    if (!c) return 0;
    o.p = 0;
    o.n = o.cap = 0;
    nka__clip_write(&o, c, 0);
    size = o.n;
    if (buf && cap >= size) {
        o.p = (unsigned char *)buf;
        o.n = 0;
        o.cap = cap;
        nka__clip_write(&o, c, size);
        o.n = size - 4;
        nka__clip_w(&o, nka__clip_fnv(o.p, size - 4));
    }
    return size;
}

/* Reads a clip after the header. With keys, vars and markers NULL it only counts them. */
static void nka__clip_read(struct nka__clip_in *r, struct nka__clip_clip *c, struct nka__clip_key *keys, union nka__clip_var *vars,
                           struct nka__clip_marker *markers, int *nkeys, int *nvars, int *nmarkers)
{
    int nt, t, k, i, nk = 0, nv = 0, nm, flags;
    nka__zero(c, sizeof *c);
    r->at = 12;
    c->id = nka__clip_r(r);
    c->duration = nka__clip_rt(r);
    c->delay = nka__clip_rt(r);
    c->loop_delay = nka__clip_rt(r);
    c->loop_count = nka__clip_ri(r);
    c->direction = nka__clip_ri(r);
    c->stagger_count = nka__clip_ri(r);
    c->stagger_delay = nka__clip_rt(r);
    c->stagger_bias = nka__clip_rt(r);
    c->stagger_ease = nka__clip_ri(r);
    flags = nka__clip_ri(r);
    if ((flags & ~7) || c->duration < 0) r->bad = 1;
    c->has_dur_var = flags & 1;
    c->has_delay_var = (flags >> 1) & 1;
    c->has_scale_var = (flags >> 2) & 1;
    nka__clip_rvarf(r, &c->dur_var);
    nka__clip_rvarf(r, &c->delay_var);
    nka__clip_rvarf(r, &c->scale_var);
    nt = nka__clip_ri(r);
    if (nt < 0 || nt > (r->n - r->at) / 24) r->bad = 1;
    for (t = 0; t < nt && !r->bad; ++t) {
        nk_hash ch = nka__clip_r(r);
        int type = nka__clip_ri(r), space = nka__clip_ri(r), anchor = nka__clip_ri(r), axis = nka__clip_ri(r);
        int count = nka__clip_ri(r);
        if (type < NKA_CHAN_FLOAT || type > NKA_CHAN_COLOR_REL || count < 1 || count > (r->n - r->at) / 76) r->bad = 1;
        for (k = 0; k < count && !r->bad; ++k) {
            struct nka__clip_key tmp, *key = keys ? &keys[nk] : &tmp;
            nka__zero(key, sizeof *key);
            key->ch = ch;
            key->type = type;
            key->space = space;
            key->anchor = anchor;
            key->axis = axis;
            key->time = nka__clip_rt(r);
            key->ease = nka__clip_ri(r);
            flags = nka__clip_ri(r);
            if ((flags & ~7) || ((flags & NKA__CLIP_VARIED) && type >= NKA_CHAN_FLOAT_REL)) r->bad = 1;
            key->flags = flags & (NKA__CLIP_BEZIER | NKA__CLIP_SPRING);
            nka__clip_rf4(r, key->bezier, 4);
            nka__clip_rf4(r, key->spring, 4);
            for (i = 0; i < 4; ++i) key->v[i].u = nka__clip_r(r);
            nka__clip_rf4(r, key->ext, 4);
            key->var = -1;
            if (flags & NKA__CLIP_VARIED) {
                union nka__clip_var tv;
                nka__clip_rvar(r, type, vars ? &vars[nv] : &tv);
                key->var = nv++;
            }
            nk++;
        }
    }
    nm = nka__clip_ri(r);
    if (nm < 0 || nm > (r->n - r->at) / 8) r->bad = 1;
    for (i = 0; i < nm && !r->bad; ++i) {
        struct nka__clip_marker tmp, *mk = markers ? &markers[i] : &tmp;
        mk->time = nka__clip_rt(r);
        mk->id = nka__clip_r(r);
        mk->fn = 0;
        mk->user = 0;
    }
    if (r->at != r->n) r->bad = 1;
    *nkeys = nk;
    *nvars = nv;
    *nmarkers = nm;
}

NKA_API int nka_clip_load(struct nka_context *a, const void *buf, int len, nk_hash *out_id)
{
    struct nka__clip_in r;
    struct nka__clip_clip c;
    struct nka__clip_key *keys = 0;
    union nka__clip_var *vars = 0;
    struct nka__clip_marker *markers = 0;
    nk_uint size, sum;
    int nk, nv, nm, res = NKA_ERR_NO_MEM;
    if (!a || !buf || len < 20) return NKA_ERR_BAD_ARG;
    r.p = (const unsigned char *)buf;
    r.n = len;
    r.at = 0;
    r.bad = 0;
    if (nka__clip_r(&r) != NKA__CLIP_MAGIC || nka__clip_r(&r) != NKA__CLIP_VERSION) return NKA_ERR_BAD_ARG;
    size = nka__clip_r(&r);
    if (size < 20 || size > (nk_uint)len || size % 4) return NKA_ERR_BAD_ARG;
    r.at = (int)size - 4;
    sum = nka__clip_r(&r);
    if (sum != nka__clip_fnv(r.p, (int)size - 4)) return NKA_ERR_BAD_ARG;
    r.n = (int)size - 4;
    nka__clip_read(&r, &c, 0, 0, 0, &nk, &nv, &nm);
    if (r.bad) return NKA_ERR_BAD_ARG;
    if (nk) keys = (struct nka__clip_key *)nka__alloc(a, (nk_size)nk * sizeof *keys);
    if (nv) vars = (union nka__clip_var *)nka__alloc(a, (nk_size)nv * sizeof *vars);
    if (nm) markers = (struct nka__clip_marker *)nka__alloc(a, (nk_size)nm * sizeof *markers);
    if ((!nk || keys) && (!nv || vars) && (!nm || markers)) {
        nka__clip_read(&r, &c, keys, vars, markers, &nk, &nv, &nm);
        res = nka__clip_install(a, &c, keys, nk, vars, nv, markers, nm, 1);
    }
    nka__free(a, keys);
    nka__free(a, vars);
    nka__free(a, markers);
    if (res == NKA_OK && out_id) *out_id = c.id;
    return res;
}

/* Oscillators, shakes and wiggles */

static double nka__fx_floor(double x)
{
    double i;
    if (!(x > -4e18 && x < 4e18)) return x;
    i = (double)(long long)x;
    return i > x ? i - 1 : i;
}

static struct nka__fx_clock *nka__fx_tick(struct nka_context *a, nk_hash id, int kind)
{
    struct nka__fx_clock *c;
    if (!a) return 0;
    c = (struct nka__fx_clock *)nka__put(a, &a->fx_clocks, nka_id_mix(id, (nk_hash)kind), 0);
    if (!c) return 0;
    c->seen = a->frame;
    if (c->frame != a->frame) {
        c->frame = a->frame;
        c->time += a->dt;
        if (c->triggered) c->since += a->dt;
    }
    return c;
}

static double nka__fx_time(struct nka_context *a, nk_hash id, int kind)
{
    struct nka__fx_clock *c = nka__fx_tick(a, id, kind);
    if (!a) return 0;
    a->busy = 1;
    return c ? c->time : a->time;
}

static float nka__fx_wave(int wave, double x)
{
    float t = (float)(x - nka__fx_floor(x));
    if (t >= 1) t = 0;
    switch (wave) {
    case NKA_WAVE_TRIANGLE: return t < 0.5f ? 4 * t - 1 : 3 - 4 * t;
    case NKA_WAVE_SAWTOOTH: return 2 * t - 1;
    case NKA_WAVE_SQUARE: return t < 0.5f ? 1.0f : -1.0f;
    default: return nka__sin(t * 2 * NKA__PI);
    }
}

static int nka__fx_round(float v) { return (int)(v > 0 ? v + 0.5f : v - 0.5f); }

static struct nk_colorf nka__fx_offset(struct nk_colorf base, struct nka_vec4 d, int space)
{
    struct nka_vec4 c = nka__to_space(nka__cv(base), space);
    c.x += d.x; c.y += d.y; c.z += d.z; c.w += d.w;
    if (space == NKA_COL_HSV) {
        c.x = nka__fmod(c.x + 1, 1);
        c.y = nka__clamp01(c.y);
        c.z = nka__clamp01(c.z);
    } else if (space == NKA_COL_OKLCH) {
        c.z = nka__fmod(c.z + 1, 1);
    }
    c = nka__from_space(c, space);
    return nka__vc(nka__vec4(nka__clamp01(c.x), nka__clamp01(c.y), nka__clamp01(c.z), nka__clamp01(c.w)));
}

NKA_API float nka_oscillate(struct nka_context *a, nk_hash id, float amplitude, float frequency, int wave, float phase)
{
    return amplitude * nka__fx_wave(wave, nka__fx_time(a, id, NKA__FX_OSC) * frequency + phase);
}

NKA_API int nka_oscillate_int(struct nka_context *a, nk_hash id, int amplitude, float frequency, int wave, float phase)
{
    return nka__fx_round(nka_oscillate(a, id, (float)amplitude, frequency, wave, phase));
}

NKA_API struct nk_vec2 nka_oscillate_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 amplitude,
                                          struct nk_vec2 frequency, int wave, struct nk_vec2 phase)
{
    double t = nka__fx_time(a, id, NKA__FX_OSC);
    return nka__vec2(amplitude.x * nka__fx_wave(wave, t * frequency.x + phase.x),
                     amplitude.y * nka__fx_wave(wave, t * frequency.y + phase.y));
}

NKA_API struct nka_vec4 nka_oscillate_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 amplitude,
                                           struct nka_vec4 frequency, int wave, struct nka_vec4 phase)
{
    double t = nka__fx_time(a, id, NKA__FX_OSC);
    return nka__vec4(amplitude.x * nka__fx_wave(wave, t * frequency.x + phase.x),
                     amplitude.y * nka__fx_wave(wave, t * frequency.y + phase.y),
                     amplitude.z * nka__fx_wave(wave, t * frequency.z + phase.z),
                     amplitude.w * nka__fx_wave(wave, t * frequency.w + phase.w));
}

NKA_API struct nk_colorf nka_oscillate_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 amplitude,
                                             float frequency, int wave, float phase, int space)
{
    float w = nka_oscillate(a, id, 1, frequency, wave, phase);
    return nka__fx_offset(base, nka__vec4(amplitude.x * w, amplitude.y * w, amplitude.z * w, amplitude.w * w), space);
}

static const nk_hash nka__fx_salt[4] = { 0, 0x12345678u, 0x23456789u, 0x3456789au };

static float nka__fx_hash(nk_hash s)
{
    s = (s ^ 61u) ^ (s >> 16);
    s += s << 3;
    s ^= s >> 4;
    s *= 0x27d4eb2du;
    s ^= s >> 15;
    return (float)(s & 0xffffu) / 32768.0f - 1;
}

static float nka__fx_jitter(nk_hash seed, double x, int smooth)
{
    double i = nka__fx_floor(x);
    float f = (float)(x - i), n0, n1;
    nk_hash k = 0;
    if (i > -4e18 && i < 4e18) k = (nk_hash)(long long)i;
    if (smooth) f = f * f * (3 - 2 * f);
    n0 = nka__fx_hash(seed + k);
    n1 = nka__fx_hash(seed + k + 1);
    return n0 + (n1 - n0) * f;
}

static void nka__fx_shake(struct nka_context *a, nk_hash id, float frequency, float decay_time, float *out, int n)
{
    struct nka__fx_clock *c = nka__fx_tick(a, id, NKA__FX_SHAKE);
    float decay;
    int i;
    for (i = 0; i < n; ++i) out[i] = 0;
    if (!c || !c->triggered) return;
    if (decay_time <= 0 || c->since >= decay_time) {
        c->triggered = 0;
        return;
    }
    decay = 1 - (float)(c->since / decay_time);
    for (i = 0; i < n; ++i) out[i] = nka__fx_jitter(id ^ nka__fx_salt[i], c->time * frequency, 0) * decay * decay;
    a->busy = 1;
}

static void nka__fx_wiggle(struct nka_context *a, nk_hash id, float frequency, float *out, int n)
{
    double t = nka__fx_time(a, id, NKA__FX_WIGGLE);
    int i;
    for (i = 0; i < n; ++i) out[i] = nka__fx_jitter(id ^ nka__fx_salt[i], t * frequency, 1);
}

NKA_API void nka_trigger_shake(struct nka_context *a, nk_hash id)
{
    struct nka__fx_clock *c;
    if (!a) return;
    c = (struct nka__fx_clock *)nka__put(a, &a->fx_clocks, nka_id_mix(id, (nk_hash)NKA__FX_SHAKE), 0);
    if (!c) return;
    c->seen = a->frame;
    c->triggered = 1;
    c->since = 0;
}

NKA_API float nka_shake(struct nka_context *a, nk_hash id, float intensity, float frequency, float decay_time)
{
    float v;
    nka__fx_shake(a, id, frequency, decay_time, &v, 1);
    return v * intensity;
}

NKA_API int nka_shake_int(struct nka_context *a, nk_hash id, int intensity, float frequency, float decay_time)
{
    return nka__fx_round(nka_shake(a, id, (float)intensity, frequency, decay_time));
}

NKA_API struct nk_vec2 nka_shake_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 intensity, float frequency, float decay_time)
{
    float v[2];
    nka__fx_shake(a, id, frequency, decay_time, v, 2);
    return nka__vec2(v[0] * intensity.x, v[1] * intensity.y);
}

NKA_API struct nka_vec4 nka_shake_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 intensity, float frequency, float decay_time)
{
    float v[4];
    nka__fx_shake(a, id, frequency, decay_time, v, 4);
    return nka__vec4(v[0] * intensity.x, v[1] * intensity.y, v[2] * intensity.z, v[3] * intensity.w);
}

NKA_API struct nk_colorf nka_shake_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 intensity,
                                         float frequency, float decay_time, int space)
{
    return nka__fx_offset(base, nka_shake_vec4(a, id, intensity, frequency, decay_time), space);
}

NKA_API float nka_wiggle(struct nka_context *a, nk_hash id, float amplitude, float frequency)
{
    float v;
    nka__fx_wiggle(a, id, frequency, &v, 1);
    return v * amplitude;
}

NKA_API int nka_wiggle_int(struct nka_context *a, nk_hash id, int amplitude, float frequency)
{
    return nka__fx_round(nka_wiggle(a, id, (float)amplitude, frequency));
}

NKA_API struct nk_vec2 nka_wiggle_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 amplitude, float frequency)
{
    float v[2];
    nka__fx_wiggle(a, id, frequency, v, 2);
    return nka__vec2(v[0] * amplitude.x, v[1] * amplitude.y);
}

NKA_API struct nka_vec4 nka_wiggle_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 amplitude, float frequency)
{
    float v[4];
    nka__fx_wiggle(a, id, frequency, v, 4);
    return nka__vec4(v[0] * amplitude.x, v[1] * amplitude.y, v[2] * amplitude.z, v[3] * amplitude.w);
}

NKA_API struct nk_colorf nka_wiggle_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 amplitude,
                                          float frequency, int space)
{
    return nka__fx_offset(base, nka_wiggle_vec4(a, id, amplitude, frequency), space);
}

/* Noise */

static const unsigned char nka__fx_perm[256] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static int nka__fx_p(int i) { return nka__fx_perm[i & 255]; }
static float nka__fx_fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

static float nka__fx_grad2(int h, float x, float y)
{
    float u = (h & 7) < 4 ? x : y, v = (h & 7) < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2 * v : 2 * v);
}

static float nka__fx_grad3(int h, float x, float y, float z)
{
    float u, v;
    h &= 15;
    u = h < 8 ? x : y;
    v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

static float nka__fx_perlin2(float x, float y, int seed)
{
    float fx, fy, u, v;
    int X, Y, A, B;
    x += (float)seed * 12.9898f;
    y += (float)seed * 78.233f;
    fx = nka__floor(x);
    fy = nka__floor(y);
    X = (int)fx & 255;
    Y = (int)fy & 255;
    x -= fx;
    y -= fy;
    u = nka__fx_fade(x);
    v = nka__fx_fade(y);
    A = nka__fx_p(X) + Y;
    B = nka__fx_p(X + 1) + Y;
    return nka__lerp(nka__lerp(nka__fx_grad2(nka__fx_p(A), x, y), nka__fx_grad2(nka__fx_p(B), x - 1, y), u),
                     nka__lerp(nka__fx_grad2(nka__fx_p(A + 1), x, y - 1), nka__fx_grad2(nka__fx_p(B + 1), x - 1, y - 1), u),
                     v) * 0.5f;
}

static float nka__fx_perlin3(float x, float y, float z, int seed)
{
    float fx, fy, fz, u, v, w, n0, n1;
    int X, Y, Z, A, AA, AB, B, BA, BB;
    x += (float)seed * 12.9898f;
    y += (float)seed * 78.233f;
    z += (float)seed * 37.719f;
    fx = nka__floor(x);
    fy = nka__floor(y);
    fz = nka__floor(z);
    X = (int)fx & 255;
    Y = (int)fy & 255;
    Z = (int)fz & 255;
    x -= fx;
    y -= fy;
    z -= fz;
    u = nka__fx_fade(x);
    v = nka__fx_fade(y);
    w = nka__fx_fade(z);
    A = nka__fx_p(X) + Y;
    AA = nka__fx_p(A) + Z;
    AB = nka__fx_p(A + 1) + Z;
    B = nka__fx_p(X + 1) + Y;
    BA = nka__fx_p(B) + Z;
    BB = nka__fx_p(B + 1) + Z;
    n0 = nka__lerp(nka__lerp(nka__fx_grad3(nka__fx_p(AA), x, y, z), nka__fx_grad3(nka__fx_p(BA), x - 1, y, z), u),
                   nka__lerp(nka__fx_grad3(nka__fx_p(AB), x, y - 1, z), nka__fx_grad3(nka__fx_p(BB), x - 1, y - 1, z), u), v);
    n1 = nka__lerp(nka__lerp(nka__fx_grad3(nka__fx_p(AA + 1), x, y, z - 1), nka__fx_grad3(nka__fx_p(BA + 1), x - 1, y, z - 1), u),
                   nka__lerp(nka__fx_grad3(nka__fx_p(AB + 1), x, y - 1, z - 1),
                             nka__fx_grad3(nka__fx_p(BB + 1), x - 1, y - 1, z - 1), u), v);
    return nka__lerp(n0, n1, w) * 0.5f;
}

static float nka__fx_corner2(int h, float x, float y)
{
    static const float g[8][2] = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
        { 0.70710678f, 0.70710678f }, { -0.70710678f, 0.70710678f },
        { 0.70710678f, -0.70710678f }, { -0.70710678f, -0.70710678f }
    };
    float t = 0.5f - x * x - y * y;
    if (t < 0) return 0;
    t *= t;
    return t * t * (g[h & 7][0] * x + g[h & 7][1] * y);
}

static float nka__fx_simplex2(float x, float y, int seed)
{
    const float f2 = 0.366025403f, g2 = 0.211324865f;
    float s, t, x0, y0;
    int i, j, i1;
    x += (float)seed * 12.9898f;
    y += (float)seed * 78.233f;
    s = (x + y) * f2;
    i = (int)nka__floor(x + s);
    j = (int)nka__floor(y + s);
    t = (float)(i + j) * g2;
    x0 = x - ((float)i - t);
    y0 = y - ((float)j - t);
    i1 = x0 > y0;
    return 45.23065f * (nka__fx_corner2(nka__fx_p(i + nka__fx_p(j)), x0, y0)
                        + nka__fx_corner2(nka__fx_p(i + i1 + nka__fx_p(j + !i1)), x0 - (float)i1 + g2, y0 - (float)!i1 + g2)
                        + nka__fx_corner2(nka__fx_p(i + 1 + nka__fx_p(j + 1)), x0 - 1 + 2 * g2, y0 - 1 + 2 * g2));
}

static float nka__fx_corner3(int h, float x, float y, float z)
{
    static const signed char g[12][3] = {
        { 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 }, { 1, 0, 1 }, { -1, 0, 1 },
        { 1, 0, -1 }, { -1, 0, -1 }, { 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
    };
    float t = 0.6f - x * x - y * y - z * z;
    if (t < 0) return 0;
    t *= t;
    h %= 12;
    return t * t * ((float)g[h][0] * x + (float)g[h][1] * y + (float)g[h][2] * z);
}

static float nka__fx_simplex3(float x, float y, float z, int seed)
{
    const float g3 = 1.0f / 6;
    float s, t, x0, y0, z0;
    int i, j, k, i1, j1, k1, i2, j2, k2;
    x += (float)seed * 12.9898f;
    y += (float)seed * 78.233f;
    z += (float)seed * 37.719f;
    s = (x + y + z) / 3;
    i = (int)nka__floor(x + s);
    j = (int)nka__floor(y + s);
    k = (int)nka__floor(z + s);
    t = (float)(i + j + k) * g3;
    x0 = x - ((float)i - t);
    y0 = y - ((float)j - t);
    z0 = z - ((float)k - t);
    if (x0 >= y0) {
        if (y0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
        else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; }
        else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; }
    } else {
        if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; }
        else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; }
        else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
    }
    return 32 * (nka__fx_corner3(nka__fx_p(i + nka__fx_p(j + nka__fx_p(k))), x0, y0, z0)
                 + nka__fx_corner3(nka__fx_p(i + i1 + nka__fx_p(j + j1 + nka__fx_p(k + k1))),
                                   x0 - (float)i1 + g3, y0 - (float)j1 + g3, z0 - (float)k1 + g3)
                 + nka__fx_corner3(nka__fx_p(i + i2 + nka__fx_p(j + j2 + nka__fx_p(k + k2))),
                                   x0 - (float)i2 + 2 * g3, y0 - (float)j2 + 2 * g3, z0 - (float)k2 + 2 * g3)
                 + nka__fx_corner3(nka__fx_p(i + 1 + nka__fx_p(j + 1 + nka__fx_p(k + 1))),
                                   x0 - 1 + 3 * g3, y0 - 1 + 3 * g3, z0 - 1 + 3 * g3));
}

static float nka__fx_lattice2(int x, int y, int seed) { return (float)nka__fx_p(nka__fx_p(x + seed) + y) / 255.0f * 2 - 1; }

static float nka__fx_lattice3(int x, int y, int z, int seed)
{
    return (float)nka__fx_p(nka__fx_p(nka__fx_p(x + seed) + y) + z) / 255.0f * 2 - 1;
}

static float nka__fx_value2(float x, float y, int seed)
{
    float fx = nka__floor(x), fy = nka__floor(y), u = nka__fx_fade(x - fx), v = nka__fx_fade(y - fy);
    int xi = (int)fx, yi = (int)fy;
    return nka__lerp(nka__lerp(nka__fx_lattice2(xi, yi, seed), nka__fx_lattice2(xi + 1, yi, seed), u),
                     nka__lerp(nka__fx_lattice2(xi, yi + 1, seed), nka__fx_lattice2(xi + 1, yi + 1, seed), u), v);
}

static float nka__fx_value3(float x, float y, float z, int seed)
{
    float fx = nka__floor(x), fy = nka__floor(y), fz = nka__floor(z), n[2];
    float u = nka__fx_fade(x - fx), v = nka__fx_fade(y - fy), w = nka__fx_fade(z - fz);
    int xi = (int)fx, yi = (int)fy, zi = (int)fz, c;
    for (c = 0; c < 2; ++c)
        n[c] = nka__lerp(nka__lerp(nka__fx_lattice3(xi, yi, zi + c, seed), nka__fx_lattice3(xi + 1, yi, zi + c, seed), u),
                         nka__lerp(nka__fx_lattice3(xi, yi + 1, zi + c, seed), nka__fx_lattice3(xi + 1, yi + 1, zi + c, seed), u), v);
    return nka__lerp(n[0], n[1], w);
}

static float nka__fx_worley2(float x, float y, int seed)
{
    float best = 1e10f;
    int xi, yi, dx, dy;
    x += (float)seed * 12.9898f;
    y += (float)seed * 78.233f;
    xi = (int)nka__floor(x);
    yi = (int)nka__floor(y);
    for (dy = -1; dy <= 1; ++dy)
        for (dx = -1; dx <= 1; ++dx) {
            int cx = xi + dx, cy = yi + dy, h = nka__fx_p(cx + nka__fx_p(cy));
            float px = x - ((float)cx + (float)nka__fx_p(h) / 255.0f);
            float py = y - ((float)cy + (float)nka__fx_p(h + 1) / 255.0f);
            float d = px * px + py * py;
            if (d < best) best = d;
        }
    return nka__sqrt(best) * 1.4f - 1;
}

static float nka__fx_worley3(float x, float y, float z, int seed)
{
    float best = 1e10f;
    int xi, yi, zi, dx, dy, dz;
    x += (float)seed * 12.9898f;
    y += (float)seed * 78.233f;
    z += (float)seed * 37.719f;
    xi = (int)nka__floor(x);
    yi = (int)nka__floor(y);
    zi = (int)nka__floor(z);
    for (dz = -1; dz <= 1; ++dz)
        for (dy = -1; dy <= 1; ++dy)
            for (dx = -1; dx <= 1; ++dx) {
                int cx = xi + dx, cy = yi + dy, cz = zi + dz, h = nka__fx_p(cx + nka__fx_p(cy + nka__fx_p(cz)));
                float px = x - ((float)cx + (float)nka__fx_p(h) / 255.0f);
                float py = y - ((float)cy + (float)nka__fx_p(h + 1) / 255.0f);
                float pz = z - ((float)cz + (float)nka__fx_p(h + 2) / 255.0f);
                float d = px * px + py * py + pz * pz;
                if (d < best) best = d;
            }
    return nka__sqrt(best) * 1.1547005f - 1;
}

static float nka__fx_octave(int type, int dims, float x, float y, float z, int seed)
{
    switch (type) {
    case NKA_NOISE_SIMPLEX: return dims == 3 ? nka__fx_simplex3(x, y, z, seed) : nka__fx_simplex2(x, y, seed);
    case NKA_NOISE_VALUE: return dims == 3 ? nka__fx_value3(x, y, z, seed) : nka__fx_value2(x, y, seed);
    case NKA_NOISE_WORLEY: return dims == 3 ? nka__fx_worley3(x, y, z, seed) : nka__fx_worley2(x, y, seed);
    default: return dims == 3 ? nka__fx_perlin3(x, y, z, seed) : nka__fx_perlin2(x, y, seed);
    }
}

static float nka__fx_fractal(int dims, float x, float y, float z, const struct nka_noise_opts *opts)
{
    struct nka_noise_opts o = opts ? *opts : nka_noise_opts_default();
    float total = 0, norm = 0, amp = 1, freq = 1, v;
    int i;
    if (o.octaves < 1) o.octaves = 1;
    for (i = 0; i < o.octaves; ++i) {
        total += nka__fx_octave(o.type, dims, x * freq, y * freq, z * freq, o.seed) * amp;
        norm += nka__fabs(amp);
        amp *= o.persistence;
        freq *= o.lacunarity;
    }
    v = total / norm;
    return v < -1 ? -1.0f : (v > 1 ? 1.0f : v);
}

NKA_API struct nka_noise_opts nka_noise_opts_default(void)
{
    struct nka_noise_opts o;
    o.type = NKA_NOISE_PERLIN;
    o.octaves = 4;
    o.persistence = 0.5f;
    o.lacunarity = 2;
    o.seed = 0;
    return o;
}

NKA_API float nka_noise_2d(float x, float y, const struct nka_noise_opts *opts) { return nka__fx_fractal(2, x, y, 0, opts); }
NKA_API float nka_noise_3d(float x, float y, float z, const struct nka_noise_opts *opts) { return nka__fx_fractal(3, x, y, z, opts); }

NKA_API float nka_noise_channel_float(struct nka_context *a, nk_hash id, float frequency, float amplitude,
                                      const struct nka_noise_opts *opts)
{
    return nka_noise_2d((float)(nka__fx_time(a, id, NKA__FX_NOISE) * frequency), NKA__FX_ROW0, opts) * amplitude;
}

NKA_API struct nk_vec2 nka_noise_channel_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 frequency, struct nk_vec2 amplitude,
                                              const struct nka_noise_opts *opts)
{
    double t = nka__fx_time(a, id, NKA__FX_NOISE);
    return nka__vec2(nka_noise_2d((float)(t * frequency.x), NKA__FX_ROW0, opts) * amplitude.x,
                     nka_noise_2d((float)(t * frequency.y), NKA__FX_ROW1, opts) * amplitude.y);
}

NKA_API struct nka_vec4 nka_noise_channel_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 frequency, struct nka_vec4 amplitude,
                                               const struct nka_noise_opts *opts)
{
    double t = nka__fx_time(a, id, NKA__FX_NOISE);
    return nka__vec4(nka_noise_2d((float)(t * frequency.x), NKA__FX_ROW0, opts) * amplitude.x,
                     nka_noise_2d((float)(t * frequency.y), NKA__FX_ROW1, opts) * amplitude.y,
                     nka_noise_2d((float)(t * frequency.z), NKA__FX_ROW2, opts) * amplitude.z,
                     nka_noise_2d((float)(t * frequency.w), NKA__FX_ROW3, opts) * amplitude.w);
}

NKA_API struct nk_colorf nka_noise_channel_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 amplitude,
                                                 float frequency, const struct nka_noise_opts *opts, int space)
{
    struct nka_vec4 f = nka__vec4(frequency, frequency, frequency, frequency);
    return nka__fx_offset(base, nka_noise_channel_vec4(a, id, f, amplitude, opts), space);
}

static struct nka_noise_opts nka__fx_smooth(void)
{
    struct nka_noise_opts o = nka_noise_opts_default();
    o.octaves = 2;
    return o;
}

NKA_API float nka_smooth_noise_float(struct nka_context *a, nk_hash id, float amplitude, float speed)
{
    struct nka_noise_opts o = nka__fx_smooth();
    return nka_noise_channel_float(a, id, speed, amplitude, &o);
}

NKA_API struct nk_vec2 nka_smooth_noise_vec2(struct nka_context *a, nk_hash id, struct nk_vec2 amplitude, float speed)
{
    struct nka_noise_opts o = nka__fx_smooth();
    return nka_noise_channel_vec2(a, id, nka__vec2(speed, speed), amplitude, &o);
}

NKA_API struct nka_vec4 nka_smooth_noise_vec4(struct nka_context *a, nk_hash id, struct nka_vec4 amplitude, float speed)
{
    struct nka_noise_opts o = nka__fx_smooth();
    return nka_noise_channel_vec4(a, id, nka__vec4(speed, speed, speed, speed), amplitude, &o);
}

NKA_API struct nk_colorf nka_smooth_noise_color(struct nka_context *a, nk_hash id, struct nk_colorf base, struct nka_vec4 amplitude,
                                                float speed, int space)
{
    struct nka_noise_opts o = nka__fx_smooth();
    return nka_noise_channel_color(a, id, base, amplitude, speed, &o, space);
}

/* Gradients */

static int nka__fx_stops(const struct nka_gradient *g)
{
    if (!g || g->count < 0) return 0;
    return g->count < NKA_GRADIENT_MAX ? g->count : NKA_GRADIENT_MAX;
}

static struct nk_colorf nka__fx_mix(struct nk_colorf x, struct nk_colorf y, float t, int space)
{
    if (t == 0) return x;
    if (t == 1) return y;
    return nka__vc(nka__lerp_color(nka__cv(x), nka__cv(y), t, space));
}

NKA_API int nka_gradient_add(struct nka_gradient *g, float position, struct nk_colorf color)
{
    int n = nka__fx_stops(g), i;
    if (!g || n >= NKA_GRADIENT_MAX) return 0;
    for (i = n; i > 0 && position < g->positions[i - 1]; --i) {
        g->positions[i] = g->positions[i - 1];
        g->colors[i] = g->colors[i - 1];
    }
    g->positions[i] = position;
    g->colors[i] = color;
    g->count = n + 1;
    return 1;
}

NKA_API int nka_gradient_add_rgba(struct nka_gradient *g, float position, struct nk_color color)
{
    struct nka_vec4 c = nka__vec4((float)color.r / 255, (float)color.g / 255, (float)color.b / 255, (float)color.a / 255);
    return nka_gradient_add(g, position, nka__vc(c));
}

NKA_API struct nk_colorf nka_gradient_sample(const struct nka_gradient *g, float t, int space)
{
    int n = nka__fx_stops(g), i;
    if (!n) return nka__vc(nka__vec4(1, 1, 1, 1));
    if (n == 1 || t <= g->positions[0]) return g->colors[0];
    if (t >= g->positions[n - 1]) return g->colors[n - 1];
    for (i = 0; i < n - 1; ++i)
        if (t >= g->positions[i] && t <= g->positions[i + 1]) {
            float range = g->positions[i + 1] - g->positions[i];
            return nka__fx_mix(g->colors[i], g->colors[i + 1], range > NKA__EPS ? (t - g->positions[i]) / range : 0, space);
        }
    return g->colors[n - 1];
}

NKA_API struct nka_gradient nka_gradient_two_color(struct nk_colorf start, struct nk_colorf end)
{
    struct nka_gradient g;
    nka__zero(&g, sizeof g);
    nka_gradient_add(&g, 0, start);
    nka_gradient_add(&g, 1, end);
    return g;
}

NKA_API struct nka_gradient nka_gradient_solid(struct nk_colorf color) { return nka_gradient_two_color(color, color); }

NKA_API struct nka_gradient nka_gradient_three_color(struct nk_colorf start, struct nk_colorf mid, struct nk_colorf end)
{
    struct nka_gradient g = nka_gradient_two_color(start, end);
    nka_gradient_add(&g, 0.5f, mid);
    return g;
}

/* The colors just before and after p, and whether g has more than one stop there. */
static int nka__fx_sides(const struct nka_gradient *g, int n, int *at, float p, int space,
                         struct nk_colorf *left, struct nk_colorf *right)
{
    int first = *at, i = *at;
    while (i < n && g->positions[i] - p <= NKA__EPS) ++i;
    *at = i;
    if (i == first) {
        *left = *right = nka_gradient_sample(g, p, space);
        return 0;
    }
    *left = g->colors[first];
    *right = g->colors[i - 1];
    return i - first > 1;
}

NKA_API struct nka_gradient nka_gradient_lerp(const struct nka_gradient *a, const struct nka_gradient *b, float t, int space)
{
    float pos[2 * NKA_GRADIENT_MAX];
    struct nk_colorf col[2 * NKA_GRADIENT_MAX];
    struct nka_gradient r;
    int na = nka__fx_stops(a), nb = nka__fx_stops(b), i = 0, j = 0, n = 0, k;
    while (i < na || j < nb) {
        struct nk_colorf al, ar, bl, br;
        float p = j >= nb || (i < na && a->positions[i] <= b->positions[j]) ? a->positions[i] : b->positions[j];
        int split = nka__fx_sides(a, na, &i, p, space, &al, &ar);
        split |= nka__fx_sides(b, nb, &j, p, space, &bl, &br);
        pos[n] = p;
        col[n++] = nka__fx_mix(al, bl, t, space);
        if (split) {
            pos[n] = p;
            col[n++] = nka__fx_mix(ar, br, t, space);
        }
    }
    while (n > NKA_GRADIENT_MAX && n > 2) {
        float least = -1;
        int best = 1;
        for (k = 1; k < n - 1; ++k) {
            float span = pos[k + 1] - pos[k - 1], w = span > NKA__EPS ? (pos[k] - pos[k - 1]) / span : 0;
            float cost = span * (nka__fabs(col[k].r - nka__lerp(col[k - 1].r, col[k + 1].r, w))
                               + nka__fabs(col[k].g - nka__lerp(col[k - 1].g, col[k + 1].g, w))
                               + nka__fabs(col[k].b - nka__lerp(col[k - 1].b, col[k + 1].b, w))
                               + nka__fabs(col[k].a - nka__lerp(col[k - 1].a, col[k + 1].a, w)));
            if (least < 0 || cost < least) {
                least = cost;
                best = k;
            }
        }
        for (k = best; k < n - 1; ++k) {
            pos[k] = pos[k + 1];
            col[k] = col[k + 1];
        }
        --n;
    }
    if (n > NKA_GRADIENT_MAX) n = NKA_GRADIENT_MAX;
    nka__zero(&r, sizeof r);
    r.count = n;
    for (k = 0; k < n; ++k) {
        r.positions[k] = pos[k];
        r.colors[k] = col[k];
    }
    return r;
}

static int nka__fx_grad_eq(const struct nka_gradient *x, const struct nka_gradient *y)
{
    int n = nka__fx_stops(x), i;
    if (n != nka__fx_stops(y)) return 0;
    for (i = 0; i < n; ++i)
        if (nka__fabs(x->positions[i] - y->positions[i]) > NKA__EPS
            || nka__fabs(x->colors[i].r - y->colors[i].r) > NKA__EPS || nka__fabs(x->colors[i].g - y->colors[i].g) > NKA__EPS
            || nka__fabs(x->colors[i].b - y->colors[i].b) > NKA__EPS || nka__fabs(x->colors[i].a - y->colors[i].a) > NKA__EPS)
            return 0;
    return 1;
}

static struct nka_gradient nka__fx_grad_now(struct nka_context *a, struct nka__fx_grad *c)
{
    float t;
    if (c->sleeping) return c->to;
    t = (float)((a->time - c->start) / c->dur);
    if (t >= 1) {
        c->sleeping = 1;
        return c->to;
    }
    a->busy = 1;
    return nka_gradient_lerp(&c->from, &c->to, nka_eval(a, c->ease, t), c->space);
}

NKA_API struct nka_gradient nka_tween_gradient(struct nka_context *a, nk_hash id, nk_hash ch, const struct nka_gradient *target,
                                               float dur, struct nka_ease ease, int policy, int space)
{
    struct nka__fx_grad *c;
    struct nka_gradient g;
    int fresh;
    nka__zero(&g, sizeof g);
    if (target) g = *target;
    if (!a) return g;
    c = (struct nka__fx_grad *)nka__put(a, &a->fx_grads, nka_id_mix(id, ch), &fresh);
    if (!c) return g;
    c->seen = a->frame;
    if (fresh) {
        c->from = c->to = g;
        c->sleeping = 1;
        c->dur = NKA__MIN_DUR;
        c->ease = ease;
        c->policy = (unsigned char)policy;
        c->space = (unsigned char)space;
    }
    if (!nka__fx_grad_eq(&c->to, &g)
        || (!c->sleeping && (c->policy != policy || c->space != space || !nka__ease_eq(c->ease, ease)))) {
        if (policy == NKA_POLICY_CUT) {
            c->from = c->to = g;
            c->sleeping = 1;
        } else {
            struct nka_gradient now = nka__fx_grad_now(a, c);
            c->from = now;
            c->to = g;
            c->start = a->time;
            c->dur = dur > NKA__MIN_DUR ? dur : NKA__MIN_DUR;
            c->sleeping = 0;
        }
        c->ease = ease;
        c->policy = (unsigned char)policy;
        c->space = (unsigned char)space;
    }
    return nka__fx_grad_now(a, c);
}

/* Transforms */

NKA_API struct nka_transform nka_transform(struct nk_vec2 position, float rotation, struct nk_vec2 scale)
{
    struct nka_transform t;
    t.position = position;
    t.scale = scale;
    t.rotation = rotation;
    return t;
}

NKA_API struct nka_transform nka_transform_identity(void) { return nka_transform(nka__vec2(0, 0), 0, nka__vec2(1, 1)); }

NKA_API struct nk_vec2 nka_transform_apply(struct nka_transform t, struct nk_vec2 point)
{
    float c = nka__cos(t.rotation), s = nka__sin(t.rotation), x = point.x * t.scale.x, y = point.y * t.scale.y;
    return nka__vec2(t.position.x + x * c - y * s, t.position.y + x * s + y * c);
}

NKA_API struct nka_transform nka_transform_compose(struct nka_transform a, struct nka_transform b)
{
    return nka_transform(nka_transform_apply(a, b.position), a.rotation + b.rotation,
                         nka__vec2(a.scale.x * b.scale.x, a.scale.y * b.scale.y));
}

NKA_API struct nka_transform nka_transform_inverse(struct nka_transform t)
{
    struct nk_vec2 s = nka__vec2(nka__fabs(t.scale.x) > NKA__EPS ? 1 / t.scale.x : 1,
                                 nka__fabs(t.scale.y) > NKA__EPS ? 1 / t.scale.y : 1);
    struct nka_transform r = nka_transform(nka__vec2(0, 0), -t.rotation, s);
    struct nk_vec2 p = nka_transform_apply(r, t.position);
    r.position = nka__vec2(-p.x, -p.y);
    return r;
}

static float nka__fx_turn(float from, float to, int mode)
{
    const float tau = 2 * NKA__PI;
    float x = nka__fmod(from, tau), y = nka__fmod(to, tau), d;
    if (x < 0) x += tau;
    if (y < 0) y += tau;
    d = y - x;
    switch (mode) {
    case NKA_ROTATION_SHORTEST:
        if (d > NKA__PI) d -= tau;
        else if (d < -NKA__PI) d += tau;
        return d;
    case NKA_ROTATION_LONGEST:
        if (d > 0 && d < NKA__PI) d -= tau;
        else if (d < 0 && d > -NKA__PI) d += tau;
        return d;
    case NKA_ROTATION_CW: return d < 0 ? d + tau : d;
    case NKA_ROTATION_CCW: return d > 0 ? d - tau : d;
    default: return to - from;
    }
}

NKA_API struct nka_transform nka_transform_lerp(struct nka_transform a, struct nka_transform b, float t, int rotation_mode)
{
    return nka_transform(nka__vec2(nka__lerp(a.position.x, b.position.x, t), nka__lerp(a.position.y, b.position.y, t)),
                         a.rotation + nka__fx_turn(a.rotation, b.rotation, rotation_mode) * t,
                         nka__vec2(nka__lerp(a.scale.x, b.scale.x, t), nka__lerp(a.scale.y, b.scale.y, t)));
}

NKA_API struct nka_transform nka_transform_from_matrix(float m00, float m01, float m10, float m11, float tx, float ty)
{
    float sign = m00 * m11 - m01 * m10 < 0 ? -1.0f : 1.0f;
    return nka_transform(nka__vec2(tx, ty), nka__atan2(sign * m10, sign * m00),
                         nka__vec2(sign * nka__sqrt(m00 * m00 + m10 * m10), nka__sqrt(m01 * m01 + m11 * m11)));
}

NKA_API void nka_transform_to_matrix(struct nka_transform t, float *out)
{
    float c = nka__cos(t.rotation), s = nka__sin(t.rotation);
    if (!out) return;
    out[0] = c * t.scale.x;
    out[1] = -s * t.scale.y;
    out[2] = t.position.x;
    out[3] = s * t.scale.x;
    out[4] = c * t.scale.y;
    out[5] = t.position.y;
}

static int nka__fx_xform_eq(struct nka_transform x, struct nka_transform y)
{
    return nka__fabs(x.position.x - y.position.x) + nka__fabs(x.position.y - y.position.y) <= NKA__EPS
        && nka__fabs(nka__fx_turn(x.rotation, y.rotation, NKA_ROTATION_SHORTEST)) <= NKA__EPS
        && nka__fabs(x.scale.x - y.scale.x) + nka__fabs(x.scale.y - y.scale.y) <= NKA__EPS;
}

static struct nka_transform nka__fx_xform_now(struct nka_context *a, struct nka__fx_xform *c)
{
    float t;
    if (c->sleeping) return c->to;
    t = (float)((a->time - c->start) / c->dur);
    if (t >= 1) {
        c->sleeping = 1;
        return c->to;
    }
    a->busy = 1;
    return nka_transform_lerp(c->from, c->to, nka_eval(a, c->ease, t), c->mode);
}

NKA_API struct nka_transform nka_tween_transform(struct nka_context *a, nk_hash id, nk_hash ch, struct nka_transform target,
                                                 float dur, struct nka_ease ease, int policy, int rotation_mode)
{
    struct nka__fx_xform *c;
    int fresh;
    if (!a) return target;
    c = (struct nka__fx_xform *)nka__put(a, &a->fx_xforms, nka_id_mix(id, ch), &fresh);
    if (!c) return target;
    c->seen = a->frame;
    if (fresh) {
        c->from = c->to = target;
        c->sleeping = 1;
        c->dur = NKA__MIN_DUR;
        c->ease = ease;
        c->policy = (unsigned char)policy;
        c->mode = (unsigned char)rotation_mode;
    }
    if (!nka__fx_xform_eq(c->to, target)
        || (!c->sleeping && (c->policy != policy || c->mode != rotation_mode || !nka__ease_eq(c->ease, ease)))) {
        if (policy == NKA_POLICY_CUT) {
            c->from = c->to = target;
            c->sleeping = 1;
        } else {
            struct nka_transform now = nka__fx_xform_now(a, c);
            c->from = now;
            c->to = target;
            c->start = a->time;
            c->dur = dur > NKA__MIN_DUR ? dur : NKA__MIN_DUR;
            c->sleeping = 0;
        }
        c->ease = ease;
        c->policy = (unsigned char)policy;
        c->mode = (unsigned char)rotation_mode;
    }
    return nka__fx_xform_now(a, c);
}

/* Drag feedback */

NKA_API struct nka_drag_opts nka_drag_opts_default(void)
{
    struct nka_drag_opts o;
    o.snap_grid = nka__vec2(0, 0);
    o.snap_points = 0;
    o.snap_points_count = 0;
    o.snap_duration = 0.2f;
    o.overshoot = 0;
    o.ease_type = NKA_EASE_OUT_CUBIC;
    return o;
}

static struct nka_drag_feedback nka__fx_loose(struct nk_vec2 pos, int dragging, float progress)
{
    struct nka_drag_feedback fb;
    fb.position = pos;
    fb.offset = fb.velocity = nka__vec2(0, 0);
    fb.is_dragging = dragging;
    fb.is_snapping = 0;
    fb.snap_progress = progress;
    return fb;
}

static struct nka_drag_feedback nka__fx_feedback(const struct nka__fx_drag *s, struct nk_vec2 pos)
{
    struct nka_drag_feedback fb;
    fb.position = pos;
    fb.offset = nka__vec2(pos.x - s->start.x, pos.y - s->start.y);
    fb.velocity = s->vel;
    fb.is_dragging = s->dragging;
    fb.is_snapping = s->snapping;
    fb.snap_progress = s->progress;
    return fb;
}

static struct nka__fx_drag *nka__fx_drag_get(struct nka_context *a, nk_hash id)
{
    return a ? (struct nka__fx_drag *)nka__get(&a->fx_drags, id) : 0;
}

NKA_API struct nka_drag_feedback nka_drag_begin(struct nka_context *a, nk_hash id, struct nk_vec2 pos)
{
    struct nka__fx_drag *s = 0;
    if (a) s = (struct nka__fx_drag *)nka__put(a, &a->fx_drags, id, 0);
    if (!s) return nka__fx_loose(pos, 1, 0);
    s->seen = s->frame = a->frame;
    s->t_prev = s->t_cur = a->time;
    s->start = s->cur = s->prev = s->to = pos;
    s->vel = nka__vec2(0, 0);
    s->dragging = 1;
    s->snapping = 0;
    s->progress = 0;
    return nka__fx_feedback(s, pos);
}

NKA_API struct nka_drag_feedback nka_drag_update(struct nka_context *a, nk_hash id, struct nk_vec2 pos)
{
    struct nka__fx_drag *s = nka__fx_drag_get(a, id);
    float dt;
    if (!s) return nka__fx_loose(pos, 0, 0);
    s->seen = a->frame;
    if (s->frame != a->frame) {
        s->frame = a->frame;
        s->prev = s->cur;
        s->t_prev = s->t_cur;
    }
    dt = (float)(a->time - s->t_prev);
    if (dt > 0) s->vel = nka__vec2((pos.x - s->prev.x) / dt, (pos.y - s->prev.y) / dt);
    s->cur = pos;
    s->t_cur = a->time;
    return nka__fx_feedback(s, pos);
}

static int nka__fx_snap(const struct nka_drag_opts *o, struct nk_vec2 pos, struct nk_vec2 *to)
{
    int snap = 0, i;
    *to = pos;
    if (o->snap_grid.x > 0 || o->snap_grid.y > 0) {
        if (o->snap_grid.x > 0) to->x = nka__floor(pos.x / o->snap_grid.x + 0.5f) * o->snap_grid.x;
        if (o->snap_grid.y > 0) to->y = nka__floor(pos.y / o->snap_grid.y + 0.5f) * o->snap_grid.y;
        snap = 1;
    }
    if (o->snap_points && o->snap_points_count > 0) {
        float best = 0;
        for (i = 0; i < o->snap_points_count; ++i) {
            float dx = o->snap_points[i].x - pos.x, dy = o->snap_points[i].y - pos.y;
            if (i == 0 || dx * dx + dy * dy < best) {
                best = dx * dx + dy * dy;
                *to = o->snap_points[i];
            }
        }
        snap = 1;
    }
    return snap;
}

NKA_API struct nka_drag_feedback nka_drag_release(struct nka_context *a, nk_hash id, struct nk_vec2 pos,
                                                  const struct nka_drag_opts *opts)
{
    struct nka__fx_drag *s = nka__fx_drag_get(a, id);
    struct nka_drag_opts d;
    float t, k;
    if (!s) return nka__fx_loose(pos, 0, 1);
    if (!opts) {
        d = nka_drag_opts_default();
        opts = &d;
    }
    s->seen = a->frame;
    if (!s->snapping) {
        int snap = nka__fx_snap(opts, pos, &s->to);
        s->dragging = 0;
        s->progress = 1;
        if (snap && opts->snap_duration > 0 && (s->to.x != pos.x || s->to.y != pos.y)) {
            s->snapping = 1;
            s->progress = 0;
            s->from = pos;
            s->t0 = a->time;
            s->dur = opts->snap_duration;
            s->overshoot = opts->overshoot;
            s->ease = opts->ease_type;
        }
    }
    if (!s->snapping) return nka__fx_feedback(s, s->to);
    t = (float)((a->time - s->t0) / s->dur);
    if (t >= 1) {
        s->snapping = 0;
        s->progress = 1;
        return nka__fx_feedback(s, s->to);
    }
    s->progress = t;
    k = nka_eval(a, nka_ease(s->ease), t) + s->overshoot * 1.70158f * t * (1 - t) * (1 - t);
    a->busy = 1;
    return nka__fx_feedback(s, nka__vec2(nka__lerp(s->from.x, s->to.x, k), nka__lerp(s->from.y, s->to.y, k)));
}

NKA_API void nka_drag_cancel(struct nka_context *a, nk_hash id)
{
    struct nka__fx_drag *s = nka__fx_drag_get(a, id);
    if (!s) return;
    s->dragging = 0;
    s->snapping = 0;
}

/* Nuklear */

NKA_API void nka_anchor_update(struct nka_context *a, struct nk_context *ctx)
{
    if (!a || !ctx || !ctx->current) return;
    nka_set_anchor(a, NKA_ANCHOR_WINDOW_CONTENT, nk_window_get_content_region_size(ctx));
    nka_set_anchor(a, NKA_ANCHOR_WINDOW, nk_window_get_size(ctx));
}

static struct nk_panel *nka__nk_panel(struct nk_context *ctx)
{
    return ctx && ctx->current && ctx->current->layout ? ctx->current->layout : 0;
}

/* The range nk_panel_end clamps a scroll offset to, from the layout so far. */
static void nka__nk_range(const struct nk_panel *p, float *max)
{
    float h = (float)(int)(p->at_y + p->row.height - p->bounds.y) - p->bounds.h;
    float w = (float)(int)(p->max_x - p->bounds.x) - p->bounds.w;
    max[0] = w > 0 ? w : 0;
    max[1] = h > 0 ? h : 0;
}

static void nka__nk_scroll_to(struct nka_context *a, struct nk_context *ctx, int axis, float to, int bottom,
                              float dur, struct nka_ease ease)
{
    struct nk_panel *p = nka__nk_panel(ctx);
    struct nka__nk_scroll *s;
    int fresh;
    nk_uint *offset;
    if (!a || !p) return;
    s = (struct nka__nk_scroll *)nka__put(a, &a->nk_scrolls, nka__ptr_key(p->offset_y), &fresh);
    if (!s) return;
    if (fresh) nka__nk_range(p, s->max);
    offset = axis ? p->offset_y : p->offset_x;
    s->seen = a->frame;
    s->from[axis] = (float)*offset;
    s->to[axis] = bottom ? s->max[axis] : (to > 0 ? to : 0);
    s->dur[axis] = dur;
    s->ease[axis] = ease;
    s->start[axis] = a->time;
    s->wrote[axis] = *offset;
    s->active[axis] = 1;
}

NKA_API void nka_scroll_to_x(struct nka_context *a, struct nk_context *ctx, float x, float dur, struct nka_ease ease)
{
    nka__nk_scroll_to(a, ctx, 0, x, 0, dur, ease);
}

NKA_API void nka_scroll_to_y(struct nka_context *a, struct nk_context *ctx, float y, float dur, struct nka_ease ease)
{
    nka__nk_scroll_to(a, ctx, 1, y, 0, dur, ease);
}

NKA_API void nka_scroll_to_top(struct nka_context *a, struct nk_context *ctx, float dur, struct nka_ease ease)
{
    nka__nk_scroll_to(a, ctx, 1, 0, 0, dur, ease);
}

NKA_API void nka_scroll_to_bottom(struct nka_context *a, struct nk_context *ctx, float dur, struct nka_ease ease)
{
    nka__nk_scroll_to(a, ctx, 1, 0, 1, dur, ease);
}

NKA_API void nka_scroll(struct nka_context *a, struct nk_context *ctx)
{
    struct nk_panel *p = nka__nk_panel(ctx);
    struct nka__nk_scroll *s;
    int axis;
    if (!a || !p) return;
    s = (struct nka__nk_scroll *)nka__put(a, &a->nk_scrolls, nka__ptr_key(p->offset_y), 0);
    if (!s) return;
    s->seen = a->frame;
    nka__nk_range(p, s->max);
    for (axis = 0; axis < 2; ++axis) {
        nk_uint *offset = axis ? p->offset_y : p->offset_x;
        nk_uint now = *offset;
        float t, v;
        if (!s->active[axis]) continue;
        if ((now > s->wrote[axis] ? now - s->wrote[axis] : s->wrote[axis] - now) > 1) {
            s->active[axis] = 0;
            continue;
        }
        t = s->dur[axis] > 0 ? (float)((a->time - s->start[axis]) / s->dur[axis]) : 1;
        v = nka__lerp(s->from[axis], s->to[axis], nka_eval(a, s->ease[axis], t));
        v = v < 0 ? 0 : (v > s->max[axis] ? s->max[axis] : v);
        *offset = (nk_uint)(v + 0.5f);
        s->wrote[axis] = *offset;
        if (t >= 1) s->active[axis] = 0;
        else a->busy = 1;
    }
}

static nk_byte nka__nk_byte(float v) { return (nk_byte)(nka__clamp01(v) * 255 + 0.5f); }

static struct nk_color nka__nk_mix(struct nk_color x, struct nk_color y, float t, int space)
{
    struct nka_vec4 v;
    struct nk_color c;
    if (x.r == y.r && x.g == y.g && x.b == y.b && x.a == y.a) return x;
    v = nka__lerp_color(nka__vec4(x.r / 255.0f, x.g / 255.0f, x.b / 255.0f, x.a / 255.0f),
                        nka__vec4(y.r / 255.0f, y.g / 255.0f, y.b / 255.0f, y.a / 255.0f), t, space);
    c.r = nka__nk_byte(v.x);
    c.g = nka__nk_byte(v.y);
    c.b = nka__nk_byte(v.z);
    c.a = nka__nk_byte(v.w);
    return c;
}

static struct nk_vec2 nka__nk_vmix(struct nk_vec2 x, struct nk_vec2 y, float t)
{
    return nka__vec2(nka__lerp(x.x, y.x, t), nka__lerp(x.y, y.y, t));
}

static void nka__nk_item(struct nk_style_item *o, const struct nk_style_item *x, const struct nk_style_item *y,
                         float t, int space)
{
    if (x->type != NK_STYLE_ITEM_COLOR || y->type != NK_STYLE_ITEM_COLOR) return;
    o->type = NK_STYLE_ITEM_COLOR;
    o->data.color = nka__nk_mix(x->data.color, y->data.color, t, space);
}

#define NKA__NK_C(f) o->f = nka__nk_mix(x->f, y->f, t, space)
#define NKA__NK_F(f) o->f = nka__lerp(x->f, y->f, t)
#define NKA__NK_V(f) o->f = nka__nk_vmix(x->f, y->f, t)
#define NKA__NK_I(f) nka__nk_item(&o->f, &x->f, &y->f, t, space)
#define NKA__NK_S(fn, f) fn(&o->f, &x->f, &y->f, t, space)

static void nka__nk_button(struct nk_style_button *o, const struct nk_style_button *x,
                           const struct nk_style_button *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active);
    NKA__NK_C(border_color); NKA__NK_F(color_factor_background);
    NKA__NK_C(text_background); NKA__NK_C(text_normal); NKA__NK_C(text_hover); NKA__NK_C(text_active);
    NKA__NK_F(color_factor_text); NKA__NK_F(border); NKA__NK_F(rounding);
    NKA__NK_V(padding); NKA__NK_V(image_padding); NKA__NK_V(touch_padding); NKA__NK_F(disabled_factor);
}

static void nka__nk_toggle(struct nk_style_toggle *o, const struct nk_style_toggle *x,
                           const struct nk_style_toggle *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active); NKA__NK_C(border_color);
    NKA__NK_I(cursor_normal); NKA__NK_I(cursor_hover);
    NKA__NK_C(text_normal); NKA__NK_C(text_hover); NKA__NK_C(text_active); NKA__NK_C(text_background);
    NKA__NK_V(padding); NKA__NK_V(touch_padding); NKA__NK_F(spacing); NKA__NK_F(border);
    NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
}

static void nka__nk_selectable(struct nk_style_selectable *o, const struct nk_style_selectable *x,
                               const struct nk_style_selectable *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(pressed);
    NKA__NK_I(normal_active); NKA__NK_I(hover_active); NKA__NK_I(pressed_active);
    NKA__NK_C(text_normal); NKA__NK_C(text_hover); NKA__NK_C(text_pressed);
    NKA__NK_C(text_normal_active); NKA__NK_C(text_hover_active); NKA__NK_C(text_pressed_active);
    NKA__NK_C(text_background); NKA__NK_F(rounding);
    NKA__NK_V(padding); NKA__NK_V(touch_padding); NKA__NK_V(image_padding);
    NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
}

static void nka__nk_slider(struct nk_style_slider *o, const struct nk_style_slider *x,
                           const struct nk_style_slider *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active); NKA__NK_C(border_color);
    NKA__NK_C(bar_normal); NKA__NK_C(bar_hover); NKA__NK_C(bar_active); NKA__NK_C(bar_filled);
    NKA__NK_I(cursor_normal); NKA__NK_I(cursor_hover); NKA__NK_I(cursor_active);
    NKA__NK_F(border); NKA__NK_F(rounding); NKA__NK_F(bar_height);
    NKA__NK_V(padding); NKA__NK_V(spacing); NKA__NK_V(cursor_size);
    NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
    NKA__NK_S(nka__nk_button, inc_button); NKA__NK_S(nka__nk_button, dec_button);
}

static void nka__nk_knob(struct nk_style_knob *o, const struct nk_style_knob *x,
                         const struct nk_style_knob *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active); NKA__NK_C(border_color);
    NKA__NK_C(knob_normal); NKA__NK_C(knob_hover); NKA__NK_C(knob_active); NKA__NK_C(knob_border_color);
    NKA__NK_C(cursor_normal); NKA__NK_C(cursor_hover); NKA__NK_C(cursor_active);
    NKA__NK_F(border); NKA__NK_F(knob_border); NKA__NK_V(padding); NKA__NK_V(spacing);
    NKA__NK_F(cursor_width); NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
}

static void nka__nk_progress(struct nk_style_progress *o, const struct nk_style_progress *x,
                             const struct nk_style_progress *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active); NKA__NK_C(border_color);
    NKA__NK_I(cursor_normal); NKA__NK_I(cursor_hover); NKA__NK_I(cursor_active); NKA__NK_C(cursor_border_color);
    NKA__NK_F(rounding); NKA__NK_F(border); NKA__NK_F(cursor_border); NKA__NK_F(cursor_rounding);
    NKA__NK_V(padding); NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
}

static void nka__nk_scrollbar(struct nk_style_scrollbar *o, const struct nk_style_scrollbar *x,
                              const struct nk_style_scrollbar *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active); NKA__NK_C(border_color);
    NKA__NK_I(cursor_normal); NKA__NK_I(cursor_hover); NKA__NK_I(cursor_active); NKA__NK_C(cursor_border_color);
    NKA__NK_F(border); NKA__NK_F(rounding); NKA__NK_F(border_cursor); NKA__NK_F(rounding_cursor);
    NKA__NK_V(padding); NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
    NKA__NK_S(nka__nk_button, inc_button); NKA__NK_S(nka__nk_button, dec_button);
}

static void nka__nk_edit(struct nk_style_edit *o, const struct nk_style_edit *x,
                         const struct nk_style_edit *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active); NKA__NK_C(border_color);
    NKA__NK_S(nka__nk_scrollbar, scrollbar);
    NKA__NK_C(cursor_normal); NKA__NK_C(cursor_hover); NKA__NK_C(cursor_text_normal); NKA__NK_C(cursor_text_hover);
    NKA__NK_C(text_normal); NKA__NK_C(text_hover); NKA__NK_C(text_active);
    NKA__NK_C(selected_normal); NKA__NK_C(selected_hover);
    NKA__NK_C(selected_text_normal); NKA__NK_C(selected_text_hover);
    NKA__NK_F(border); NKA__NK_F(rounding); NKA__NK_F(cursor_size);
    NKA__NK_V(scrollbar_size); NKA__NK_V(padding); NKA__NK_F(row_padding);
    NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
}

static void nka__nk_property(struct nk_style_property *o, const struct nk_style_property *x,
                             const struct nk_style_property *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active); NKA__NK_C(border_color);
    NKA__NK_C(label_normal); NKA__NK_C(label_hover); NKA__NK_C(label_active);
    NKA__NK_F(border); NKA__NK_F(rounding); NKA__NK_V(padding);
    NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
    NKA__NK_S(nka__nk_edit, edit);
    NKA__NK_S(nka__nk_button, inc_button); NKA__NK_S(nka__nk_button, dec_button);
}

static void nka__nk_chart(struct nk_style_chart *o, const struct nk_style_chart *x,
                          const struct nk_style_chart *y, float t, int space)
{
    NKA__NK_I(background); NKA__NK_C(border_color); NKA__NK_C(selected_color); NKA__NK_C(color);
    NKA__NK_F(border); NKA__NK_F(rounding); NKA__NK_V(padding);
    NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
}

static void nka__nk_combo(struct nk_style_combo *o, const struct nk_style_combo *x,
                          const struct nk_style_combo *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active); NKA__NK_C(border_color);
    NKA__NK_C(label_normal); NKA__NK_C(label_hover); NKA__NK_C(label_active);
    NKA__NK_C(symbol_normal); NKA__NK_C(symbol_hover); NKA__NK_C(symbol_active);
    NKA__NK_S(nka__nk_button, button);
    NKA__NK_F(border); NKA__NK_F(rounding);
    NKA__NK_V(content_padding); NKA__NK_V(button_padding); NKA__NK_V(spacing);
    NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
}

static void nka__nk_tab(struct nk_style_tab *o, const struct nk_style_tab *x,
                        const struct nk_style_tab *y, float t, int space)
{
    NKA__NK_I(background); NKA__NK_C(border_color); NKA__NK_C(text);
    NKA__NK_S(nka__nk_button, tab_maximize_button); NKA__NK_S(nka__nk_button, tab_minimize_button);
    NKA__NK_S(nka__nk_button, node_maximize_button); NKA__NK_S(nka__nk_button, node_minimize_button);
    NKA__NK_F(border); NKA__NK_F(rounding); NKA__NK_F(indent);
    NKA__NK_V(padding); NKA__NK_V(spacing); NKA__NK_F(color_factor); NKA__NK_F(disabled_factor);
}

static void nka__nk_header(struct nk_style_window_header *o, const struct nk_style_window_header *x,
                           const struct nk_style_window_header *y, float t, int space)
{
    NKA__NK_I(normal); NKA__NK_I(hover); NKA__NK_I(active);
    NKA__NK_S(nka__nk_button, close_button); NKA__NK_S(nka__nk_button, minimize_button);
    NKA__NK_C(label_normal); NKA__NK_C(label_hover); NKA__NK_C(label_active);
    NKA__NK_V(padding); NKA__NK_V(label_padding); NKA__NK_V(spacing);
}

static void nka__nk_window(struct nk_style_window *o, const struct nk_style_window *x,
                           const struct nk_style_window *y, float t, int space)
{
    NKA__NK_S(nka__nk_header, header);
    NKA__NK_I(fixed_background); NKA__NK_C(background); NKA__NK_C(border_color);
    NKA__NK_C(popup_border_color); NKA__NK_C(combo_border_color); NKA__NK_C(contextual_border_color);
    NKA__NK_C(menu_border_color); NKA__NK_C(group_border_color); NKA__NK_C(tooltip_border_color);
    NKA__NK_I(scaler);
    NKA__NK_F(border); NKA__NK_F(combo_border); NKA__NK_F(contextual_border); NKA__NK_F(menu_border);
    NKA__NK_F(group_border); NKA__NK_F(tooltip_border); NKA__NK_F(popup_border);
    NKA__NK_F(min_row_height_padding); NKA__NK_F(rounding);
    NKA__NK_V(spacing); NKA__NK_V(scrollbar_size); NKA__NK_V(min_size); NKA__NK_V(padding);
    NKA__NK_V(group_padding); NKA__NK_V(popup_padding); NKA__NK_V(combo_padding);
    NKA__NK_V(contextual_padding); NKA__NK_V(menu_padding); NKA__NK_V(tooltip_padding);
    NKA__NK_V(tooltip_offset); NKA__NK_F(tooltip_delay);
}

/* t stays within 0 and 1: an overshoot would make sizes negative. */
static void nka__nk_blend(const struct nk_style *x, const struct nk_style *y, float t, int space, struct nk_style *o)
{
    *o = t < 0.5f ? *x : *y;
    if (t <= 0 || t >= 1) return;
    NKA__NK_C(text.color); NKA__NK_V(text.padding); NKA__NK_F(text.color_factor); NKA__NK_F(text.disabled_factor);
    NKA__NK_S(nka__nk_button, button);
    NKA__NK_S(nka__nk_button, contextual_button);
    NKA__NK_S(nka__nk_button, menu_button);
    NKA__NK_S(nka__nk_toggle, option);
    NKA__NK_S(nka__nk_toggle, checkbox);
    NKA__NK_S(nka__nk_selectable, selectable);
    NKA__NK_S(nka__nk_slider, slider);
    NKA__NK_S(nka__nk_knob, knob);
    NKA__NK_S(nka__nk_progress, progress);
    NKA__NK_S(nka__nk_property, property);
    NKA__NK_S(nka__nk_edit, edit);
    NKA__NK_S(nka__nk_chart, chart);
    NKA__NK_S(nka__nk_scrollbar, scrollh);
    NKA__NK_S(nka__nk_scrollbar, scrollv);
    NKA__NK_S(nka__nk_tab, tab);
    NKA__NK_S(nka__nk_combo, combo);
    NKA__NK_S(nka__nk_window, window);
}

#undef NKA__NK_C
#undef NKA__NK_F
#undef NKA__NK_V
#undef NKA__NK_I
#undef NKA__NK_S

static const struct nk_style *nka__nk_find(const struct nka_context *a, nk_hash style_id)
{
    const struct nka__nk_style *s = (const struct nka__nk_style *)nka__get(&a->nk_styles, style_id);
    return s ? s->style : 0;
}

NKA_API void nka_style_register(struct nka_context *a, nk_hash style_id, const struct nk_style *style)
{
    struct nka__nk_style *s;
    struct nk_style *copy;
    if (!a || !style) return;
    copy = (struct nk_style *)nka__nk_find(a, style_id);
    if (!copy) {
        copy = (struct nk_style *)nka__alloc(a, sizeof *copy);
        if (!copy) return;
        s = (struct nka__nk_style *)nka__put(a, &a->nk_styles, style_id, 0);
        if (!s) { nka__free(a, copy); return; }
        s->style = copy;
    }
    *copy = *style;
}

NKA_API void nka_style_register_current(struct nka_context *a, const struct nk_context *ctx, nk_hash style_id)
{
    if (ctx) nka_style_register(a, style_id, &ctx->style);
}

NKA_API void nka_style_register_table(struct nka_context *a, struct nk_context *ctx, nk_hash style_id,
                                      const struct nk_color *table)
{
    struct nk_style keep;
    if (!a || !ctx) return;
    keep = ctx->style;
    nk_style_from_table(ctx, table);
    nka_style_register(a, style_id, &ctx->style);
    ctx->style = keep;
}

NKA_API int nka_style_exists(const struct nka_context *a, nk_hash style_id)
{
    return a && nka__nk_find(a, style_id) != 0;
}

NKA_API void nka_style_unregister(struct nka_context *a, nk_hash style_id)
{
    struct nk_style *style;
    if (!a) return;
    style = (struct nk_style *)nka__nk_find(a, style_id);
    if (!style) return;
    nka__free(a, style);
    nka__del(&a->nk_styles, style_id);
}

NKA_API void nka_style_blend_to(struct nka_context *a, nk_hash style_a, nk_hash style_b, float t,
                                struct nk_style *out, int space)
{
    const struct nk_style *x, *y;
    if (!a || !out) return;
    x = nka__nk_find(a, style_a);
    y = nka__nk_find(a, style_b);
    if (x && y) nka__nk_blend(x, y, t, space, out);
}

NKA_API void nka_style_blend(struct nka_context *a, struct nk_context *ctx, nk_hash style_a, nk_hash style_b,
                             float t, int space)
{
    if (ctx) nka_style_blend_to(a, style_a, style_b, t, &ctx->style, space);
}

NKA_API void nka_style_tween(struct nka_context *a, struct nk_context *ctx, nk_hash id, nk_hash target_style,
                             float dur, struct nka_ease ease, int space)
{
    struct nka__nk_style_tween *s;
    const struct nk_style *to;
    int fresh;
    float t;
    if (!a || !ctx) return;
    to = nka__nk_find(a, target_style);
    if (!to) return;
    s = (struct nka__nk_style_tween *)nka__put(a, &a->nk_style_tweens, id, &fresh);
    if (!s) return;
    s->seen = a->frame;
    if (fresh || s->target != target_style) {
        if (!s->from) s->from = (struct nk_style *)nka__alloc(a, sizeof *s->from);
        if (!s->from) return;
        *s->from = ctx->style;
        s->target = target_style;
        s->start = a->time;
        s->dur = dur;
        s->ease = ease;
        s->space = space;
        s->active = 1;
    }
    if (!s->active) return;
    t = s->dur > 0 ? (float)((a->time - s->start) / s->dur) : 1;
    if (t >= 1) s->active = 0;
    else a->busy = 1;
    nka__nk_blend(s->from, to, nka_eval(a, s->ease, t), s->space, &ctx->style);
}

static void nka__nk_free_tweens(struct nka_context *a)
{
    int i;
    for (i = 0; i < a->nk_style_tweens.cap; ++i)
        if (a->nk_style_tweens.keys[i])
            nka__free(a, ((struct nka__nk_style_tween *)nka__at(&a->nk_style_tweens, i))->from);
    nka__map_free(a, &a->nk_style_tweens);
}

static void nka__nk_gc(struct nka_context *a, unsigned max_age)
{
    int i = 0;
    nka__sweep(&a->nk_scrolls, a->frame, max_age);
    while (i < a->nk_style_tweens.cap) {
        struct nka__nk_style_tween *s = (struct nka__nk_style_tween *)nka__at(&a->nk_style_tweens, i);
        if (a->nk_style_tweens.keys[i] && a->frame - s->seen > max_age) {
            nka__free(a, s->from);
            nka__del_at(&a->nk_style_tweens, i);
        } else ++i;
    }
}

static void nka__nk_free(struct nka_context *a)
{
    int i;
    struct nka__nk_fonts *f = a->nk_fonts;
    nka__free(a, a->nk_profiler);
    a->nk_profiler = 0;
    while (f) {
        struct nka__nk_fonts *next = f->next;
        nka__free(a, f);
        f = next;
    }
    a->nk_fonts = 0;
    for (i = 0; i < a->nk_styles.cap; ++i)
        if (a->nk_styles.keys[i]) nka__free(a, ((struct nka__nk_style *)nka__at(&a->nk_styles, i))->style);
    nka__map_free(a, &a->nk_styles);
    nka__nk_free_tweens(a);
    nka__map_free(a, &a->nk_scrolls);
}

/* Lives until the next nka_update, as long as Nuklear's text commands need it. */
static const struct nk_user_font *nka__nk_scaled(struct nka_context *a, const struct nk_user_font *font, float scale)
{
    struct nka__nk_fonts *f = a->nk_fonts;
    struct nk_user_font *copy;
    while (f && f->used == NKA__NK_FONTS) f = f->next;
    if (!f) {
        f = (struct nka__nk_fonts *)nka__alloc(a, sizeof *f);
        if (!f) return font;
        f->used = 0;
        f->next = a->nk_fonts;
        a->nk_fonts = f;
    }
    copy = &f->font[f->used++];
    *copy = *font;
    copy->height = font->height * scale;
    return copy;
}

NKA_API struct nka_text_stagger_opts nka_text_stagger_opts_default(void)
{
    struct nka_text_stagger_opts o;
    nka__zero(&o, sizeof o);
    o.effect = NKA_TEXT_FX_FADE;
    o.char_delay = 0.05f;
    o.char_duration = 0.3f;
    o.effect_intensity = 20;
    o.ease = nka_ease(NKA_EASE_OUT_CUBIC);
    o.color.r = o.color.g = o.color.b = o.color.a = 255;
    o.font_scale = 1;
    return o;
}

static int nka__nk_glyph(const char *text, int left)
{
    nk_rune rune;
    return nk_utf_decode(text, &rune, left);
}

static int nka__nk_strlen(const char *text)
{
    int n = 0;
    while (text[n]) ++n;
    return n;
}

NKA_API float nka_text_stagger_duration(const char *text, const struct nka_text_stagger_opts *opts)
{
    struct nka_text_stagger_opts d = nka_text_stagger_opts_default();
    int count = 0, i = 0, len;
    if (!text) return 0;
    if (!opts) opts = &d;
    len = nka__nk_strlen(text);
    while (i < len) {
        int g = nka__nk_glyph(text + i, len - i);
        if (!g) break;
        i += g;
        ++count;
    }
    return count ? (float)(count - 1) * opts->char_delay + opts->char_duration : 0;
}

NKA_API float nka_text_stagger_width(const struct nk_context *ctx, const char *text,
                                     const struct nka_text_stagger_opts *opts)
{
    struct nka_text_stagger_opts d = nka_text_stagger_opts_default();
    const struct nk_user_font *font;
    float w = 0;
    int i = 0, len;
    if (!text) return 0;
    if (!opts) opts = &d;
    font = opts->font ? opts->font : (ctx ? ctx->style.font : 0);
    if (!font) return 0;
    len = nka__nk_strlen(text);
    while (i < len) {
        int g = nka__nk_glyph(text + i, len - i);
        if (!g) break;
        w += font->width(font->userdata, font->height, text + i, g) * opts->font_scale;
        i += g;
        if (i < len) w += opts->letter_spacing;
    }
    return w;
}

NKA_API void nka_text_stagger(struct nka_context *a, struct nk_context *ctx, const char *text, float progress,
                              const struct nka_text_stagger_opts *opts)
{
    struct nka_text_stagger_opts d = nka_text_stagger_opts_default();
    const struct nk_user_font *font;
    struct nk_command_buffer *out;
    struct nk_color clear = { 0, 0, 0, 0 };
    float x, now, h;
    int i = 0, index = 0, len;
    if (!a || !ctx || !ctx->current || !text || !*text) return;
    if (!opts) opts = &d;
    font = opts->font ? opts->font : ctx->style.font;
    if (!font) return;
    out = nk_window_get_canvas(ctx);
    len = nka__nk_strlen(text);
    now = progress * nka_text_stagger_duration(text, opts);
    h = font->height * opts->font_scale;
    x = opts->pos.x;
    while (i < len) {
        int g = nka__nk_glyph(text + i, len - i);
        float w, begin, k = 0, alpha = 1, scale = 1, dx = 0, dy = 0;
        if (!g) break;
        w = font->width(font->userdata, font->height, text + i, g) * opts->font_scale;
        begin = (float)index * opts->char_delay;
        if (now >= begin + opts->char_duration) k = 1;
        else if (now > begin) k = nka_eval(a, opts->ease, (now - begin) / opts->char_duration);
        if (k > 0 || opts->effect == NKA_TEXT_FX_WAVE) {
            switch (opts->effect) {
            case NKA_TEXT_FX_NONE: case NKA_TEXT_FX_TYPEWRITER: alpha = k > 0 ? 1.0f : 0.0f; break;
            case NKA_TEXT_FX_SCALE: alpha = k; scale = k; break;
            case NKA_TEXT_FX_SLIDE_UP: alpha = k; dy = (1 - k) * opts->effect_intensity; break;
            case NKA_TEXT_FX_SLIDE_DOWN: alpha = k; dy = -(1 - k) * opts->effect_intensity; break;
            case NKA_TEXT_FX_SLIDE_LEFT: alpha = k; dx = (1 - k) * opts->effect_intensity; break;
            case NKA_TEXT_FX_SLIDE_RIGHT: alpha = k; dx = -(1 - k) * opts->effect_intensity; break;
            case NKA_TEXT_FX_BOUNCE: alpha = k; scale = 1 - nka__back(1 - k, 1.70158f); break;
            case NKA_TEXT_FX_WAVE:
                dy = nka__sin(progress * 2 * NKA__PI + (float)index * 0.3f) * opts->effect_intensity * 0.5f;
                break;
            default: alpha = k; break;
            }
            if (alpha > 0 && scale > 0) {
                struct nk_color c = opts->color;
                const struct nk_user_font *f = font;
                float s = scale * opts->font_scale;
                c.a = (nk_byte)((float)c.a * (alpha < 1 ? alpha : 1) + 0.5f);
                if (s != 1) f = nka__nk_scaled(a, font, s);
                dx += w * (1 - scale) / 2;
                dy += h * (1 - scale) / 2;
                nk_draw_text(out, nk_rect(x + dx, opts->pos.y + dy, w * scale + h, h * scale), text + i, g, f, clear, c);
            }
        }
        x += w + opts->letter_spacing;
        i += g;
        ++index;
    }
}

NKA_API struct nka_text_path_opts nka_text_path_opts_default(void)
{
    struct nka_text_path_opts o;
    nka__zero(&o, sizeof o);
    o.color.r = o.color.g = o.color.b = o.color.a = 255;
    o.font_scale = 1;
    return o;
}

NKA_API float nka_text_path_width(const struct nk_context *ctx, const char *text, const struct nka_text_path_opts *opts)
{
    struct nka_text_stagger_opts s = nka_text_stagger_opts_default();
    struct nka_text_path_opts d = nka_text_path_opts_default();
    if (!opts) opts = &d;
    s.font = opts->font;
    s.font_scale = opts->font_scale;
    s.letter_spacing = opts->letter_spacing;
    return nka_text_stagger_width(ctx, text, &s);
}

static void nka__nk_text_path(struct nka_context *a, struct nk_context *ctx, nk_hash path_id, const char *text,
                              float progress, const struct nka_text_path_opts *opts)
{
    struct nka_text_path_opts d = nka_text_path_opts_default();
    const struct nk_user_font *font, *drawn;
    struct nk_command_buffer *out;
    struct nk_color clear = { 0, 0, 0, 0 };
    float len, at, h, shown = 0;
    int i = 0, index = 0, n, count = 0;
    if (!a || !ctx || !ctx->current || !text || !*text) return;
    if (!opts) opts = &d;
    font = opts->font ? opts->font : ctx->style.font;
    if (!font || !nka_path_exists(a, path_id)) return;
    if (!nka_path_has_arc_lut(a, path_id)) nka_path_build_arc_lut(a, path_id, 64);
    len = nka_path_length(a, path_id);
    if (len <= 0) return;
    n = nka__nk_strlen(text);
    if (progress < 1) {
        while (i < n) {
            int g = nka__nk_glyph(text + i, n - i);
            if (!g) break;
            i += g;
            ++count;
        }
        shown = (progress > 0 ? progress : 0) * (float)count;
        i = 0;
    }
    drawn = opts->font_scale != 1 ? nka__nk_scaled(a, font, opts->font_scale) : font;
    out = nk_window_get_canvas(ctx);
    h = font->height * opts->font_scale;
    at = opts->offset;
    if (opts->align == NKA_TEXT_ALIGN_CENTER) at += (len - nka_text_path_width(ctx, text, opts)) / 2;
    else if (opts->align == NKA_TEXT_ALIGN_END) at += len - nka_text_path_width(ctx, text, opts);
    while (i < n) {
        int g = nka__nk_glyph(text + i, n - i);
        float w, mid;
        struct nk_color c = opts->color;
        if (!g || (progress < 1 && (float)index >= shown)) break;
        w = font->width(font->userdata, font->height, text + i, g) * opts->font_scale;
        mid = at + w / 2;
        if (progress < 1 && shown - (float)index < 1)
            c.a = (nk_byte)((float)c.a * (shown - (float)index) + 0.5f);
        if (mid >= 0 && mid <= len) {
            struct nk_vec2 p = nka_path_evaluate_at_distance(a, path_id, mid);
            p.x += opts->origin.x;
            p.y += opts->origin.y;
            nk_draw_text(out, nk_rect(p.x - w / 2, opts->flip_y ? p.y : p.y - h, w + h, h), text + i, g, drawn, clear, c);
        }
        at += w + opts->letter_spacing;
        i += g;
        ++index;
    }
}

NKA_API void nka_text_path(struct nka_context *a, struct nk_context *ctx, nk_hash path_id, const char *text,
                           const struct nka_text_path_opts *opts)
{
    nka__nk_text_path(a, ctx, path_id, text, 1, opts);
}

NKA_API void nka_text_path_animated(struct nka_context *a, struct nk_context *ctx, nk_hash path_id, const char *text,
                                    float progress, const struct nka_text_path_opts *opts)
{
    nka__nk_text_path(a, ctx, path_id, text, progress < 1 ? progress : 1, opts);
}

static struct nka__nk_profiler *nka__nk_prof(struct nka_context *a)
{
    if (!a->nk_profiler) {
        a->nk_profiler = (struct nka__nk_profiler *)nka__alloc(a, sizeof *a->nk_profiler);
        if (a->nk_profiler) nka__zero(a->nk_profiler, sizeof *a->nk_profiler);
    }
    return a->nk_profiler;
}

static int nka__nk_ready(const struct nka_context *a)
{
    return a && a->nk_profiler && a->nk_profiler->enabled && a->nk_profiler->now;
}

NKA_API void nka_profiler_set_clock(struct nka_context *a, double (*now)(void *user), void *user)
{
    struct nka__nk_profiler *p = a ? nka__nk_prof(a) : 0;
    if (!p) return;
    p->now = now;
    p->user = user;
}

NKA_API void nka_profiler_enable(struct nka_context *a, int on)
{
    struct nka__nk_profiler *p = a ? nka__nk_prof(a) : 0;
    if (p) p->enabled = on != 0;
}

NKA_API int nka_profiler_is_enabled(const struct nka_context *a)
{
    return a && a->nk_profiler && a->nk_profiler->enabled;
}

NKA_API void nka_profiler_begin_frame(struct nka_context *a)
{
    struct nka__nk_profiler *p;
    int i;
    if (!nka__nk_ready(a)) return;
    p = a->nk_profiler;
    p->frame_start = p->now(p->user);
    p->depth = 0;
    for (i = 0; i < p->count; ++i) {
        p->sections[i].ms = 0;
        p->sections[i].calls = 0;
    }
}

NKA_API void nka_profiler_end_frame(struct nka_context *a)
{
    struct nka__nk_profiler *p;
    int i;
    if (!nka__nk_ready(a)) return;
    p = a->nk_profiler;
    p->frame_ms = (p->now(p->user) - p->frame_start) * 1000;
    p->frame[p->at] = (float)p->frame_ms;
    for (i = 0; i < p->count; ++i) p->sections[i].history[p->at] = (float)p->sections[i].ms;
    p->at = (p->at + 1) % NKA__NK_HISTORY;
}

/* Names are kept to 31 characters, so longer ones match on those. */
static int nka__nk_named(const char *name, const char *kept)
{
    int k = 0;
    while (k < 31 && name[k] && name[k] == kept[k]) ++k;
    return k == 31 || name[k] == kept[k];
}

NKA_API void nka_profiler_begin(struct nka_context *a, const char *name)
{
    struct nka__nk_profiler *p;
    int i, k;
    if (!nka__nk_ready(a) || !name) return;
    p = a->nk_profiler;
    for (i = 0; i < p->count && !nka__nk_named(name, p->sections[i].name); ++i) continue;
    if (i == p->count) {
        if (p->count == NKA__NK_SECTIONS) return;
        for (k = 0; k < 31 && name[k]; ++k) p->sections[i].name[k] = name[k];
        p->sections[i].name[k] = 0;
        ++p->count;
    }
    p->sections[i].start = p->now(p->user);
    p->sections[i].calls++;
    if (p->depth < NKA__NK_STACK) p->stack[p->depth++] = i;
}

NKA_API void nka_profiler_end(struct nka_context *a)
{
    struct nka__nk_profiler *p;
    struct nka__nk_section *s;
    if (!nka__nk_ready(a) || a->nk_profiler->depth <= 0) return;
    p = a->nk_profiler;
    s = &p->sections[p->stack[--p->depth]];
    s->ms += (p->now(p->user) - s->start) * 1000;
}

/* Label text built without the varargs Nuklear may be compiled without. */
struct nka__nk_text { char s[96]; int n; };

static void nka__nk_put(struct nka__nk_text *t, const char *s)
{
    while (*s && t->n < 95) t->s[t->n++] = *s++;
    t->s[t->n] = 0;
}

static void nka__nk_hex(struct nka__nk_text *t, nk_hash v)
{
    static const char digits[] = "0123456789ABCDEF";
    char s[11];
    int k;
    s[0] = '0';
    s[1] = 'x';
    for (k = 0; k < 8; ++k) s[2 + k] = digits[(v >> ((7 - k) * 4)) & 15];
    s[10] = 0;
    nka__nk_put(t, s);
}

static void nka__nk_num(struct nka__nk_text *t, float v, int decimals)
{
    char s[24], digits[16];
    int i = 0, n = 0, k;
    unsigned long scale = 1, x;
    for (k = 0; k < decimals; ++k) scale *= 10;
    if (v < 0) { s[i++] = '-'; v = -v; }
    if (v > 1e6f) v = 1e6f;
    x = (unsigned long)(v * (float)scale + 0.5f);
    do { digits[n++] = (char)('0' + x % 10); x /= 10; } while (x || n <= decimals);
    while (n > 0) {
        s[i++] = digits[--n];
        if (n == decimals && n) s[i++] = '.';
    }
    s[i] = 0;
    nka__nk_put(t, s);
}

static void nka__nk_ease(struct nka__nk_text *t, int type)
{
    static const char *families[] = { "quad", "cubic", "quart", "quint", "sine", "expo", "circ", "back", "elastic", "bounce" };
    static const char *variants[] = { "in ", "out ", "in-out " };
    static const char *others[] = { "steps", "cubic bezier", "spring", "custom" };
    if (type == NKA_EASE_LINEAR) nka__nk_put(t, "linear");
    else if (type > 0 && type <= NKA_EASE_IN_OUT_BOUNCE) {
        nka__nk_put(t, variants[(type - 1) % 3]);
        nka__nk_put(t, families[(type - 1) / 3]);
    } else if (type >= NKA_EASE_STEPS && type < NKA_EASE_COUNT) nka__nk_put(t, others[type - NKA_EASE_STEPS]);
    else nka__nk_put(t, "?");
}

static void nka__nk_value(struct nk_context *ctx, const char *prefix, float v, int decimals)
{
    struct nka__nk_text t;
    t.n = 0;
    nka__nk_put(&t, prefix);
    nka__nk_put(&t, ": ");
    nka__nk_num(&t, v, decimals);
    nk_label(ctx, t.s, NK_TEXT_LEFT);
}

static void nka__nk_plot(struct nk_context *ctx, const float *values, int at, float height)
{
    float top = 0.01f;
    int i;
    for (i = 0; i < NKA__NK_HISTORY; ++i) if (values[i] > top) top = values[i];
    nk_layout_row_dynamic(ctx, height, 1);
    if (nk_chart_begin(ctx, NK_CHART_LINES, NKA__NK_HISTORY, 0, top * 1.2f)) {
        for (i = 0; i < NKA__NK_HISTORY; ++i) nk_chart_push(ctx, values[(at + i) % NKA__NK_HISTORY]);
        nk_chart_end(ctx);
    }
}

static void nka__nk_tweens(struct nka_context *a, struct nk_context *ctx)
{
    static const char *kinds[] = { "Float", "Vec2", "Vec4", "Int", "Color" };
    int count[5] = { 0, 0, 0, 0, 0 }, moving = 0, shown = 0, i;
    for (i = 0; i < a->chans.cap; ++i) {
        const struct nka__chan *c = (const struct nka__chan *)nka__at(&a->chans, i);
        if (!a->chans.keys[i]) continue;
        ++count[c->kind];
        moving += !c->sleeping;
    }
    nk_layout_row_dynamic(ctx, 18, 2);
    for (i = 0; i < 5; ++i) nka__nk_value(ctx, kinds[i], (float)count[i], 0);
    nka__nk_value(ctx, "Animate", (float)a->anims.len, 0);
    nka__nk_value(ctx, "Total", (float)(a->chans.len + a->anims.len), 0);
    nka__nk_value(ctx, "Moving", (float)moving, 0);
    for (i = 0; i < a->chans.cap && shown < 12; ++i) {
        const struct nka__chan *c = (const struct nka__chan *)nka__at(&a->chans, i);
        struct nka__nk_text id, ch;
        nk_size progress;
        if (!a->chans.keys[i] || c->sleeping) continue;
        progress = (nk_size)(nka__clamp01((float)((a->time - c->start) / c->dur)) * 100);
        id.n = ch.n = 0;
        nka__nk_hex(&id, c->id);
        nka__nk_hex(&ch, c->ch);
        nk_layout_row_dynamic(ctx, 18, 4);
        nk_label(ctx, kinds[c->kind], NK_TEXT_LEFT);
        nk_label(ctx, id.s, NK_TEXT_LEFT);
        nk_label(ctx, ch.s, NK_TEXT_LEFT);
        nk_progress(ctx, &progress, 100, NK_FIXED);
        ++shown;
    }
}

static void nka__nk_performance(struct nka_context *a, struct nk_context *ctx)
{
    struct nka__nk_profiler *p = nka__nk_prof(a);
    nk_bool on;
    int i;
    if (!p) return;
    on = p->enabled;
    nk_layout_row_dynamic(ctx, 22, 1);
    if (nk_checkbox_label(ctx, "Profiler", &on)) p->enabled = on != 0;
    if (!p->now) {
        nk_label(ctx, "Give it a clock with nka_profiler_set_clock.", NK_TEXT_LEFT);
        return;
    }
    if (!p->enabled) return;
    {
        float sum = 0, top = 0;
        for (i = 0; i < NKA__NK_HISTORY; ++i) {
            sum += p->frame[i];
            if (p->frame[i] > top) top = p->frame[i];
        }
        nk_layout_row_dynamic(ctx, 18, 3);
        nka__nk_value(ctx, "Frame ms", (float)p->frame_ms, 2);
        nka__nk_value(ctx, "Avg", sum / NKA__NK_HISTORY, 2);
        nka__nk_value(ctx, "Max", top, 2);
        nka__nk_plot(ctx, p->frame, p->at, 60);
    }
    if (!p->count) {
        nk_layout_row_dynamic(ctx, 18, 1);
        nk_label(ctx, "Time sections with nka_profiler_begin and _end.", NK_TEXT_LEFT);
    }
    for (i = 0; i < p->count; ++i) {
        nk_layout_row_dynamic(ctx, 18, 3);
        nk_label(ctx, p->sections[i].name, NK_TEXT_LEFT);
        nka__nk_value(ctx, "ms", (float)p->sections[i].ms, 2);
        nka__nk_value(ctx, "calls", (float)p->sections[i].calls, 0);
        nka__nk_plot(ctx, p->sections[i].history, p->at, 30);
    }
}

static void nka__nk_say(struct nk_command_buffer *out, const struct nk_user_font *font, struct nk_rect r,
                        const char *text, int len, struct nk_color color)
{
    struct nk_color clear = { 0, 0, 0, 0 };
    nk_draw_text(out, r, text, len, font, clear, color);
}

NKA_API void nka_show_debug_timeline(struct nka_context *a, struct nk_context *ctx, nk_hash instance_id)
{
    static const char *types[] = { "float", "vec2", "vec4", "int", "color", "float rel", "vec2 rel", "vec4 rel", "color rel" };
    static const struct nk_color c1 = { 91, 194, 231, 255 }, c2 = { 204, 120, 88, 255 };
    static const struct nk_color c1_dim = { 91, 194, 231, 80 }, c2_dim = { 204, 120, 88, 80 };
    static const struct nk_color c1_lit = { 120, 210, 240, 255 }, c2_lit = { 230, 140, 110, 255 };
    static const struct nk_color ground = { 30, 32, 40, 255 }, lane = { 40, 44, 55, 255 }, grid = { 60, 65, 80, 255 };
    static const struct nk_color ink = { 180, 185, 195, 255 }, head = { 255, 255, 255, 220 };
    static const struct nk_color wait = { 50, 55, 70, 255 }, hatch = { 70, 75, 90, 255 };
    const struct nka__clip_inst *in;
    const struct nka__clip_clip *c;
    const struct nk_user_font *font;
    struct nk_command_buffer *out;
    struct nka__nk_text t;
    struct nk_rect r;
    float lh, label, x0, w, y0, dur, total, ry, step, g, at;
    int tracks, i, k;
    if (!a || !ctx || !ctx->current || !ctx->style.font) return;
    font = ctx->style.font;
    lh = font->height;
    in = nka__clip_inst(a, instance_id);
    c = in ? nka__clip_get(a, in->clip) : 0;
    t.n = 0;
    if (!c) {
        nka__nk_put(&t, in ? "No clip " : "No instance ");
        nka__nk_hex(&t, in ? in->clip : instance_id);
        nk_layout_row_dynamic(ctx, lh + 6, 1);
        nk_label(ctx, t.s, NK_TEXT_LEFT);
        return;
    }
    tracks = c->ntracks > 0 ? c->ntracks : 1;
    nk_layout_row_dynamic(ctx, 44 + (float)tracks * 22 + 2 + 36 + 4, 1);
    if (nk_widget(&r, ctx) == NK_WIDGET_INVALID) return;
    out = nk_window_get_canvas(ctx);
    nk_fill_rect(out, r, 4, ground);

    nka__nk_put(&t, "Clip ");
    nka__nk_hex(&t, c->id);
    nka__nk_put(&t, "   ");
    if (in->delay > 0) {
        nka__nk_put(&t, "delay ");
        nka__nk_num(&t, in->delay, 2);
        nka__nk_put(&t, "s   ");
    } else {
        nka__nk_num(&t, in->time, 2);
        nka__nk_put(&t, "s / ");
        nka__nk_num(&t, c->duration, 2);
        nka__nk_put(&t, "s   ");
    }
    nka__nk_put(&t, in->paused ? "paused" : !in->playing ? "stopped" : in->delay > 0 ? "waiting" : "playing");
    nka__nk_say(out, font, nk_rect(r.x + 4, r.y + 4, r.w - 8, lh + 2), t.s, t.n, in->playing ? c1 : ink);

    label = font->width(font->userdata, font->height, "color rel", 9) + 8;
    x0 = r.x + 4 + label;
    w = r.w - label - 8;
    if (w < 100) w = 100;
    y0 = r.y + 44;
    dur = c->duration > 0 ? c->duration : 1;
    total = c->delay + dur;
    step = dur > 10 ? 2.0f : dur > 5 ? 1.0f : 0.5f;
    for (i = 0; i < c->ntracks; ++i) {
        const struct nka__clip_track *tr = &c->tracks[i];
        float ty = y0 + (float)i * 22;
        const char *name = tr->type >= 0 && tr->type < 9 ? types[tr->type] : "?";
        nk_fill_rect(out, nk_rect(x0, ty, w, 20), 2, lane);
        nka__nk_say(out, font, nk_rect(r.x + 4, ty + (20 - lh) / 2, label, lh + 2), name, nka__nk_strlen(name), ink);
        if (c->delay > 0) {
            float end = x0 + c->delay / total * w;
            nk_fill_rect(out, nk_rect(x0, ty + 2, end - x0, 16), 2, wait);
            for (g = x0 + 4; g < end - 4; g += 12) nk_stroke_line(out, g, ty + 4, g + 6, ty + 16, 1, hatch);
        }
        for (k = 0; k < tr->count; ++k) {
            const struct nka__clip_key *key = &c->keys[tr->first + k];
            float next = k + 1 < tr->count ? c->keys[tr->first + k + 1].time : dur;
            float x1 = x0 + (c->delay + key->time) / total * w, x2 = x0 + (c->delay + next) / total * w;
            int on = in->delay <= 0 && in->time >= key->time && in->time < next;
            struct nk_rect seg = nk_rect(x1, ty + 2, x2 - x1, 16), dot = nk_rect(x1 - 8, ty + 2, 16, 16);
            struct nk_rect next_dot = nk_rect(x2 - 8, ty + 2, 16, 16);
            if (on) nk_fill_rect(out, nk_rect(x1 - 1, ty + 1, x2 - x1 + 2, 18), 3, i & 1 ? c2_lit : c1_lit);
            nk_fill_rect(out, seg, 2, on ? (i & 1 ? c2 : c1) : (i & 1 ? c2_dim : c1_dim));
            nk_fill_circle(out, dot, on ? head : (i & 1 ? c2 : c1));
            t.n = 0;
            if (nk_input_is_mouse_hovering_rect(&ctx->input, dot)) {
                nk_stroke_circle(out, nk_rect(x1 - 11, ty - 1, 22, 22), 2, head);
                nka__nk_num(&t, key->time, 3);
                nka__nk_put(&t, "s   ");
                nka__nk_ease(&t, key->ease);
                nk_tooltip(ctx, t.s);
            } else if (nk_input_is_mouse_hovering_rect(&ctx->input, seg) &&
                       !(k + 1 < tr->count && nk_input_is_mouse_hovering_rect(&ctx->input, next_dot))) {
                nk_stroke_rect(out, seg, 2, 2, head);
                nka__nk_num(&t, key->time, 2);
                nka__nk_put(&t, "s - ");
                nka__nk_num(&t, next, 2);
                nka__nk_put(&t, "s   ");
                nka__nk_ease(&t, key->ease);
                nk_tooltip(ctx, t.s);
            }
        }
        for (g = step; g < dur; g += step) {
            float x = x0 + (c->delay + g) / total * w;
            nk_stroke_line(out, x, ty, x, ty + 20, 1, grid);
        }
    }
    if (!c->ntracks) {
        nk_fill_rect(out, nk_rect(x0, y0, w, 20), 2, lane);
        nka__nk_say(out, font, nk_rect(x0 + 10, y0 + (20 - lh) / 2, w - 10, lh + 2), "No tracks", 9, ink);
    }

    ry = y0 + (float)tracks * 22 + 2;
    nk_fill_rect(out, nk_rect(x0, ry, w, 36), 0, lane);
    step = total > 10 ? 2.0f : total > 5 ? 1.0f : total < 1 ? 0.1f : 0.5f;
    t.n = 0;
    nka__nk_num(&t, total, 1);
    nka__nk_put(&t, "s");
    g = font->width(font->userdata, font->height, t.s, t.n) + 8;
    while (step / total * w < g && step < total) {
        float m = step;
        while (m >= 10) m /= 10;
        while (m < 1) m *= 10;
        step *= m > 1.5f && m < 3 ? 2.5f : 2.0f;
    }
    for (g = 0; g <= total + 0.001f; g += step) {
        float x = x0 + g / total * w, tw;
        nk_stroke_line(out, x, ry, x, ry + 6, 1, grid);
        t.n = 0;
        nka__nk_num(&t, g, 1);
        nka__nk_put(&t, "s");
        tw = font->width(font->userdata, font->height, t.s, t.n);
        if (x + tw / 2 < x0 + w) nka__nk_say(out, font, nk_rect(x - tw / 2, ry + 6, tw + 2, lh + 2), t.s, t.n, ink);
    }
    if (c->delay > 0) {
        float x = x0 + c->delay / total * w;
        nk_stroke_line(out, x, ry, x, ry + 36, 2, c1_dim);
        nka__nk_say(out, font, nk_rect(x + 2, ry + 2, w, lh + 2), "delay end", 9, c1_dim);
    }

    at = in->delay > 0 ? c->delay - in->delay : c->delay + in->time;
    if (at >= 0 && at <= total) {
        float x = x0 + at / total * w;
        nk_stroke_line(out, x, y0, x, ry + 36, 2, head);
        nk_fill_triangle(out, x, y0, x - 5, y0 - 8, x + 5, y0 - 8, head);
    }
}

static void nka__nk_clips(struct nka_context *a, struct nk_context *ctx)
{
    int i, shown = 0;
    nk_layout_row_dynamic(ctx, 18, 2);
    nka__nk_value(ctx, "Clips", (float)a->clip_clips.len, 0);
    nka__nk_value(ctx, "Instances", (float)a->clip_insts.len, 0);
    for (i = 0; i < a->clip_insts.cap && shown < 16; ++i) {
        const struct nka__clip_inst *in = (const struct nka__clip_inst *)nka__at(&a->clip_insts, i);
        struct nka__nk_text t;
        nk_hash id;
        if (!a->clip_insts.keys[i]) continue;
        id = in->id;
        t.n = 0;
        nka__nk_put(&t, "Instance ");
        nka__nk_hex(&t, id);
        if (nk_tree_push_hashed(ctx, NK_TREE_NODE, t.s, NK_MINIMIZED, (const char *)&id, (int)sizeof id, 0)) {
            nk_layout_row_dynamic(ctx, 18, 2);
            t.n = 0;
            nka__nk_put(&t, "Clip: ");
            nka__nk_hex(&t, in->clip);
            nk_label(ctx, t.s, NK_TEXT_LEFT);
            nka__nk_value(ctx, "Time", in->time, 2);
            nk_label(ctx, in->paused ? "Paused" : in->playing ? "Playing" : "Stopped", NK_TEXT_LEFT);
            nka__nk_value(ctx, "Loops left", (float)in->loops, 0);
            nka_show_debug_timeline(a, ctx, id);
            nk_tree_pop(ctx);
        }
        ++shown;
    }
}

NKA_API void nka_show_unified_inspector(struct nka_context *a, struct nk_context *ctx, int *open)
{
    static const char *name = "NukAnim";
    nk_flags flags = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;
    if (!a || !ctx || (open && !*open)) return;
    if (open) {
        flags |= NK_WINDOW_CLOSABLE;
        nk_window_show(ctx, name, NK_SHOWN);
    }
    if (nk_begin(ctx, name, nk_rect(20, 20, 380, 520), flags)) {
        if (nk_tree_push(ctx, NK_TREE_TAB, "Time scale", NK_MAXIMIZED)) {
            float scale = a->scale;
            nk_layout_row_dynamic(ctx, 22, 1);
            nka__nk_value(ctx, "Time scale", scale, 2);
            nk_slider_float(ctx, 0, &scale, 2, 0.05f);
            nk_layout_row_dynamic(ctx, 22, 5);
            if (nk_button_label(ctx, "0.1x")) scale = 0.1f;
            if (nk_button_label(ctx, "0.25x")) scale = 0.25f;
            if (nk_button_label(ctx, "0.5x")) scale = 0.5f;
            if (nk_button_label(ctx, "1x")) scale = 1;
            if (nk_button_label(ctx, "2x")) scale = 2;
            nka_set_time_scale(a, scale);
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Tweens", NK_MAXIMIZED)) {
            nka__nk_tweens(a, ctx);
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Clips", NK_MAXIMIZED)) {
            nka__nk_clips(a, ctx);
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "Performance", NK_MINIMIZED)) {
            nka__nk_performance(a, ctx);
            nk_tree_pop(ctx);
        }
    } else if (open) {
        *open = 0;
    }
    nk_end(ctx);
}

/* Lifetime and frames */

NKA_API struct nka_context *nka_create(const struct nk_allocator *alloc)
{
    struct nk_allocator al;
    struct nka_context *a;
    if (alloc) al = *alloc;
    else {
#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
        al.userdata.ptr = 0;
        al.alloc = nka__malloc;
        al.free = nka__mfree;
#else
        return 0;
#endif
    }
    a = (struct nka_context *)al.alloc(al.userdata, 0, sizeof *a);
    if (!a) return 0;
    nka__zero(a, sizeof *a);
    a->alloc = al;
    a->scale = 1;
    a->lazy = 1;
    a->chans.size = sizeof(struct nka__chan);
    a->anims.size = sizeof(struct nka__anim);
    a->path_map.size = sizeof(struct nka__path_data);
    a->clip_clips.size = sizeof(struct nka__clip_clip);
    a->clip_insts.size = sizeof(struct nka__clip_inst);
    a->clip_layers.size = sizeof(struct nka__clip_layer);
    a->clip_acc.size = sizeof(struct nka__clip_acc);
    a->fx_clocks.size = sizeof(struct nka__fx_clock);
    a->fx_grads.size = sizeof(struct nka__fx_grad);
    a->fx_xforms.size = sizeof(struct nka__fx_xform);
    a->fx_drags.size = sizeof(struct nka__fx_drag);
    a->nk_scrolls.size = sizeof(struct nka__nk_scroll);
    a->nk_styles.size = sizeof(struct nka__nk_style);
    a->nk_style_tweens.size = sizeof(struct nka__nk_style_tween);
    return a;
}

NKA_API void nka_destroy(struct nka_context *a)
{
    struct nk_allocator al;
    if (!a) return;
    nka__path_free(a);
    nka__clip_destroy(a);
    nka__map_free(a, &a->fx_clocks);
    nka__map_free(a, &a->fx_grads);
    nka__map_free(a, &a->fx_xforms);
    nka__map_free(a, &a->fx_drags);
    nka__nk_free(a);
    nka__map_free(a, &a->chans);
    nka__map_free(a, &a->anims);
    al = a->alloc;
    al.free(al.userdata, a);
}

NKA_API void nka_update(struct nka_context *a, float dt)
{
    if (!a) return;
    a->frame++;
    a->dt = (dt > 0 ? dt : 0) * a->scale;
    a->time += a->dt;
    a->busy = 0;
    nka__clip_update(a);
    {
        struct nka__nk_fonts *f;
        for (f = a->nk_fonts; f; f = f->next) f->used = 0;
    }
}

NKA_API int nka_busy(const struct nka_context *a) { return a ? a->busy : 0; }
NKA_API double nka_time(const struct nka_context *a) { return a ? a->time : 0; }

NKA_API void nka_gc(struct nka_context *a, unsigned max_age)
{
    if (!a) return;
    nka__sweep(&a->chans, a->frame, max_age);
    nka__sweep(&a->anims, a->frame, max_age);
    nka__clip_gc(a, max_age);
    nka__sweep(&a->fx_clocks, a->frame, max_age);
    nka__sweep(&a->fx_grads, a->frame, max_age);
    nka__sweep(&a->fx_xforms, a->frame, max_age);
    nka__sweep(&a->fx_drags, a->frame, max_age);
    nka__nk_gc(a, max_age);
}

NKA_API void nka_clear(struct nka_context *a)
{
    if (!a) return;
    nka__map_free(a, &a->chans);
    nka__map_free(a, &a->anims);
    nka__clip_clear(a);
    nka__map_free(a, &a->fx_clocks);
    nka__map_free(a, &a->fx_grads);
    nka__map_free(a, &a->fx_xforms);
    nka__map_free(a, &a->fx_drags);
    nka__nk_free_tweens(a);
    nka__map_free(a, &a->nk_scrolls);
}

NKA_API void nka_reserve(struct nka_context *a, int tweens)
{
    int cap = 16;
    if (!a) return;
    while (cap < tweens * 2) cap *= 2;
    if (cap > a->chans.cap) nka__grow(a, &a->chans, cap);
}

NKA_API void nka_set_time_scale(struct nka_context *a, float scale) { if (a) a->scale = scale > 0 ? scale : 0; }
NKA_API float nka_time_scale(const struct nka_context *a) { return a ? a->scale : 1; }
NKA_API void nka_set_lazy_init(struct nka_context *a, int on) { if (a) a->lazy = on != 0; }
NKA_API int nka_lazy_init(const struct nka_context *a) { return a ? a->lazy : 1; }

#endif /* NKA__IMPLEMENTED */
#endif /* NUKANIM_IMPLEMENTATION */

/*
MIT License

Copyright (c) 2026 Alpaq92
Copyright (c) 2025 SoufianeKHIAT (ImAnim)
Copyright (c) 2024-2025 Raidcore (ImAnimate)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
