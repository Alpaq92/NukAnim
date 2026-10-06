#include <stdio.h>
#include <string.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"
#define NUKANIM_IMPLEMENTATION
#include "nukanim.h"

static int failures;

static void check(int ok, const char *what, int line)
{
    if (ok) return;
    printf("nk.c:%d: %s\n", line, what);
    failures++;
}

#define CHECK(c) check((c) != 0, #c, __LINE__)
#define NEAR(x, y, eps) (fabsf((float)(x) - (float)(y)) <= (eps))

static const struct nka_ease linear = { NKA_EASE_LINEAR, 0, 0, 0, 0 };
static struct nk_context ctx;
static struct nk_user_font font;

static float text_width(nk_handle h, float height, const char *s, int len)
{
    (void)h; (void)s;
    return (float)len * height * 0.5f;
}

/* One frame of a 200x200 window holding 50 rows, or a group of them. */
static nk_uint scroll_frame(struct nka_context *a, int group, int action, const struct nk_vec2 *wheel)
{
    nk_uint x = 0, y = 0;
    int i;
    nk_input_begin(&ctx);
    if (wheel) {
        nk_input_motion(&ctx, 50, 50);
        nk_input_scroll(&ctx, *wheel);
    }
    nk_input_end(&ctx);
    if (nk_begin(&ctx, "list", nk_rect(0, 0, 200, 200), 0)) {
        if (group) {
            nk_layout_row_dynamic(&ctx, 150, 1);
            if (nk_group_begin(&ctx, "g", 0)) {
                nk_layout_row_dynamic(&ctx, 20, 1);
                for (i = 0; i < 50; ++i) nk_label(&ctx, "row", NK_TEXT_LEFT);
                if (action == 1) nka_scroll_to_y(a, &ctx, 300, 1, linear);
                if (action == 2) nka_scroll_to_bottom(a, &ctx, 1, linear);
                nka_scroll(a, &ctx);
                nk_group_end(&ctx);
            }
            nk_group_get_scroll(&ctx, "g", &x, &y);
        } else {
            nk_layout_row_dynamic(&ctx, 20, 1);
            for (i = 0; i < 50; ++i) nk_label(&ctx, "row", NK_TEXT_LEFT);
            if (action == 1) nka_scroll_to_y(a, &ctx, 300, 1, linear);
            if (action == 2) nka_scroll_to_bottom(a, &ctx, 1, linear);
            if (action == 3) nka_scroll_to_top(a, &ctx, 1, linear);
            nka_scroll(a, &ctx);
        }
    }
    nk_end(&ctx);
    if (!group) y = nk_window_find(&ctx, "list")->scrollbar.y;
    nk_clear(&ctx);
    return y;
}

static struct nka__nk_scroll *last_scroll(struct nka_context *a)
{
    int i;
    for (i = 0; i < a->nk_scrolls.cap; ++i)
        if (a->nk_scrolls.keys[i]) return (struct nka__nk_scroll *)nka__at(&a->nk_scrolls, i);
    return 0;
}

static void test_scroll(int group)
{
    struct nka_context *a = nka_create(0);
    struct nk_vec2 up = { 0, 1 };
    nk_uint max, y;
    nka_update(a, 0.016f);
    CHECK(scroll_frame(a, group, 0, 0) == 0);
    nka_update(a, 0);
    CHECK(scroll_frame(a, group, 1, 0) == 0);
    CHECK(nka_busy(a));
    nka_update(a, 0.25f);
    CHECK(scroll_frame(a, group, 0, 0) == 75);
    nka_update(a, 0.25f);
    CHECK(scroll_frame(a, group, 0, 0) == 150);
    nka_update(a, 0.5f);
    CHECK(scroll_frame(a, group, 0, 0) == 300);
    CHECK(!nka_busy(a));
    nka_update(a, 0.5f);
    CHECK(scroll_frame(a, group, 0, 0) == 300);

    nka_update(a, 0);
    CHECK(scroll_frame(a, group, 2, 0) == 300);
    max = (nk_uint)last_scroll(a)->max[1];
    CHECK(max > 300 && max < 50 * 30);
    nka_update(a, 0.5f);
    y = scroll_frame(a, group, 0, 0);
    CHECK(y > 300 && y < max);
    nka_update(a, 0.5f);
    CHECK(scroll_frame(a, group, 0, 0) == max);
    nka_update(a, 0.5f);
    CHECK(scroll_frame(a, group, 0, 0) == max);

    if (!group) {
        nka_update(a, 0);
        scroll_frame(a, group, 3, 0);
        nka_update(a, 0.25f);
        y = scroll_frame(a, group, 0, &up);
        CHECK(last_scroll(a)->active[1]);
        nka_update(a, 0.25f);
        y = scroll_frame(a, group, 0, 0);
        CHECK(!last_scroll(a)->active[1]);
        nka_update(a, 0.25f);
        CHECK(scroll_frame(a, group, 0, 0) == y);
    }
    nka_destroy(a);
}

static void test_style(void)
{
    struct nka_context *a = nka_create(0);
    struct nk_color table[NK_COLOR_COUNT];
    struct nk_style keep = ctx.style, base, wide, out;
    struct nk_user_font other = font;
    nk_hash d = nka_id("default"), red = nka_id("red"), big = nka_id("big");
    int i;

    for (i = 0; i < NK_COLOR_COUNT; ++i) table[i] = nk_rgba(200, 30, 30, 255);
    nka_style_register_current(a, &ctx, d);
    nka_style_register_table(a, &ctx, red, table);
    CHECK(memcmp(&keep, &ctx.style, sizeof keep) == 0);
    CHECK(nka_style_exists(a, d) && nka_style_exists(a, red) && !nka_style_exists(a, big));

    base = ctx.style;
    wide = ctx.style;
    wide.window.rounding = base.window.rounding + 10;
    wide.window.padding = nk_vec2(base.window.padding.x + 20, base.window.padding.y);
    wide.text.color = nk_rgba(0, 0, 0, 255);
    wide.font = &other;
    wide.button.normal = nk_style_item_hide();
    nka_style_register(a, big, &wide);

    nka_style_blend_to(a, d, big, 0, &out, NKA_COL_SRGB);
    CHECK(memcmp(&out, &base, sizeof out) == 0);
    nka_style_blend_to(a, d, big, 1, &out, NKA_COL_SRGB);
    CHECK(memcmp(&out, &wide, sizeof out) == 0);
    nka_style_blend_to(a, d, big, 0.4f, &out, NKA_COL_SRGB);
    CHECK(NEAR(out.window.rounding, base.window.rounding + 4, 1e-4f));
    CHECK(NEAR(out.window.padding.x, base.window.padding.x + 8, 1e-4f));
    CHECK(out.text.color.r == (nk_byte)(base.text.color.r * 0.6f + 0.5f));
    CHECK(out.font == base.font && out.button.normal.type == NK_STYLE_ITEM_COLOR);
    nka_style_blend_to(a, d, big, 0.6f, &out, NKA_COL_SRGB);
    CHECK(out.font == &other && out.button.normal.type == wide.button.normal.type);

    nka_style_blend_to(a, d, red, 0.5f, &out, NKA_COL_OKLAB);
    CHECK(out.window.background.r > base.window.background.r && out.window.background.r < 200);
    CHECK(out.window.rounding == base.window.rounding);

    nka_update(a, 0.016f);
    nka_style_tween(a, &ctx, 1, red, 1, linear, NKA_COL_SRGB);
    CHECK(memcmp(&keep, &ctx.style, sizeof keep) == 0 && nka_busy(a));
    nka_update(a, 0.5f);
    nka_style_tween(a, &ctx, 1, red, 1, linear, NKA_COL_SRGB);
    CHECK(ctx.style.window.background.r > keep.window.background.r && ctx.style.window.background.r < 200);
    nka_update(a, 0.5f);
    nka_style_tween(a, &ctx, 1, red, 1, linear, NKA_COL_SRGB);
    CHECK(!nka_busy(a));
    nka_style_blend_to(a, d, red, 1, &out, NKA_COL_SRGB);
    CHECK(memcmp(&out, &ctx.style, sizeof out) == 0);
    nka_update(a, 0.5f);
    nka_style_tween(a, &ctx, 1, red, 1, linear, NKA_COL_SRGB);
    CHECK(!nka_busy(a) && memcmp(&out, &ctx.style, sizeof out) == 0);
    nka_style_tween(a, &ctx, 1, d, 1, linear, NKA_COL_SRGB);
    CHECK(nka_busy(a));
    nka_update(a, 1);
    nka_style_tween(a, &ctx, 1, d, 1, linear, NKA_COL_SRGB);
    CHECK(memcmp(&keep, &ctx.style, sizeof keep) == 0);

    nka_style_unregister(a, red);
    CHECK(!nka_style_exists(a, red));
    out = keep;
    nka_style_blend_to(a, d, red, 0.5f, &out, NKA_COL_SRGB);
    CHECK(memcmp(&keep, &out, sizeof out) == 0);
    nka_destroy(a);
    ctx.style = keep;
}

struct glyph { float x, y, height; nk_byte alpha; char c; const struct nk_user_font *font; };

/* Draws text at (20, 40) in a fresh window and collects the glyphs it drew. */
static int stagger(struct nka_context *a, const char *text, float progress, struct nka_text_stagger_opts *o,
                   struct glyph *out)
{
    const struct nk_command *cmd;
    int n = 0;
    nk_input_begin(&ctx);
    nk_input_end(&ctx);
    o->pos = nk_vec2(20, 40);
    if (nk_begin(&ctx, "text", nk_rect(0, 0, 300, 200), NK_WINDOW_NO_SCROLLBAR))
        nka_text_stagger(a, &ctx, text, progress, o);
    nk_end(&ctx);
    nk_foreach(cmd, &ctx) {
        const struct nk_command_text *t = (const struct nk_command_text *)cmd;
        if (cmd->type != NK_COMMAND_TEXT || n == 16) continue;
        out[n].x = t->x;
        out[n].y = t->y;
        out[n].height = t->height;
        out[n].alpha = t->foreground.a;
        out[n].c = t->string[0];
        out[n].font = t->font;
        ++n;
    }
    nk_clear(&ctx);
    return n;
}

static void test_text(void)
{
    struct nka_context *a = nka_create(0);
    struct nka_text_stagger_opts o = nka_text_stagger_opts_default();
    struct glyph g[16];
    int n, i;

    CHECK(NEAR(nka_text_stagger_duration("Hello", &o), 0.5f, 1e-6f));
    CHECK(nka_text_stagger_duration("", &o) == 0 && nka_text_stagger_duration("h\xc3\xa9llo", 0) == nka_text_stagger_duration("hello", 0));
    o.letter_spacing = 2;
    CHECK(nka_text_stagger_width(&ctx, "Hello", &o) == 5 * 8 + 4 * 2);
    o.letter_spacing = 0;

    o.ease = linear;
    nka_update(a, 0.016f);
    n = stagger(a, "Hello", 0.3f, &o, g);
    CHECK(n == 3);
    for (i = 0; i < n; ++i) CHECK(g[i].x == 20 + i * 8 && g[i].y == 40 && g[i].c == "Hello"[i]);
    CHECK(NEAR(g[0].alpha, 128, 1) && NEAR(g[1].alpha, 85, 1) && NEAR(g[2].alpha, 43, 1));
    n = stagger(a, "Hello", 0.8f, &o, g);
    CHECK(n == 5 && g[2].alpha == 255 && NEAR(g[3].alpha, 213, 1) && NEAR(g[4].alpha, 170, 1));
    n = stagger(a, "Hello", 1, &o, g);
    CHECK(n == 5 && g[4].alpha == 255 && g[4].font == &font);
    CHECK(stagger(a, "Hello", 0, &o, g) == 0);
    CHECK(stagger(a, "h\xc3\xa9llo", 1, &o, g) == 5);

    o.effect = NKA_TEXT_FX_SLIDE_UP;
    n = stagger(a, "Hello", 0.8f, &o, g);
    CHECK(g[0].y == 40 && g[3].y > 40 && g[4].y > g[3].y);
    o.effect = NKA_TEXT_FX_SLIDE_RIGHT;
    n = stagger(a, "Hello", 0.8f, &o, g);
    CHECK(g[0].x == 20 && g[3].x < 44);

    o.effect = NKA_TEXT_FX_SCALE;
    n = stagger(a, "Hello", 0.8f, &o, g);
    CHECK(n == 5 && g[2].height == 16 && g[2].font == &font);
    CHECK(g[3].height < 16 && g[3].font != &font && g[3].font->height == g[3].height && g[3].font->width == font.width);
    CHECK(g[4].height < g[3].height && g[4].y > 40);
    CHECK(a->nk_fonts && a->nk_fonts->used == 2);
    nka_update(a, 0.016f);
    CHECK(a->nk_fonts->used == 0);

    o.effect = NKA_TEXT_FX_BOUNCE;
    n = stagger(a, "Hello", 0.8f, &o, g);
    CHECK(n == 5 && g[3].height > 16);

    o.effect = NKA_TEXT_FX_ROTATE;
    n = stagger(a, "Hello", 0.8f, &o, g);
    CHECK(n == 5 && g[3].height == 16 && g[3].alpha < 255);

    o.effect = NKA_TEXT_FX_TYPEWRITER;
    n = stagger(a, "Hello", 0.3f, &o, g);
    CHECK(n == 3 && g[2].alpha == 255);

    o.effect = NKA_TEXT_FX_WAVE;
    n = stagger(a, "Hello", 0, &o, g);
    CHECK(n == 5 && g[0].y == 40 && g[1].y != 40);

    o.effect = NKA_TEXT_FX_FADE;
    o.font_scale = 2;
    n = stagger(a, "Hello", 1, &o, g);
    CHECK(n == 5 && g[1].x == 20 + 16 && g[1].height == 32);
    nka_destroy(a);
}

static int path_text(struct nka_context *a, nk_hash path, const char *text, float progress,
                     const struct nka_text_path_opts *o, struct glyph *out)
{
    const struct nk_command *cmd;
    int n = 0;
    nk_input_begin(&ctx);
    nk_input_end(&ctx);
    if (nk_begin(&ctx, "text", nk_rect(0, 0, 300, 200), NK_WINDOW_NO_SCROLLBAR))
        nka_text_path_animated(a, &ctx, path, text, progress, o);
    nk_end(&ctx);
    nk_foreach(cmd, &ctx) {
        const struct nk_command_text *t = (const struct nk_command_text *)cmd;
        if (cmd->type != NK_COMMAND_TEXT || n == 16) continue;
        out[n].x = t->x;
        out[n].y = t->y;
        out[n].height = t->height;
        out[n].alpha = t->foreground.a;
        out[n].c = t->string[0];
        out[n].font = t->font;
        ++n;
    }
    nk_clear(&ctx);
    return n;
}

static void test_text_path(void)
{
    struct nka_context *a = nka_create(0);
    struct nka_text_path_opts o = nka_text_path_opts_default();
    struct glyph g[16];
    nk_hash line = nka_id("line"), down = nka_id("down");
    int n;
    nka_path_begin(a, line, nk_vec2(10, 100));
    nka_path_line_to(a, nk_vec2(210, 100));
    nka_path_end(a);
    nka_path_begin(a, down, nk_vec2(150, 20));
    nka_path_line_to(a, nk_vec2(150, 180));
    nka_path_end(a);
    nka_update(a, 0.016f);
    CHECK(nka_text_path_width(&ctx, "abcd", &o) == 32);
    n = path_text(a, line, "abcd", 1, &o, g);
    CHECK(n == 4 && g[0].x == 10 && g[1].x == 18 && g[3].x == 34 && g[0].y == 84 && g[2].c == 'c');
    CHECK(nka_path_has_arc_lut(a, line));
    o.align = NKA_TEXT_ALIGN_CENTER;
    n = path_text(a, line, "abcd", 1, &o, g);
    CHECK(n == 4 && g[0].x == 94);
    o.align = NKA_TEXT_ALIGN_END;
    n = path_text(a, line, "abcd", 1, &o, g);
    CHECK(n == 4 && g[0].x == 178 && g[3].x == 202);
    o.align = NKA_TEXT_ALIGN_START;
    o.offset = -12;
    n = path_text(a, line, "abcd", 1, &o, g);
    CHECK(n == 3 && g[0].c == 'b' && g[0].x == 6);
    o.offset = 0;
    o.flip_y = 1;
    n = path_text(a, line, "abcd", 1, &o, g);
    CHECK(n == 4 && g[0].y == 100);
    o.flip_y = 0;
    n = path_text(a, line, "abcd", 0.5f, &o, g);
    CHECK(n == 2 && g[1].alpha == 255);
    n = path_text(a, line, "abcd", 0.625f, &o, g);
    CHECK(n == 3 && NEAR(g[2].alpha, 128, 1));
    CHECK(path_text(a, line, "abcd", 0, &o, g) == 0);
    n = path_text(a, down, "ab", 1, &o, g);
    CHECK(n == 2 && g[0].x == 146 && g[0].y == 8 && g[1].y == 16);
    o.font_scale = 2;
    n = path_text(a, line, "abcd", 1, &o, g);
    CHECK(n == 4 && g[1].x == 26 && g[0].height == 32 && g[0].font != &font);
    CHECK(path_text(a, nka_id("missing"), "abcd", 1, &o, g) == 0);
    nka_destroy(a);
}

static double clock_now;

static double fake_clock(void *user)
{
    (void)user;
    return clock_now;
}

static int window_texts(const char *needle)
{
    const struct nk_command *cmd;
    int n = 0, len = (int)strlen(needle);
    nk_foreach(cmd, &ctx) {
        const struct nk_command_text *t = (const struct nk_command_text *)cmd;
        if (cmd->type == NK_COMMAND_TEXT && t->length >= len && !strncmp(t->string, needle, (size_t)len)) ++n;
    }
    return n;
}

static void test_inspector(void)
{
    struct nka_context *a = nka_create(0);
    struct nka__nk_profiler *p;
    int open = 1, i;
    nka_profiler_begin(a, "ignored");
    CHECK(!nka_profiler_is_enabled(a));
    nka_profiler_enable(a, 1);
    nka_profiler_begin_frame(a);
    nka_profiler_begin(a, "nothing without a clock");
    nka_profiler_end(a);
    p = a->nk_profiler;
    CHECK(p && p->count == 0);
    nka_profiler_set_clock(a, fake_clock, 0);
    for (i = 0; i < 2; ++i) {
        nka_profiler_begin_frame(a);
        nka_profiler_begin(a, "layout");
        clock_now += 0.002;
        nka_profiler_begin(a, "a section with a name longer than thirty-one characters");
        clock_now += 0.001;
        nka_profiler_end(a);
        nka_profiler_end(a);
        nka_profiler_begin(a, "a section with a name longer than thirty-one characters, again");
        clock_now += 0.001;
        nka_profiler_end(a);
        nka_profiler_end_frame(a);
    }
    CHECK(p->count == 2 && p->at == 2);
    CHECK(NEAR(p->sections[0].ms, 3, 1e-3f) && p->sections[0].calls == 1);
    CHECK(NEAR(p->sections[1].ms, 2, 1e-3f) && p->sections[1].calls == 2);
    CHECK(NEAR(p->frame_ms, 4, 1e-3f) && NEAR(p->frame[1], 4, 1e-3f) && NEAR(p->sections[1].history[1], 2, 1e-3f));

    nka_update(a, 0.016f);
    nka_tween_float(a, 7, 8, 1, 1, linear, NKA_POLICY_CROSSFADE, 0);
    nk_input_begin(&ctx);
    nk_input_end(&ctx);
    nka_show_unified_inspector(a, &ctx, &open);
    CHECK(open && nk_window_find(&ctx, "NukAnim") != 0);
    CHECK(window_texts("Time scale") == 2 && window_texts("Moving") == 1 && window_texts("0x00000007") == 1);
    nk_clear(&ctx);
    open = 0;
    nk_input_begin(&ctx);
    nk_input_end(&ctx);
    nka_show_unified_inspector(a, &ctx, &open);
    CHECK(window_texts("Time scale") == 0);
    nk_clear(&ctx);
    nka_destroy(a);
}

static int count_commands(enum nk_command_type type)
{
    const struct nk_command *cmd;
    int n = 0;
    nk_foreach(cmd, &ctx) n += cmd->type == type;
    return n;
}

/* The timeline's background, the first filled rect in its ground color. */
static struct nk_rect timeline_rect(void)
{
    const struct nk_command *cmd;
    nk_foreach(cmd, &ctx) {
        const struct nk_command_rect_filled *f = (const struct nk_command_rect_filled *)cmd;
        if (cmd->type == NK_COMMAND_RECT_FILLED && f->color.r == 30 && f->color.g == 32 && f->color.b == 40)
            return nk_rect(f->x, f->y, f->w, f->h);
    }
    return nk_rect(0, 0, 0, 0);
}

static void test_timeline(void)
{
    struct nka_context *a = nka_create(0);
    nk_hash clip = nka_id("fade in"), inst;
    struct nk_rect r;
    float label, x0, w;
    nka_clip_begin(a, clip);
    nka_clip_key_float(a, nka_id("alpha"), 0, 0, NKA_EASE_OUT_CUBIC, 0);
    nka_clip_key_float(a, nka_id("alpha"), 1, 1, NKA_EASE_LINEAR, 0);
    nka_clip_key_vec2(a, nka_id("pos"), 0, nk_vec2(0, 0), NKA_EASE_IN_OUT_SINE, 0);
    nka_clip_key_vec2(a, nka_id("pos"), 2, nk_vec2(10, 0), NKA_EASE_LINEAR, 0);
    nka_clip_set_delay(a, 0.5f);
    CHECK(nka_clip_end(a) == NKA_OK);
    nka_update(a, 0.016f);
    inst = nka_play(a, clip, 9);
    CHECK(inst == 9);
    nka_update(a, 1.0f);

    nk_input_begin(&ctx);
    nk_input_end(&ctx);
    if (nk_begin(&ctx, "timeline", nk_rect(0, 0, 400, 300), NK_WINDOW_NO_SCROLLBAR)) {
        nka_show_debug_timeline(a, &ctx, inst);
        nka_show_debug_timeline(a, &ctx, 12345);
    }
    nk_end(&ctx);
    CHECK(window_texts("Clip 0x") == 1 && window_texts("No instance 0x00003039") == 1);
    CHECK(window_texts("float") == 1 && window_texts("vec2") == 1 && window_texts("delay end") == 1);
    CHECK(window_texts("0.0s") == 1 && window_texts("2.0s") == 1 && window_texts("2.5s") == 0);
    CHECK(count_commands(NK_COMMAND_TRIANGLE_FILLED) == 1 && count_commands(NK_COMMAND_CIRCLE_FILLED) == 4);
    r = timeline_rect();
    CHECK(r.w > 300 && r.h == 130);
    nk_clear(&ctx);

    label = font.width(font.userdata, font.height, "color rel", 9) + 8;
    x0 = r.x + 4 + label;
    w = r.w - label - 8;
    nk_input_begin(&ctx);
    nk_input_motion(&ctx, (int)(x0 + 0.5f / 2.5f * w), (int)(r.y + 44 + 10));
    nk_input_end(&ctx);
    if (nk_begin(&ctx, "timeline", nk_rect(0, 0, 400, 300), NK_WINDOW_NO_SCROLLBAR))
        nka_show_debug_timeline(a, &ctx, inst);
    nk_end(&ctx);
    CHECK(window_texts("0.000s   out cubic") == 1);
    nk_clear(&ctx);
    nka_destroy(a);
}

static void test_anchors(void)
{
    struct nka_context *a = nka_create(0);
    struct nk_vec2 s;
    nk_input_begin(&ctx);
    nk_input_end(&ctx);
    if (nk_begin(&ctx, "anchored", nk_rect(10, 10, 240, 160), 0)) {
        nka_anchor_update(a, &ctx);
        nk_layout_row_dynamic(&ctx, 20, 1);
        CHECK(NEAR(nka_tween_float_rel(a, 1, 2, 0.5f, 0, 1, linear, NKA_POLICY_CUT, NKA_ANCHOR_WINDOW, 0), 120, 1e-4f));
    }
    nk_end(&ctx);
    nk_clear(&ctx);
    s = nka_anchor(a, NKA_ANCHOR_WINDOW);
    CHECK(s.x == 240 && s.y == 160);
    s = nka_anchor(a, NKA_ANCHOR_WINDOW_CONTENT);
    CHECK(s.x > 0 && s.x < 240 && s.y > 0 && s.y < 160);
    nka_destroy(a);
}

int main(void)
{
    font.height = 16;
    font.width = text_width;
    nk_init_default(&ctx, &font);
    test_scroll(0);
    test_scroll(1);
    test_style();
    test_text();
    test_text_path();
    test_inspector();
    test_timeline();
    test_anchors();
    nk_free(&ctx);
    if (failures) printf("%d failed\n", failures);
    else printf("nk: ok\n");
    return failures != 0;
}
