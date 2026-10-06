#include <stdio.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"
#define NUKANIM_IMPLEMENTATION
#include "nukanim.h"

static int failures;

static void check(int ok, const char *what, int line)
{
    if (ok) return;
    printf("fx.c:%d: %s\n", line, what);
    failures++;
}

#define CHECK(c) check((c) != 0, #c, __LINE__)
#define NEAR(x, y, eps) (fabsf((float)(x) - (float)(y)) <= (eps))

static const struct nka_ease linear = { NKA_EASE_LINEAR, 0, 0, 0, 0 };
static const float tau = 6.2831853f;

static struct nk_colorf rgba(float r, float g, float b, float a)
{
    struct nk_colorf c;
    c.r = r; c.g = g; c.b = b; c.a = a;
    return c;
}

static struct nk_colorf average(struct nk_colorf x, struct nk_colorf y)
{
    return rgba((x.r + y.r) / 2, (x.g + y.g) / 2, (x.b + y.b) / 2, (x.a + y.a) / 2);
}

static int same_color(struct nk_colorf x, struct nk_colorf y, float eps)
{
    return NEAR(x.r, y.r, eps) && NEAR(x.g, y.g, eps) && NEAR(x.b, y.b, eps) && NEAR(x.a, y.a, eps);
}

static int unit_color(struct nk_colorf c)
{
    return c.r >= 0 && c.r <= 1 && c.g >= 0 && c.g <= 1 && c.b >= 0 && c.b <= 1 && c.a >= 0 && c.a <= 1;
}

static int same_vec2(struct nk_vec2 x, struct nk_vec2 y, float eps) { return NEAR(x.x, y.x, eps) && NEAR(x.y, y.y, eps); }

static int same_gradient(const struct nka_gradient *x, const struct nka_gradient *y)
{
    int i;
    if (x->count != y->count) return 0;
    for (i = 0; i < x->count; ++i)
        if (x->positions[i] != y->positions[i] || !same_color(x->colors[i], y->colors[i], 0)) return 0;
    return 1;
}

static struct nka_transform xf(float x, float y, float rotation, float sx, float sy)
{
    return nka_transform(nka__vec2(x, y), rotation, nka__vec2(sx, sy));
}

static int same_xf(struct nka_transform x, struct nka_transform y)
{
    return x.position.x == y.position.x && x.position.y == y.position.y && x.rotation == y.rotation
        && x.scale.x == y.scale.x && x.scale.y == y.scale.y;
}

static void test_waves(void)
{
    static const float phase[5] = { 0, 0.25f, 0.5f, 0.75f, 0.125f };
    static const float shape[4][5] = {
        { 0, 1, 0, -1, 0.70710678f },
        { -1, 0, 1, 0, -0.5f },
        { -1, -0.5f, 0, 0.5f, -0.75f },
        { 1, 1, -1, -1, 1 }
    };
    struct nka_context *a = nka_create(0);
    struct nk_vec2 amp2 = { 2, 3 }, f2 = { 1, 2 }, p2 = { 0.25f, 0.125f }, v2;
    struct nka_vec4 amp4 = { 1, 2, 3, 4 }, f4 = { 1, 1, 1, 1 }, p4 = { 0, 0.25f, 0.5f, 0.75f }, v4;
    int wave, k;

    for (wave = NKA_WAVE_SINE; wave <= NKA_WAVE_SQUARE; ++wave)
        for (k = 0; k < 5; ++k) {
            CHECK(NEAR(nka_oscillate(a, 1, 3, 1, wave, phase[k]), 3 * shape[wave][k], 1e-5f));
            CHECK(NEAR(nka_oscillate(a, 1, 3, 1, wave, phase[k] + 2), 3 * shape[wave][k], 1e-5f));
            CHECK(NEAR(nka_oscillate(a, 1, 3, 1, wave, phase[k] - 1), 3 * shape[wave][k], 1e-5f));
        }
    CHECK(nka_busy(a));
    v2 = nka_oscillate_vec2(a, 2, amp2, f2, NKA_WAVE_SINE, p2);
    CHECK(NEAR(v2.x, 2, 1e-5f) && NEAR(v2.y, 3 * 0.70710678f, 1e-5f));
    v4 = nka_oscillate_vec4(a, 3, amp4, f4, NKA_WAVE_TRIANGLE, p4);
    CHECK(NEAR(v4.x, -1, 1e-5f) && NEAR(v4.y, 0, 1e-5f) && NEAR(v4.z, 3, 1e-5f) && NEAR(v4.w, 0, 1e-5f));
    CHECK(nka_oscillate_int(a, 4, 5, 1, NKA_WAVE_SAWTOOTH, 0.75f) == 3);
    CHECK(nka_oscillate_int(a, 4, 5, 1, NKA_WAVE_SAWTOOTH, 0.25f) == -3);
    CHECK(nka_oscillate_int(a, 4, 5, 1, NKA_WAVE_SAWTOOTH, 0.5f) == 0);
    CHECK(nka_oscillate_int(a, 4, 5, 1, NKA_WAVE_SINE, 0.25f) == 5);
    nka_destroy(a);

    for (wave = NKA_WAVE_SINE; wave <= NKA_WAVE_SQUARE; ++wave) {
        a = nka_create(0);
        for (k = 1; k <= 8; ++k) {
            nka_update(a, 0.125f);
            CHECK(NEAR(nka_oscillate(a, 7, 2, 2, wave, 0), 2 * shape[wave][k % 4], 1e-5f));
        }
        nka_destroy(a);
    }

    a = nka_create(0);
    nka_update(a, 0.1f);
    v2 = nka_oscillate_vec2(a, 2, amp2, f2, NKA_WAVE_SINE, p2);
    CHECK(NEAR(v2.x, 2 * sinf(tau * (0.1f + 0.25f)), 1e-5f) && NEAR(v2.y, 3 * sinf(tau * (0.2f + 0.125f)), 1e-5f));
    nka_destroy(a);

    for (wave = NKA_WAVE_SINE; wave <= NKA_WAVE_SQUARE; ++wave) {
        struct nk_vec2 f2b = { 1.3f, 0.7f }, p2b = { 0.1f, 0.6f };
        struct nka_vec4 f4b = { 0.9f, 1.7f, 2.3f, 0.4f }, p4b = { 0, 0.3f, 0.5f, 0.9f };
        float lo = 0, hi = 0;
        int out = 0;
        a = nka_create(0);
        for (k = 0; k < 400; ++k) {
            float v;
            int iv;
            nka_update(a, 0.0137f);
            v = nka_oscillate(a, 1, 4, 1.1f, wave, 0);
            iv = nka_oscillate_int(a, 2, 7, 0.9f, wave, 0.2f);
            v2 = nka_oscillate_vec2(a, 3, amp2, f2b, wave, p2b);
            v4 = nka_oscillate_vec4(a, 4, amp4, f4b, wave, p4b);
            out += fabsf(v) > 4 + 1e-5f || iv < -7 || iv > 7 || !nka_busy(a);
            out += fabsf(v2.x) > 2 + 1e-5f || fabsf(v2.y) > 3 + 1e-5f;
            out += fabsf(v4.x) > 1 + 1e-5f || fabsf(v4.y) > 2 + 1e-5f || fabsf(v4.z) > 3 + 1e-5f || fabsf(v4.w) > 4 + 1e-5f;
            if (v < lo) lo = v;
            if (v > hi) hi = v;
        }
        CHECK(out == 0 && lo < -3.8f && hi > 3.8f);
        nka_destroy(a);
    }

    {
        struct nk_colorf gray = rgba(0.5f, 0.5f, 0.5f, 1), red = rgba(1, 0, 0, 1), sky = rgba(0.2f, 0.5f, 1, 1);
        struct nk_colorf beige = rgba(0.6f, 0.5f, 0.4f, 0.8f), c;
        struct nka_vec4 d = { 0.25f, 0.25f, 0.25f, 0 }, hue = { 0.5f, 0, 0, 0 }, dim = { 0, 0, -0.5f, 0 };
        struct nka_vec4 turn = { 0, 0, 0.5f, 0 }, big = { 2, 2, 2, 2 }, lch, rot;
        int space, out = 0;
        a = nka_create(0);
        c = nka_oscillate_color(a, 5, gray, d, 1, NKA_WAVE_SINE, 0.25f, NKA_COL_SRGB);
        CHECK(same_color(c, rgba(0.75f, 0.75f, 0.75f, 1), 1e-5f));
        c = nka_oscillate_color(a, 5, red, hue, 1, NKA_WAVE_SINE, 0.25f, NKA_COL_HSV);
        CHECK(same_color(c, rgba(0, 1, 1, 1), 1e-4f));
        c = nka_oscillate_color(a, 5, sky, dim, 1, NKA_WAVE_SINE, 0.25f, NKA_COL_HSV);
        CHECK(same_color(c, rgba(0.1f, 0.25f, 0.5f, 1), 1e-4f));
        c = nka_oscillate_color(a, 5, beige, turn, 1, NKA_WAVE_SINE, 0.25f, NKA_COL_OKLCH);
        lch = nka__to_space(nka__cv(beige), NKA_COL_OKLCH);
        rot = nka__to_space(nka__cv(c), NKA_COL_OKLCH);
        CHECK(NEAR(rot.x, lch.x, 1e-3f) && NEAR(rot.y, lch.y, 1e-3f) && NEAR(fmodf(rot.z - lch.z + 1, 1), 0.5f, 1e-3f));
        CHECK(c.a == 0.8f);
        for (space = NKA_COL_SRGB; space <= NKA_COL_OKLCH; ++space)
            for (k = 0; k < 100; ++k) {
                nka_update(a, 0.05f);
                out += !unit_color(nka_oscillate_color(a, 6, sky, big, 1.3f, k & 3, 0, space));
            }
        CHECK(out == 0);
        nka_destroy(a);
    }
}

static void test_frames(void)
{
    struct nka_context *a = nka_create(0);
    struct nk_vec2 one = { 1, 1 }, half = { 0.5f, 0.5f }, zero = { 0, 0 };
    struct nka__fx_clock *c;
    float v;
    int k;

    nka_update(a, 0.25f);
    v = nka_oscillate(a, 9, 1, 0.5f, NKA_WAVE_SAWTOOTH, 0);
    CHECK(v == -0.75f && v == nka_oscillate(a, 9, 1, 0.5f, NKA_WAVE_SAWTOOTH, 0));
    CHECK(nka_oscillate_vec2(a, 9, one, half, NKA_WAVE_SAWTOOTH, zero).x == v);
    nka_update(a, 0.25f);
    CHECK(nka_oscillate(a, 9, 1, 0.5f, NKA_WAVE_SAWTOOTH, 0) == -0.5f);
    c = (struct nka__fx_clock *)nka__get(&a->fx_clocks, nka_id_mix(9, (nk_hash)NKA__FX_OSC));
    CHECK(c && c->time == 0.5);
    if (c) c->time = 86400;
    for (k = 0; k < 15; ++k) {
        nka_update(a, 1.0f / 60);
        v = nka_oscillate(a, 9, 1, 4, NKA_WAVE_SAWTOOTH, 0.5f);
    }
    CHECK(NEAR(v, 0, 1e-4f));

    nka_update(a, 0.25f);
    v = nka_wiggle(a, 9, 10, 3);
    CHECK(v == nka_wiggle(a, 9, 10, 3));
    v = nka_noise_channel_float(a, 9, 1.3f, 10, 0);
    CHECK(v == nka_noise_channel_float(a, 9, 1.3f, 10, 0));
    nka_smooth_noise_float(a, 9, 10, 1.3f);
    c = (struct nka__fx_clock *)nka__get(&a->fx_clocks, nka_id_mix(9, (nk_hash)NKA__FX_WIGGLE));
    CHECK(c && c->time == 0.25);
    c = (struct nka__fx_clock *)nka__get(&a->fx_clocks, nka_id_mix(9, (nk_hash)NKA__FX_NOISE));
    CHECK(c && c->time == 0.25);

    nka_trigger_shake(a, 9);
    nka_update(a, 0.25f);
    v = nka_shake(a, 9, 10, 20, 1);
    CHECK(v != 0 && v == nka_shake(a, 9, 10, 20, 1));
    c = (struct nka__fx_clock *)nka__get(&a->fx_clocks, nka_id_mix(9, (nk_hash)NKA__FX_SHAKE));
    CHECK(c && c->since == 0.25);
    nka_destroy(a);
}

static void test_shake(void)
{
    struct nka_context *a = nka_create(0);
    const float dt = 1.0f / 64;
    struct nk_vec2 i2 = { 10, 10 }, v2;
    struct nka_vec4 i4 = { 1, 2, 3, 4 }, ci = { 0.3f, 0.3f, 0.3f, 0 }, v4;
    struct nk_colorf gray = rgba(0.5f, 0.5f, 0.5f, 1), c = gray;
    int frame, moved = 0, out = 0, axes[4] = { 0, 0, 0, 0 }, k;
    float v;

    nka_update(a, dt);
    CHECK(nka_shake(a, 1, 10, 20, 0.5f) == 0 && !nka_busy(a));
    nka_trigger_shake(a, 1);
    for (frame = 1; frame <= 40; ++frame) {
        float env = 1 - (float)frame * dt / 0.5f;
        nka_update(a, dt);
        v = nka_shake(a, 1, 10, 20, 0.5f);
        if (env > 0) {
            out += fabsf(v) > 10 * env * env + 1e-4f || !nka_busy(a);
            moved += fabsf(v) > 0.5f;
        } else {
            out += v != 0 || nka_busy(a);
        }
    }
    CHECK(out == 0 && moved > 5);
    nka_trigger_shake(a, 1);
    nka_update(a, dt);
    v = nka_shake(a, 1, 10, 20, 0.5f);
    CHECK(v != 0 && nka_busy(a));

    moved = 0;
    nka_trigger_shake(a, 2);
    for (frame = 1; frame <= 40; ++frame) {
        nka_update(a, dt);
        v2 = nka_shake_vec2(a, 2, i2, 20, 0.5f);
        v4 = nka_shake_vec4(a, 2, i4, 20, 0.5f);
        k = nka_shake_int(a, 2, 10, 20, 0.5f);
        c = nka_shake_color(a, 2, gray, ci, 20, 0.5f, NKA_COL_OKLAB);
        moved += fabsf(v2.y) > 0.5f;
        axes[0] += fabsf(v4.x) > 0.05f;
        axes[1] += fabsf(v4.y) > 0.1f;
        axes[2] += fabsf(v4.z) > 0.15f;
        axes[3] += fabsf(v4.w) > 0.2f;
        out += fabsf(v2.x) > 10 || fabsf(v2.y) > 10 || k < -10 || k > 10 || !unit_color(c);
        out += fabsf(v4.x) > 1 || fabsf(v4.y) > 2 || fabsf(v4.z) > 3 || fabsf(v4.w) > 4;
    }
    CHECK(out == 0 && moved > 5 && axes[0] > 5 && axes[1] > 5 && axes[2] > 5 && axes[3] > 5);
    CHECK(same_color(c, gray, 1e-4f));
    nka_trigger_shake(a, 2);
    nka_update(a, dt);
    v2 = nka_shake_vec2(a, 2, i2, 20, 0.5f);
    CHECK(v2.x != 0 && v2.x == nka_shake(a, 2, 10, 20, 0.5f));

    for (frame = 0; frame < 4; ++frame) {
        nka_update(a, dt);
        c = nka_shake_color(a, 3, gray, ci, 20, 0.5f, NKA_COL_SRGB);
    }
    CHECK(same_color(c, gray, 0) && !nka_busy(a));
    nka_trigger_shake(a, 4);
    nka_update(a, dt);
    CHECK(nka_shake(a, 4, 10, 20, 0) == 0 && !nka_busy(a));
    nka_trigger_shake(a, 5);
    nka_update(a, dt);
    v = nka_shake(a, 5, 10, 0, 1);
    CHECK(v == v && fabsf(v) <= 10);
    nka_destroy(a);
}

static void test_wiggle(void)
{
    struct nka_context *a = nka_create(0);
    const float dt = 1.0f / 60;
    struct nk_vec2 amp2 = { 3, 7 }, v2;
    struct nka_vec4 amp4 = { 1, 2, 3, 4 }, v4;
    struct nk_colorf sky = rgba(0.2f, 0.5f, 1, 1);
    float prev = 0, v;
    int frame, changed = 0, out = 0, k, space;
    for (frame = 0; frame < 600; ++frame) {
        nka_update(a, dt);
        v = nka_wiggle(a, 1, 10, 3);
        out += fabsf(v) > 10 || !nka_busy(a);
        if (frame) {
            out += fabsf(v - prev) > 10 * 2 * 1.5f * 3 * dt + 1e-4f;
            changed += fabsf(v - prev) > 0.01f;
        }
        prev = v;
        v2 = nka_wiggle_vec2(a, 2, amp2, 2);
        v4 = nka_wiggle_vec4(a, 3, amp4, 5);
        k = nka_wiggle_int(a, 4, 6, 1);
        out += fabsf(v2.x) > 3 || fabsf(v2.y) > 7 || k < -6 || k > 6;
        out += fabsf(v4.x) > 1 || fabsf(v4.y) > 2 || fabsf(v4.z) > 3 || fabsf(v4.w) > 4;
        for (space = NKA_COL_SRGB; space <= NKA_COL_OKLCH; ++space)
            out += !unit_color(nka_wiggle_color(a, (nk_hash)(5 + space), sky, amp4, 2, space));
    }
    CHECK(out == 0 && changed > 200);
    nka_destroy(a);
}

static float roughness(int type, int octaves, int dims)
{
    struct nka_noise_opts o = nka_noise_opts_default();
    float sum = 0, h = 0.02f;
    int i;
    o.type = type;
    o.octaves = octaves;
    for (i = 1; i < 400; ++i) {
        float x = (float)i * 0.0371f, y = 0.37f;
        if (dims == 3)
            sum += fabsf(nka_noise_3d(x + h, y, 0.5f, &o) - 2 * nka_noise_3d(x, y, 0.5f, &o) + nka_noise_3d(x - h, y, 0.5f, &o));
        else
            sum += fabsf(nka_noise_2d(x + h, y, &o) - 2 * nka_noise_2d(x, y, &o) + nka_noise_2d(x - h, y, &o));
    }
    return sum;
}

static void test_noise(void)
{
    struct nka_noise_opts o = nka_noise_opts_default();
    int type, octaves, i, j;
    float v;

    CHECK(o.type == NKA_NOISE_PERLIN && o.octaves == 4 && o.persistence == 0.5f && o.lacunarity == 2 && o.seed == 0);
    CHECK(nka_noise_2d(1.3f, 2.7f, 0) == nka_noise_2d(1.3f, 2.7f, &o));
    CHECK(nka_noise_3d(1.3f, 2.7f, 0.1f, 0) == nka_noise_3d(1.3f, 2.7f, 0.1f, &o));
    for (type = NKA_NOISE_PERLIN; type <= NKA_NOISE_WORLEY; ++type) {
        int differs2 = 0, differs3 = 0, same = 1;
        for (octaves = 1; octaves <= 6; octaves += 5) {
            float lo2 = 2, hi2 = -2, lo3 = 2, hi3 = -2;
            o.type = type;
            o.octaves = octaves;
            o.seed = 3;
            for (i = 0; i < 64; ++i)
                for (j = 0; j < 64; ++j) {
                    float x = (float)i * 0.173f - 5, y = (float)j * 0.191f - 6, z = (float)((i * 7 + j) % 13) * 0.37f;
                    float n2 = nka_noise_2d(x, y, &o), n3 = nka_noise_3d(x, y, z, &o);
                    if (n2 < lo2) lo2 = n2;
                    if (n2 > hi2) hi2 = n2;
                    if (n3 < lo3) lo3 = n3;
                    if (n3 > hi3) hi3 = n3;
                }
            CHECK(lo2 >= -1 && hi2 <= 1 && hi2 - lo2 > 0.4f);
            CHECK(lo3 >= -1 && hi3 <= 1 && hi3 - lo3 > 0.4f);
        }
        o.octaves = 3;
        for (i = 0; i < 50; ++i) {
            float x = (float)i * 0.731f - 9, y = (float)i * 0.317f + 2, z = (float)i * 0.113f;
            o.seed = 7;
            v = nka_noise_2d(x, y, &o);
            same &= v == nka_noise_2d(x, y, &o);
            o.seed = 8;
            differs2 += v != nka_noise_2d(x, y, &o);
            o.seed = 7;
            v = nka_noise_3d(x, y, z, &o);
            same &= v == nka_noise_3d(x, y, z, &o);
            o.seed = 8;
            differs3 += v != nka_noise_3d(x, y, z, &o);
        }
        CHECK(same && differs2 > 40 && differs3 > 40);
        CHECK(roughness(type, 4, 2) > 1.5f * roughness(type, 1, 2));
        CHECK(roughness(type, 4, 3) > 1.5f * roughness(type, 1, 3));
    }

    for (type = NKA_NOISE_SIMPLEX; type <= NKA_NOISE_WORLEY; ++type) {
        struct nka_noise_opts perlin = nka_noise_opts_default();
        int differs = 0;
        o = perlin;
        o.type = type;
        for (i = 0; i < 50; ++i) {
            float x = (float)i * 0.531f - 7, y = (float)i * 0.277f + 1, z = (float)i * 0.193f - 3;
            differs += nka_noise_3d(x, y, z, &o) != nka_noise_3d(x, y, z, &perlin);
        }
        CHECK(differs > 40);
    }

    o = nka_noise_opts_default();
    o.octaves = 0;
    v = nka_noise_2d(0.3f, 0.4f, &o);
    o.octaves = 1;
    CHECK(v == nka_noise_2d(0.3f, 0.4f, &o));
    {
        float first = nka_noise_2d(0.3f, 0.4f, &o), second = nka_noise_2d(0.6f, 0.8f, &o);
        o.octaves = 2;
        o.persistence = -1;
        v = nka_noise_2d(0.3f, 0.4f, &o);
        CHECK(NEAR(v, (first - second) / 2, 1e-6f));
    }

    {
        struct nka_context *a = nka_create(0);
        struct nk_vec2 f2 = { 0.5f, 0.7f }, a2 = { 30, 20 }, v2;
        struct nka_vec4 f4 = { 0.3f, 0.4f, 0.5f, 0.6f }, a4 = { 1, 2, 3, 4 }, v4, big = { 1, 1, 1, 1 };
        struct nk_colorf sky = rgba(0.2f, 0.5f, 1, 1);
        float prev = 0, sprev = 0, s;
        int frame, changed = 0, schanged = 0, out = 0, space;
        o = nka_noise_opts_default();
        o.type = NKA_NOISE_SIMPLEX;
        for (frame = 1; frame <= 128; ++frame) {
            nka_update(a, 1.0f / 64);
            v = nka_noise_channel_float(a, 1, 1.5f, 5, &o);
            out += v != nka_noise_2d((float)frame / 64 * 1.5f, NKA__FX_ROW0, &o) * 5;
            out += fabsf(v) > 5 || !nka_busy(a);
            changed += v != prev;
            prev = v;
            v2 = nka_noise_channel_vec2(a, 2, f2, a2, &o);
            out += fabsf(v2.x) > 30 || fabsf(v2.y) > 20;
            v4 = nka_noise_channel_vec4(a, 3, f4, a4, &o);
            out += fabsf(v4.x) > 1 || fabsf(v4.y) > 2 || fabsf(v4.z) > 3 || fabsf(v4.w) > 4;
            s = nka_smooth_noise_float(a, 4, 10, 2);
            schanged += s != sprev;
            sprev = s;
            out += fabsf(s) > 10;
            v2 = nka_smooth_noise_vec2(a, 5, a2, 1);
            out += fabsf(v2.x) > 30 || fabsf(v2.y) > 20;
            v4 = nka_smooth_noise_vec4(a, 6, a4, 1);
            out += fabsf(v4.x) > 1 || fabsf(v4.y) > 2 || fabsf(v4.z) > 3 || fabsf(v4.w) > 4;
            for (space = NKA_COL_SRGB; space <= NKA_COL_OKLCH; ++space) {
                out += !unit_color(nka_noise_channel_color(a, (nk_hash)(7 + space), sky, big, 2, &o, space));
                out += !unit_color(nka_smooth_noise_color(a, (nk_hash)(20 + space), sky, big, 2, space));
            }
        }
        CHECK(out == 0 && changed > 100 && schanged > 100);
        nka_destroy(a);
    }
}

static void test_gradients(void)
{
    struct nk_colorf red = rgba(1, 0, 0, 1), green = rgba(0, 1, 0, 1), blue = rgba(0, 0, 1, 1);
    struct nk_colorf white = rgba(1, 1, 1, 1), black = rgba(0, 0, 0, 1);
    struct nk_color byte_red = { 255, 0, 0, 255 };
    struct nka_gradient g = { 0 }, h = { 0 }, x, y, r;
    int i, space, sorted = 1, inside = 1, same = 1;

    CHECK(nka_gradient_add(&g, 1, blue) && nka_gradient_add(&g, 0, red) && nka_gradient_add(&g, 0.5f, green));
    CHECK(g.count == 3 && g.positions[0] == 0 && g.positions[1] == 0.5f && g.positions[2] == 1);
    CHECK(same_color(g.colors[0], red, 0) && same_color(g.colors[1], green, 0) && same_color(g.colors[2], blue, 0));
    for (space = NKA_COL_SRGB; space <= NKA_COL_OKLCH; ++space) {
        CHECK(same_color(nka_gradient_sample(&g, 0, space), red, 0));
        CHECK(same_color(nka_gradient_sample(&g, 0.5f, space), green, 0));
        CHECK(same_color(nka_gradient_sample(&g, 1, space), blue, 0));
        CHECK(same_color(nka_gradient_sample(&g, -3, space), red, 0));
        CHECK(same_color(nka_gradient_sample(&g, 7, space), blue, 0));
        CHECK(same_color(nka_gradient_sample(&g, 0.25f, space), nka_color_blend(red, green, 0.5f, space), 1e-6f));
        CHECK(same_color(nka_gradient_sample(&g, 0.875f, space), nka_color_blend(green, blue, 0.75f, space), 1e-6f));
    }
    CHECK(same_color(nka_gradient_sample(&g, 0.25f, NKA_COL_SRGB), rgba(0.5f, 0.5f, 0, 1), 1e-6f));
    CHECK(!same_color(nka_gradient_sample(&g, 0.25f, NKA_COL_OKLAB), rgba(0.5f, 0.5f, 0, 1), 0.05f));

    CHECK(same_color(nka_gradient_sample(&h, 0.3f, NKA_COL_OKLAB), white, 0));
    CHECK(same_color(nka_gradient_sample(0, 0.3f, NKA_COL_OKLAB), white, 0));
    CHECK(nka_gradient_add(&h, 0.4f, red) && same_color(nka_gradient_sample(&h, 0.9f, NKA_COL_OKLAB), red, 0));
    CHECK(!nka_gradient_add(0, 0, red));

    h = nka_gradient_two_color(black, white);
    CHECK(nka_gradient_add(&h, 0.5f, red) && nka_gradient_add(&h, 0.5f, blue));
    CHECK(h.count == 4 && same_color(h.colors[1], red, 0) && same_color(h.colors[2], blue, 0));
    CHECK(same_color(nka_gradient_sample(&h, 0.5f, NKA_COL_SRGB), red, 0));
    CHECK(same_color(nka_gradient_sample(&h, 0.499f, NKA_COL_SRGB), red, 0.01f));
    CHECK(same_color(nka_gradient_sample(&h, 0.501f, NKA_COL_SRGB), blue, 0.01f));

    nka__zero(&x, sizeof x);
    for (i = 0; i < NKA_GRADIENT_MAX; ++i)
        CHECK(nka_gradient_add(&x, (float)(NKA_GRADIENT_MAX - 1 - i) / (NKA_GRADIENT_MAX - 1), i & 1 ? white : black));
    CHECK(!nka_gradient_add(&x, 0.5f, red) && x.count == NKA_GRADIENT_MAX);
    for (i = 1; i < x.count; ++i) sorted &= x.positions[i] > x.positions[i - 1];
    CHECK(sorted);

    nka__zero(&y, sizeof y);
    CHECK(nka_gradient_add_rgba(&y, 0.3f, byte_red) && same_color(y.colors[0], red, 0) && y.positions[0] == 0.3f);
    r = nka_gradient_solid(green);
    CHECK(r.count == 2 && r.positions[0] == 0 && r.positions[1] == 1);
    CHECK(same_color(r.colors[0], green, 0) && same_color(r.colors[1], green, 0));
    CHECK(same_color(nka_gradient_sample(&r, 0.37f, NKA_COL_OKLAB), green, 1e-4f));
    r = nka_gradient_two_color(red, blue);
    CHECK(r.count == 2 && same_color(r.colors[0], red, 0) && same_color(r.colors[1], blue, 0));
    r = nka_gradient_three_color(red, green, blue);
    CHECK(r.count == 3 && r.positions[1] == 0.5f && same_color(r.colors[1], green, 0) && same_color(r.colors[2], blue, 0));

    x = nka_gradient_two_color(red, blue);
    y = nka_gradient_three_color(black, white, black);
    for (space = NKA_COL_SRGB; space <= NKA_COL_OKLCH; ++space) {
        r = nka_gradient_lerp(&x, &y, 0, space);
        CHECK(r.count == 3);
        for (i = 0; i < 3; ++i) CHECK(same_color(r.colors[i], nka_gradient_sample(&x, r.positions[i], space), 0));
        r = nka_gradient_lerp(&x, &y, 1, space);
        for (i = 0; i < 3; ++i) CHECK(same_color(r.colors[i], nka_gradient_sample(&y, r.positions[i], space), 0));
    }
    r = nka_gradient_lerp(&x, &y, 0.5f, NKA_COL_SRGB);
    for (i = 0; i <= 20; ++i) {
        float t = (float)i / 20;
        same &= same_color(nka_gradient_sample(&r, t, NKA_COL_SRGB),
                           average(nka_gradient_sample(&x, t, NKA_COL_SRGB), nka_gradient_sample(&y, t, NKA_COL_SRGB)), 1e-6f);
    }
    CHECK(same && same_color(nka_gradient_sample(&r, 0.5f, NKA_COL_SRGB), rgba(0.75f, 0.5f, 0.75f, 1), 1e-6f));

    r = nka_gradient_lerp(&h, &h, 0.5f, NKA_COL_SRGB);
    CHECK(r.count == 4);
    for (i = 0; i <= 1000; ++i)
        same &= same_color(nka_gradient_sample(&r, (float)i / 1000, NKA_COL_SRGB), nka_gradient_sample(&h, (float)i / 1000, NKA_COL_SRGB), 1e-6f);
    CHECK(same && same_color(nka_gradient_sample(&r, 0.501f, NKA_COL_SRGB), blue, 0.01f));
    r = nka_gradient_lerp(&h, &g, 0.5f, NKA_COL_SRGB);
    CHECK(r.count == 4);
    for (i = 0; i <= 1000; ++i) {
        float t = (float)i / 1000;
        same &= same_color(nka_gradient_sample(&r, t, NKA_COL_SRGB),
                           average(nka_gradient_sample(&h, t, NKA_COL_SRGB), nka_gradient_sample(&g, t, NKA_COL_SRGB)), 1e-6f);
    }
    CHECK(same);

    nka__zero(&x, sizeof x);
    nka__zero(&y, sizeof y);
    for (i = 0; i < NKA_GRADIENT_MAX; ++i) {
        nka_gradient_add(&x, (float)i / (NKA_GRADIENT_MAX - 1), i & 1 ? red : blue);
        nka_gradient_add(&y, ((float)i + 0.5f) / NKA_GRADIENT_MAX, i & 1 ? green : black);
    }
    r = nka_gradient_lerp(&x, &y, 0.5f, NKA_COL_OKLAB);
    CHECK(r.count == NKA_GRADIENT_MAX && r.positions[0] == 0 && r.positions[NKA_GRADIENT_MAX - 1] == 1);
    for (i = 1; i < r.count; ++i) sorted &= r.positions[i] >= r.positions[i - 1];
    for (i = 0; i < r.count; ++i) inside &= unit_color(r.colors[i]);
    CHECK(sorted && inside);

    nka__zero(&y, sizeof y);
    nka_gradient_add(&y, 0, black);
    nka_gradient_add(&y, 1, white);
    for (i = 0; i < NKA_GRADIENT_MAX - 2; ++i) {
        float p = ((float)i + 0.5f) / (NKA_GRADIENT_MAX - 1);
        nka_gradient_add(&y, p, rgba(p, p, p, 1));
    }
    r = nka_gradient_lerp(&x, &y, 0.5f, NKA_COL_SRGB);
    CHECK(r.count == NKA_GRADIENT_MAX);
    for (i = 0; i <= 300; ++i) {
        float t = (float)i / 300;
        same &= same_color(nka_gradient_sample(&r, t, NKA_COL_SRGB),
                           average(nka_gradient_sample(&x, t, NKA_COL_SRGB), nka_gradient_sample(&y, t, NKA_COL_SRGB)), 1e-5f);
    }
    CHECK(same);
    r = nka_gradient_lerp(0, 0, 0.5f, NKA_COL_OKLAB);
    CHECK(r.count == 0);
}

static void test_gradient_tween(void)
{
    struct nka_context *a = nka_create(0);
    struct nka_gradient sunset = nka_gradient_three_color(rgba(1, 0.3f, 0, 1), rgba(1, 0.6f, 0.2f, 1), rgba(0.4f, 0.1f, 0.3f, 1));
    struct nka_gradient ocean = nka_gradient_two_color(rgba(0, 0.3f, 0.6f, 1), rgba(0, 0.9f, 0.9f, 1));
    struct nka_gradient forest = nka_gradient_solid(rgba(0.1f, 0.3f, 0.1f, 1)), r, r2, mid;
    int i, same = 1;

    nka_update(a, 0.125f);
    r = nka_tween_gradient(a, 1, 2, &sunset, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_SRGB);
    CHECK(same_gradient(&r, &sunset) && !nka_busy(a));
    nka_update(a, 0.125f);
    r = nka_tween_gradient(a, 1, 2, &ocean, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_SRGB);
    CHECK(nka_busy(a));
    for (i = 0; i <= 20; ++i)
        same &= same_color(nka_gradient_sample(&r, (float)i / 20, NKA_COL_SRGB), nka_gradient_sample(&sunset, (float)i / 20, NKA_COL_SRGB), 1e-6f);
    nka_update(a, 0.5f);
    r = nka_tween_gradient(a, 1, 2, &ocean, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_SRGB);
    r2 = nka_tween_gradient(a, 1, 2, &ocean, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_SRGB);
    CHECK(same_gradient(&r, &r2) && nka_busy(a));
    for (i = 0; i <= 20; ++i) {
        float t = (float)i / 20;
        same &= same_color(nka_gradient_sample(&r, t, NKA_COL_SRGB),
                           average(nka_gradient_sample(&sunset, t, NKA_COL_SRGB), nka_gradient_sample(&ocean, t, NKA_COL_SRGB)), 1e-6f);
    }
    CHECK(same);
    nka_update(a, 0.5f);
    r = nka_tween_gradient(a, 1, 2, &ocean, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_SRGB);
    CHECK(same_gradient(&r, &ocean) && !nka_busy(a));
    nka_update(a, 0.5f);
    r = nka_tween_gradient(a, 1, 2, &ocean, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_SRGB);
    CHECK(same_gradient(&r, &ocean) && !nka_busy(a));

    nka_update(a, 0.125f);
    nka_tween_gradient(a, 1, 2, &forest, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB);
    nka_update(a, 0.25f);
    mid = nka_tween_gradient(a, 1, 2, &forest, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB);
    r = nka_tween_gradient(a, 1, 2, &sunset, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB);
    for (i = 0; i <= 20; ++i)
        same &= same_color(nka_gradient_sample(&r, (float)i / 20, NKA_COL_OKLAB), nka_gradient_sample(&mid, (float)i / 20, NKA_COL_OKLAB), 1e-5f);
    CHECK(same && nka_busy(a));

    r = nka_tween_gradient(a, 1, 2, &ocean, 1, linear, NKA_POLICY_CUT, NKA_COL_OKLAB);
    CHECK(same_gradient(&r, &ocean));
    nka_update(a, 0.125f);
    r = nka_tween_gradient(a, 1, 2, &ocean, 1, linear, NKA_POLICY_CUT, NKA_COL_OKLAB);
    CHECK(same_gradient(&r, &ocean) && !nka_busy(a));
    nka_destroy(a);
}

static float turned(float from, float to, int mode, float t)
{
    return nka_transform_lerp(xf(0, 0, from, 1, 1), xf(0, 0, to, 1, 1), t, mode).rotation;
}

static int same_angle(float x, float y) { return NEAR(cosf(x), cosf(y), 1e-4f) && NEAR(sinf(x), sinf(y), 1e-4f); }

static void test_transforms(void)
{
    struct nka_transform id = nka_transform_identity(), t1 = xf(10, 20, 0.5f, 2, 2), t2 = xf(-3, 4, -1.2f, 0.5f, 0.5f), r;
    struct nka_transform ts[4];
    struct nk_vec2 p = { 7, -2 }, q;
    float m[6], m2[6], late = tau - 0.1f;
    int i, k, mode;

    CHECK(id.position.x == 0 && id.position.y == 0 && id.scale.x == 1 && id.scale.y == 1 && id.rotation == 0);
    q = nka_transform_apply(id, p);
    CHECK(q.x == 7 && q.y == -2);
    q = nka_transform_apply(xf(1, 2, 0, 3, 4), p);
    CHECK(q.x == 22 && q.y == -6);
    CHECK(same_vec2(nka_transform_apply(xf(0, 0, NKA__PI / 2, 1, 1), nka__vec2(1, 0)), nka__vec2(0, 1), 1e-6f));

    r = nka_transform_compose(t1, t2);
    CHECK(same_vec2(nka_transform_apply(r, p), nka_transform_apply(t1, nka_transform_apply(t2, p)), 1e-4f));
    CHECK(NEAR(r.rotation, -0.7f, 1e-6f) && r.scale.x == 1 && r.scale.y == 1);
    t1.scale = nka__vec2(2, 0.5f);
    r = nka_transform_compose(t1, t2);
    CHECK(same_vec2(r.position, nka_transform_apply(t1, t2.position), 1e-5f) && r.scale.x == 1 && r.scale.y == 0.25f);
    t1.scale = nka__vec2(2, 2);
    r = nka_transform_inverse(t1);
    CHECK(same_vec2(nka_transform_apply(r, nka_transform_apply(t1, p)), p, 1e-4f));
    CHECK(same_vec2(nka_transform_apply(t1, nka_transform_apply(r, p)), p, 1e-4f));
    t1.scale = nka__vec2(3, 0.25f);
    r = nka_transform_compose(nka_transform_inverse(t1), t1);
    CHECK(same_vec2(r.position, nka__vec2(0, 0), 1e-4f) && NEAR(r.rotation, 0, 1e-6f));
    CHECK(NEAR(r.scale.x, 1, 1e-6f) && NEAR(r.scale.y, 1, 1e-6f));
    r = nka_transform_inverse(xf(0, 0, 0, 0, 2));
    CHECK(r.scale.x == 1 && r.scale.y == 0.5f);

    CHECK(NEAR(turned(0.1f, late, NKA_ROTATION_SHORTEST, 1), -0.1f, 1e-5f));
    CHECK(NEAR(turned(0.1f, late, NKA_ROTATION_SHORTEST, 0.5f), 0, 1e-5f));
    CHECK(NEAR(turned(0.1f, late, NKA_ROTATION_LONGEST, 1), late, 1e-5f));
    CHECK(NEAR(turned(0.1f, late, NKA_ROTATION_LONGEST, 0.5f), NKA__PI, 1e-5f));
    CHECK(NEAR(turned(0.1f, late, NKA_ROTATION_CW, 1), late, 1e-5f));
    CHECK(NEAR(turned(0.1f, late, NKA_ROTATION_CCW, 1), -0.1f, 1e-5f));
    CHECK(NEAR(turned(0.1f, late, NKA_ROTATION_DIRECT, 0.5f), NKA__PI, 1e-5f));
    CHECK(NEAR(turned(late, 0.1f, NKA_ROTATION_SHORTEST, 1), tau + 0.1f, 1e-5f));
    CHECK(NEAR(turned(late, 0.1f, NKA_ROTATION_CW, 1), tau + 0.1f, 1e-5f));
    CHECK(NEAR(turned(late, 0.1f, NKA_ROTATION_CCW, 1), 0.1f, 1e-5f));
    CHECK(NEAR(turned(late, 0.1f, NKA_ROTATION_DIRECT, 1), 0.1f, 1e-5f));
    CHECK(NEAR(turned(3, -3, NKA_ROTATION_SHORTEST, 1), tau - 3, 1e-5f));
    CHECK(NEAR(turned(3, -3, NKA_ROTATION_SHORTEST, 0.5f), NKA__PI, 1e-5f));
    CHECK(NEAR(turned(3, -3, NKA_ROTATION_CW, 1), tau - 3, 1e-5f));
    CHECK(NEAR(turned(3, -3, NKA_ROTATION_CCW, 1), -3, 1e-5f));
    CHECK(NEAR(turned(3, -3, NKA_ROTATION_LONGEST, 1), -3, 1e-5f));
    CHECK(NEAR(turned(3, -3, NKA_ROTATION_DIRECT, 0.5f), 0, 1e-6f));
    CHECK(NEAR(turned(-3, 3, NKA_ROTATION_SHORTEST, 1), 3 - tau, 1e-5f));
    CHECK(NEAR(turned(-3, 3, NKA_ROTATION_SHORTEST, 0.5f), -NKA__PI, 1e-5f));
    CHECK(NEAR(turned(-3, 3, NKA_ROTATION_CW, 1), 3, 1e-5f));
    CHECK(NEAR(turned(-3, 3, NKA_ROTATION_CCW, 1), 3 - tau, 1e-5f));
    CHECK(NEAR(turned(-3, 3, NKA_ROTATION_LONGEST, 1), 3, 1e-5f));
    for (mode = NKA_ROTATION_SHORTEST; mode <= NKA_ROTATION_DIRECT; ++mode) {
        CHECK(same_angle(turned(3, -3, mode, 1), -3) && same_angle(turned(0.1f, late, mode, 1), late));
        CHECK(turned(1, 2, mode, 0) == 1);
    }
    for (k = 1; k <= 10; ++k) {
        CHECK(turned(3, -3, NKA_ROTATION_CW, (float)k / 10) > turned(3, -3, NKA_ROTATION_CW, (float)(k - 1) / 10));
        CHECK(turned(-3, 3, NKA_ROTATION_CCW, (float)k / 10) < turned(-3, 3, NKA_ROTATION_CCW, (float)(k - 1) / 10));
    }

    ts[0] = xf(5, -7, 0.7f, 2, 3);
    ts[1] = xf(1, 2, -2.5f, -1.5f, 0.5f);
    ts[2] = xf(0, 0, 3, 1, -2);
    ts[3] = id;
    for (i = 0; i < 4; ++i) {
        nka_transform_to_matrix(ts[i], m);
        r = nka_transform_from_matrix(m[0], m[1], m[3], m[4], m[2], m[5]);
        nka_transform_to_matrix(r, m2);
        for (k = 0; k < 6; ++k) CHECK(NEAR(m[k], m2[k], 1e-5f));
        CHECK(same_vec2(nka_transform_apply(r, p), nka_transform_apply(ts[i], p), 1e-4f));
    }
    nka_transform_to_matrix(ts[0], m);
    r = nka_transform_from_matrix(m[0], m[1], m[3], m[4], m[2], m[5]);
    CHECK(NEAR(r.rotation, 0.7f, 1e-6f) && NEAR(r.scale.x, 2, 1e-5f) && NEAR(r.scale.y, 3, 1e-5f));
    CHECK(r.position.x == 5 && r.position.y == -7);
    nka_transform_to_matrix(ts[1], m);
    r = nka_transform_from_matrix(m[0], m[1], m[3], m[4], m[2], m[5]);
    CHECK(NEAR(r.rotation, -2.5f, 1e-5f) && NEAR(r.scale.x, -1.5f, 1e-5f) && NEAR(r.scale.y, 0.5f, 1e-5f));
    r = nka_transform_from_matrix(0, -1, 1, 0, 4, 6);
    CHECK(NEAR(r.rotation, NKA__PI / 2, 1e-6f) && NEAR(r.scale.x, 1, 1e-6f) && NEAR(r.scale.y, 1, 1e-6f));
    nka_transform_to_matrix(id, 0);
}

static void test_transform_tween(void)
{
    struct nka_context *a = nka_create(0);
    struct nka_transform t0 = xf(10, 10, 3, 1, 1), t1 = xf(30, -10, -3, 2, 0.5f), r, r2;

    nka_update(a, 0.125f);
    r = nka_tween_transform(a, 1, 2, t0, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_SHORTEST);
    CHECK(same_xf(r, t0) && !nka_busy(a));
    nka_update(a, 0.125f);
    r = nka_tween_transform(a, 1, 2, t1, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_SHORTEST);
    CHECK(same_xf(r, t0) && nka_busy(a));
    nka_update(a, 0.5f);
    r = nka_tween_transform(a, 1, 2, t1, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_SHORTEST);
    r2 = nka_tween_transform(a, 1, 2, t1, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_SHORTEST);
    CHECK(same_xf(r, r2) && nka_busy(a));
    CHECK(NEAR(r.position.x, 20, 1e-4f) && NEAR(r.position.y, 0, 1e-4f) && NEAR(r.scale.x, 1.5f, 1e-5f) && NEAR(r.scale.y, 0.75f, 1e-5f));
    CHECK(NEAR(r.rotation, NKA__PI, 1e-4f));
    nka_update(a, 0.5f);
    r = nka_tween_transform(a, 1, 2, t1, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_SHORTEST);
    CHECK(same_xf(r, t1) && !nka_busy(a));
    nka_update(a, 0.5f);
    r = nka_tween_transform(a, 1, 2, t1, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_SHORTEST);
    CHECK(same_xf(r, t1) && !nka_busy(a));
    r = nka_tween_transform(a, 1, 2, t1, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_CW);
    CHECK(same_xf(r, t1) && !nka_busy(a));

    nka_update(a, 0.125f);
    nka_tween_transform(a, 1, 2, t0, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_CW);
    nka_update(a, 0.5f);
    r = nka_tween_transform(a, 1, 2, t0, 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_CW);
    CHECK(NEAR(r.rotation, 0, 1e-4f) && nka_busy(a));

    r = nka_tween_transform(a, 1, 2, t1, 1, linear, NKA_POLICY_CUT, NKA_ROTATION_SHORTEST);
    CHECK(same_xf(r, t1));
    nka_update(a, 0.125f);
    r = nka_tween_transform(a, 1, 2, t1, 1, linear, NKA_POLICY_CUT, NKA_ROTATION_SHORTEST);
    CHECK(same_xf(r, t1) && !nka_busy(a));
    nka_destroy(a);
}

static void test_drag(void)
{
    struct nka_context *a = nka_create(0);
    struct nka_drag_opts o = nka_drag_opts_default();
    struct nka_drag_feedback fb;
    struct nk_vec2 pts[3];
    int i, over = 0;

    CHECK(o.snap_grid.x == 0 && o.snap_grid.y == 0 && o.snap_points == 0 && o.snap_points_count == 0);
    CHECK(o.snap_duration == 0.2f && o.overshoot == 0 && o.ease_type == NKA_EASE_OUT_CUBIC);
    fb = nka_drag_update(a, 1, nka__vec2(5, 5));
    CHECK(!fb.is_dragging && fb.position.x == 5 && fb.velocity.x == 0);
    fb = nka_drag_release(a, 1, nka__vec2(5, 5), &o);
    CHECK(!fb.is_snapping && fb.snap_progress == 1 && fb.position.x == 5);

    nka_update(a, 0.125f);
    fb = nka_drag_begin(a, 1, nka__vec2(100, 100));
    CHECK(fb.is_dragging && !fb.is_snapping && fb.position.x == 100 && fb.offset.x == 0 && fb.velocity.x == 0);
    fb = nka_drag_update(a, 1, nka__vec2(100, 100));
    CHECK(fb.is_dragging && fb.velocity.x == 0 && fb.velocity.y == 0);
    nka_update(a, 0.125f);
    fb = nka_drag_update(a, 1, nka__vec2(110, 100));
    CHECK(fb.velocity.x == 80 && fb.velocity.y == 0 && fb.offset.x == 10 && fb.offset.y == 0);
    nka_update(a, 0.125f);
    fb = nka_drag_update(a, 1, nka__vec2(115, 102));
    CHECK(fb.velocity.x == 40 && fb.velocity.y == 16);
    fb = nka_drag_update(a, 1, nka__vec2(115, 102));
    CHECK(fb.velocity.x == 40 && fb.velocity.y == 16 && fb.is_dragging);

    o.snap_grid = nka__vec2(32, 32);
    o.snap_duration = 0.5f;
    o.ease_type = NKA_EASE_LINEAR;
    nka_update(a, 0.125f);
    fb = nka_drag_release(a, 1, nka__vec2(115, 102), &o);
    CHECK(!fb.is_dragging && fb.is_snapping && fb.snap_progress == 0 && nka_busy(a));
    CHECK(fb.position.x == 115 && fb.position.y == 102 && fb.velocity.x == 40);
    for (i = 1; i <= 4; ++i) {
        nka_update(a, 0.125f);
        fb = nka_drag_release(a, 1, fb.position, &o);
        if (i < 4)
            CHECK(fb.is_snapping && nka_busy(a) && fb.snap_progress == (float)i / 4
                  && NEAR(fb.position.x, 115 + 13 * (float)i / 4, 1e-4f) && NEAR(fb.position.y, 102 - 6 * (float)i / 4, 1e-4f));
    }
    CHECK(!fb.is_snapping && !nka_busy(a) && fb.snap_progress == 1 && fb.position.x == 128 && fb.position.y == 96);
    CHECK(fb.offset.x == 28 && fb.offset.y == -4);
    nka_update(a, 0.125f);
    fb = nka_drag_release(a, 1, fb.position, &o);
    CHECK(!fb.is_snapping && !nka_busy(a) && fb.position.x == 128 && fb.position.y == 96);

    pts[0] = nka__vec2(0, 0);
    pts[1] = nka__vec2(50, 50);
    pts[2] = nka__vec2(100, 0);
    o = nka_drag_opts_default();
    o.snap_grid = nka__vec2(32, 32);
    o.snap_points = pts;
    o.snap_points_count = 3;
    o.snap_duration = 0.25f;
    nka_update(a, 0.125f);
    nka_drag_begin(a, 2, nka__vec2(10, 10));
    nka_update(a, 0.125f);
    nka_drag_update(a, 2, nka__vec2(40, 45));
    nka_update(a, 0.125f);
    fb = nka_drag_release(a, 2, nka__vec2(40, 45), &o);
    CHECK(fb.is_snapping && nka_busy(a));
    nka_update(a, 0.125f);
    fb = nka_drag_release(a, 2, fb.position, &o);
    CHECK(fb.is_snapping && NEAR(fb.position.x, 48.75f, 1e-4f) && NEAR(fb.position.y, 49.375f, 1e-4f));
    nka_update(a, 0.125f);
    fb = nka_drag_release(a, 2, fb.position, &o);
    CHECK(!fb.is_snapping && fb.position.x == 50 && fb.position.y == 50 && !nka_busy(a));

    o = nka_drag_opts_default();
    pts[0] = nka__vec2(100, 0);
    o.snap_points = pts;
    o.snap_points_count = 1;
    o.snap_duration = 1;
    o.overshoot = 1;
    nka_update(a, 0.125f);
    nka_drag_begin(a, 3, nka__vec2(0, 0));
    fb = nka_drag_release(a, 3, nka__vec2(0, 0), &o);
    for (i = 1; i <= 8; ++i) {
        nka_update(a, 0.125f);
        fb = nka_drag_release(a, 3, fb.position, &o);
        if (i < 8) CHECK(NEAR(fb.position.x, 100 * nka_eval_preset(NKA_EASE_OUT_BACK, (float)i / 8), 2e-3f));
        over += fb.position.x > 100.5f;
    }
    CHECK(over > 0 && fb.position.x == 100 && !fb.is_snapping);
    o.overshoot = 0;
    over = 0;
    nka_drag_begin(a, 4, nka__vec2(0, 0));
    fb = nka_drag_release(a, 4, nka__vec2(0, 0), &o);
    for (i = 1; i <= 8; ++i) {
        nka_update(a, 0.125f);
        fb = nka_drag_release(a, 4, fb.position, &o);
        over += fb.position.x > 100;
    }
    CHECK(over == 0 && fb.position.x == 100);

    o.snap_duration = 0;
    nka_drag_begin(a, 5, nka__vec2(30, 0));
    fb = nka_drag_release(a, 5, nka__vec2(30, 0), &o);
    CHECK(!fb.is_snapping && fb.position.x == 100 && fb.snap_progress == 1);
    o.snap_duration = 1;
    nka_drag_begin(a, 6, nka__vec2(100, 0));
    nka_update(a, 0.125f);
    fb = nka_drag_release(a, 6, nka__vec2(100, 0), &o);
    CHECK(!fb.is_snapping && fb.position.x == 100 && !nka_busy(a));

    nka_update(a, 0.125f);
    nka_drag_begin(a, 7, nka__vec2(1, 1));
    nka_drag_cancel(a, 7);
    fb = nka_drag_update(a, 7, nka__vec2(2, 2));
    CHECK(!fb.is_dragging && !fb.is_snapping);
    o = nka_drag_opts_default();
    o.snap_grid = nka__vec2(32, 32);
    nka_drag_begin(a, 8, nka__vec2(10, 10));
    fb = nka_drag_release(a, 8, nka__vec2(10, 10), &o);
    CHECK(fb.is_snapping);
    nka_drag_cancel(a, 8);
    nka_update(a, 0.125f);
    fb = nka_drag_release(a, 8, nka__vec2(10, 10), 0);
    CHECK(!fb.is_snapping && fb.position.x == 10 && fb.position.y == 10 && !nka_busy(a));

    nka_drag_begin(a, 0, nka__vec2(1, 1));
    nka_drag_begin(a, 1, nka__vec2(9, 9));
    fb = nka_drag_update(a, 0, nka__vec2(2, 2));
    CHECK(fb.is_dragging && fb.offset.x == 1 && fb.offset.y == 1);
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

static void touch(struct nka_context *a, nk_hash id, const struct nka_gradient *g, int frame)
{
    nka_oscillate(a, id, 1, 1, NKA_WAVE_SINE, 0);
    nka_trigger_shake(a, id);
    nka_wiggle(a, id, 1, 1);
    nka_noise_channel_float(a, id, 1, 1, 0);
    nka_tween_gradient(a, id, 0, g, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB);
    nka_tween_transform(a, id, 0, nka_transform_identity(), 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_SHORTEST);
    if (frame) nka_drag_update(a, id, nka__vec2(1, 1));
    else nka_drag_begin(a, id, nka__vec2(0, 0));
}

static void test_memory(void)
{
    struct nk_allocator al;
    struct nka_context *a;
    struct nka_gradient g = nka_gradient_two_color(rgba(1, 0, 0, 1), rgba(0, 0, 1, 1)), r;
    struct nka_drag_feedback fb;
    int i, frame;

    al.userdata.ptr = 0;
    al.alloc = count_alloc;
    al.free = count_free;
    a = nka_create(&al);
    nka_update(a, 0.1f);
    for (i = 0; i < 40; ++i) touch(a, (nk_hash)i, &g, 0);
    CHECK(a->fx_clocks.len == 160 && a->fx_grads.len == 40 && a->fx_xforms.len == 40 && a->fx_drags.len == 40);
    for (frame = 1; frame <= 10; ++frame) {
        nka_update(a, 0.1f);
        for (i = 0; i < 40; i += 2) touch(a, (nk_hash)i, &g, frame);
    }
    nka_gc(a, 5);
    CHECK(a->fx_clocks.len == 80 && a->fx_grads.len == 20 && a->fx_xforms.len == 20 && a->fx_drags.len == 20);
    CHECK(live > 1);
    nka_clear(a);
    CHECK(!a->fx_clocks.cap && !a->fx_grads.cap && !a->fx_xforms.cap && !a->fx_drags.cap);
    touch(a, 1, &g, 0);
    CHECK(a->fx_clocks.len == 4 && a->fx_drags.len == 1);
    nka_destroy(a);
    CHECK(live == 0);

    CHECK(nka_oscillate(0, 1, 2, 1, NKA_WAVE_SINE, 0.25f) == 2);
    CHECK(nka_shake(0, 1, 2, 1, 1) == 0 && nka_wiggle(0, 1, 0, 1) == 0 && nka_noise_channel_float(0, 1, 1, 0, 0) == 0);
    nka_trigger_shake(0, 1);
    r = nka_tween_gradient(0, 1, 2, &g, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB);
    CHECK(same_gradient(&r, &g));
    r = nka_tween_gradient(0, 1, 2, 0, 1, linear, NKA_POLICY_CROSSFADE, NKA_COL_OKLAB);
    CHECK(r.count == 0);
    CHECK(nka_tween_transform(0, 1, 2, nka_transform_identity(), 1, linear, NKA_POLICY_CROSSFADE, NKA_ROTATION_SHORTEST).scale.x == 1);
    fb = nka_drag_begin(0, 1, nka__vec2(3, 4));
    CHECK(fb.position.x == 3 && fb.is_dragging);
    fb = nka_drag_update(0, 1, nka__vec2(3, 4));
    CHECK(!fb.is_dragging);
    fb = nka_drag_release(0, 1, nka__vec2(3, 4), 0);
    CHECK(fb.position.y == 4 && !fb.is_snapping);
    nka_drag_cancel(0, 1);
}

int main(void)
{
    test_waves();
    test_frames();
    test_shake();
    test_wiggle();
    test_noise();
    test_gradients();
    test_gradient_tween();
    test_transforms();
    test_transform_tween();
    test_drag();
    test_memory();
    if (failures) printf("%d failed\n", failures);
    else printf("fx: ok\n");
    return failures != 0;
}
