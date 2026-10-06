#include <stdio.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"
#define NUKANIM_IMPLEMENTATION
#include "nukanim.h"

static int failures;

static void check(int ok, const char *what, int line)
{
    if (ok) return;
    printf("path.c:%d: %s\n", line, what);
    failures++;
}

#define CHECK(c) check((c) != 0, #c, __LINE__)
#define NEAR(x, y, eps) (fabsf((float)(x) - (float)(y)) <= (eps))

static const struct nka_ease linear = { NKA_EASE_LINEAR, 0, 0, 0, 0 };
static const float pi = 3.14159265f;

static struct nk_vec2 v2(float x, float y)
{
    struct nk_vec2 v;
    v.x = x; v.y = y;
    return v;
}

static int near2(struct nk_vec2 p, struct nk_vec2 q, float eps) { return NEAR(p.x, q.x, eps) && NEAR(p.y, q.y, eps); }
static float len2(struct nk_vec2 p) { return sqrtf(p.x * p.x + p.y * p.y); }

/* The central difference between two points 2h apart. */
static struct nk_vec2 slope(struct nk_vec2 lo, struct nk_vec2 hi, float h)
{
    return v2((hi.x - lo.x) / (2 * h), (hi.y - lo.y) / (2 * h));
}

static void test_curves(void)
{
    struct nk_vec2 p0 = v2(0, 0), p1 = v2(10, 40), p2 = v2(50, -20), p3 = v2(80, 30), q;
    const float tension[3] = { 0, 0.5f, 1 }, h = 1e-3f;
    int i, k;

    CHECK(near2(nka_bezier_quadratic(p0, p1, p2, 0), p0, 0));
    CHECK(near2(nka_bezier_quadratic(p0, p1, p2, 1), p2, 0));
    q = nka_bezier_quadratic(p0, p1, p2, 0.5f);
    CHECK(near2(q, v2((p0.x + 2 * p1.x + p2.x) / 4, (p0.y + 2 * p1.y + p2.y) / 4), 1e-4f));
    CHECK(near2(nka_bezier_cubic(p0, p1, p2, p3, 0), p0, 0));
    CHECK(near2(nka_bezier_cubic(p0, p1, p2, p3, 1), p3, 0));
    q = nka_bezier_cubic(p0, p1, p2, p3, 0.5f);
    CHECK(near2(q, v2((p0.x + 3 * p1.x + 3 * p2.x + p3.x) / 8, (p0.y + 3 * p1.y + 3 * p2.y + p3.y) / 8), 1e-4f));
    for (k = 0; k < 3; ++k) {
        CHECK(near2(nka_catmull_rom(p0, p1, p2, p3, 0, tension[k]), p1, 1e-4f));
        CHECK(near2(nka_catmull_rom(p0, p1, p2, p3, 1, tension[k]), p2, 1e-4f));
    }
    q = nka_catmull_rom(p0, p1, p2, p3, 0.5f, 0);
    CHECK(near2(q, v2((-p0.x + 9 * p1.x + 9 * p2.x - p3.x) / 16, (-p0.y + 9 * p1.y + 9 * p2.y - p3.y) / 16), 1e-4f));
    q = nka_catmull_rom_deriv(p0, p1, p2, p3, 0, 0);
    CHECK(near2(q, v2((p2.x - p0.x) / 2, (p2.y - p0.y) / 2), 1e-4f));
    CHECK(len2(nka_catmull_rom_deriv(p0, p1, p2, p3, 0, 1)) < 1e-5f);
    CHECK(len2(nka_catmull_rom_deriv(p0, p1, p2, p3, 1, 1)) < 1e-5f);

    for (i = 0; i <= 10; ++i) {
        float t = (float)i / 10;
        q = slope(nka_bezier_quadratic(p0, p1, p2, t - h), nka_bezier_quadratic(p0, p1, p2, t + h), h);
        CHECK(near2(nka_bezier_quadratic_deriv(p0, p1, p2, t), q, 0.05f));
        q = slope(nka_bezier_cubic(p0, p1, p2, p3, t - h), nka_bezier_cubic(p0, p1, p2, p3, t + h), h);
        CHECK(near2(nka_bezier_cubic_deriv(p0, p1, p2, p3, t), q, 0.05f));
        for (k = 0; k < 3; ++k) {
            q = slope(nka_catmull_rom(p0, p1, p2, p3, t - h, tension[k]), nka_catmull_rom(p0, p1, p2, p3, t + h, tension[k]), h);
            CHECK(near2(nka_catmull_rom_deriv(p0, p1, p2, p3, t, tension[k]), q, 0.05f));
        }
    }
}

static void circle(struct nka_context *a, nk_hash id, float r)
{
    float k = 0.5522847498f * r;
    nka_path_begin(a, id, v2(r, 0));
    nka_path_cubic_to(a, v2(r, k), v2(k, r), v2(0, r));
    nka_path_cubic_to(a, v2(-k, r), v2(-r, k), v2(-r, 0));
    nka_path_cubic_to(a, v2(-r, -k), v2(-k, -r), v2(0, -r));
    nka_path_cubic_to(a, v2(k, -r), v2(r, -k), v2(r, 0));
    nka_path_end(a);
}

static void test_build(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash line = nka_id("line"), ring = nka_id("ring"), tri = nka_id("tri"), cr = nka_id("cr");
    const struct nk_vec2 pts[4] = { { 0, 0 }, { 50, 40 }, { 100, -10 }, { 150, 30 } };
    struct nka__path_data *p;
    struct nk_vec2 q;
    float acc;
    int i;

    CHECK(!nka_path_exists(a, line) && nka_path_length(a, line) == 0);
    CHECK(near2(nka_path_evaluate(a, line, 0.5f), v2(0, 0), 0));
    CHECK(near2(nka_path_tangent(a, line, 0.5f), v2(1, 0), 0) && nka_path_angle(a, line, 0.5f) == 0);
    nka_path_line_to(a, v2(1, 1));
    nka_path_end(a);
    CHECK(a->path_map.len == 0);

    nka_path_begin(a, line, v2(10, 20));
    nka_path_line_to(a, v2(40, 60));
    CHECK(!nka_path_exists(a, line));
    nka_path_end(a);
    CHECK(nka_path_exists(a, line));
    CHECK(nka_path_length(a, line) == 50);
    CHECK(near2(nka_path_evaluate(a, line, 0), v2(10, 20), 0));
    CHECK(near2(nka_path_evaluate(a, line, 1), v2(40, 60), 0));
    CHECK(near2(nka_path_evaluate(a, line, 0.5f), v2(25, 40), 1e-4f));
    CHECK(near2(nka_path_evaluate(a, line, -3), v2(10, 20), 0) && near2(nka_path_evaluate(a, line, 7), v2(40, 60), 0));
    CHECK(near2(nka_path_tangent(a, line, 0.3f), v2(0.6f, 0.8f), 1e-6f));
    nka_path_build_arc_lut(a, line, 16);
    nka_path_begin(a, line, v2(0, 0));
    nka_path_line_to(a, v2(0, 10));
    CHECK(nka_path_length(a, line) == 50 && nka_path_has_arc_lut(a, line));
    nka_path_end(a);
    CHECK(nka_path_length(a, line) == 10 && !nka_path_has_arc_lut(a, line));
    CHECK(near2(nka_path_evaluate(a, line, 1), v2(0, 10), 0) && a->path_map.len == 1);

    circle(a, ring, 100);
    CHECK(NEAR(nka_path_length(a, ring), 2 * pi * 100, 0.5f));
    for (i = 0; i <= 16; ++i) {
        float t = (float)i / 16;
        q = nka_path_evaluate(a, ring, t);
        CHECK(NEAR(len2(q), 100, 0.05f));
        CHECK(NEAR(nka_path_angle(a, ring, t), atan2f(q.x, -q.y), 2e-3f) || NEAR(fabsf(atan2f(q.x, -q.y)), pi, 2e-3f));
    }
    CHECK(near2(nka_path_evaluate(a, ring, 0.125f), v2(70.7107f, 70.7107f), 0.05f));

    nka_path_begin(a, tri, v2(0, 0));
    nka_path_line_to(a, v2(30, 0));
    nka_path_line_to(a, v2(30, 40));
    nka_path_close(a);
    nka_path_close(a);
    nka_path_end(a);
    p = nka__path_get(a, tri);
    CHECK(p && p->n == 3);
    CHECK(near2(nka_path_evaluate(a, tri, 1), v2(0, 0), 0));
    CHECK(NEAR(nka_path_length(a, tri), 120, 1e-3f));

    nka_path_begin(a, cr, pts[0]);
    for (i = 1; i < 4; ++i) nka_path_catmull_to(a, pts[i], 0.5f);
    nka_path_end(a);
    p = nka__path_get(a, cr);
    CHECK(p && p->n == 3);
    if (p) {
        CHECK(near2(p->segs[0].p[0], pts[0], 0) && near2(p->segs[0].p[1], pts[0], 0));
        CHECK(near2(p->segs[0].p[2], pts[1], 0) && near2(p->segs[0].p[3], pts[2], 0));
        CHECK(near2(p->segs[1].p[0], pts[0], 0) && near2(p->segs[1].p[3], pts[3], 0));
        CHECK(near2(p->segs[2].p[0], pts[1], 0) && near2(p->segs[2].p[2], pts[3], 0) && near2(p->segs[2].p[3], pts[3], 0));
        acc = 0;
        for (i = 0; i < 3; ++i) {
            acc += p->segs[i].length;
            CHECK(near2(nka_path_evaluate(a, cr, acc / p->seg_len), pts[i + 1], 1e-3f));
        }
    }

    nka_path_begin(a, nka_id("lost"), v2(0, 0));
    nka_path_line_to(a, v2(5, 0));
    nka_path_begin(a, nka_id("kept"), v2(1, 2));
    nka_path_end(a);
    CHECK(!nka_path_exists(a, nka_id("lost")) && nka_path_exists(a, nka_id("kept")));
    CHECK(near2(nka_path_evaluate(a, nka_id("kept"), 0.5f), v2(1, 2), 0) && nka_path_length(a, nka_id("kept")) == 0);

    nka_path_begin(a, 0, v2(0, 0));
    nka_path_line_to(a, v2(10, 0));
    nka_path_end(a);
    nka_path_begin(a, 1, v2(0, 0));
    nka_path_line_to(a, v2(20, 0));
    nka_path_end(a);
    CHECK(nka_path_length(a, 0) == 10 && nka_path_length(a, 1) == 20);
    nka_destroy(a);
}

static void test_tangents(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash ring = nka_id("ring"), cusp = nka_id("cusp"), dot = nka_id("dot"), stop = nka_id("stop");
    struct nk_vec2 q, d;
    int i;

    circle(a, ring, 100);
    for (i = 0; i <= 40; ++i) {
        float t = (float)i / 40;
        d = nka_path_tangent(a, ring, t);
        q = nka_path_evaluate(a, ring, t);
        CHECK(NEAR(len2(d), 1, 1e-5f));
        CHECK(nka_path_angle(a, ring, t) == atan2f(d.y, d.x));
        CHECK(NEAR(q.x * d.x + q.y * d.y, 0, 1));
        CHECK(q.x * d.y - q.y * d.x > 0);
    }

    nka_path_begin(a, cusp, v2(0, 0));
    nka_path_cubic_to(a, v2(0, 0), v2(0, 50), v2(50, 50));
    nka_path_end(a);
    CHECK(near2(nka_path_tangent(a, cusp, 0), v2(0, 1), 1e-3f));
    CHECK(NEAR(nka_path_angle(a, cusp, 0), pi / 2, 1e-3f));
    CHECK(near2(nka_path_tangent(a, cusp, 1), v2(1, 0), 1e-6f));

    nka_path_begin(a, stop, v2(0, 0));
    nka_path_catmull_to(a, v2(0, 40), 1);
    nka_path_catmull_to(a, v2(40, 40), 1);
    nka_path_end(a);
    CHECK(near2(nka_path_tangent(a, stop, 0), v2(0, 1), 1e-3f));
    CHECK(near2(nka_path_tangent(a, stop, 1), v2(1, 0), 1e-3f));

    nka_path_begin(a, dot, v2(5, 5));
    nka_path_line_to(a, v2(5, 5));
    nka_path_end(a);
    CHECK(near2(nka_path_tangent(a, dot, 0.5f), v2(1, 0), 0) && nka_path_angle(a, dot, 0.5f) == 0);
    CHECK(nka_path_length(a, dot) == 0 && near2(nka_path_evaluate(a, dot, 0.5f), v2(5, 5), 0));
    nka_path_build_arc_lut(a, dot, 64);
    CHECK(!nka_path_has_arc_lut(a, dot));
    nka_destroy(a);
}

static void uneven(struct nka_context *a, nk_hash id)
{
    nka_path_begin(a, id, v2(0, 0));
    nka_path_line_to(a, v2(50, 0));
    nka_path_cubic_to(a, v2(140, 0), v2(145, 0), v2(150, 0));
    nka_path_end(a);
}

static void test_arc_length(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash id = nka_id("uneven"), ring = nka_id("ring"), none = nka_id("none");
    struct nk_vec2 before[11], q, d;
    int i;

    uneven(a, id);
    CHECK(!nka_path_has_arc_lut(a, id));
    CHECK(NEAR(nka_path_length(a, id), 150, 1e-3f));
    CHECK(fabsf(nka_path_evaluate(a, id, 0.4f).x - 60) > 5);
    CHECK(NEAR(nka_path_distance_to_t(a, id, 75), 0.5f, 1e-6f));
    CHECK(nka_path_distance_to_t(a, id, -5) == 0 && nka_path_distance_to_t(a, id, 500) == 1);
    for (i = 0; i <= 10; ++i) before[i] = nka_path_evaluate(a, id, (float)i / 10);

    nka_path_build_arc_lut(a, id, 256);
    CHECK(nka_path_has_arc_lut(a, id) && nka__path_get(a, id)->lut_n == 257);
    CHECK(NEAR(nka_path_length(a, id), 150, 1e-2f));
    for (i = 0; i <= 10; ++i) CHECK(near2(nka_path_evaluate(a, id, (float)i / 10), before[i], 0));
    /* The table steps evenly in t, so the step across the corner at 50 interpolates two speeds. */
    for (i = 0; i <= 30; ++i) {
        float dist = (float)i * 5;
        q = nka_path_evaluate_at_distance(a, id, dist);
        CHECK(NEAR(q.x, dist, fabsf(dist - 50) < 2 ? 0.25f : 0.01f) && q.y == 0);
        CHECK(near2(nka_path_tangent_at_distance(a, id, dist), v2(1, 0), 1e-6f));
        CHECK(nka_path_angle_at_distance(a, id, dist) == 0);
    }
    for (i = 0; i <= 300; ++i) CHECK(NEAR(nka_path_evaluate_at_distance(a, id, (float)i / 2).x, (float)i / 2, 0.25f));
    nka_path_begin(a, nka_id("smooth"), v2(0, 0));
    nka_path_cubic_to(a, v2(90, 0), v2(95, 0), v2(100, 0));
    nka_path_end(a);
    CHECK(fabsf(nka_path_evaluate(a, nka_id("smooth"), 0.5f).x - 50) > 30);
    nka_path_build_arc_lut(a, nka_id("smooth"), 256);
    for (i = 0; i <= 200; ++i)
        CHECK(NEAR(nka_path_evaluate_at_distance(a, nka_id("smooth"), (float)i / 2).x, (float)i / 2, 0.01f));
    CHECK(near2(nka_path_evaluate_at_distance(a, id, -10), v2(0, 0), 0));
    CHECK(near2(nka_path_evaluate_at_distance(a, id, 1000), v2(150, 0), 1e-4f));
    CHECK(nka_path_distance_to_t(a, id, -1) == 0 && nka_path_distance_to_t(a, id, 1e6f) == 1);
    CHECK(NEAR(nka_path_distance_to_t(a, id, 25), 25.0f / 150, 1e-3f));

    circle(a, ring, 100);
    for (i = 0; i <= 10; ++i) before[i] = nka_path_evaluate(a, ring, (float)i / 10);
    nka_path_build_arc_lut(a, ring, 0);
    CHECK(nka__path_get(a, ring)->lut_n == 65);
    nka_path_build_arc_lut(a, ring, 512);
    CHECK(NEAR(nka_path_length(a, ring), 628.4f, 0.02f));
    for (i = 0; i <= 10; ++i) CHECK(near2(nka_path_evaluate(a, ring, (float)i / 10), before[i], 0));
    for (i = 0; i < 24; ++i) {
        float dist = nka_path_length(a, ring) * (float)i / 24, turn = 2 * pi * (float)i / 24;
        q = nka_path_evaluate_at_distance(a, ring, dist);
        d = nka_path_tangent_at_distance(a, ring, dist);
        CHECK(near2(v2(q.x / len2(q), q.y / len2(q)), v2(cosf(turn), sinf(turn)), 2e-3f));
        CHECK(NEAR(len2(d), 1, 1e-5f));
        CHECK(nka_path_angle_at_distance(a, ring, dist) == atan2f(d.y, d.x));
    }

    uneven(a, id);
    CHECK(!nka_path_has_arc_lut(a, id) && NEAR(nka_path_length(a, id), 150, 1e-3f));

    nka_path_build_arc_lut(a, none, 64);
    CHECK(!nka_path_has_arc_lut(a, none) && nka_path_distance_to_t(a, none, 5) == 0);
    CHECK(near2(nka_path_evaluate_at_distance(a, none, 5), v2(0, 0), 0));
    CHECK(near2(nka_path_tangent_at_distance(a, none, 5), v2(1, 0), 0) && nka_path_angle_at_distance(a, none, 5) == 0);
    nka_destroy(a);
}

static void test_tweens(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash id = nka_id("mover"), ch = nka_id("pos"), path = nka_id("L"), flat = nka_id("flat");
    struct nk_vec2 q;
    int i;

    nka_update(a, 0.016f);
    q = nka_tween_path(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE);
    CHECK(near2(q, v2(0, 0), 0) && a->chans.len == 0 && !nka_busy(a));
    CHECK(nka_tween_path_angle(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE) == 0 && a->chans.len == 0);

    nka_path_begin(a, path, v2(0, 0));
    nka_path_line_to(a, v2(100, 0));
    nka_path_line_to(a, v2(100, 100));
    nka_path_end(a);

    CHECK(near2(nka_tween_path(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE), v2(0, 0), 0) && nka_busy(a));
    CHECK(nka_tween_path_angle(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE) == 0);
    nka_update(a, 0.25f);
    CHECK(!nka_busy(a));
    CHECK(near2(nka_tween_path(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE), v2(50, 0), 1e-3f) && nka_busy(a));
    CHECK(nka_tween_path_angle(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE) == 0);
    nka_update(a, 0.5f);
    CHECK(near2(nka_tween_path(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE), v2(100, 50), 1e-3f));
    CHECK(NEAR(nka_tween_path_angle(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE), pi / 2, 1e-6f));
    nka_update(a, 0.25f);
    CHECK(near2(nka_tween_path(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE), v2(100, 100), 0));
    nka_update(a, 0.25f);
    CHECK(near2(nka_tween_path(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE), v2(100, 100), 0));
    CHECK(NEAR(nka_tween_path_angle(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE), pi / 2, 1e-6f));
    CHECK(!nka_busy(a));

    /* As upstream: a cut still animates, and a new ease does not start over. */
    CHECK(near2(nka_tween_path(a, id, nka_id("cut"), path, 1, linear, NKA_POLICY_CUT), v2(0, 0), 0) && nka_busy(a));
    nka_tween_path(a, id, nka_id("ease"), path, 1, linear, NKA_POLICY_CROSSFADE);
    nka_update(a, 0.5f);
    q = nka_tween_path(a, id, nka_id("ease"), path, 1, nka_ease(NKA_EASE_IN_QUAD), NKA_POLICY_CROSSFADE);
    CHECK(near2(q, v2(100, 0), 1e-3f));
    nka_update(a, 0.25f);
    q = nka_tween_path(a, id, nka_id("ease"), path, 1, nka_ease(NKA_EASE_IN_QUAD), NKA_POLICY_CROSSFADE);
    CHECK(near2(q, v2(100, 50), 1e-3f));

    uneven(a, flat);
    nka_tween_path(a, id, nka_id("raw"), flat, 1, linear, NKA_POLICY_CROSSFADE);
    nka_update(a, 0.5f);
    q = nka_tween_path(a, id, nka_id("raw"), flat, 1, linear, NKA_POLICY_CROSSFADE);
    CHECK(q.x > 100);
    nka_path_build_arc_lut(a, flat, 256);
    q = nka_tween_path(a, id, nka_id("raw"), flat, 1, linear, NKA_POLICY_CROSSFADE);
    CHECK(NEAR(q.x, 75, 0.02f));

    circle(a, nka_id("ring"), 100);
    nka_path_build_arc_lut(a, nka_id("ring"), 256);
    nka_tween_path_angle(a, id, nka_id("spin"), nka_id("ring"), 1, linear, NKA_POLICY_CROSSFADE);
    nka_update(a, 0.25f);
    CHECK(NEAR(fabsf(nka_tween_path_angle(a, id, nka_id("spin"), nka_id("ring"), 1, linear, NKA_POLICY_CROSSFADE)), pi, 2e-3f));
    nka_update(a, 0.25f);
    CHECK(NEAR(nka_tween_path_angle(a, id, nka_id("spin"), nka_id("ring"), 1, linear, NKA_POLICY_CROSSFADE), -pi / 2, 2e-3f));

    for (i = 0; i < 10; ++i) nka_update(a, 0.1f);
    nka_gc(a, 5);
    CHECK(a->chans.len == 0);
    CHECK(near2(nka_tween_path(a, id, ch, path, 1, linear, NKA_POLICY_CROSSFADE), v2(0, 0), 0) && nka_busy(a));
    nka_clear(a);
    CHECK(nka_path_exists(a, path) && nka_path_has_arc_lut(a, flat));
    nka_destroy(a);
}

static void test_morph(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash pa = nka_id("a"), pb = nka_id("b"), sq = nka_id("square"), ring = nka_id("ring"), bare = nka_id("bare");
    struct nka_morph_opts o = nka_morph_opts_default();
    volatile float zero = 0;
    struct nk_vec2 q;
    int i;

    CHECK(o.samples == 64 && o.match_endpoints && o.use_arc_length);
    nka_path_begin(a, pa, v2(0, 0));
    nka_path_line_to(a, v2(100, 0));
    nka_path_end(a);
    nka_path_begin(a, pb, v2(0, 100));
    nka_path_line_to(a, v2(30, 100));
    nka_path_line_to(a, v2(100, 100));
    nka_path_end(a);
    nka_path_morph(a, pa, pb, 0.5f, 0, 0);
    CHECK(nka_path_has_arc_lut(a, pa) && !nka_path_has_arc_lut(a, pb));
    for (i = 0; i <= 10; ++i) {
        float t = (float)i / 10;
        CHECK(near2(nka_path_morph(a, pa, pb, t, 0, 0), nka_path_evaluate_at_distance(a, pa, t * nka_path_length(a, pa)), 0));
        CHECK(near2(nka_path_morph(a, pa, pb, t, 1, 0), nka_path_evaluate_at_distance(a, pb, t * nka_path_length(a, pb)), 0));
        CHECK(near2(nka_path_morph(a, pa, pb, t, -2, 0), nka_path_morph(a, pa, pb, t, 0, 0), 0));
        CHECK(near2(nka_path_morph(a, pa, pb, t, 0.5f, 0), v2(100 * t, 50), 1e-3f));
        CHECK(near2(nka_path_morph(a, pa, pb, t, 0.25f, &o), v2(100 * t, 25), 1e-3f));
        CHECK(near2(nka_path_morph_tangent(a, pa, pb, t, 0.5f, 0), v2(1, 0), 1e-4f));
        CHECK(NEAR(nka_path_morph_angle(a, pa, pb, t, 0.5f, 0), 0, 1e-4f));
        CHECK(near2(nka_path_morph_tangent(a, pa, pb, t, 0, 0), nka_path_tangent_at_distance(a, pa, t * nka_path_length(a, pa)), 0));
    }
    CHECK(nka_path_has_arc_lut(a, pa) && nka_path_has_arc_lut(a, pb));

    /* A blend leaving 0 or reaching 1 does not jump, even where t is far from arc length. */
    nka_path_begin(a, nka_id("smooth"), v2(0, 0));
    nka_path_cubic_to(a, v2(90, 0), v2(95, 0), v2(100, 0));
    nka_path_end(a);
    for (i = 0; i <= 10; ++i) {
        float t = (float)i / 10;
        CHECK(near2(nka_path_morph(a, nka_id("smooth"), pb, t, 0, 0), nka_path_morph(a, nka_id("smooth"), pb, t, 1e-4f, 0), 0.05f));
        CHECK(near2(nka_path_morph(a, pb, nka_id("smooth"), t, 1, 0), nka_path_morph(a, pb, nka_id("smooth"), t, 1 - 1e-4f, 0), 0.05f));
    }
    o.use_arc_length = 0;
    for (i = 0; i <= 10; ++i) {
        float t = (float)i / 10;
        CHECK(near2(nka_path_morph(a, nka_id("smooth"), pb, t, 0, &o), nka_path_evaluate(a, nka_id("smooth"), t), 0));
        CHECK(near2(nka_path_morph_tangent(a, nka_id("smooth"), pb, t, 0, &o), nka_path_tangent(a, nka_id("smooth"), t), 0));
    }
    o = nka_morph_opts_default();

    nka_path_begin(a, sq, v2(50, -50));
    nka_path_line_to(a, v2(50, 50));
    nka_path_line_to(a, v2(-50, 50));
    nka_path_line_to(a, v2(-50, -50));
    nka_path_close(a);
    nka_path_end(a);
    circle(a, ring, 70);
    o.samples = 32;
    nka_path_morph(a, sq, ring, 0.5f, 0.5f, &o);
    CHECK(nka__path_get(a, sq)->lut_n == 65 && nka__path_get(a, ring)->lut_n == 65);
    for (i = 0; i < 32; i += 3) {
        float t = (float)i / 31;
        struct nk_vec2 x = nka_path_evaluate_at_distance(a, sq, t * nka_path_length(a, sq));
        struct nk_vec2 y = nka_path_evaluate_at_distance(a, ring, t * nka_path_length(a, ring));
        q = nka_path_morph(a, sq, ring, t, 0.5f, &o);
        CHECK(near2(q, v2((x.x + y.x) / 2, (x.y + y.y) / 2), 1e-3f));
    }
    CHECK(near2(nka_path_morph(a, sq, ring, 0, 0.5f, &o), v2(60, -25), 1e-3f));
    CHECK(near2(nka_path_morph(a, sq, ring, 1, 0.5f, &o), v2(60, -25), 1e-3f));

    o.use_arc_length = 0;
    nka_path_begin(a, bare, v2(0, 0));
    nka_path_quadratic_to(a, v2(50, 100), v2(100, 0));
    nka_path_end(a);
    q = nka_path_morph(a, bare, pa, 0.5f, 0.5f, &o);
    CHECK(!nka_path_has_arc_lut(a, bare));
    CHECK(near2(q, v2(50, 25), 0.1f));

    o = nka_morph_opts_default();
    o.samples = 1;
    CHECK(near2(nka_path_morph(a, pa, pb, 0.5f, 0.5f, &o), v2(50, 50), 1e-3f));
    o.samples = -5;
    CHECK(near2(nka_path_morph(a, pa, pb, 0.25f, 0.5f, &o), v2(25, 50), 1e-3f));
    CHECK(near2(nka_path_morph(a, pa, nka_id("missing"), 0.5f, 0.5f, 0), v2(25, 0), 1e-3f));
    CHECK(near2(nka_path_morph(a, pa, pb, zero / zero, 0.5f, 0), v2(0, 50), 0));
    CHECK(near2(nka_path_morph(a, pa, pb, 0.5f, zero / zero, 0), v2(50, 0), 0));
    nka_destroy(a);
}

static void test_morph_tween(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash pa = nka_id("a"), pb = nka_id("b"), id = nka_id("mt"), ch = nka_id("c");
    struct nk_vec2 q;

    nka_path_begin(a, pa, v2(0, 0));
    nka_path_line_to(a, v2(100, 0));
    nka_path_end(a);
    nka_path_begin(a, pb, v2(0, 100));
    nka_path_cubic_to(a, v2(30, 100), v2(70, 100), v2(100, 100));
    nka_path_end(a);

    nka_update(a, 0.016f);
    CHECK(nka_get_morph_blend(a, id, ch) == 0);
    q = nka_tween_path_morph(a, id, ch, pa, pb, 1, 1, linear, linear, NKA_POLICY_CROSSFADE, 0);
    CHECK(near2(q, v2(0, 0), 0) && nka_busy(a) && nka_get_morph_blend(a, id, ch) == 0);
    nka_update(a, 0.5f);
    q = nka_tween_path_morph(a, id, ch, pa, pb, 1, 1, linear, linear, NKA_POLICY_CROSSFADE, 0);
    CHECK(NEAR(nka_get_morph_blend(a, id, ch), 0.5f, 1e-5f));
    CHECK(near2(q, v2(50, 50), 0.01f));
    nka_update(a, 0.25f);
    q = nka_tween_path_morph(a, id, ch, pa, pb, 1, 1, linear, linear, NKA_POLICY_CROSSFADE, 0);
    CHECK(NEAR(nka_get_morph_blend(a, id, ch), 0.75f, 1e-5f) && near2(q, v2(75, 75), 0.01f));
    nka_update(a, 0.25f);
    q = nka_tween_path_morph(a, id, ch, pa, pb, 1, 1, linear, linear, NKA_POLICY_CROSSFADE, 0);
    CHECK(nka_get_morph_blend(a, id, ch) == 1 && near2(q, v2(100, 100), 0));
    nka_update(a, 0.1f);
    q = nka_tween_path_morph(a, id, ch, pa, pb, 1, 1, linear, linear, NKA_POLICY_CROSSFADE, 0);
    CHECK(near2(q, v2(100, 100), 0) && !nka_busy(a));

    nka_tween_path_morph(a, id, ch, pa, pb, 0, 1, linear, linear, NKA_POLICY_CROSSFADE, 0);
    nka_update(a, 0.5f);
    q = nka_tween_path_morph(a, id, ch, pa, pb, 0, 1, linear, linear, NKA_POLICY_CROSSFADE, 0);
    CHECK(NEAR(nka_get_morph_blend(a, id, ch), 0.5f, 1e-5f) && near2(q, v2(100, 50), 1e-3f) && nka_busy(a));
    q = nka_tween_path_morph(a, id, ch, pa, pb, 1, 1, linear, linear, NKA_POLICY_CUT, 0);
    CHECK(nka_get_morph_blend(a, id, ch) == 1 && near2(q, v2(100, 100), 0));
    nka_update(a, 2);
    q = nka_tween_path_morph(a, id, ch, pa, pb, 1, 1, linear, linear, NKA_POLICY_CUT, 0);
    CHECK(near2(q, v2(100, 100), 0) && !nka_busy(a));

    q = nka_tween_path_morph(a, id, nka_id("cut"), pa, pb, 0.25f, 1, linear, linear, NKA_POLICY_CUT, 0);
    CHECK(nka_get_morph_blend(a, id, nka_id("cut")) == 0.25f && near2(q, v2(0, 25), 1e-3f) && nka_busy(a));
    CHECK(nka_get_morph_blend(a, id, nka_id("none")) == 0 && nka_get_morph_blend(0, id, ch) == 0);
    nka_destroy(a);
}

static void test_quads(void)
{
    struct nk_vec2 quad[4], glyph[4];
    int i;

    nka_make_glyph_quad(quad, v2(10, 20), 0, 8, 12, 0);
    CHECK(near2(quad[0], v2(6, 20), 1e-5f) && near2(quad[1], v2(14, 20), 1e-5f));
    CHECK(near2(quad[2], v2(14, 8), 1e-5f) && near2(quad[3], v2(6, 8), 1e-5f));
    nka_make_glyph_quad(quad, v2(10, 20), 0, 8, 12, 3);
    CHECK(near2(quad[0], v2(6, 17), 1e-5f) && near2(quad[2], v2(14, 5), 1e-5f));
    nka_make_glyph_quad(glyph, v2(0, 0), pi / 2, 8, 12, 0);
    CHECK(near2(glyph[0], v2(0, -4), 1e-5f) && near2(glyph[1], v2(0, 4), 1e-5f));
    CHECK(near2(glyph[2], v2(12, 4), 1e-5f) && near2(glyph[3], v2(12, -4), 1e-5f));
    nka_make_glyph_quad(quad, v2(0, 0), 0, 8, 12, 0);
    nka_transform_quad(quad, v2(0, 0), pi / 2, v2(0, 0));
    for (i = 0; i < 4; ++i) CHECK(near2(quad[i], glyph[i], 1e-5f));

    quad[0] = v2(0, 0); quad[1] = v2(10, 0); quad[2] = v2(10, 10); quad[3] = v2(0, 10);
    nka_transform_quad(quad, v2(5, 5), pi / 2, v2(100, 50));
    CHECK(near2(quad[0], v2(110, 50), 1e-4f) && near2(quad[1], v2(110, 60), 1e-4f));
    CHECK(near2(quad[2], v2(100, 60), 1e-4f) && near2(quad[3], v2(100, 50), 1e-4f));
    nka_transform_quad(quad, v2(0, 0), 0, v2(-100, -50));
    CHECK(near2(quad[0], v2(10, 0), 1e-4f) && near2(quad[2], v2(0, 10), 1e-4f));
    nka_transform_quad(0, v2(0, 0), 1, v2(0, 0));
    nka_make_glyph_quad(0, v2(0, 0), 1, 1, 1, 1);
}

static int live;

static void *count_alloc(nk_handle h, void *old, nk_size n)
{
    (void)h; (void)old;
    live++;
    return malloc(n);
}

static void count_free(nk_handle h, void *p)
{
    (void)h;
    live--;
    free(p);
}

static void wave(struct nka_context *a, nk_hash id, int n)
{
    int i;
    nka_path_begin(a, id, v2(0, 0));
    for (i = 0; i < n; ++i) nka_path_quadratic_to(a, v2((float)i * 20 + 10, i & 1 ? 20.0f : -20.0f), v2((float)i * 20 + 20, 0));
    nka_path_end(a);
}

static void frame(struct nka_context *a)
{
    nka_update(a, 0.016f);
    wave(a, 1, 20);
    nka_path_build_arc_lut(a, 1, 128);
    wave(a, 2, 5);
    nka_path_morph(a, 1, 2, 0.5f, 0.5f, 0);
    nka_tween_path(a, 7, 7, 1, 1, linear, NKA_POLICY_CROSSFADE);
    nka_tween_path_morph(a, 7, 8, 1, 2, 1, 1, linear, linear, NKA_POLICY_CROSSFADE, 0);
}

static void test_memory(void)
{
    struct nk_allocator al;
    struct nka_context *a;
    int i, settled;
    al.userdata.ptr = 0;
    al.alloc = count_alloc;
    al.free = count_free;
    a = nka_create(&al);
    for (i = 0; i < 3; ++i) frame(a);
    settled = live;
    for (i = 0; i < 100; ++i) frame(a);
    CHECK(live == settled);
    CHECK(nka_path_exists(a, 1) && nka_path_has_arc_lut(a, 1) && nka_path_has_arc_lut(a, 2));
    wave(a, 1, 300);
    wave(a, 1, 3);
    nka_path_build_arc_lut(a, 1, 4000);
    CHECK(nka__path_get(a, 1)->n == 3);
    nka_path_begin(a, 9, v2(0, 0));
    nka_path_line_to(a, v2(1, 0));
    nka_destroy(a);
    CHECK(live == 0);
    nka_path_begin(0, 1, v2(0, 0));
    nka_path_line_to(0, v2(1, 0));
    nka_path_end(0);
    CHECK(!nka_path_exists(0, 1) && near2(nka_path_morph(0, 1, 2, 0.5f, 0.5f, 0), v2(0, 0), 0));
    CHECK(near2(nka_tween_path(0, 1, 1, 1, 1, linear, NKA_POLICY_CROSSFADE), v2(0, 0), 0));
}

int main(void)
{
    test_curves();
    test_build();
    test_tangents();
    test_arc_length();
    test_tweens();
    test_morph();
    test_morph_tween();
    test_quads();
    test_memory();
    if (failures) printf("%d failed\n", failures);
    else printf("path: ok\n");
    return failures != 0;
}
