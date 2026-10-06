#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"
#define NUKANIM_IMPLEMENTATION
#include "nukanim.h"

static int failures;

static void check(int ok, const char *what, int line)
{
    if (ok) return;
    printf("clip.c:%d: %s\n", line, what);
    failures++;
}

#define CHECK(c) check((c) != 0, #c, __LINE__)
#define NEAR(x, y, eps) (fabsf((float)(x) - (float)(y)) <= (eps))

static nk_hash H(const char *s) { return nka_id(s); }

static struct nk_vec2 v2(float x, float y)
{
    struct nk_vec2 v;
    v.x = x; v.y = y;
    return v;
}

static struct nka_vec4 v4(float x, float y, float z, float w)
{
    struct nka_vec4 v;
    v.x = x; v.y = y; v.z = z; v.w = w;
    return v;
}

static struct nk_colorf rgba(float r, float g, float b, float al)
{
    struct nk_colorf c;
    c.r = r; c.g = g; c.b = b; c.a = al;
    return c;
}

static float getf(struct nka_context *a, nk_hash inst, const char *ch)
{
    float v = -12345;
    nka_instance_get_float(a, inst, H(ch), &v);
    return v;
}

static int geti(struct nka_context *a, nk_hash inst, const char *ch)
{
    int v = -12345;
    nka_instance_get_int(a, inst, H(ch), &v);
    return v;
}

static struct nka__clip_inst *inst_of(struct nka_context *a, nk_hash inst)
{
    return (struct nka__clip_inst *)nka__get(&a->clip_insts, inst);
}

static void key2(struct nka_context *a, const char *ch, float t0, float x0, float t1, float x1)
{
    nka_clip_key_float(a, H(ch), t0, x0, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H(ch), t1, x1, NKA_EASE_LINEAR, 0);
}

static void steps(struct nka_context *a, int n, float dt)
{
    while (n-- > 0) nka_update(a, dt);
}

/* Callback logs */

static char events[256];
static int nevents, loops_seen[64], nloops;

static void ev(char c)
{
    if (nevents < 255) events[nevents++] = c;
    events[nevents] = 0;
}

static void reset_events(void)
{
    nevents = 0;
    events[0] = 0;
    nloops = 0;
}

static void on_begin(struct nka_context *a, nk_hash inst, void *user) { (void)a; (void)inst; (void)user; ev('B'); }
static void on_update(struct nka_context *a, nk_hash inst, void *user) { (void)a; (void)inst; (void)user; ev('U'); }
static void on_complete(struct nka_context *a, nk_hash inst, void *user) { (void)a; (void)inst; (void)user; ev('C'); }
static void on_pause(struct nka_context *a, nk_hash inst, void *user) { (void)a; (void)inst; (void)user; ev('P'); }

static void on_loop(struct nka_context *a, nk_hash inst, int loop, void *user)
{
    (void)a; (void)inst; (void)user;
    ev('L');
    if (nloops < 64) loops_seen[nloops++] = loop;
}

static struct {
    nk_hash inst[64], id[64];
    float time[64];
    int n;
} marks;

static void on_mark(struct nka_context *a, nk_hash inst, nk_hash id, float time, void *user)
{
    (void)a; (void)user;
    if (marks.n >= 64) return;
    marks.inst[marks.n] = inst;
    marks.id[marks.n] = id;
    marks.time[marks.n] = time;
    marks.n++;
}

static int marks_are(nk_hash a0, nk_hash a1, nk_hash a2, nk_hash a3)
{
    nk_hash want[4];
    int i;
    want[0] = a0; want[1] = a1; want[2] = a2; want[3] = a3;
    for (i = 0; i < marks.n && i < 4; ++i)
        if (marks.id[i] != want[i]) return 0;
    return 1;
}

/* Keys and channels */

static void test_keys(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash clip = H("keys"), inst = H("k");
    const float bz[4] = { 0.25f, 0.1f, 0.25f, 1 };
    struct nk_colorf red = rgba(1, 0, 0, 1), blue = rgba(0, 0, 1, 0.5f), c, want;
    struct nk_vec2 p;
    struct nka_vec4 q;
    int k, s;

    nka_clip_begin(a, clip);
    nka_clip_key_float(a, H("f"), 2, 30, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("f"), 1, 10, NKA_EASE_OUT_QUAD, 0);
    nka_clip_key_float(a, H("f"), 0, 0, NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2(a, H("v2"), 0, v2(0, 0), NKA_EASE_IN_QUAD, 0);
    nka_clip_key_vec2(a, H("v2"), 2, v2(10, 20), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4(a, H("v4"), 0, v4(1, 2, 3, 4), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4(a, H("v4"), 2, v4(5, 6, 7, 8), NKA_EASE_LINEAR, 0);
    nka_clip_key_int(a, H("i"), 0, 10, NKA_EASE_LINEAR);
    nka_clip_key_int(a, H("i"), 2, 0, NKA_EASE_LINEAR);
    for (s = NKA_COL_SRGB; s <= NKA_COL_OKLCH; ++s) {
        nka_clip_key_color(a, nka_id_mix(H("c"), (nk_hash)s), 0, red, s, NKA_EASE_LINEAR, 0);
        nka_clip_key_color(a, nka_id_mix(H("c"), (nk_hash)s), 2, blue, s, NKA_EASE_LINEAR, 0);
    }
    nka_clip_key_float(a, H("bz"), 0, 0, NKA_EASE_CUBIC_BEZIER, bz);
    nka_clip_key_float(a, H("bz"), 2, 100, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("nobz"), 0, 0, NKA_EASE_LINEAR, bz);
    nka_clip_key_float(a, H("nobz"), 2, 100, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("late"), 1, 5, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("late"), 1.5f, 7, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("step"), 2, 3, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("step"), 1, 1, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("step"), 0, 0, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("step"), 1, 2, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("steps"), 0, 0, NKA_EASE_STEPS, 0);
    nka_clip_key_float(a, H("steps"), 2, 1, NKA_EASE_LINEAR, 0);
    nka_clip_key_int(a, H("big"), 0, -2147483647 - 1, NKA_EASE_LINEAR);
    nka_clip_key_int(a, H("big"), 2, 2147483647, NKA_EASE_LINEAR);
    nka_clip_key_int(a, H("near"), 0, 150, NKA_EASE_LINEAR);
    nka_clip_key_int(a, H("near"), 1, 2147483647, NKA_EASE_IN_BACK);
    nka_clip_key_int(a, H("near"), 2, 2147483000, NKA_EASE_LINEAR);
    CHECK(nka_clip_end(a) == NKA_OK);
    CHECK(nka_clip_exists(a, clip) && nka_clip_duration(a, clip) == 2);
    CHECK(nka_play(a, clip, inst) == inst && nka_instance_duration(a, inst) == 2);

    for (k = 0; k <= 16; ++k) {
        float t = (float)k / 8, u = t / 2;
        nka_instance_seek(a, inst, t);
        CHECK(nka_instance_time(a, inst) == t);
        CHECK(NEAR(getf(a, inst, "f"), t <= 1 ? 10 * t : 10 + 20 * nka_eval_preset(NKA_EASE_OUT_QUAD, t - 1), 1e-5f));
        CHECK(nka_instance_get_vec2(a, inst, H("v2"), &p));
        CHECK(NEAR(p.x, 10 * u * u, 1e-5f) && NEAR(p.y, 20 * u * u, 1e-5f));
        CHECK(nka_instance_get_vec4(a, inst, H("v4"), &q));
        CHECK(NEAR(q.x, 1 + 4 * u, 1e-5f) && NEAR(q.y, 2 + 4 * u, 1e-5f) && NEAR(q.z, 3 + 4 * u, 1e-5f) && NEAR(q.w, 4 + 4 * u, 1e-5f));
        CHECK(geti(a, inst, "i") == 10 + (int)floorf(-10 * u + 0.5f));
        for (s = NKA_COL_SRGB; s <= NKA_COL_OKLCH; ++s) {
            CHECK(nka_instance_get_color(a, inst, nka_id_mix(H("c"), (nk_hash)s), &c));
            want = nka_color_blend(red, blue, u, s);
            CHECK(NEAR(c.r, want.r, 1e-5f) && NEAR(c.g, want.g, 1e-5f) && NEAR(c.b, want.b, 1e-5f) && NEAR(c.a, want.a, 1e-5f));
        }
        CHECK(NEAR(getf(a, inst, "bz"), 100 * nka_eval(a, nka_ease_bezier(bz[0], bz[1], bz[2], bz[3]), u), 1e-4f));
        CHECK(NEAR(getf(a, inst, "nobz"), 100 * u, 1e-4f));
        CHECK(NEAR(getf(a, inst, "late"), t <= 1 ? 5 : (t >= 1.5f ? 7 : 5 + 4 * (t - 1)), 1e-5f));
        CHECK(NEAR(getf(a, inst, "step"), t <= 1 ? t : 1 + t, 1e-5f));
        CHECK(getf(a, inst, "steps") == (t < 2 ? 0 : 1));
    }
    /* Ints round half up on the way down too: upstream sat one above until the last key. */
    nka_instance_seek(a, inst, 1.9f);
    CHECK(geti(a, inst, "i") == 1);
    nka_instance_seek(a, inst, 1.96f);
    CHECK(geti(a, inst, "i") == 0);
    nka_instance_seek(a, inst, 0.5f);
    CHECK(geti(a, inst, "i") == 8);
    /* Ints of any size land exactly on their keys, and overshoot saturates. */
    nka_instance_seek(a, inst, 0);
    CHECK(geti(a, inst, "big") == -2147483647 - 1 && geti(a, inst, "near") == 150);
    nka_instance_seek(a, inst, 1);
    CHECK(geti(a, inst, "big") == 0 && geti(a, inst, "near") == 2147483647);
    nka_instance_seek(a, inst, 1.25f);
    CHECK(geti(a, inst, "near") == 2147483647);
    nka_instance_seek(a, inst, 1.95f);
    CHECK(geti(a, inst, "near") < 2147483647 - 400 && geti(a, inst, "near") > 2147483000);
    nka_instance_seek(a, inst, 2);
    CHECK(geti(a, inst, "big") == 2147483647 && geti(a, inst, "near") == 2147483000);

    nka_instance_restart(a, inst);
    CHECK(nka_instance_time(a, inst) == 0 && getf(a, inst, "f") == 0);
    nka_update(a, 0.5f);
    CHECK(getf(a, inst, "f") == 5);
    nka_update(a, 1);
    CHECK(NEAR(getf(a, inst, "f"), 25, 1e-5f));
    nka_update(a, 0.5f);
    CHECK(getf(a, inst, "f") == 30 && nka_instance_is_playing(a, inst));
    nka_update(a, 0.25f);
    CHECK(getf(a, inst, "f") == 30 && !nka_instance_is_playing(a, inst));

    /* A channel the clip lacks reads zero. Upstream said yes for floats and ints. */
    {
        float f = 5;
        int i = 5;
        CHECK(!nka_instance_get_float(a, inst, H("none"), &f) && f == 0);
        CHECK(!nka_instance_get_int(a, inst, H("none"), &i) && i == 0);
        CHECK(!nka_instance_get_color(a, inst, H("none"), &c) && c.r == 0 && c.a == 1);
        CHECK(!nka_instance_get_vec2(a, inst, H("f"), &p) && p.x == 0);
        CHECK(!nka_instance_get_float(a, inst, H("f"), 0));
        CHECK(!nka_instance_get_float(a, H("nobody"), H("f"), &f));
    }
    nka_destroy(a);
}

static void test_groups(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash inst = H("g");

    nka_clip_begin(a, H("seq"));
    nka_clip_seq_begin(a);
    key2(a, "x", 0, 0, 0.5f, 100);
    nka_clip_seq_end(a);
    nka_clip_seq_begin(a);
    key2(a, "y", 0, 0, 0.5f, 50);
    nka_clip_seq_end(a);
    nka_clip_seq_begin(a);
    key2(a, "s", 0, 1, 0.5f, 1.5f);
    nka_clip_marker(a, 0.25f, H("in"), on_mark, 0);
    nka_clip_seq_end(a);
    nka_clip_end(a);
    CHECK(nka_clip_duration(a, H("seq")) == 1.5f);
    nka_play(a, H("seq"), inst);
    nka_instance_seek(a, inst, 0.75f);
    CHECK(getf(a, inst, "x") == 100 && getf(a, inst, "y") == 25 && getf(a, inst, "s") == 1);
    nka_instance_seek(a, inst, 1.25f);
    CHECK(getf(a, inst, "s") == 1.25f);
    marks.n = 0;
    nka_instance_seek(a, inst, 1);
    nka_update(a, 0.5f);
    CHECK(marks.n == 1 && marks.time[0] == 1.25f);

    nka_clip_begin(a, H("par"));
    nka_clip_par_begin(a);
    key2(a, "x", 0, 0, 0.6f, 100);
    key2(a, "y", 0, 0, 0.6f, 50);
    nka_clip_par_end(a);
    key2(a, "a", 0, 0, 0.3f, 1);
    nka_clip_end(a);
    CHECK(NEAR(nka_clip_duration(a, H("par")), 0.9f, 1e-6f));
    nka_play(a, H("par"), inst);
    nka_instance_seek(a, inst, 0.75f);
    CHECK(NEAR(getf(a, inst, "a"), 0.5f, 1e-5f) && getf(a, inst, "x") == 100);

    /* A group that ends with a nested one keeps its keys: upstream went back to its start. */
    nka_clip_begin(a, H("nest"));
    nka_clip_seq_begin(a);
    nka_clip_par_begin(a);
    key2(a, "x", 0, 0, 0.3f, 30);
    nka_clip_par_end(a);
    nka_clip_seq_end(a);
    key2(a, "z", 0, 0, 0.2f, 20);
    nka_clip_end(a);
    CHECK(NEAR(nka_clip_duration(a, H("nest")), 0.5f, 1e-6f));
    nka_play(a, H("nest"), inst);
    nka_instance_seek(a, inst, 0.3f);
    CHECK(getf(a, inst, "z") == 0);
    nka_instance_seek(a, inst, 0.4f);
    CHECK(NEAR(getf(a, inst, "z"), 10, 1e-4f));

    nka_clip_begin(a, H("doc"));
    nka_clip_seq_begin(a);
    nka_clip_par_begin(a);
    key2(a, "x", 0, 0, 0.3f, 50);
    key2(a, "y", 0, 0, 0.3f, 50);
    nka_clip_par_end(a);
    key2(a, "s", 0, 1, 0.2f, 1.5f);
    nka_clip_seq_end(a);
    key2(a, "a", 0, 1, 0.3f, 0);
    nka_clip_end(a);
    CHECK(NEAR(nka_clip_duration(a, H("doc")), 0.8f, 1e-6f));
    nka_play(a, H("doc"), inst);
    nka_instance_seek(a, inst, 0.4f);
    CHECK(NEAR(getf(a, inst, "s"), 1.25f, 1e-5f) && getf(a, inst, "a") == 1);

    /* An unmatched end and keys outside any build are ignored. */
    nka_clip_seq_end(a);
    nka_clip_key_float(a, H("x"), 0, 1, NKA_EASE_LINEAR, 0);
    CHECK(nka_clip_end(a) == NKA_ERR_BAD_ARG);
    nka_destroy(a);
}

static void test_delay(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash clip = H("delayed"), inst = H("d");
    nka_clip_begin(a, clip);
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_delay(a, 1);
    nka_clip_on_begin(a, on_begin, 0);
    nka_clip_end(a);
    reset_events();
    nka_play(a, clip, inst);
    CHECK(getf(a, inst, "f") == 0 && nka_busy(a));
    nka_update(a, 0.5f);
    CHECK(getf(a, inst, "f") == 0 && nka_instance_time(a, inst) == 0 && nka_busy(a) && nevents == 0);
    nka_update(a, 0.75f);
    CHECK(strcmp(events, "B") == 0);
    CHECK(nka_instance_time(a, inst) == 0.25f && getf(a, inst, "f") == 2.5f);
    nka_update(a, 1);
    CHECK(!nka_instance_is_playing(a, inst) && getf(a, inst, "f") == 10);

    CHECK(nka_play_with_delay(a, clip, inst, 0.25f) == inst);
    nka_update(a, 0.5f);
    CHECK(nka_instance_time(a, inst) == 0.25f);
    nka_play_with_delay(a, clip, inst, -1);
    nka_update(a, 0.5f);
    CHECK(nka_instance_time(a, inst) == 0.5f);
    CHECK(nka_play_with_delay(a, H("nope"), inst, 1) == 0);
    nka_destroy(a);
}

static void test_loops(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash inst = H("l");

    nka_clip_begin(a, H("n2"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, 2);
    nka_clip_on_loop(a, on_loop, 0);
    nka_clip_on_complete(a, on_complete, 0);
    nka_clip_end(a);
    reset_events();
    nka_play(a, H("n2"), inst);
    steps(a, 4, 0.25f);
    CHECK(nka_instance_time(a, inst) == 1 && getf(a, inst, "f") == 10 && nloops == 0);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.25f && nloops == 1 && loops_seen[0] == 1);
    steps(a, 7, 0.25f);
    CHECK(nka_instance_is_playing(a, inst) && nloops == 2 && loops_seen[1] == 2 && nka_instance_time(a, inst) == 1);
    nka_update(a, 0.25f);
    CHECK(!nka_instance_is_playing(a, inst) && strcmp(events, "LLC") == 0 && getf(a, inst, "f") == 10);

    nka_clip_begin(a, H("inf"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, -1);
    nka_clip_end(a);
    nka_play(a, H("inf"), inst);
    steps(a, 400, 0.25f);
    CHECK(nka_instance_is_playing(a, inst) && inst_of(a, inst)->loop == 99);

    /* Reverse starts at the end; upstream finished on the first frame. */
    nka_clip_begin(a, H("rev"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 0, NKA_DIR_REVERSE, 0);
    nka_clip_end(a);
    nka_play(a, H("rev"), inst);
    CHECK(nka_instance_time(a, inst) == 1 && getf(a, inst, "f") == 10);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.75f && getf(a, inst, "f") == 7.5f && nka_instance_is_playing(a, inst));
    steps(a, 3, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0 && nka_instance_is_playing(a, inst));
    nka_update(a, 0.25f);
    CHECK(!nka_instance_is_playing(a, inst) && getf(a, inst, "f") == 0);

    nka_clip_begin(a, H("rev1"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_REVERSE, 1);
    nka_clip_on_loop(a, on_loop, 0);
    nka_clip_end(a);
    reset_events();
    nka_play(a, H("rev1"), inst);
    nka_update(a, 0.25f);
    CHECK(nloops == 0 && nka_instance_time(a, inst) == 0.75f);
    steps(a, 4, 0.25f);
    CHECK(nloops == 1 && loops_seen[0] == 1 && nka_instance_time(a, inst) == 0.75f);
    steps(a, 3, 0.25f);
    CHECK(nka_instance_is_playing(a, inst) && nka_instance_time(a, inst) == 0);
    nka_update(a, 0.25f);
    CHECK(!nka_instance_is_playing(a, inst) && nka_instance_time(a, inst) == 0);

    nka_clip_begin(a, H("alt"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_ALTERNATE, 2);
    nka_clip_end(a);
    nka_play(a, H("alt"), inst);
    steps(a, 5, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.75f && getf(a, inst, "f") == 7.5f);
    steps(a, 4, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.25f && inst_of(a, inst)->dir == 1);
    steps(a, 3, 0.25f);
    CHECK(nka_instance_time(a, inst) == 1 && nka_instance_is_playing(a, inst));
    nka_update(a, 0.25f);
    CHECK(!nka_instance_is_playing(a, inst) && nka_instance_time(a, inst) == 1 && getf(a, inst, "f") == 10);

    /* The pause between loops starts at the boundary and takes the rest of the frame. */
    nka_clip_begin(a, H("ld"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, 1);
    nka_clip_set_loop_delay(a, 0.5f);
    nka_clip_end(a);
    nka_play(a, H("ld"), inst);
    steps(a, 5, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0 && getf(a, inst, "f") == 0 && nka_busy(a));
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0 && nka_busy(a));
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.25f);
    nka_play(a, H("ld"), inst);
    steps(a, 3, 0.4f);
    CHECK(nka_instance_time(a, inst) == 0 && NEAR(inst_of(a, inst)->delay, 0.3f, 1e-6f));
    nka_update(a, 0.4f);
    CHECK(NEAR(nka_instance_time(a, inst), 0.1f, 1e-6f) && NEAR(getf(a, inst, "f"), 1, 1e-5f));

    /* Without loop, set_loop still sets the direction. */
    nka_clip_begin(a, H("once"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 0, NKA_DIR_ALTERNATE, 5);
    nka_clip_end(a);
    nka_play(a, H("once"), inst);
    steps(a, 5, 0.25f);
    CHECK(!nka_instance_is_playing(a, inst) && nka_instance_time(a, inst) == 1);
    nka_destroy(a);
}

static void test_markers(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash clip = H("marks"), inst = H("m"), m0, mhalf, mid, end = H("end");
    int i;

    nka_clip_begin(a, clip);
    key2(a, "f", 0, 0, 3, 1);
    m0 = nka_clip_marker(a, 0, 0, on_mark, 0);
    mhalf = nka_clip_marker(a, 0.5f, 0, on_mark, 0);
    mid = nka_clip_marker(a, 1.5f, H("mid"), on_mark, 0);
    nka_clip_marker(a, 3, end, on_mark, 0);
    nka_clip_end(a);
    CHECK(m0 && mhalf && m0 != mhalf && mid == H("mid"));
    CHECK(nka_clip_marker(a, 1, 0, on_mark, 0) == 0);

    marks.n = 0;
    nka_play(a, clip, inst);
    nka_update(a, 0.4f);
    CHECK(marks.n == 1 && marks.id[0] == m0 && marks.time[0] == 0 && marks.inst[0] == inst);
    steps(a, 7, 0.4f);
    CHECK(marks.n == 4 && marks_are(m0, mhalf, mid, end) && marks.time[3] == 3);
    CHECK(!nka_instance_is_playing(a, inst));

    nka_clip_begin(a, H("marks2"));
    key2(a, "f", 0, 0, 3, 1);
    nka_clip_marker(a, 0, m0, on_mark, 0);
    nka_clip_marker(a, 0.5f, mhalf, on_mark, 0);
    nka_clip_marker(a, 1.5f, mid, on_mark, 0);
    nka_clip_marker(a, 3, end, on_mark, 0);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, 1);
    nka_clip_end(a);
    marks.n = 0;
    nka_play(a, H("marks2"), inst);
    steps(a, 20, 0.4f);
    CHECK(marks.n == 8 && !nka_instance_is_playing(a, inst));
    for (i = 0; i < 8 && i < marks.n; ++i) CHECK(marks.id[i] == marks.id[i % 4] && marks.id[i] == (i % 4 == 0 ? m0 : i % 4 == 1 ? mhalf : i % 4 == 2 ? mid : end));

    /* A seek back fires the markers again; a seek ahead skips what it jumps over. */
    marks.n = 0;
    nka_play(a, clip, inst);
    steps(a, 5, 0.4f);
    CHECK(marks.n == 3);
    nka_instance_seek(a, inst, 1);
    steps(a, 3, 0.4f);
    CHECK(marks.n == 4 && marks.id[3] == mid);
    nka_instance_seek(a, inst, 0.2f);
    nka_instance_seek(a, inst, 2.6f);
    steps(a, 2, 0.4f);
    CHECK(marks.n == 5 && marks.id[4] == end && !nka_instance_is_playing(a, inst));

    marks.n = 0;
    nka_play(a, clip, inst);
    nka_instance_pause(a, inst);
    nka_instance_seek(a, inst, 1.5f);
    nka_update(a, 0.1f);
    CHECK(marks.n == 0);
    nka_instance_resume(a, inst);
    nka_update(a, 0.1f);
    CHECK(marks.n == 1 && marks.id[0] == mid);

    /* Played backward, markers fire in the order they are passed. */
    nka_clip_begin(a, H("rmarks"));
    key2(a, "f", 0, 0, 3, 1);
    nka_clip_marker(a, 0, H("a0"), on_mark, 0);
    nka_clip_marker(a, 0.5f, H("a1"), on_mark, 0);
    nka_clip_marker(a, 1.5f, H("a2"), on_mark, 0);
    nka_clip_set_loop(a, 0, NKA_DIR_REVERSE, 0);
    nka_clip_end(a);
    marks.n = 0;
    nka_play(a, H("rmarks"), inst);
    steps(a, 3, 1);
    CHECK(marks.n == 3 && marks_are(H("a2"), H("a1"), H("a0"), 0));

    /* Ping-pong fires a marker at a turn once, not as the end of one pass and the start of the next. */
    nka_clip_begin(a, H("amarks"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_marker(a, 0, H("lo"), on_mark, 0);
    nka_clip_marker(a, 1, H("hi"), on_mark, 0);
    nka_clip_set_loop(a, 1, NKA_DIR_ALTERNATE, 2);
    nka_clip_end(a);
    marks.n = 0;
    nka_play(a, H("amarks"), inst);
    steps(a, 20, 0.25f);
    CHECK(!nka_instance_is_playing(a, inst) && marks.n == 4 && marks_are(H("lo"), H("hi"), H("lo"), H("hi")));

    /* A marker past the keys makes the clip longer. */
    nka_clip_begin(a, H("long"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_marker(a, 2, H("tail"), 0, 0);
    nka_clip_end(a);
    CHECK(nka_clip_duration(a, H("long")) == 2);
    nka_destroy(a);
}

/* Callbacks that change things under the update */

static void destroy_self(struct nka_context *a, nk_hash inst, void *user)
{
    (void)user;
    nka_instance_destroy(a, inst);
    ev('D');
}

static void replay_self(struct nka_context *a, nk_hash inst, void *user)
{
    nka_play(a, *(const nk_hash *)user, inst);
    ev('R');
}

static void mark_destroy(struct nka_context *a, nk_hash inst, nk_hash id, float time, void *user)
{
    (void)id; (void)time; (void)user;
    nka_instance_destroy(a, inst);
}

static void rebuild(struct nka_context *a, nk_hash inst, nk_hash id, float time, void *user)
{
    (void)inst; (void)id; (void)time;
    nka_clip_begin(a, *(const nk_hash *)user);
    key2(a, "g", 0, 5, 1, 6);
    nka_clip_end(a);
}

static void clear_all(struct nka_context *a, nk_hash inst, void *user)
{
    (void)inst; (void)user;
    nka_clear(a);
}

static int nested;

static void update_inside(struct nka_context *a, nk_hash inst, void *user)
{
    (void)inst; (void)user;
    if (nested++) return;
    nka_update(a, 0.1f);
}

static nk_hash counted_ids[20];
static int counted[20], stray, spawned, killed;

static void count_update(struct nka_context *a, nk_hash inst, void *user)
{
    int i, at = -1;
    for (i = 0; i < 20; ++i)
        if (counted_ids[i] == inst) at = i;
    if (at < 0) {
        stray++;
        return;
    }
    counted[at]++;
    if (at == 7 && !spawned) {
        spawned = 1;
        for (i = 0; i < 200; ++i) nka_play(a, *(const nk_hash *)user, nka_id_mix(H("spawn"), (nk_hash)i));
    }
    if (at == 3 && !killed) {
        killed = 1;
        for (i = 10; i < 20; ++i) nka_instance_destroy(a, counted_ids[i]);
    }
}

static void test_callbacks(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash inst = H("c"), again = H("again"), rb = H("rb"), counted_clip = H("counted");
    float f;
    int i, ok;

    nka_clip_begin(a, H("cb"));
    key2(a, "f", 0, 0, 0.5f, 1);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, 1);
    nka_clip_on_begin(a, on_begin, 0);
    nka_clip_on_update(a, on_update, 0);
    nka_clip_on_complete(a, on_complete, 0);
    nka_clip_on_loop(a, on_loop, 0);
    nka_clip_on_pause(a, on_pause, 0);
    nka_clip_end(a);
    reset_events();
    nka_play(a, H("cb"), inst);
    CHECK(nevents == 0);
    steps(a, 6, 0.25f);
    CHECK(strcmp(events, "BUULUUC") == 0 && nloops == 1 && loops_seen[0] == 1);

    /* Pause tells once per pause; upstream told every call. */
    reset_events();
    nka_play(a, H("cb"), inst);
    nka_instance_pause(a, inst);
    nka_instance_pause(a, inst);
    CHECK(strcmp(events, "P") == 0 && nka_instance_is_paused(a, inst));
    nka_update(a, 0.25f);
    CHECK(strcmp(events, "P") == 0 && nka_instance_time(a, inst) == 0 && !nka_busy(a));
    nka_instance_resume(a, inst);
    CHECK(nka_busy(a) && !nka_instance_is_paused(a, inst));
    nka_update(a, 0.25f);
    CHECK(strcmp(events, "PBU") == 0);
    nka_instance_pause(a, inst);
    CHECK(strcmp(events, "PBUP") == 0);

    nka_clip_begin(a, H("selfdestruct"));
    key2(a, "f", 0, 0, 0.25f, 1);
    nka_clip_on_complete(a, destroy_self, 0);
    nka_clip_end(a);
    reset_events();
    nka_play(a, H("selfdestruct"), inst);
    nka_instance_then(a, inst, H("cb"), H("next"));
    nka_update(a, 0.5f);
    CHECK(strcmp(events, "D") == 0 && !nka_instance_valid(a, inst) && !nka_instance_valid(a, H("next")));

    nka_clip_begin(a, again);
    key2(a, "f", 0, 0, 0.25f, 1);
    nka_clip_on_complete(a, replay_self, &again);
    nka_clip_end(a);
    reset_events();
    nka_play(a, again, inst);
    nka_update(a, 0.5f);
    CHECK(strcmp(events, "R") == 0 && nka_instance_is_playing(a, inst) && nka_instance_time(a, inst) == 0);
    nka_update(a, 0.25f);
    CHECK(strcmp(events, "R") == 0 && nka_instance_time(a, inst) == 0.25f);

    nka_clip_begin(a, H("md"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_marker(a, 0.5f, 0, mark_destroy, 0);
    nka_clip_on_update(a, on_update, 0);
    nka_clip_end(a);
    reset_events();
    nka_play(a, H("md"), inst);
    nka_update(a, 0.25f);
    nka_update(a, 0.5f);
    CHECK(strcmp(events, "U") == 0 && !nka_instance_valid(a, inst));

    /* Authoring the playing clip again from its marker: the step stops, the next read catches up. */
    nka_clip_begin(a, rb);
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_marker(a, 0.5f, 0, rebuild, &rb);
    nka_clip_end(a);
    nka_play(a, rb, inst);
    nka_update(a, 0.75f);
    CHECK(NEAR(getf(a, inst, "g"), 5.75f, 1e-5f) && !nka_instance_get_float(a, inst, H("f"), &f) && f == 0);
    nka_update(a, 0.5f);
    CHECK(!nka_instance_is_playing(a, inst) && getf(a, inst, "g") == 6);

    /* Instances made and destroyed by callbacks during an update: each other one steps once. */
    nka_clip_begin(a, counted_clip);
    key2(a, "f", 0, 0, 10, 1);
    nka_clip_on_update(a, count_update, &counted_clip);
    nka_clip_end(a);
    for (i = 0; i < 20; ++i) {
        counted_ids[i] = nka_id_mix(H("counted"), (nk_hash)i);
        counted[i] = 0;
        nka_play(a, counted_clip, counted_ids[i]);
    }
    nka_update(a, 0.1f);
    ok = stray == 0 && spawned && killed;
    for (i = 0; i < 10; ++i) ok = ok && counted[i] == 1;
    for (i = 10; i < 20; ++i) ok = ok && counted[i] <= 1 && !nka_instance_valid(a, counted_ids[i]);
    CHECK(ok);
    CHECK(nka_instance_time(a, nka_id_mix(H("spawn"), 5)) == 0);
    nka_update(a, 0.1f);
    ok = stray == 200;
    for (i = 0; i < 10; ++i) ok = ok && counted[i] == 2;
    CHECK(ok);

    nested = 0;
    nka_clip_begin(a, H("nested"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_on_update(a, update_inside, 0);
    nka_clip_end(a);
    nka_play(a, H("nested"), H("n"));
    nka_update(a, 0.25f);
    CHECK(nested == 1 && nka_instance_time(a, H("n")) == 0.25f);

    nka_clip_begin(a, H("clr"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_on_update(a, clear_all, 0);
    nka_clip_end(a);
    for (i = 0; i < 3; ++i) nka_play(a, H("clr"), nka_id_mix(H("clr"), (nk_hash)i));
    nka_update(a, 0.25f);
    CHECK(a->clip_insts.len == 0 && nka_clip_exists(a, H("clr")));
    nka_destroy(a);
}

static void test_chain(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash A = H("ca"), B = H("cb"), ia = H("ia"), ib = H("ib"), made;
    nka_clip_begin(a, A);
    key2(a, "f", 0, 0, 0.5f, 1);
    nka_clip_end(a);
    nka_clip_begin(a, B);
    key2(a, "g", 0, 0, 0.5f, 1);
    nka_clip_end(a);

    nka_play(a, A, ia);
    CHECK(nka_instance_then(a, ia, B, ib) == ib);
    nka_instance_then_delay(a, ia, 0.25f);
    steps(a, 2, 0.25f);
    CHECK(!nka_instance_valid(a, ib) && nka_instance_time(a, ia) == 0.5f);
    nka_update(a, 0.25f);
    CHECK(!nka_instance_is_playing(a, ia) && nka_instance_is_playing(a, ib) && nka_instance_time(a, ib) == 0 && nka_busy(a));
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, ib) == 0 && nka_busy(a));
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, ib) == 0.25f && getf(a, ib, "g") == 0.5f);

    nka_play(a, A, ia);
    made = nka_instance_then(a, ia, B, 0);
    CHECK(made && made != ia && made != ib && !nka_instance_valid(a, made));
    steps(a, 3, 0.25f);
    CHECK(nka_instance_is_playing(a, made) && nka_instance_time(a, made) == 0);

    nka_play(a, A, ia);
    nka_instance_then(a, ia, B, H("ic"));
    nka_instance_destroy(a, ia);
    steps(a, 4, 0.25f);
    CHECK(!nka_instance_valid(a, H("ic")));

    nka_play(a, A, ia);
    nka_instance_then(a, ia, B, H("id"));
    nka_play(a, A, ia);
    steps(a, 4, 0.25f);
    CHECK(!nka_instance_valid(a, H("id")));

    nka_play(a, A, ia);
    nka_instance_then(a, ia, H("unknown"), H("ie"));
    steps(a, 4, 0.25f);
    CHECK(!nka_instance_valid(a, H("ie")));
    CHECK(nka_instance_then(a, H("nobody"), B, 0) == 0);
    nka_destroy(a);
}

static void test_control(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash inst = H("s");
    nka_clip_begin(a, H("sk"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_end(a);
    nka_clip_begin(a, H("rsk"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 0, NKA_DIR_REVERSE, 0);
    nka_clip_end(a);

    CHECK(nka_get_instance(a, inst) == 0);
    nka_play(a, H("sk"), inst);
    CHECK(nka_get_instance(a, inst) == inst);
    nka_update(a, 0.25f);
    nka_instance_pause(a, inst);
    nka_instance_seek(a, inst, 0.75f);
    CHECK(nka_instance_time(a, inst) == 0.75f && getf(a, inst, "f") == 7.5f);
    nka_instance_seek(a, inst, 5);
    CHECK(nka_instance_time(a, inst) == 1 && getf(a, inst, "f") == 10);
    nka_instance_seek(a, inst, -1);
    CHECK(nka_instance_time(a, inst) == 0 && getf(a, inst, "f") == 0);
    nka_instance_resume(a, inst);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.25f && getf(a, inst, "f") == 2.5f);

    nka_instance_stop(a, inst);
    CHECK(!nka_instance_is_playing(a, inst) && nka_instance_time(a, inst) == 0 && getf(a, inst, "f") == 2.5f);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0);

    nka_instance_set_time_scale(a, inst, 2);
    nka_instance_restart(a, inst);
    CHECK(nka_instance_is_playing(a, inst) && getf(a, inst, "f") == 0);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.5f);
    nka_instance_refresh(a, inst);
    CHECK(getf(a, inst, "f") == 5);

    nka_instance_reset(a, inst);
    CHECK(!nka_instance_is_playing(a, inst) && nka_instance_time(a, inst) == 0 && getf(a, inst, "f") == 0);

    /* Reset of a reversed clip goes to its start, the end; upstream said time 0 there. */
    nka_play(a, H("rsk"), inst);
    nka_update(a, 0.5f);
    nka_instance_reset(a, inst);
    CHECK(nka_instance_time(a, inst) == 1 && getf(a, inst, "f") == 10 && !nka_instance_is_playing(a, inst));

    /* Playing another clip on an instance drops the channels of the old one. */
    nka_clip_begin(a, H("other"));
    key2(a, "g", 0, 3, 1, 4);
    nka_clip_end(a);
    nka_play(a, H("other"), inst);
    CHECK(getf(a, inst, "g") == 3 && getf(a, inst, "f") == 0 && !nka_instance_get_float(a, inst, H("f"), 0));
    CHECK(nka_play(a, H("other"), 0) == 0 && nka_play(a, H("missing"), inst) == 0);

    nka_instance_destroy(a, inst);
    CHECK(!nka_instance_valid(a, inst) && nka_instance_time(a, inst) == 0 && !nka_instance_is_playing(a, inst));
    nka_instance_destroy(a, inst);
    nka_instance_seek(a, inst, 1);
    nka_instance_reset(a, inst);
    nka_instance_restart(a, inst);
    nka_instance_refresh(a, inst);
    nka_instance_pause(a, inst);
    nka_instance_resume(a, inst);
    nka_instance_stop(a, inst);
    CHECK(!nka_instance_valid(a, inst));
    nka_destroy(a);
}

static void test_time_scale(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash inst = H("t");
    nka_clip_begin(a, H("ts"));
    key2(a, "f", 0, 0, 4, 10);
    nka_clip_end(a);
    nka_play(a, H("ts"), inst);
    nka_instance_set_time_scale(a, inst, 2);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.5f);
    nka_instance_set_time_scale(a, inst, 0);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 0.75f);
    nka_instance_set_time_scale(a, inst, -3);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, inst) == 1);
    nka_instance_set_time_scale(a, inst, 1);
    nka_set_time_scale(a, 0.5f);
    nka_update(a, 0.5f);
    CHECK(nka_instance_time(a, inst) == 1.25f);
    nka_set_time_scale(a, 0);
    nka_update(a, 0.5f);
    CHECK(nka_instance_time(a, inst) == 1.25f && nka_busy(a));
    nka_set_time_scale(a, 1);
    nka_update(a, 3);
    CHECK(nka_instance_time(a, inst) == 2.25f);
    nka_destroy(a);
}

static void test_layers(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash ia = H("la"), ib = H("lb"), target = H("blend");
    struct nk_vec2 p;
    struct nka_vec4 q;
    float f;
    int i;
    nka_set_anchor(a, NKA_ANCHOR_WINDOW, v2(200, 100));
    nka_clip_begin(a, H("ca"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_key_vec2(a, H("v"), 0, v2(0, 0), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2(a, H("v"), 1, v2(10, 20), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4(a, H("q"), 0, v4(1, 2, 3, 4), NKA_EASE_LINEAR, 0);
    nka_clip_key_int(a, H("i"), 0, 0, NKA_EASE_LINEAR);
    nka_clip_key_int(a, H("i"), 1, 10, NKA_EASE_LINEAR);
    nka_clip_key_float(a, H("only"), 0, 42, NKA_EASE_LINEAR, 0);
    nka_clip_key_color(a, H("c"), 0, rgba(1, 0, 0, 1), NKA_COL_SRGB, NKA_EASE_LINEAR, 0);
    nka_clip_key_float_rel(a, H("x"), 0, 0.5f, 0, NKA_ANCHOR_WINDOW, 0, NKA_EASE_LINEAR, 0);
    nka_clip_end(a);
    nka_clip_begin(a, H("cb"));
    nka_clip_key_float(a, H("f"), 0, 20, NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2(a, H("v"), 0, v2(100, 100), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4(a, H("q"), 0, v4(5, 6, 7, 8), NKA_EASE_LINEAR, 0);
    nka_clip_key_int(a, H("i"), 0, 20, NKA_EASE_LINEAR);
    nka_clip_key_float(a, H("x"), 0, 300, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("f"), 1, 20, NKA_EASE_LINEAR, 0);
    nka_clip_end(a);
    nka_play(a, H("ca"), ia);
    nka_play(a, H("cb"), ib);
    nka_instance_seek(a, ia, 0.5f);

    /* The target need not be an instance; upstream wrote nowhere unless it was. */
    CHECK(!nka_get_blended_float(a, target, H("f"), &f));
    nka_layer_begin(a, target);
    nka_layer_add(a, ia, 1);
    nka_layer_add(a, ib, 3);
    nka_layer_end(a, target);
    CHECK(nka_get_blended_float(a, target, H("f"), &f) && f == 16.25f);
    CHECK(nka_get_blended_vec2(a, target, H("v"), &p) && p.x == 76.25f && p.y == 77.5f);
    CHECK(nka_get_blended_vec4(a, target, H("q"), &q) && q.x == 4 && q.w == 7);
    CHECK(nka_get_blended_int(a, target, H("i"), &i) && i == 16);
    CHECK(nka_get_blended_float(a, target, H("only"), &f) && f == 42);
    CHECK(nka_get_blended_float(a, target, H("x"), &f) && f == 250);
    CHECK(!nka_get_blended_float(a, target, H("c"), &f) && !nka_get_blended_vec4(a, target, H("c"), &q));
    CHECK(!nka_get_blended_float(a, target, H("v"), &f) && !nka_get_blended_float(a, H("other"), H("f"), &f));
    CHECK(!nka_get_blended_float(a, target, H("f"), 0));

    /* The instance's weight multiplies the layer's. */
    nka_instance_set_weight(a, ib, 0.5f);
    nka_layer_begin(a, target);
    nka_layer_add(a, ia, 1);
    nka_layer_add(a, ib, 2);
    nka_layer_add(a, ib, 0);
    nka_layer_add(a, H("nobody"), 1);
    nka_layer_end(a, target);
    CHECK(nka_get_blended_float(a, target, H("f"), &f) && f == 12.5f);
    CHECK(nka_get_blended_int(a, target, H("i"), &i) && i == 13);

    /* An end for another target is ignored; an end with no weight keeps what was there. */
    nka_layer_begin(a, target);
    nka_layer_add(a, ia, 1);
    nka_layer_end(a, H("wrong"));
    CHECK(nka_get_blended_float(a, target, H("f"), &f) && f == 12.5f && !nka_get_blended_float(a, H("wrong"), H("f"), &f));
    nka_layer_end(a, target);
    CHECK(nka_get_blended_float(a, target, H("f"), &f) && f == 5);
    nka_layer_begin(a, target);
    nka_layer_add(a, ia, 0);
    nka_layer_end(a, target);
    CHECK(nka_get_blended_float(a, target, H("f"), &f) && f == 5);
    nka_layer_add(a, ib, 1);
    nka_layer_end(a, target);
    CHECK(nka_get_blended_float(a, target, H("f"), &f) && f == 5);

    /* The instance itself can be the target. */
    nka_layer_begin(a, ia);
    nka_layer_add(a, ib, 1);
    nka_layer_end(a, ia);
    CHECK(nka_get_blended_float(a, ia, H("f"), &f) && f == 20);
    CHECK(getf(a, ia, "f") == 5);
    nka_destroy(a);
}

/* Variations */

static nk_uint xs(nk_uint *s)
{
    nk_uint x = *s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *s = x;
    return x;
}

static float unit_of(nk_uint *s) { return (float)(xs(s) & 0x7fffffffu) / (float)0x7fffffff; }

static float tens(int loop, void *user) { (void)user; return (float)loop * 10; }
static int hundreds(int loop, void *user) { (void)user; return loop * 100; }
static struct nk_vec2 pairs(int loop, void *user) { (void)user; return v2((float)loop, (float)-loop); }
static struct nka_vec4 grey(int loop, void *user) { (void)user; return v4(0.1f * (float)loop, 0.2f, 0.3f, 0.4f); }

/* Loops 0 to n-1 of a clip whose "v" carries one varied key, read halfway through each. */
static void build_var(struct nka_context *a, nk_hash clip)
{
    nka_clip_key_float(a, H("t"), 1, 0, NKA_EASE_LINEAR, 0);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, -1);
    nka_clip_end(a);
    nka_play(a, clip, H("vi"));
    nka_update(a, 0.5f);
}

static void next_loop(struct nka_context *a) { steps(a, 2, 0.5f); }

static void varf_run(struct nka_variation_float var, float base, const float *want, int n, int line)
{
    struct nka_context *a = nka_create(0);
    int k;
    nka_clip_begin(a, H("vf"));
    nka_clip_key_float_var(a, H("v"), 0, base, var, NKA_EASE_LINEAR, 0);
    build_var(a, H("vf"));
    for (k = 0; k < n; ++k) {
        float v = getf(a, H("vi"), "v");
        check(inst_of(a, H("vi"))->loop == k && NEAR(v, want[k], 1e-5f), "varf", line);
        next_loop(a);
    }
    nka_destroy(a);
}

static void test_variations(void)
{
    struct nka_context *a;
    float want[8];
    nk_uint s;
    int k;
    {
        const float inc[4] = { 1, 1.5f, 2, 2.5f }, dec[4] = { 1, 0.5f, 0, -0.5f }, mul[4] = { 4, 2, 1, 0.5f };
        const float pp[6] = { 0, 1, 0, -1, 0, 1 }, cl[4] = { 1, 2, 2, 2 }, fn[3] = { 0, 10, 20 }, none[2] = { 3, 3 };
        varf_run(nka_varf_inc(0.5f), 1, inc, 4, __LINE__);
        varf_run(nka_varf_dec(0.5f), 1, dec, 4, __LINE__);
        varf_run(nka_varf_mul(0.5f), 4, mul, 4, __LINE__);
        varf_run(nka_varf_pingpong(1), 0, pp, 6, __LINE__);
        varf_run(nka_varf_clamp(nka_varf_inc(1), 0, 2), 1, cl, 4, __LINE__);
        varf_run(nka_varf_fn(tens, 0), 7, fn, 3, __LINE__);
        varf_run(nka_varf_none(), 3, none, 2, __LINE__);
    }
    for (k = 0; k < 6; ++k) {
        s = 1234u + (nk_uint)k * 1664525u;
        want[k] = 5 + (unit_of(&s) * 2 - 1) * 10;
    }
    varf_run(nka_varf_seed(nka_varf_rand(10), 1234), 5, want, 6, __LINE__);
    for (k = 0; k < 6; ++k) {
        s = 77u + (nk_uint)k * 1664525u;
        want[k] = 5 + unit_of(&s) * 2;
    }
    varf_run(nka_varf_seed(nka_varf_rand_abs(2), 77), 5, want, 6, __LINE__);

    /* Unseeded draws hold for a loop, and repeat for the same instance; upstream drew every frame. */
    {
        struct nka_context *b = nka_create(0);
        float first[3], v, w;
        a = nka_create(0);
        nka_clip_begin(a, H("vf"));
        nka_clip_key_float_var(a, H("v"), 0, 0, nka_varf_rand(100), NKA_EASE_LINEAR, 0);
        build_var(a, H("vf"));
        nka_clip_begin(b, H("vf"));
        nka_clip_key_float_var(b, H("v"), 0, 0, nka_varf_rand(100), NKA_EASE_LINEAR, 0);
        build_var(b, H("vf"));
        for (k = 0; k < 3; ++k) {
            v = getf(a, H("vi"), "v");
            nka_update(a, 0.25f);
            w = getf(a, H("vi"), "v");
            CHECK(v == w && v >= -100 && v <= 100 && v == getf(b, H("vi"), "v"));
            first[k] = v;
            nka_update(a, 0.25f);
            nka_update(a, 0.5f);
            next_loop(b);
        }
        CHECK(first[0] != first[1] && first[1] != first[2]);
        nka_destroy(b);
        nka_destroy(a);
    }

    a = nka_create(0);
    nka_clip_begin(a, H("many"));
    nka_clip_key_int_var(a, H("i"), 0, 1, nka_vari_inc(3), NKA_EASE_LINEAR);
    nka_clip_key_int_var(a, H("j"), 0, 1, nka_vari_clamp(nka_vari_dec(5), -6, 100), NKA_EASE_LINEAR);
    nka_clip_key_int_var(a, H("k"), 0, 1, nka_vari_seed(nka_vari_rand(50), 9), NKA_EASE_LINEAR);
    nka_clip_key_int_var(a, H("l"), 0, 1, nka_vari_fn(hundreds, 0), NKA_EASE_LINEAR);
    nka_clip_key_vec2_var(a, H("p"), 0, v2(0, 0), nka_varv2_inc(1, 2), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2_var(a, H("pa"), 0, v2(0, 0), nka_varv2_axis(nka_varf_inc(1), nka_varf_dec(1)), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2_var(a, H("pm"), 0, v2(1, 3), nka_varv2_clamp(nka_varv2_mul(2), v2(0, 0), v2(100, 20)), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2_var(a, H("pr"), 0, v2(0, 0), nka_varv2_seed(nka_varv2_rand(1, 2), 31), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2_var(a, H("pf"), 0, v2(0, 0), nka_varv2_fn(pairs, 0), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2_var(a, H("pd"), 0, v2(0, 0), nka_varv2_dec(1, 1), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_var(a, H("q"), 0, v4(0, 0, 0, 0), nka_varv4_inc(1, 2, 3, 4), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_var(a, H("qa"), 0, v4(0, 0, 0, 0),
                          nka_varv4_axis(nka_varf_inc(1), nka_varf_none(), nka_varf_dec(1), nka_varf_mul(2)), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_var(a, H("qm"), 0, v4(1, 1, 1, 1), nka_varv4_clamp(nka_varv4_mul(3), v4(0, 0, 0, 0), v4(5, 5, 5, 5)),
                          NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_var(a, H("qr"), 0, v4(0, 0, 0, 0), nka_varv4_seed(nka_varv4_rand(1, 1, 1, 1), 5), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_var(a, H("qd"), 0, v4(0, 0, 0, 0), nka_varv4_dec(1, 1, 1, 1), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_var(a, H("qf"), 0, v4(0, 0, 0, 0), nka_varv4_fn(grey, 0), NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("cs"), 0, rgba(0.25f, 0, 0, 1), nka_varc_space(nka_varc_inc(0.25f, 0, 0, 0), NKA_COL_SRGB),
                           NKA_COL_SRGB, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("cb"), 0, rgba(0, 0, 1, 1), nka_varc_dec(0, 0, 0, 0.25f), NKA_COL_OKLAB, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("ch"), 0, rgba(1, 0, 0, 1),
                           nka_varc_space(nka_varc_channel(nka_varf_inc(0.5f), nka_varf_none(), nka_varf_none(), nka_varf_none()), NKA_COL_HSV),
                           NKA_COL_SRGB, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("cf"), 0, rgba(1, 1, 1, 1), nka_varc_fn(grey, 0), NKA_COL_SRGB, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("cm"), 0, rgba(0.5f, 0.5f, 0.5f, 0.5f), nka_varc_space(nka_varc_mul(0.5f), NKA_COL_SRGB_LINEAR),
                           NKA_COL_SRGB, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("cr"), 0, rgba(0.5f, 0.5f, 0.5f, 1),
                           nka_varc_clamp(nka_varc_seed(nka_varc_rand(0.1f, 0.1f, 0.1f, 0), 3), v4(0, -1, -1, 0), v4(1, 1, 1, 1)),
                           NKA_COL_OKLAB, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("cd"), 0, rgba(0.5f, 0.5f, 0.5f, 1), nka_varc_space(nka_varc_dec(0.5f, 0, 0, 0), NKA_COL_SRGB),
                           NKA_COL_SRGB, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("cn"), 0, rgba(0.3f, 0.6f, 0.9f, 1), nka_varc_none(), NKA_COL_SRGB, NKA_EASE_LINEAR, 0);
    build_var(a, H("many"));
    for (k = 0; k < 4; ++k) {
        struct nk_vec2 p;
        struct nka_vec4 q;
        struct nk_colorf c;
        nk_uint s1;
        float kf = (float)k;
        CHECK(geti(a, H("vi"), "i") == 1 + 3 * k);
        CHECK(geti(a, H("vi"), "j") == (1 - 5 * k < -6 ? -6 : 1 - 5 * k));
        s1 = 9u + (nk_uint)k * 1664525u;
        CHECK(geti(a, H("vi"), "k") == 1 + (int)((unit_of(&s1) * 2 - 1) * 50));
        CHECK(geti(a, H("vi"), "l") == 100 * k);
        nka_instance_get_vec2(a, H("vi"), H("p"), &p);
        CHECK(p.x == kf && p.y == 2 * kf);
        nka_instance_get_vec2(a, H("vi"), H("pa"), &p);
        CHECK(p.x == kf && p.y == -kf);
        nka_instance_get_vec2(a, H("vi"), H("pm"), &p);
        CHECK(p.x == powf(2, kf) && p.y == (3 * powf(2, kf) > 20 ? 20 : 3 * powf(2, kf)));
        nka_instance_get_vec2(a, H("vi"), H("pr"), &p);
        s1 = 31u + (nk_uint)k * 1664525u;
        CHECK(NEAR(p.x, (unit_of(&s1) * 2 - 1) * 1, 1e-6f));
        CHECK(NEAR(p.y, (unit_of(&s1) * 2 - 1) * 2, 1e-6f));
        nka_instance_get_vec2(a, H("vi"), H("pf"), &p);
        CHECK(p.x == kf && p.y == -kf);
        nka_instance_get_vec2(a, H("vi"), H("pd"), &p);
        CHECK(p.x == -kf && p.y == -kf);
        nka_instance_get_vec4(a, H("vi"), H("q"), &q);
        CHECK(q.x == kf && q.y == 2 * kf && q.z == 3 * kf && q.w == 4 * kf);
        nka_instance_get_vec4(a, H("vi"), H("qa"), &q);
        CHECK(q.x == kf && q.y == 0 && q.z == -kf && q.w == 0);
        nka_instance_get_vec4(a, H("vi"), H("qm"), &q);
        CHECK(q.x == (k < 2 ? powf(3, kf) : 5));
        nka_instance_get_vec4(a, H("vi"), H("qr"), &q);
        s1 = 5u + (nk_uint)k * 1664525u;
        CHECK(NEAR(q.x, unit_of(&s1) * 2 - 1, 1e-6f) && NEAR(q.y, unit_of(&s1) * 2 - 1, 1e-6f));
        nka_instance_get_vec4(a, H("vi"), H("qd"), &q);
        CHECK(q.x == -kf && q.w == -kf);
        nka_instance_get_vec4(a, H("vi"), H("qf"), &q);
        CHECK(NEAR(q.x, 0.1f * kf, 1e-6f) && q.w == 0.4f);
        nka_instance_get_color(a, H("vi"), H("cs"), &c);
        CHECK(NEAR(c.r, k < 3 ? 0.25f * (kf + 1) : 1, 1e-6f) && c.g == 0 && c.a == 1);
        /* Blue keeps its hue while its alpha fades: upstream clamped OKLAB's b to 0 and lost it. */
        nka_instance_get_color(a, H("vi"), H("cb"), &c);
        CHECK(NEAR(c.b, 1, 1e-3f) && c.r < 1e-3f && c.g < 1e-3f && NEAR(c.a, 1 - 0.25f * kf, 1e-5f));
        nka_instance_get_color(a, H("vi"), H("ch"), &c);
        CHECK(k % 2 ? (NEAR(c.r, 0, 1e-5f) && NEAR(c.g, 1, 1e-5f)) : (NEAR(c.r, 1, 1e-5f) && NEAR(c.g, 0, 1e-5f)));
        nka_instance_get_color(a, H("vi"), H("cf"), &c);
        CHECK(NEAR(c.r, 0.1f * kf, 1e-6f) && c.g == 0.2f && c.a == 0.4f);
        nka_instance_get_color(a, H("vi"), H("cm"), &c);
        CHECK(NEAR(c.a, 0.5f, 1e-6f) && (k == 0 ? NEAR(c.r, 0.5f, 1e-5f) : c.r < 0.5f));
        nka_instance_get_color(a, H("vi"), H("cr"), &c);
        CHECK(c.r >= 0 && c.r <= 1 && c.g >= 0 && c.g <= 1 && c.a == 1);
        nka_instance_get_color(a, H("vi"), H("cd"), &c);
        CHECK(NEAR(c.r, k ? 0 : 0.5f, 1e-6f));
        nka_instance_get_color(a, H("vi"), H("cn"), &c);
        CHECK(c.r == 0.3f && c.g == 0.6f && c.b == 0.9f);
        next_loop(a);
    }
    nka_destroy(a);

    /* Duration variation changes the speed of a loop; upstream cut the loop short. */
    a = nka_create(0);
    nka_clip_begin(a, H("dv"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, 2);
    nka_clip_set_duration_var(a, nka_varf_mul(0.5f));
    nka_clip_end(a);
    nka_play(a, H("dv"), H("d"));
    steps(a, 5, 0.25f);
    CHECK(nka_instance_time(a, H("d")) == 0.5f && getf(a, H("d"), "f") == 5);
    steps(a, 2, 0.25f);
    CHECK(nka_instance_time(a, H("d")) == 1 && nka_instance_is_playing(a, H("d")) && inst_of(a, H("d"))->loop == 2);
    nka_update(a, 0.25f);
    CHECK(!nka_instance_is_playing(a, H("d")) && getf(a, H("d"), "f") == 10);

    nka_clip_begin(a, H("delv"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, 2);
    nka_clip_set_delay_var(a, nka_varf_inc(0.5f));
    nka_clip_end(a);
    nka_play(a, H("delv"), H("d"));
    steps(a, 5, 0.25f);
    CHECK(nka_instance_time(a, H("d")) == 0 && NEAR(inst_of(a, H("d"))->delay, 0.25f, 1e-6f));
    steps(a, 2, 0.25f);
    CHECK(nka_instance_time(a, H("d")) == 0.25f);
    steps(a, 4, 0.25f);
    CHECK(nka_instance_time(a, H("d")) == 0 && NEAR(inst_of(a, H("d"))->delay, 0.75f, 1e-6f));

    nka_clip_begin(a, H("negv"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, 2);
    nka_clip_set_delay_var(a, nka_varf_dec(0.5f));
    nka_clip_end(a);
    nka_play(a, H("negv"), H("d"));
    steps(a, 5, 0.25f);
    CHECK(nka_instance_time(a, H("d")) == 0.25f);

    nka_clip_begin(a, H("tsv"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, 2);
    nka_clip_set_timescale_var(a, nka_varf_mul(2));
    nka_clip_end(a);
    nka_play(a, H("tsv"), H("d"));
    nka_instance_set_time_scale(a, H("d"), 0.5f);
    steps(a, 8, 0.25f);
    CHECK(nka_instance_time(a, H("d")) == 1 && inst_of(a, H("d"))->loop == 0);
    nka_update(a, 0.25f);
    CHECK(nka_instance_time(a, H("d")) == 0.5f && inst_of(a, H("d"))->scale == 2);
    nka_destroy(a);
}

static void test_stagger(void)
{
    struct nka_context *a = nka_create(0);
    struct nka_stagger_grid_opts o = nka_stagger_grid_opts_default();
    int k, from, axis, ease, col, row, ok = 1;

    nka_clip_begin(a, H("st"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_set_stagger(a, 5, 0.1f, 0);
    nka_clip_set_delay(a, 0.05f);
    nka_clip_end(a);
    for (k = 0; k < 5; ++k) CHECK(NEAR(nka_stagger_delay(a, H("st"), k), 0.1f * (float)k, 1e-6f));
    CHECK(nka_play_stagger(a, H("st"), H("s3"), 3) == H("s3") && NEAR(inst_of(a, H("s3"))->delay, 0.35f, 1e-6f));
    CHECK(nka_play_stagger(a, H("missing"), H("s4"), 3) == 0 && !nka_instance_valid(a, H("s4")));

    nka_clip_begin(a, H("stc"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_set_stagger(a, 5, 0.1f, 1);
    nka_clip_end(a);
    CHECK(NEAR(nka_stagger_delay(a, H("stc"), 0), 0.32f, 1e-6f) && nka_stagger_delay(a, H("stc"), 2) == 0);
    CHECK(NEAR(nka_stagger_delay(a, H("stc"), 4), 0.32f, 1e-6f));

    nka_clip_begin(a, H("sth"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_set_stagger(a, 5, 0.1f, 0.5f);
    nka_clip_end(a);
    CHECK(NEAR(nka_stagger_delay(a, H("sth"), 0), 0.16f, 1e-6f) && NEAR(nka_stagger_delay(a, H("sth"), 4), 0.36f, 1e-6f));

    nka_clip_begin(a, H("ste"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_set_stagger(a, 5, 0.1f, -3);
    nka_clip_set_stagger_ease(a, NKA_EASE_IN_QUAD);
    nka_clip_end(a);
    CHECK(NEAR(nka_stagger_delay(a, H("ste"), 2), 0.1f, 1e-6f) && NEAR(nka_stagger_delay(a, H("ste"), 9), 0.4f, 1e-6f));

    nka_clip_begin(a, H("st1"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_set_stagger(a, 0, 0.1f, 0);
    nka_clip_end(a);
    CHECK(nka_stagger_delay(a, H("st1"), 3) == 0 && nka_stagger_delay(a, H("none"), 3) == 0);

    /* Against upstream's search of every cell for the farthest one. */
    o.cols = 4;
    o.rows = 3;
    o.delay = 0.1f;
    for (from = NKA_STAGGER_FIRST; from <= NKA_STAGGER_INDEX + 1; ++from)
        for (axis = NKA_STAGGER_BOTH; axis <= NKA_STAGGER_Y; ++axis)
            for (ease = 0; ease < 2; ++ease)
                for (row = 0; row < 3; ++row)
                    for (col = 0; col < 4; ++col) {
                        float ox = 0, oy = 0, most = 0, d, t, want;
                        int r, c, at = 13;
                        o.from = from;
                        o.from_index = 13;
                        o.axis = axis;
                        o.ease = ease ? NKA_EASE_IN_QUAD : NKA_EASE_LINEAR;
                        o.start_delay = 0.5f;
                        if (from == NKA_STAGGER_LAST) { ox = 3; oy = 2; }
                        if (from == NKA_STAGGER_CENTER) { ox = 1.5f; oy = 1; }
                        if (from == NKA_STAGGER_INDEX) { at = at > 11 ? 11 : at; ox = (float)(at % 4); oy = (float)(at / 4); }
                        for (r = 0; r < 3; ++r)
                            for (c = 0; c < 4; ++c) {
                                float dx = (float)c - ox, dy = (float)r - oy;
                                d = axis == NKA_STAGGER_X ? fabsf(dx) : axis == NKA_STAGGER_Y ? fabsf(dy) : sqrtf(dx * dx + dy * dy);
                                if (d > most) most = d;
                            }
                        {
                            float dx = (float)col - ox, dy = (float)row - oy;
                            d = axis == NKA_STAGGER_X ? fabsf(dx) : axis == NKA_STAGGER_Y ? fabsf(dy) : sqrtf(dx * dx + dy * dy);
                        }
                        t = most > 0 ? d / most : 0;
                        if (ease) t = nka_eval_preset(NKA_EASE_IN_QUAD, t);
                        want = 0.5f + t * 0.1f * 11;
                        ok = ok && nka_stagger_grid_delay(a, col, row, &o) == want;
                        ok = ok && nka_stagger_grid_delay_index(a, row * 4 + col, &o) == want;
                    }
    CHECK(ok);
    o = nka_stagger_grid_opts_default();
    o.cols = 4;
    o.rows = 3;
    o.delay = 0.1f;
    CHECK(nka_stagger_grid_delay(a, 0, 0, &o) == 0 && NEAR(nka_stagger_grid_delay(a, 3, 2, &o), 1.1f, 1e-6f));
    o.from = NKA_STAGGER_LAST;
    CHECK(NEAR(nka_stagger_grid_delay(a, 0, 0, &o), 1.1f, 1e-6f) && nka_stagger_grid_delay(a, 3, 2, &o) == 0);
    o.from = NKA_STAGGER_INDEX;
    o.from_index = 5;
    CHECK(nka_stagger_grid_delay(a, 1, 1, &o) == 0 && NEAR(nka_stagger_grid_delay(a, 3, 0, &o), 1.1f, 1e-6f));
    o.from = NKA_STAGGER_FIRST;
    o.axis = NKA_STAGGER_X;
    CHECK(NEAR(nka_stagger_grid_delay(a, 2, 2, &o), 1.1f * 2 / 3, 1e-6f));
    CHECK(nka_stagger_grid_delay(a, 5, 5, 0) == 0 && nka_stagger_grid_delay_index(0, 7, 0) == 0);
    o.cols = 0;
    o.rows = -2;
    o.start_delay = 0.25f;
    CHECK(nka_stagger_grid_delay(a, 1, 1, &o) == 0.25f);
    nka_destroy(a);
}

static void test_spring(void)
{
    struct nka_context *a = nka_create(0);
    struct nka_spring_params sp = { 1, 100, 20, 0 };
    nk_hash inst = H("sp");
    int k;
    nka_clip_begin(a, H("spring"));
    nka_clip_key_float_spring(a, H("f"), 0, 0, sp);
    nka_clip_key_float(a, H("f"), 1, 10, NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("g"), 0, 0, NKA_EASE_LINEAR, 0);
    nka_clip_key_float_spring(a, H("g"), 1, 10, sp);
    nka_clip_end(a);
    nka_play(a, H("spring"), inst);
    for (k = 0; k < 10; ++k) {
        float u = (float)k / 10;
        nka_instance_seek(a, inst, u);
        CHECK(NEAR(getf(a, inst, "f"), 10 * (1 - expf(-10 * u) * (1 + 10 * u)), 1e-4f));
        CHECK(NEAR(getf(a, inst, "g"), 10 * u, 1e-5f));
    }
    nka_instance_seek(a, inst, 1);
    CHECK(getf(a, inst, "f") == 10 && getf(a, inst, "g") == 10);
    nka_instance_seek(a, inst, 0.5f);
    CHECK(NEAR(getf(a, inst, "f"), 10 * (1 - expf(-5) * 6), 1e-4f));
    nka_destroy(a);
}

static void test_relative(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash inst = H("r");
    struct nk_vec2 p;
    struct nka_vec4 q;
    struct nk_colorf c;
    nka_set_anchor(a, NKA_ANCHOR_WINDOW, v2(200, 100));
    nka_set_anchor(a, NKA_ANCHOR_VIEWPORT, v2(1000, 500));
    nka_clip_begin(a, H("rel"));
    nka_clip_key_float_rel(a, H("x"), 0, 0, 35, NKA_ANCHOR_WINDOW, 0, NKA_EASE_LINEAR, 0);
    nka_clip_key_float_rel(a, H("x"), 2, 1, -35, NKA_ANCHOR_WINDOW, 0, NKA_EASE_LINEAR, 0);
    nka_clip_key_float_rel(a, H("y"), 0, 0.5f, 0, NKA_ANCHOR_WINDOW, 1, NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2_rel(a, H("p"), 0, v2(0.5f, 0.5f), v2(10, -10), NKA_ANCHOR_VIEWPORT, NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_rel(a, H("q"), 0, v4(0.5f, 0.5f, 2, 3), v4(1, 2, 3, 4), NKA_ANCHOR_WINDOW, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_rel(a, H("c"), 0, v4(0.001f, 0.002f, 0.003f, 0.004f), v4(0, 0, 0.1f, 0), NKA_COL_SRGB, NKA_ANCHOR_WINDOW,
                           NKA_EASE_LINEAR, 0);
    nka_clip_key_float(a, H("x"), 0, 1000, NKA_EASE_LINEAR, 0);
    nka_clip_end(a);
    nka_play(a, H("rel"), inst);
    CHECK(getf(a, inst, "x") == 35 && getf(a, inst, "y") == 50);
    nka_instance_seek(a, inst, 1);
    CHECK(getf(a, inst, "x") == 100);
    nka_set_anchor(a, NKA_ANCHOR_WINDOW, v2(400, 300));
    CHECK(getf(a, inst, "x") == 200 && getf(a, inst, "y") == 150);
    CHECK(nka_instance_get_vec2(a, inst, H("p"), &p) && p.x == 510 && p.y == 240);
    CHECK(nka_instance_get_vec4(a, inst, H("q"), &q) && q.x == 201 && q.y == 152 && q.z == 5 && q.w == 7);
    CHECK(nka_instance_get_color(a, inst, H("c"), &c) && NEAR(c.r, 0.4f, 1e-6f) && NEAR(c.g, 0.6f, 1e-6f)
          && NEAR(c.b, 1.3f, 1e-6f) && NEAR(c.a, 1.2f, 1e-6f));
    nka_update(a, 0.5f);
    CHECK(getf(a, inst, "x") == 1.5f * 400 / 2 + 35 - 35 * 1.5f);
    nka_destroy(a);
}

/* Saving */

static nk_hash rich(struct nka_context *a)
{
    const float bz[4] = { 0.4f, 0, 0.2f, 1 };
    struct nka_spring_params sp = { 1, 150, 12, 2 };
    nk_hash clip = H("rich");
    nka_clip_begin(a, clip);
    nka_clip_key_float(a, H("f"), 0, 1, NKA_EASE_OUT_BACK, 0);
    nka_clip_key_float(a, H("f"), 0.8f, -2, NKA_EASE_CUBIC_BEZIER, bz);
    nka_clip_key_float(a, H("f"), 1.6f, 3, NKA_EASE_LINEAR, 0);
    nka_clip_key_float_spring(a, H("s"), 0, 0, sp);
    nka_clip_key_float(a, H("s"), 1, 1, NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2_var(a, H("v2"), 0, v2(1, 2), nka_varv2_seed(nka_varv2_rand(3, 4), 99), NKA_EASE_IN_OUT_SINE, 0);
    nka_clip_key_vec2_var(a, H("v2"), 1.2f, v2(5, 6), nka_varv2_axis(nka_varf_inc(1), nka_varf_rand(2)), NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_var(a, H("v4"), 0, v4(1, 2, 3, 4), nka_varv4_clamp(nka_varv4_inc(1, 1, 1, 1), v4(0, 0, 0, 0), v4(5, 5, 5, 5)),
                          NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_var(a, H("v4"), 2, v4(4, 3, 2, 1), nka_varv4_axis(nka_varf_none(), nka_varf_mul(2), nka_varf_none(), nka_varf_dec(1)),
                          NKA_EASE_IN_QUAD, 0);
    nka_clip_key_int_var(a, H("i"), 0, 3, nka_vari_seed(nka_vari_rand(7), 5), NKA_EASE_LINEAR);
    nka_clip_key_int(a, H("i"), 1, -40, NKA_EASE_OUT_CUBIC);
    nka_clip_key_int_var(a, H("i"), 2, 2147483000, nka_vari_clamp(nka_vari_inc(1000), 0, 2147483600), NKA_EASE_LINEAR);
    nka_clip_key_color_var(a, H("c"), 0, rgba(1, 0.5f, 0, 1), nka_varc_space(nka_varc_rand(0.1f, 0.1f, 0.1f, 0), NKA_COL_HSV),
                           NKA_COL_OKLCH, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_var(a, H("c"), 1.5f, rgba(0, 0.2f, 1, 0.5f),
                           nka_varc_channel(nka_varf_none(), nka_varf_inc(0.01f), nka_varf_none(), nka_varf_dec(0.1f)),
                           NKA_COL_OKLCH, NKA_EASE_IN_OUT_CUBIC, 0);
    nka_clip_key_float_rel(a, H("x"), 0, 0, 35, NKA_ANCHOR_WINDOW, 0, NKA_EASE_LINEAR, 0);
    nka_clip_key_float_rel(a, H("x"), 2, 1, -35, NKA_ANCHOR_WINDOW, 0, NKA_EASE_OUT_QUAD, 0);
    nka_clip_key_vec2_rel(a, H("p"), 0.5f, v2(0.5f, 0.5f), v2(1, 2), NKA_ANCHOR_VIEWPORT, NKA_EASE_LINEAR, 0);
    nka_clip_key_vec4_rel(a, H("r"), 0, v4(0.1f, 0.2f, 1, 2), v4(1, 2, 3, 4), NKA_ANCHOR_LAST_ITEM, NKA_EASE_LINEAR, 0);
    nka_clip_key_color_rel(a, H("cr"), 0, v4(0.001f, 0.002f, 0.003f, 0.004f), v4(0, 0, 0, 0), NKA_COL_SRGB, NKA_ANCHOR_WINDOW,
                           NKA_EASE_LINEAR, 0);
    nka_clip_key_float_var(a, H("u"), 0, 2, nka_varf_rand(1), NKA_EASE_LINEAR, 0);
    nka_clip_key_float_var(a, H("u"), 1, 4, nka_varf_clamp(nka_varf_pingpong(1.5f), 0, 5), NKA_EASE_LINEAR, 0);
    nka_clip_seq_begin(a);
    key2(a, "q", 0, 0, 0.5f, 1);
    nka_clip_seq_end(a);
    nka_clip_marker(a, 0.25f, 0, on_mark, 0);
    nka_clip_marker(a, 0, H("m1"), on_mark, 0);
    nka_clip_marker(a, -0.1f, H("m2"), on_mark, 0);
    nka_clip_set_loop(a, 1, NKA_DIR_ALTERNATE, 3);
    nka_clip_set_loop_delay(a, 0.1f);
    nka_clip_set_delay(a, 0.05f);
    nka_clip_set_stagger(a, 4, 0.1f, 0.5f);
    nka_clip_set_stagger_ease(a, NKA_EASE_OUT_QUAD);
    nka_clip_set_duration_var(a, nka_varf_seed(nka_varf_rand(0.2f), 7));
    nka_clip_set_delay_var(a, nka_varf_inc(0.05f));
    nka_clip_set_timescale_var(a, nka_varf_pingpong(0.25f));
    nka_clip_on_begin(a, on_begin, 0);
    nka_clip_on_complete(a, on_complete, 0);
    nka_clip_end(a);
    return clip;
}

static int same_values(struct nka_context *a, struct nka_context *b, nk_hash inst)
{
    static const char *const floats[] = { "f", "s", "x", "u", "q" };
    struct nk_vec2 pa, pb;
    struct nka_vec4 qa, qb;
    struct nk_colorf ca, cb;
    int i, ok = nka_instance_time(a, inst) == nka_instance_time(b, inst);
    ok = ok && nka_instance_is_playing(a, inst) == nka_instance_is_playing(b, inst);
    for (i = 0; i < 5; ++i) ok = ok && getf(a, inst, floats[i]) == getf(b, inst, floats[i]);
    ok = ok && geti(a, inst, "i") == geti(b, inst, "i");
    nka_instance_get_vec2(a, inst, H("v2"), &pa);
    nka_instance_get_vec2(b, inst, H("v2"), &pb);
    ok = ok && pa.x == pb.x && pa.y == pb.y;
    nka_instance_get_vec2(a, inst, H("p"), &pa);
    nka_instance_get_vec2(b, inst, H("p"), &pb);
    ok = ok && pa.x == pb.x && pa.y == pb.y;
    nka_instance_get_vec4(a, inst, H("v4"), &qa);
    nka_instance_get_vec4(b, inst, H("v4"), &qb);
    ok = ok && qa.x == qb.x && qa.y == qb.y && qa.z == qb.z && qa.w == qb.w;
    nka_instance_get_vec4(a, inst, H("r"), &qa);
    nka_instance_get_vec4(b, inst, H("r"), &qb);
    ok = ok && qa.x == qb.x && qa.w == qb.w;
    nka_instance_get_color(a, inst, H("c"), &ca);
    nka_instance_get_color(b, inst, H("c"), &cb);
    ok = ok && ca.r == cb.r && ca.g == cb.g && ca.b == cb.b && ca.a == cb.a;
    nka_instance_get_color(a, inst, H("cr"), &ca);
    nka_instance_get_color(b, inst, H("cr"), &cb);
    ok = ok && ca.r == cb.r && ca.a == cb.a;
    return ok;
}

static void put32(unsigned char *p, nk_uint v)
{
    p[0] = (unsigned char)(v & 0xff);
    p[1] = (unsigned char)((v >> 8) & 0xff);
    p[2] = (unsigned char)((v >> 16) & 0xff);
    p[3] = (unsigned char)(v >> 24);
}

static nk_uint get32(const unsigned char *p)
{
    return (nk_uint)p[0] | ((nk_uint)p[1] << 8) | ((nk_uint)p[2] << 16) | ((nk_uint)p[3] << 24);
}

static void reseal(unsigned char *p, int size) { put32(p + size - 4, nka__clip_fnv(p, size - 4)); }

static void test_persist(void)
{
    struct nka_context *a = nka_create(0), *b = nka_create(0);
    nk_hash clip = rich(a), out = 0, inst = H("p");
    unsigned char *buf, *copy, *again;
    int size, i, bad = 0, ok = 1, frame;

    CHECK(nka_clip_exists(a, clip) && nka_clip_save(a, H("nothing"), 0, 0) == 0);
    size = nka_clip_save(a, clip, 0, 0);
    CHECK(size > 128 && size % 4 == 0);
    buf = (unsigned char *)malloc((size_t)size + 8);
    copy = (unsigned char *)malloc((size_t)size + 8);
    again = (unsigned char *)malloc((size_t)size + 8);
    memset(buf, 0xab, (size_t)size + 8);
    CHECK(nka_clip_save(a, clip, buf, size - 1) == size);
    for (i = 0; i < size + 8; ++i) ok = ok && buf[i] == 0xab;
    CHECK(ok);
    CHECK(nka_clip_save(a, clip, buf, size) == size && buf[size] == 0xab);
    CHECK(buf[0] == 'N' && buf[1] == 'K' && buf[2] == 'A' && buf[3] == 'C');
    CHECK(get32(buf + 4) == 1 && get32(buf + 8) == (nk_uint)size && get32(buf + 12) == clip);

    CHECK(nka_clip_load(b, buf, size, &out) == NKA_OK && out == clip);
    CHECK(nka_clip_duration(b, clip) == nka_clip_duration(a, clip) && nka_clip_duration(b, clip) == 2);
    CHECK(nka_clip_save(b, clip, again, size) == size && memcmp(buf, again, (size_t)size) == 0);
    CHECK(nka_stagger_delay(a, clip, 3) == nka_stagger_delay(b, clip, 3) && nka_stagger_delay(a, clip, 3) > 0);

    /* Played side by side, the copy matches bit for bit. */
    nka_set_anchor(a, NKA_ANCHOR_WINDOW, v2(300, 200));
    nka_set_anchor(b, NKA_ANCHOR_WINDOW, v2(300, 200));
    nka_set_anchor(a, NKA_ANCHOR_VIEWPORT, v2(800, 600));
    nka_set_anchor(b, NKA_ANCHOR_VIEWPORT, v2(800, 600));
    nka_set_anchor(a, NKA_ANCHOR_LAST_ITEM, v2(40, 20));
    nka_set_anchor(b, NKA_ANCHOR_LAST_ITEM, v2(40, 20));
    nka_play(a, clip, inst);
    nka_play(b, clip, inst);
    ok = same_values(a, b, inst);
    marks.n = 0;
    reset_events();
    for (frame = 0; frame < 300; ++frame) {
        nka_update(a, 0.037f);
        nka_update(b, 0.037f);
        ok = ok && same_values(a, b, inst);
    }
    CHECK(ok && !nka_instance_is_playing(a, inst) && inst_of(a, inst)->loop == 3);
    CHECK(marks.n == 12 && strcmp(events, "BC") == 0);

    /* Loading over a clip keeps its callbacks and its markers' by id. */
    CHECK(nka_clip_load(a, buf, size, 0) == NKA_OK);
    CHECK(nka_clip_save(a, clip, again, size) == size && memcmp(buf, again, (size_t)size) == 0);
    marks.n = 0;
    reset_events();
    nka_play(a, clip, inst);
    steps(a, 300, 0.037f);
    CHECK(marks.n == 12 && strcmp(events, "BC") == 0);

    /* Every short read, every flipped byte, and trailing bytes. */
    for (i = 0; i < size; ++i) bad += nka_clip_load(b, buf, i, &out) == NKA_ERR_BAD_ARG;
    CHECK(bad == size);
    bad = 0;
    for (i = 0; i < size; ++i) {
        memcpy(copy, buf, (size_t)size);
        copy[i] ^= 0x5a;
        bad += nka_clip_load(b, copy, size, &out) == NKA_ERR_BAD_ARG;
    }
    CHECK(bad == size);
    CHECK(nka_clip_load(b, buf, size + 8, &out) == NKA_OK);

    /* A sum that fits does not let a broken clip through. */
    memcpy(copy, buf, (size_t)size);
    put32(copy + 4, 2);
    reseal(copy, size);
    CHECK(nka_clip_load(b, copy, size, &out) == NKA_ERR_BAD_ARG);
    memcpy(copy, buf, (size_t)size);
    put32(copy + 124, 99);
    reseal(copy, size);
    CHECK(nka_clip_load(b, copy, size, &out) == NKA_ERR_BAD_ARG);
    memcpy(copy, buf, (size_t)size);
    put32(copy + 116, 1000000);
    reseal(copy, size);
    CHECK(nka_clip_load(b, copy, size, &out) == NKA_ERR_BAD_ARG);
    memcpy(copy, buf, (size_t)size);
    put32(copy + 144, 0x7fc00000u);
    reseal(copy, size);
    CHECK(nka_clip_load(b, copy, size, &out) == NKA_ERR_BAD_ARG);
    memcpy(copy, buf, (size_t)size);
    put32(copy + 8, (nk_uint)size + 4);
    put32(copy + size, 0);
    reseal(copy, size + 4);
    CHECK(nka_clip_load(b, copy, size + 4, &out) == NKA_ERR_BAD_ARG);
    memcpy(copy, buf, (size_t)size);
    put32(copy + 8, 12);
    CHECK(nka_clip_load(b, copy, size, &out) == NKA_ERR_BAD_ARG);
    CHECK(nka_clip_load(b, 0, size, &out) == NKA_ERR_BAD_ARG && nka_clip_load(0, buf, size, &out) == NKA_ERR_BAD_ARG);
    CHECK(nka_clip_load(b, buf, -5, &out) == NKA_ERR_BAD_ARG);

    /* A clip with nothing in it, under id 0. */
    nka_clip_begin(a, 0);
    CHECK(nka_clip_end(a) == NKA_OK && nka_clip_exists(a, 0) && nka_clip_duration(a, 0) == 0);
    size = nka_clip_save(a, 0, again, 128);
    CHECK(size == 128 && nka_clip_load(b, again, size, &out) == NKA_OK && out == 0 && nka_clip_exists(b, 0));

    free(buf);
    free(copy);
    free(again);
    nka_destroy(a);
    nka_destroy(b);
}

static void test_gc(void)
{
    struct nka_context *a = nka_create(0);
    float f;
    int i;
    nka_clip_begin(a, H("short"));
    key2(a, "f", 0, 0, 0.25f, 1);
    nka_clip_end(a);
    nka_clip_begin(a, H("forever"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, -1);
    nka_clip_end(a);
    nka_play(a, H("short"), H("done_untouched"));
    nka_play(a, H("short"), H("done_read"));
    nka_play(a, H("forever"), H("playing"));
    nka_play(a, H("forever"), H("paused_read"));
    nka_play(a, H("forever"), H("paused_untouched"));
    nka_instance_pause(a, H("paused_read"));
    nka_instance_pause(a, H("paused_untouched"));
    nka_layer_begin(a, H("lt"));
    nka_layer_add(a, H("playing"), 1);
    nka_layer_end(a, H("lt"));
    nka_layer_begin(a, H("lt_read"));
    nka_layer_add(a, H("playing"), 1);
    nka_layer_end(a, H("lt_read"));
    for (i = 0; i < 20; ++i) {
        nka_update(a, 0.1f);
        getf(a, H("done_read"), "f");
        nka_instance_is_paused(a, H("paused_read"));
        nka_get_blended_float(a, H("lt_read"), H("f"), &f);
    }
    nka_gc(a, 10);
    CHECK(a->clip_insts.len == 3 && a->clip_layers.len == 1);
    CHECK(!nka_instance_valid(a, H("done_untouched")) && !nka_instance_valid(a, H("paused_untouched")));
    CHECK(nka_instance_valid(a, H("done_read")) && nka_instance_valid(a, H("playing")) && nka_instance_valid(a, H("paused_read")));
    CHECK(!nka_get_blended_float(a, H("lt"), H("f"), &f) && nka_get_blended_float(a, H("lt_read"), H("f"), &f));
    nka_clip_reserve(a, 100, 1000);
    CHECK(a->clip_clips.cap == 256 && a->clip_insts.cap == 2048 && nka_instance_valid(a, H("playing")));
    nka_clear(a);
    CHECK(a->clip_insts.len == 0 && a->clip_layers.len == 0 && !nka_instance_valid(a, H("playing")));
    CHECK(nka_clip_exists(a, H("forever")) && nka_play(a, H("forever"), H("again")) == H("again"));
    nka_update(a, 0.5f);
    CHECK(getf(a, H("again"), "f") == 0.5f);
    nka_destroy(a);
}

static void test_busy(void)
{
    struct nka_context *a = nka_create(0);
    nka_clip_begin(a, H("b"));
    key2(a, "f", 0, 0, 0.5f, 1);
    nka_clip_end(a);
    nka_update(a, 0.25f);
    CHECK(!nka_busy(a));
    nka_play(a, H("b"), H("i"));
    CHECK(nka_busy(a));
    nka_update(a, 0.25f);
    CHECK(nka_busy(a));
    nka_update(a, 0.25f);
    CHECK(nka_busy(a) && nka_instance_is_playing(a, H("i")));
    nka_update(a, 0.25f);
    CHECK(!nka_busy(a) && !nka_instance_is_playing(a, H("i")));
    nka_update(a, 0.25f);
    CHECK(!nka_busy(a));
    nka_play_with_delay(a, H("b"), H("i"), 1);
    nka_update(a, 0.25f);
    CHECK(nka_busy(a));
    nka_instance_pause(a, H("i"));
    nka_update(a, 0.25f);
    CHECK(!nka_busy(a));
    nka_destroy(a);
}

static void test_misc(void)
{
    struct nka_context *a = nka_create(0);
    unsigned char buf[8] = { 0 };
    struct nk_vec2 p;
    float f;
    int i;

    /* A clip with no length completes at once; upstream played forever. */
    nka_clip_begin(a, H("zero"));
    nka_clip_key_float(a, H("f"), 0, 7, NKA_EASE_LINEAR, 0);
    nka_clip_marker(a, 0, H("z"), on_mark, 0);
    nka_clip_set_loop(a, 1, NKA_DIR_NORMAL, -1);
    nka_clip_on_complete(a, on_complete, 0);
    nka_clip_end(a);
    reset_events();
    marks.n = 0;
    nka_play(a, H("zero"), H("z"));
    CHECK(getf(a, H("z"), "f") == 7 && nka_instance_is_playing(a, H("z")));
    nka_update(a, 0.1f);
    CHECK(!nka_instance_is_playing(a, H("z")) && marks.n == 1 && strcmp(events, "C") == 0 && !nka_busy(a));

    /* A clip authored again while it plays. */
    nka_clip_begin(a, H("rb"));
    key2(a, "f", 0, 0, 1, 10);
    nka_clip_end(a);
    nka_play(a, H("rb"), H("r"));
    nka_update(a, 0.5f);
    nka_clip_begin(a, H("rb"));
    key2(a, "g", 0, 0, 0.25f, 1);
    key2(a, "f", 0, 100, 0.25f, 100);
    CHECK(getf(a, H("r"), "f") == 5);
    nka_clip_end(a);
    CHECK(getf(a, H("r"), "f") == 100 && getf(a, H("r"), "g") == 1 && nka_instance_time(a, H("r")) == 0.25f);
    nka_update(a, 0.1f);
    CHECK(!nka_instance_is_playing(a, H("r")));

    /* A second begin drops the first build. */
    nka_clip_begin(a, H("first"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_begin(a, H("second"));
    key2(a, "f", 0, 0, 1, 1);
    CHECK(nka_clip_end(a) == NKA_OK && !nka_clip_exists(a, H("first")) && nka_clip_exists(a, H("second")));
    CHECK(nka_clip_end(a) == NKA_ERR_BAD_ARG);

    for (i = 0; i < 300; ++i) {
        nka_clip_begin(a, nka_id_mix(H("bulk"), (nk_hash)i));
        key2(a, "f", 0, (float)i, 1, 0);
        nka_clip_end(a);
    }
    for (i = 0; i < 300; ++i) CHECK(nka_play(a, nka_id_mix(H("bulk"), (nk_hash)i), nka_id_mix(H("bi"), (nk_hash)i)));
    for (i = 0; i < 300; ++i) CHECK(getf(a, nka_id_mix(H("bi"), (nk_hash)i), "f") == (float)i);
    nka_destroy(a);

    nka_clip_begin(0, 1);
    nka_clip_key_float(0, 1, 0, 1, NKA_EASE_LINEAR, 0);
    nka_clip_key_int(0, 1, 0, 1, NKA_EASE_LINEAR);
    nka_clip_seq_begin(0);
    nka_clip_par_end(0);
    nka_clip_set_loop(0, 1, 0, 1);
    nka_clip_on_begin(0, on_begin, 0);
    CHECK(nka_clip_marker(0, 0, 0, 0, 0) == 0 && nka_clip_end(0) == NKA_ERR_BAD_ARG);
    CHECK(nka_play(0, 1, 1) == 0 && nka_play_stagger(0, 1, 1, 1) == 0 && nka_play_with_delay(0, 1, 1, 1) == 0);
    CHECK(nka_get_instance(0, 1) == 0 && !nka_instance_valid(0, 1) && nka_instance_then(0, 1, 1, 0) == 0);
    nka_instance_pause(0, 1);
    nka_instance_seek(0, 1, 1);
    nka_instance_destroy(0, 1);
    nka_instance_set_weight(0, 1, 1);
    nka_instance_then_delay(0, 1, 1);
    CHECK(nka_instance_time(0, 1) == 0 && nka_instance_duration(0, 1) == 0 && !nka_instance_is_playing(0, 1));
    CHECK(!nka_instance_get_float(0, 1, 1, &f) && f == 0 && !nka_instance_get_vec2(0, 1, 1, &p));
    CHECK(nka_clip_duration(0, 1) == 0 && !nka_clip_exists(0, 1) && nka_stagger_delay(0, 1, 2) == 0);
    CHECK(nka_stagger_grid_delay(0, 1, 1, 0) == 0);
    nka_layer_begin(0, 1);
    nka_layer_add(0, 1, 1);
    nka_layer_end(0, 1);
    CHECK(!nka_get_blended_float(0, 1, 1, &f) && !nka_get_blended_int(0, 1, 1, &i));
    CHECK(nka_clip_save(0, 1, buf, 8) == 0 && nka_clip_load(0, buf, 8, 0) == NKA_ERR_BAD_ARG);
    nka_clip_reserve(0, 1, 1);
}

/* Memory */

static int live, allocs, fail_at = -1;

static void *count_alloc(nk_handle h, void *old, nk_size n)
{
    (void)h; (void)old;
    if (allocs++ == fail_at) return 0;
    live++;
    return malloc(n);
}

static void count_free(nk_handle h, void *p)
{
    (void)h;
    live--;
    free(p);
}

static void scenario(struct nka_context *a)
{
    static unsigned char buf[16384];
    nk_hash clip = rich(a), out;
    float f;
    int i, size;
    nka_clip_begin(a, H("plain"));
    key2(a, "f", 0, 0, 1, 1);
    nka_clip_end(a);
    for (i = 0; i < 40; ++i) nka_play(a, i & 1 ? clip : H("plain"), nka_id_mix(H("o"), (nk_hash)i));
    nka_instance_then(a, nka_id_mix(H("o"), 0), clip, 0);
    nka_layer_begin(a, H("l"));
    for (i = 0; i < 40; ++i) nka_layer_add(a, nka_id_mix(H("o"), (nk_hash)i), 1);
    nka_layer_end(a, H("l"));
    nka_get_blended_float(a, H("l"), H("f"), &f);
    size = nka_clip_save(a, clip, buf, (int)sizeof buf);
    nka_clip_load(a, buf, size, &out);
    nka_clip_begin(a, H("plain"));
    key2(a, "g", 0, 0, 1, 1);
    nka_clip_end(a);
    steps(a, 40, 0.05f);
    for (i = 0; i < 40; ++i) getf(a, nka_id_mix(H("o"), (nk_hash)i), "f");
    steps(a, 10, 0.05f);
    nka_gc(a, 5);
    CHECK(a->clip_insts.len < 41 && a->clip_layers.len == 0);
    nka_clear(a);
    nka_play(a, clip, H("after"));
    steps(a, 3, 0.05f);
}

static void test_alloc(void)
{
    struct nk_allocator al;
    struct nka_context *a;
    int n, total, ok = 1;
    al.userdata.ptr = 0;
    al.alloc = count_alloc;
    al.free = count_free;
    live = allocs = 0;
    fail_at = -1;
    a = nka_create(&al);
    scenario(a);
    CHECK(live > 1);
    nka_destroy(a);
    CHECK(live == 0);
    total = allocs;
    /* Every allocation failing in turn: nothing leaks and nothing breaks. */
    for (n = 0; n < total; ++n) {
        live = allocs = 0;
        fail_at = n;
        a = nka_create(&al);
        if (a) {
            scenario(a);
            nka_destroy(a);
        }
        ok = ok && live == 0;
    }
    CHECK(ok && total > 50);
    fail_at = -1;
}

int main(void)
{
    test_keys();
    test_groups();
    test_delay();
    test_loops();
    test_markers();
    test_callbacks();
    test_chain();
    test_control();
    test_time_scale();
    test_layers();
    test_variations();
    test_stagger();
    test_spring();
    test_relative();
    test_persist();
    test_gc();
    test_busy();
    test_misc();
    test_alloc();
    if (failures) printf("%d failed\n", failures);
    else printf("clip: ok\n");
    return failures != 0;
}
