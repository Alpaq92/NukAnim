#include <stdio.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"
#define NUKANIM_IMPLEMENTATION
#include "nukanim.h"

static int failures;

static void check(int ok, const char *what, int line)
{
    if (ok) return;
    printf("core.c:%d: %s\n", line, what);
    failures++;
}

#define CHECK(c) check((c) != 0, #c, __LINE__)
#define NEAR(x, y, eps) (fabsf((float)(x) - (float)(y)) <= (eps))

static const struct nka_ease linear = { NKA_EASE_LINEAR, 0, 0, 0, 0 };

/* ImAnimate's curves, as ImAnimate.cpp writes them, in its ECurve order. */
static float imanimate(int curve, float p)
{
    const float pi = 3.14159f;
    switch (curve) {
    case 0: return p;
    case 1: return 1 - cosf((p * pi) / 2);
    case 2: return sinf((p * pi) / 2);
    case 3: return -(cosf(pi * p) - 1) / 2;
    case 4: return p * p;
    case 5: return 1 - (1 - p) * (1 - p);
    case 6: return p < 0.5f ? 2 * p * p : 1 - powf(-2 * p + 2, 2) / 2.0f;
    case 7: return p * p * p;
    case 8: return 1 - powf(1 - p, 3);
    case 9: return p < 0.5f ? 4 * p * p * p : 1 - powf(-2 * p + 2, 3) / 2.0f;
    case 10: return p * p * p * p;
    case 11: return 1 - powf(1 - p, 4);
    case 12: return p < 0.5f ? 8 * p * p * p * p : 1 - powf(-2 * p + 2, 4) / 2.0f;
    case 13: return p * p * p * p * p;
    case 14: return 1 - powf(1 - p, 5);
    case 15: return p < 0.5f ? 16 * p * p * p * p * p : 1 - powf(-2 * p + 2, 5) / 2.0f;
    case 16: return p == 0 ? 0 : powf(2, 10 * p - 10);
    case 17: return p == 1 ? 1 : 1 - powf(2, -10 * p);
    case 18: return p == 0 ? 0 : p == 1 ? 1 : p < 0.5f ? powf(2, 20 * p - 10) / 2.0f : (2 - powf(2, -20 * p + 10)) / 2.0f;
    case 19: return 1 - sqrtf(1 - powf(p, 2));
    case 20: return sqrtf(1 - powf(p - 1, 2));
    default: return p < 0.5f ? (1 - sqrtf(1 - powf(2 * p, 2))) / 2.0f : (sqrtf(1 - powf(-2 * p + 2, 2)) + 1) / 2.0f;
    }
}

static const int imanimate_curves[22] = {
    NKA_EASE_LINEAR,
    NKA_EASE_IN_SINE, NKA_EASE_OUT_SINE, NKA_EASE_IN_OUT_SINE,
    NKA_EASE_IN_QUAD, NKA_EASE_OUT_QUAD, NKA_EASE_IN_OUT_QUAD,
    NKA_EASE_IN_CUBIC, NKA_EASE_OUT_CUBIC, NKA_EASE_IN_OUT_CUBIC,
    NKA_EASE_IN_QUART, NKA_EASE_OUT_QUART, NKA_EASE_IN_OUT_QUART,
    NKA_EASE_IN_QUINT, NKA_EASE_OUT_QUINT, NKA_EASE_IN_OUT_QUINT,
    NKA_EASE_IN_EXPO, NKA_EASE_OUT_EXPO, NKA_EASE_IN_OUT_EXPO,
    NKA_EASE_IN_CIRC, NKA_EASE_OUT_CIRC, NKA_EASE_IN_OUT_CIRC
};

static float square(float t) { return t * t; }

static void test_easing(void)
{
    struct nka_context *a = nka_create(0);
    int type, i, k;
    for (type = NKA_EASE_LINEAR; type <= NKA_EASE_IN_OUT_BOUNCE; ++type) {
        CHECK(NEAR(nka_eval_preset(type, 0), 0, 1e-6f));
        CHECK(NEAR(nka_eval_preset(type, 1), 1, 1e-5f));
    }
    for (type = NKA_EASE_IN_QUAD; type <= NKA_EASE_IN_BOUNCE; type += 3) {
        CHECK(NEAR(nka_eval_preset(type + 2, 0.5f), 0.5f, 1e-5f));
        for (k = 0; k <= 20; ++k) {
            float t = (float)k / 20;
            CHECK(NEAR(nka_eval_preset(type + 1, t), 1 - nka_eval_preset(type, 1 - t), 1e-5f));
        }
    }
    for (i = 0; i < 22; ++i)
        for (k = 0; k <= 64; ++k) {
            float t = (float)k / 64;
            CHECK(NEAR(nka_eval_preset(imanimate_curves[i], t), imanimate(i, t), 1e-5f));
        }

    for (k = 0; k <= 20; ++k) {
        float t = (float)k / 20;
        struct nka_ease in_back = nka_ease(NKA_EASE_IN_BACK), in_el = nka_ease(NKA_EASE_IN_ELASTIC);
        in_back.p0 = 1.70158f;
        in_el.p0 = 1;
        in_el.p1 = 0.3f;
        CHECK(NEAR(nka_eval(a, nka_ease_back(1.70158f), t), nka_eval_preset(NKA_EASE_OUT_BACK, t), 1e-5f));
        CHECK(NEAR(nka_eval(a, in_back, t), nka_eval_preset(NKA_EASE_IN_BACK, t), 1e-5f));
        CHECK(NEAR(nka_eval(a, nka_ease_elastic(1, 0.3f), t), nka_eval_preset(NKA_EASE_OUT_ELASTIC, t), 1e-5f));
        CHECK(NEAR(nka_eval(a, in_el, t), nka_eval_preset(NKA_EASE_IN_ELASTIC, t), 1e-5f));
        CHECK(nka_eval(a, nka_ease_elastic(0.5f, 0.3f), t) == nka_eval(a, nka_ease_elastic(0.5f, 0.3f), t));
        CHECK(NEAR(nka_eval(a, nka_ease_bezier(0, 0, 1, 1), t), t, 1e-4f));
    }
    CHECK(NEAR(nka_eval(a, nka_ease_back(3), 0.8f), 1 - nka__back(0.2f, 3), 1e-6f));
    CHECK(nka_eval(a, nka_ease_back(3), 0.8f) > 1);

    CHECK(nka_eval(a, nka_ease_steps(4, 0), 0) == 0);
    CHECK(nka_eval(a, nka_ease_steps(4, 0), 0.3f) == 0.25f);
    CHECK(nka_eval(a, nka_ease_steps(4, 0), 0.99f) == 0.75f);
    CHECK(nka_eval(a, nka_ease_steps(4, 0), 1) == 1);
    CHECK(nka_eval(a, nka_ease_steps(4, 1), 0) == 0.25f);
    CHECK(nka_eval(a, nka_ease_steps(4, 1), 0.3f) == 0.5f);
    CHECK(nka_eval(a, nka_ease_steps(4, 1), 1) == 1);
    CHECK(NEAR(nka_eval(a, nka_ease_steps(4, 2), 0), 0.2f, 1e-6f));
    CHECK(NEAR(nka_eval(a, nka_ease_steps(4, 2), 0.3f), 0.4f, 1e-6f));
    CHECK(NEAR(nka_eval(a, nka_ease_steps(4, 2), 0.99f), 0.8f, 1e-6f));
    CHECK(nka_eval(a, nka_ease_steps(4, 2), 1) == 1);

    /* CSS ease at the middle */
    CHECK(NEAR(nka_eval(a, nka_ease_bezier(0.25f, 0.1f, 0.25f, 1), 0.5f), 0.8024f, 1e-3f));

    CHECK(NEAR(nka_eval(a, nka_ease_spring(0, 0, 0, 0), 0), 0, 1e-6f));
    CHECK(NEAR(nka_eval(a, nka_ease_spring(0, 0, 0, 0), 1), 1, 1e-3f));
    CHECK(nka_eval(a, nka_ease_spring(1, 120, 20, 8), 0.01f) > nka_eval(a, nka_ease_spring(1, 120, 20, 0), 0.01f));
    {
        float prev = 0;
        for (k = 1; k <= 50; ++k) {
            float v = nka_eval(a, nka_ease_spring(1, 120, 100, 0), (float)k / 50);
            CHECK(v > prev && v < 1);
            prev = v;
        }
        CHECK(NEAR(nka_eval(a, nka_ease_spring(1, 100, 20, 0), 0.5f), 1 - expf(-5) * (1 + 5), 1e-5f));
    }

    CHECK(nka_eval(a, nka_ease_custom(3), 0.5f) == 0.5f);
    nka_register_ease(a, 3, square);
    CHECK(nka_custom_ease(a, 3) == square);
    CHECK(nka_eval(a, nka_ease_custom(3), 0.5f) == 0.25f);
    nka_register_ease(a, 16, square);
    CHECK(nka_custom_ease(a, 16) == 0);
    nka_destroy(a);
}

static void test_tweens(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash id = nka_id("box"), x = nka_id("x");
    float v;

    nka_update(a, 0.016f);
    CHECK(nka_tween_float(a, id, x, 0, 1, linear, NKA_POLICY_CROSSFADE, 0) == 0);
    CHECK(a->chans.len == 0);
    CHECK(nka_tween_float(a, id, x, 10, 1, linear, NKA_POLICY_CROSSFADE, 0) == 0);
    CHECK(a->chans.len == 1 && nka_busy(a));
    nka_update(a, 0.25f);
    CHECK(NEAR(nka_tween_float(a, id, x, 10, 1, linear, NKA_POLICY_CROSSFADE, 0), 2.5f, 1e-4f));
    nka_update(a, 0.5f);
    CHECK(NEAR(nka_tween_float(a, id, x, 10, 1, linear, NKA_POLICY_CROSSFADE, 0), 7.5f, 1e-4f));
    CHECK(NEAR(nka_tween_float(a, id, x, 0, 1, linear, NKA_POLICY_CROSSFADE, 0), 7.5f, 1e-4f));
    nka_update(a, 0.5f);
    CHECK(NEAR(nka_tween_float(a, id, x, 0, 1, linear, NKA_POLICY_CROSSFADE, 0), 3.75f, 1e-4f));
    CHECK(nka_busy(a));
    nka_update(a, 0.6f);
    CHECK(nka_tween_float(a, id, x, 0, 1, linear, NKA_POLICY_CROSSFADE, 0) == 0);
    CHECK(!nka_busy(a));

    nka_set_lazy_init(a, 0);
    CHECK(nka_tween_float(a, id, nka_id("eager"), 3, 1, linear, NKA_POLICY_CROSSFADE, 3) == 3);
    CHECK(a->chans.len == 2 && !nka_busy(a));
    nka_set_lazy_init(a, 1);

    CHECK(nka_tween_float(a, id, nka_id("cut"), 5, 1, linear, NKA_POLICY_CUT, 0) == 5);
    CHECK(!nka_busy(a));

    v = nka_tween_float(a, id, nka_id("q"), 10, 1, linear, NKA_POLICY_QUEUE, 0);
    CHECK(v == 0);
    nka_update(a, 0.5f);
    CHECK(NEAR(nka_tween_float(a, id, nka_id("q"), 20, 1, linear, NKA_POLICY_QUEUE, 0), 5, 1e-4f));
    nka_update(a, 0.25f);
    CHECK(NEAR(nka_tween_float(a, id, nka_id("q"), 20, 1, linear, NKA_POLICY_QUEUE, 0), 7.5f, 1e-4f));
    nka_update(a, 0.25f);
    CHECK(NEAR(nka_tween_float(a, id, nka_id("q"), 20, 1, linear, NKA_POLICY_QUEUE, 0), 10, 1e-4f));
    nka_update(a, 0.5f);
    CHECK(NEAR(nka_tween_float(a, id, nka_id("q"), 20, 1, linear, NKA_POLICY_QUEUE, 0), 15, 1e-4f));

    CHECK(nka_tween_float(a, id, nka_id("add"), 5, 1, linear, NKA_POLICY_ADDITIVE, 0) == 0);
    nka_update(a, 1);
    CHECK(nka_tween_float(a, id, nka_id("add"), 5, 1, linear, NKA_POLICY_ADDITIVE, 0) == 5);
    nka_update(a, 1);
    CHECK(nka_tween_float(a, id, nka_id("add"), 5, 1, linear, NKA_POLICY_ADDITIVE, 0) == 5);
    CHECK(nka_tween_float(a, id, nka_id("add"), 3, 1, linear, NKA_POLICY_ADDITIVE, 0) == 5);
    nka_update(a, 1);
    CHECK(nka_tween_float(a, id, nka_id("add"), 3, 1, linear, NKA_POLICY_ADDITIVE, 0) == 8);

    CHECK(nka_tween_float(a, id, nka_id("mul"), 3, 1, linear, NKA_POLICY_MULTIPLY, 2) == 2);
    nka_update(a, 0.5f);
    CHECK(NEAR(nka_tween_float(a, id, nka_id("mul"), 3, 1, linear, NKA_POLICY_MULTIPLY, 2), 4, 1e-4f));
    nka_update(a, 0.5f);
    CHECK(nka_tween_float(a, id, nka_id("mul"), 3, 1, linear, NKA_POLICY_MULTIPLY, 2) == 6);

    nka_tween_float(a, id, nka_id("re"), 10, 1, linear, NKA_POLICY_CROSSFADE, 0);
    nka_update(a, 0.5f);
    CHECK(NEAR(nka_tween_float(a, id, nka_id("re"), 10, 1, linear, NKA_POLICY_CROSSFADE, 0), 5, 1e-4f));
    nka_rebase_float(a, id, nka_id("re"), 20);
    nka_update(a, 0.25f);
    CHECK(NEAR(nka_tween_float(a, id, nka_id("re"), 20, 1, linear, NKA_POLICY_CROSSFADE, 0), 12.5f, 1e-4f));
    nka_update(a, 0.25f);
    CHECK(nka_tween_float(a, id, nka_id("re"), 20, 1, linear, NKA_POLICY_CROSSFADE, 0) == 20);
    nka_rebase_float(a, id, nka_id("none"), 1);

    CHECK(nka_tween_int(a, id, nka_id("i"), 10, 1, linear, NKA_POLICY_CROSSFADE, 0) == 0);
    nka_update(a, 0.25f);
    CHECK(nka_tween_int(a, id, nka_id("i"), 10, 1, linear, NKA_POLICY_CROSSFADE, 0) == 3);
    nka_update(a, 0.04f);
    CHECK(nka_tween_int(a, id, nka_id("i"), 10, 1, linear, NKA_POLICY_CROSSFADE, 0) == 3);
    nka_update(a, 1);
    CHECK(nka_tween_int(a, id, nka_id("i"), 10, 1, linear, NKA_POLICY_CROSSFADE, 0) == 10);
    nka_rebase_int(a, id, nka_id("i"), -4);
    CHECK(nka_tween_int(a, id, nka_id("i"), -4, 1, linear, NKA_POLICY_CROSSFADE, 0) == -4);

    {
        struct nk_vec2 init = { 0, 0 }, to = { 100, 50 }, r;
        struct nka_vec4 z4 = { 0, 0, 0, 0 }, t4 = { 1, 2, 3, 4 }, r4;
        struct nka_ease_axes axes;
        axes.x = linear;
        axes.y = nka_ease(NKA_EASE_IN_QUAD);
        axes.z = linear;
        axes.w = linear;
        r = nka_tween_vec2(a, id, nka_id("v2"), to, 1, linear, NKA_POLICY_CROSSFADE, init);
        CHECK(r.x == 0 && r.y == 0);
        nka_tween_vec2_per_axis(a, id, nka_id("pa"), to, 1, axes, NKA_POLICY_CROSSFADE);
        r4 = nka_tween_vec4(a, id, nka_id("v4"), t4, 1, linear, NKA_POLICY_CROSSFADE, z4);
        CHECK(r4.w == 0);
        nka_update(a, 0.5f);
        r = nka_tween_vec2(a, id, nka_id("v2"), to, 1, linear, NKA_POLICY_CROSSFADE, init);
        CHECK(NEAR(r.x, 50, 1e-3f) && NEAR(r.y, 25, 1e-3f));
        r = nka_tween_vec2_per_axis(a, id, nka_id("pa"), to, 1, axes, NKA_POLICY_CROSSFADE);
        CHECK(NEAR(r.x, 50, 1e-3f) && NEAR(r.y, 12.5f, 1e-3f));
        r4 = nka_tween_vec4(a, id, nka_id("v4"), t4, 1, linear, NKA_POLICY_CROSSFADE, z4);
        CHECK(NEAR(r4.x, 0.5f, 1e-4f) && NEAR(r4.w, 2, 1e-4f));
        nka_rebase_vec2(a, id, nka_id("v2"), init);
        nka_rebase_vec4(a, id, nka_id("v4"), z4);
        nka_update(a, 0.5f);
        r = nka_tween_vec2(a, id, nka_id("v2"), init, 1, linear, NKA_POLICY_CROSSFADE, init);
        CHECK(r.x == 0 && r.y == 0);
        r4 = nka_tween_vec4(a, id, nka_id("v4"), z4, 1, linear, NKA_POLICY_CROSSFADE, z4);
        CHECK(r4.x == 0 && r4.w == 0);
    }
    {
        struct nk_vec2 half = { 0.5f, 0.5f }, bias = { 10, 0 }, size = { 200, 100 }, r;
        nka_set_anchor(a, NKA_ANCHOR_WINDOW, size);
        CHECK(nka_anchor(a, NKA_ANCHOR_WINDOW).x == 200 && nka_anchor(a, 9).x == 0);
        r = nka_tween_vec2_rel(a, id, nka_id("rel"), half, bias, 1, linear, NKA_POLICY_CUT, NKA_ANCHOR_WINDOW);
        CHECK(r.x == 110 && r.y == 50);
        CHECK(nka_tween_float_rel(a, id, nka_id("relf"), 0.25f, 1, 1, linear, NKA_POLICY_CUT, NKA_ANCHOR_WINDOW, 1) == 26);
    }
    nka_destroy(a);
}

static void test_progress(void)
{
    struct nka_context *a = nka_create(0);
    struct nka_ease quad = nka_ease(NKA_EASE_IN_QUAD);
    struct nk_vec2 zero = { 0, 0 }, far = { 10, 10 };
    nk_hash id = nka_id("p");
    CHECK(nka_tween_progress(a, id, 1) == -1 && nka_tween_progress(0, id, 1) == -1);
    nka_tween_float(a, id, 1, 10, 1, linear, NKA_POLICY_CROSSFADE, 0);
    CHECK(nka_tween_progress(a, id, 1) == 0);
    nka_update(a, 0.25f);
    nka_tween_float(a, id, 1, 10, 1, linear, NKA_POLICY_CROSSFADE, 0);
    CHECK(NEAR(nka_tween_progress(a, id, 1), 0.25f, 1e-5f));
    nka_update(a, 1);
    CHECK(nka_tween_float(a, id, 1, 10, 1, linear, NKA_POLICY_CROSSFADE, 0) == 10);
    CHECK(nka_tween_progress(a, id, 1) == -1);

    nka_update(a, 0.1f);
    CHECK(nka_tween_float(a, id, 1, 10, 0.5f, quad, NKA_POLICY_CROSSFADE, 0) == 10);
    CHECK(!nka_busy(a) && nka_tween_progress(a, id, 1) == -1);
    CHECK(nka_tween_float(a, id, 1, 20, 1, quad, NKA_POLICY_CROSSFADE, 0) == 10);
    nka_update(a, 0.5f);
    CHECK(NEAR(nka_tween_float(a, id, 1, 20, 1, quad, NKA_POLICY_CROSSFADE, 0), 12.5f, 1e-4f));

    nka_tween_vec2(a, id, 2, far, 2, linear, NKA_POLICY_CROSSFADE, zero);
    nka_update(a, 0.5f);
    nka_tween_vec2(a, id, 2, far, 2, linear, NKA_POLICY_CROSSFADE, zero);
    CHECK(NEAR(nka_tween_progress(a, id, 2), 0.25f, 1e-5f));
    nka_destroy(a);
}

static float forty_two(void *user) { (void)user; return 42; }

static void test_time(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash id = nka_id("t");
    nka_tween_float(a, id, 0, 1, 1, linear, NKA_POLICY_CROSSFADE, 0);
    nka_set_time_scale(a, 0.5f);
    CHECK(nka_time_scale(a) == 0.5f);
    nka_update(a, 1);
    CHECK(nka_time(a) == 0.5);
    CHECK(NEAR(nka_tween_float(a, id, 0, 1, 1, linear, NKA_POLICY_CROSSFADE, 0), 0.5f, 1e-5f));
    nka_set_time_scale(a, -1);
    CHECK(nka_time_scale(a) == 0);
    nka_update(a, 1);
    CHECK(nka_time(a) == 0.5);
    CHECK(nka_tween_float_resolved(a, id, 1, forty_two, 0, 1, linear, NKA_POLICY_CUT) == 42);
    nka_destroy(a);
}

static void test_colors(void)
{
    struct nka_context *a = nka_create(0);
    struct nk_colorf black = { 0, 0, 0, 1 }, white = { 1, 1, 1, 1 }, red = { 1, 0, 0, 1 }, pink = { 1, 0, 0.6f, 1 }, c;
    int space, r, g, b;
    for (space = NKA_COL_SRGB; space <= NKA_COL_OKLCH; ++space)
        for (r = 0; r <= 4; ++r)
            for (g = 0; g <= 4; ++g)
                for (b = 0; b <= 4; ++b) {
                    struct nka_vec4 in = { (float)r / 4, (float)g / 4, (float)b / 4, 0.5f };
                    struct nka_vec4 out = nka__from_space(nka__to_space(in, space), space);
                    CHECK(NEAR(in.x, out.x, 1e-4f) && NEAR(in.y, out.y, 1e-4f) && NEAR(in.z, out.z, 1e-4f) && out.w == 0.5f);
                }
    c = nka_color_blend(red, pink, 0.5f, NKA_COL_HSV);
    CHECK(NEAR(c.r, 1, 1e-4f) && NEAR(c.g, 0, 1e-4f) && NEAR(c.b, 0.3f, 1e-4f));
    for (space = NKA_COL_SRGB; space <= NKA_COL_OKLCH; ++space) {
        c = nka_color_blend(red, pink, 0, space);
        CHECK(NEAR(c.r, 1, 1e-4f) && NEAR(c.b, 0, 1e-4f));
        c = nka_color_blend(red, pink, 1, space);
        CHECK(NEAR(c.r, 1, 1e-4f) && NEAR(c.b, 0.6f, 1e-4f));
    }
    c = nka_color_blend(black, white, 0.5f, NKA_COL_SRGB_LINEAR);
    CHECK(NEAR(c.r, 0.7354f, 1e-3f));

    c = nka_tween_color(a, 1, 2, white, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB, black);
    CHECK(c.r == 0 && c.a == 1);
    nka_update(a, 0.5f);
    c = nka_tween_color(a, 1, 2, white, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB, black);
    CHECK(c.r > 0.3f && c.r < 0.6f && NEAR(c.r, c.g, 1e-4f));
    nka_update(a, 0.5f);
    c = nka_tween_color(a, 1, 2, white, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB, black);
    CHECK(c.r == 1 && c.g == 1 && c.b == 1);
    nka_rebase_color(a, 1, 2, red);
    nka_update(a, 0.1f);
    c = nka_tween_color(a, 1, 2, red, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB, black);
    CHECK(c.r == 1 && c.g == 0);
    {
        struct nka_ease_axes axes;
        axes.x = axes.y = axes.z = axes.w = linear;
        c = nka_tween_color_per_axis(a, 1, 3, red, 0, axes, NKA_POLICY_CUT, NKA_COL_OKLCH);
        CHECK(NEAR(c.r, 1, 1e-4f) && NEAR(c.g, 0, 1e-4f));
    }
    nka_destroy(a);
}

static void test_animate(void)
{
    struct nka_context *a = nka_create(0);
    float size = 1, other = 0.5f;
    nka_update(a, 0);
    nka_animate(a, 1, 1.1f, 500, &size, NKA_EASE_IN_CUBIC);
    CHECK(size == 1 && nka_busy(a));
    nka_update(a, 0.25f);
    nka_animate(a, 1, 1.1f, 500, &size, NKA_EASE_IN_CUBIC);
    CHECK(NEAR(size, 1.0125f, 1e-5f));
    nka_animate(a, 1.1f, 1, 500, &size, NKA_EASE_OUT_CUBIC);
    CHECK(NEAR(size, 1.0125f, 1e-5f));
    nka_update(a, 0.125f);
    nka_animate(a, 1.1f, 1, 500, &size, NKA_EASE_OUT_CUBIC);
    CHECK(NEAR(size, 1.1f - 0.1f * (1 - 0.25f * 0.25f * 0.25f), 1e-5f));
    nka_update(a, 0.125f);
    nka_animate(a, 1.1f, 1, 500, &size, NKA_EASE_OUT_CUBIC);
    CHECK(size == 1);
    nka_update(a, 0.1f);
    nka_animate(a, 1.1f, 1, 500, &size, NKA_EASE_OUT_CUBIC);
    CHECK(size == 1 && !nka_busy(a));

    nka_animate(a, 0, 1, 1000, &other, NKA_EASE_LINEAR);
    CHECK(NEAR(other, 0.5f, 1e-6f));
    nka_update(a, 0.25f);
    nka_animate(a, 0, 1, 1000, &other, NKA_EASE_LINEAR);
    CHECK(NEAR(other, 0.75f, 1e-5f));
    other = 3;
    nka_animate(a, 0, 1, 1000, &other, NKA_EASE_LINEAR);
    CHECK(other == 1);
    other = 0.2f;
    nka_animate(a, 0, 1, 0, &other, NKA_EASE_LINEAR);
    CHECK(other == 1);
    nka_destroy(a);
}

static void test_gc_and_map(void)
{
    struct nka_context *a = nka_create(0);
    int i, frame;
    for (i = 0; i < 1000; ++i) nka_tween_float(a, (nk_hash)i, 0, 1, 1, linear, NKA_POLICY_CUT, 0);
    CHECK(a->chans.len == 1000);
    for (frame = 0; frame < 10; ++frame) {
        nka_update(a, 0.016f);
        for (i = 0; i < 1000; i += 2) nka_tween_float(a, (nk_hash)i, 0, 1, 1, linear, NKA_POLICY_CUT, 0);
    }
    nka_gc(a, 5);
    CHECK(a->chans.len == 500);
    for (i = 0; i < 1000; ++i) {
        struct nka__chan *c = (struct nka__chan *)nka__get(&a->chans, nka__key((nk_hash)i, 0, NKA__FLOAT));
        CHECK((c != 0) == !(i & 1));
        if (c) CHECK(c->cur[0] == 1 && c->id == (nk_hash)i);
    }
    nka_clear(a);
    CHECK(a->chans.len == 0 && a->chans.cap == 0);
    nka_reserve(a, 100);
    CHECK(a->chans.cap == 256);

    {
        static unsigned char in[40000];
        struct nka__map m = { 0, 0, 0, 0, sizeof(int) };
        nk_hash s = 7;
        for (i = 0; i < 40000; ++i) {
            int *v = (int *)nka__put(a, &m, (nk_hash)i * 2654435761u, 0);
            *v = i;
            in[i] = 1;
        }
        for (i = 0; i < 60000; ++i) {
            int k;
            s = s * 1103515245u + 12345u;
            k = (int)((s >> 8) % 40000);
            nka__del(&m, (nk_hash)k * 2654435761u);
            in[k] = 0;
        }
        frame = 0;
        for (i = 0; i < 40000; ++i) {
            int *v = (int *)nka__get(&m, (nk_hash)i * 2654435761u);
            frame += in[i];
            CHECK((v != 0) == (in[i] != 0));
            if (v) CHECK(*v == i);
        }
        CHECK(m.len == frame);
        nka__map_free(a, &m);
        *(int *)nka__put(a, &m, 0, 0) = 10;
        *(int *)nka__put(a, &m, 1, 0) = 11;
        CHECK(m.len == 2 && *(int *)nka__get(&m, 0) == 10 && *(int *)nka__get(&m, 1) == 11);
        nka__del(&m, 0);
        CHECK(!nka__get(&m, 0) && *(int *)nka__get(&m, 1) == 11);
        nka__map_free(a, &m);
    }
    CHECK(nka_id("") == 2166136261u && nka_id("a") == 0xe40c292cu && nka_id(0) == 2166136261u);
    nka_destroy(a);
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

static void test_allocator(void)
{
    struct nk_allocator al;
    struct nka_context *a;
    float v = 0;
    int i;
    al.userdata.ptr = 0;
    al.alloc = count_alloc;
    al.free = count_free;
    a = nka_create(&al);
    for (i = 0; i < 300; ++i) nka_tween_float(a, (nk_hash)i, 1, 1, 1, linear, NKA_POLICY_CROSSFADE, 0);
    nka_animate(a, 0, 1, 100, &v, NKA_EASE_LINEAR);
    CHECK(live > 1);
    nka_destroy(a);
    CHECK(live == 0);
    CHECK(nka_tween_float(0, 1, 1, 3, 1, linear, NKA_POLICY_CROSSFADE, 0) == 3);
    nka_update(0, 1);
    nka_destroy(0);
}

int main(void)
{
    test_easing();
    test_tweens();
    test_progress();
    test_time();
    test_colors();
    test_animate();
    test_gc_and_map();
    test_allocator();
    if (failures) printf("%d failed\n", failures);
    else printf("core: ok\n");
    return failures != 0;
}
