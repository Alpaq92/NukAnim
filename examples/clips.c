/* The README's clips: Nuklear's anti-aliased vertex output, rasterized in
 * software, written as raw RGBA frames for tools/gif.py to turn into GIFs.
 *
 *     clips <scene> <out.raw>      run from the repository's root
 */
#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nk.h"
#define NUKANIM_IMPLEMENTATION
#include "nukanim.h"

#define DT 0.03f
#define FONTS "external/nuklear/extra_font/"

struct vertex { float x, y, u, v; nk_byte c[4]; };

static struct nk_context ctx;
static struct nka_context *anim;
static struct nk_font *font, *bold, *small;
static struct nk_draw_null_texture null_tex;
static unsigned char *tex, *fb;
static int tex_w, tex_h, fb_w, fb_h;

static const struct nk_color ink = { 230, 232, 236, 255 }, dim = { 138, 143, 152, 255 };
static const struct nk_color ground = { 22, 24, 29, 255 }, panel = { 31, 34, 41, 255 }, line = { 48, 52, 62, 255 };
static const struct nk_color cyan = { 91, 194, 231, 255 }, copper = { 204, 120, 88, 255 };

static void sample(float u, float v, float *out)
{
    float x = u * (float)tex_w - 0.5f, y = v * (float)tex_h - 0.5f;
    int x0 = (int)floorf(x), y0 = (int)floorf(y), i, j, k;
    float fx = x - (float)x0, fy = y - (float)y0;
    out[0] = out[1] = out[2] = out[3] = 0;
    for (j = 0; j < 2; ++j)
        for (i = 0; i < 2; ++i) {
            int tx = x0 + i < 0 ? 0 : (x0 + i >= tex_w ? tex_w - 1 : x0 + i);
            int ty = y0 + j < 0 ? 0 : (y0 + j >= tex_h ? tex_h - 1 : y0 + j);
            float w = (i ? fx : 1 - fx) * (j ? fy : 1 - fy);
            const unsigned char *p = tex + ((size_t)ty * (size_t)tex_w + (size_t)tx) * 4;
            for (k = 0; k < 4; ++k) out[k] += w * (float)p[k] / 255.0f;
        }
}

/* Edges in 1/256 pixel fixed point, so two triangles sharing an edge agree on
 * every pixel; one on the edge goes to the triangle whose top or left edge it
 * is, and nothing blends twice or not at all. */
struct fixed { long long x, y; };

static struct fixed fix(const struct vertex *v)
{
    struct fixed f;
    f.x = (long long)floorf(v->x * 256 + 0.5f);
    f.y = (long long)floorf(v->y * 256 + 0.5f);
    return f;
}

static long long edge(struct fixed a, struct fixed b, long long x, long long y)
{
    return (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);
}

static int top_left(struct fixed a, struct fixed b)
{
    return (b.y == a.y && b.x > a.x) || b.y < a.y;
}

static int covers(long long w, int tl) { return w > 0 || (w == 0 && tl); }

static void triangle(const struct vertex *v0, const struct vertex *v1, const struct vertex *v2, struct nk_rect clip)
{
    const struct vertex *v[3];
    struct fixed f[3];
    long long area;
    int x0, x1, y0, y1, x, y, k, tl[3];
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
    for (k = 0; k < 3; ++k) f[k] = fix(v[k]);
    area = edge(f[0], f[1], f[2].x, f[2].y);
    if (area == 0) return;
    if (area < 0) {
        struct fixed t = f[1];
        f[1] = f[2];
        f[2] = t;
        v[1] = v2;
        v[2] = v1;
        area = -area;
    }
    x0 = (int)floorf(fminf(v0->x, fminf(v1->x, v2->x)));
    x1 = (int)ceilf(fmaxf(v0->x, fmaxf(v1->x, v2->x)));
    y0 = (int)floorf(fminf(v0->y, fminf(v1->y, v2->y)));
    y1 = (int)ceilf(fmaxf(v0->y, fmaxf(v1->y, v2->y)));
    if (x0 < (int)floorf(clip.x)) x0 = (int)floorf(clip.x);
    if (y0 < (int)floorf(clip.y)) y0 = (int)floorf(clip.y);
    if (x1 > (int)ceilf(clip.x + clip.w)) x1 = (int)ceilf(clip.x + clip.w);
    if (y1 > (int)ceilf(clip.y + clip.h)) y1 = (int)ceilf(clip.y + clip.h);
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > fb_w) x1 = fb_w;
    if (y1 > fb_h) y1 = fb_h;
    tl[0] = top_left(f[1], f[2]);
    tl[1] = top_left(f[2], f[0]);
    tl[2] = top_left(f[0], f[1]);
    for (y = y0; y < y1; ++y)
        for (x = x0; x < x1; ++x) {
            long long px = (long long)x * 256 + 128, py = (long long)y * 256 + 128, e[3];
            float w, t[4], s[4], u = 0, vv = 0;
            unsigned char *d;
            e[0] = edge(f[1], f[2], px, py);
            e[1] = edge(f[2], f[0], px, py);
            e[2] = edge(f[0], f[1], px, py);
            if (!covers(e[0], tl[0]) || !covers(e[1], tl[1]) || !covers(e[2], tl[2])) continue;
            s[0] = s[1] = s[2] = s[3] = 0;
            for (k = 0; k < 3; ++k) {
                int c;
                w = (float)e[k] / (float)area;
                u += w * v[k]->u;
                vv += w * v[k]->v;
                for (c = 0; c < 4; ++c) s[c] += w * (float)v[k]->c[c] / 255.0f;
            }
            sample(u, vv, t);
            for (k = 0; k < 4; ++k) s[k] *= t[k];
            d = fb + ((size_t)y * (size_t)fb_w + (size_t)x) * 4;
            for (k = 0; k < 3; ++k) d[k] = (unsigned char)((float)d[k] * (1 - s[3]) + s[k] * 255.0f * s[3] + 0.5f);
            d[3] = 255;
        }
}

static void render(struct nk_color clear)
{
    static const struct nk_draw_vertex_layout_element layout[] = {
        { NK_VERTEX_POSITION, NK_FORMAT_FLOAT, 0 },
        { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, 8 },
        { NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, 16 },
        { NK_VERTEX_LAYOUT_END }
    };
    struct nk_convert_config cfg;
    struct nk_buffer cmds, verts, idx;
    const struct nk_draw_command *cmd;
    const nk_draw_index *offset;
    const struct vertex *vtx;
    int i;
    for (i = 0; i < fb_w * fb_h; ++i) {
        fb[i * 4 + 0] = clear.r;
        fb[i * 4 + 1] = clear.g;
        fb[i * 4 + 2] = clear.b;
        fb[i * 4 + 3] = 255;
    }
    memset(&cfg, 0, sizeof cfg);
    cfg.vertex_layout = layout;
    cfg.vertex_size = sizeof(struct vertex);
    cfg.vertex_alignment = 4;
    cfg.tex_null = null_tex;
    cfg.circle_segment_count = 48;
    cfg.curve_segment_count = 32;
    cfg.arc_segment_count = 32;
    cfg.global_alpha = 1;
    cfg.shape_AA = NK_ANTI_ALIASING_ON;
    cfg.line_AA = NK_ANTI_ALIASING_ON;
    nk_buffer_init_default(&cmds);
    nk_buffer_init_default(&verts);
    nk_buffer_init_default(&idx);
    nk_convert(&ctx, &cmds, &verts, &idx, &cfg);
    vtx = (const struct vertex *)nk_buffer_memory_const(&verts);
    offset = (const nk_draw_index *)nk_buffer_memory_const(&idx);
    nk_draw_foreach(cmd, &ctx, &cmds) {
        unsigned int e;
        for (e = 0; e + 2 < cmd->elem_count; e += 3)
            triangle(&vtx[offset[e]], &vtx[offset[e + 1]], &vtx[offset[e + 2]], cmd->clip_rect);
        offset += cmd->elem_count;
    }
    nk_buffer_free(&cmds);
    nk_buffer_free(&verts);
    nk_buffer_free(&idx);
    nk_clear(&ctx);
}

static void setup(int w, int h)
{
    struct nk_font_atlas atlas;
    const void *image;
    struct nk_color table[NK_COLOR_COUNT];
    int i;
    fb_w = w;
    fb_h = h;
    fb = (unsigned char *)malloc((size_t)w * (size_t)h * 4);
    nk_font_atlas_init_default(&atlas);
    nk_font_atlas_begin(&atlas);
    font = nk_font_atlas_add_from_file(&atlas, FONTS "Roboto-Regular.ttf", 17, 0);
    bold = nk_font_atlas_add_from_file(&atlas, FONTS "Roboto-Bold.ttf", 18, 0);
    small = nk_font_atlas_add_from_file(&atlas, FONTS "Roboto-Regular.ttf", 13, 0);
    if (!font || !bold || !small) {
        fprintf(stderr, "clips: run from the repository's root, the fonts are in " FONTS "\n");
        exit(1);
    }
    image = nk_font_atlas_bake(&atlas, &tex_w, &tex_h, NK_FONT_ATLAS_RGBA32);
    tex = (unsigned char *)malloc((size_t)tex_w * (size_t)tex_h * 4);
    memcpy(tex, image, (size_t)tex_w * (size_t)tex_h * 4);
    nk_font_atlas_end(&atlas, nk_handle_id(0), &null_tex);
    nk_init_default(&ctx, &font->handle);
    for (i = 0; i < NK_COLOR_COUNT; ++i) table[i] = panel;
    table[NK_COLOR_TEXT] = ink;
    table[NK_COLOR_WINDOW] = ground;
    table[NK_COLOR_BORDER] = line;
    table[NK_COLOR_BUTTON_HOVER] = line;
    table[NK_COLOR_BUTTON_ACTIVE] = line;
    nk_style_from_table(&ctx, table);
    ctx.style.window.padding = nk_vec2(0, 0);
    ctx.style.window.spacing = nk_vec2(0, 0);
    anim = nka_create(0);
}

static struct nk_command_buffer *canvas(void)
{
    nk_begin(&ctx, "clip", nk_rect(0, 0, (float)fb_w, (float)fb_h), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND);
    return nk_window_get_canvas(&ctx);
}

static void text(struct nk_command_buffer *out, const struct nk_font *f, float x, float y, const char *s,
                 struct nk_color c)
{
    nk_draw_text(out, nk_rect(x, y, 400, f->handle.height + 4), s, (int)strlen(s), &f->handle,
                 nk_rgba(0, 0, 0, 0), c);
}

static float text_width(const struct nk_font *f, const char *s)
{
    return f->handle.width(f->handle.userdata, f->handle.height, s, (int)strlen(s));
}

/* Scenes take the frame's time and draw into one window that fills the clip. */

static void easing(float time)
{
    static const char *names[] = { "out cubic", "in out sine", "out back", "out elastic", "out bounce",
                                   "spring", "steps", "bezier" };
    struct nka_ease eases[8];
    struct nk_command_buffer *out = canvas();
    float target = fmodf(time, 4.4f) < 2.2f ? 1.0f : 0.0f;
    int i;
    eases[0] = nka_ease(NKA_EASE_OUT_CUBIC);
    eases[1] = nka_ease(NKA_EASE_IN_OUT_SINE);
    eases[2] = nka_ease(NKA_EASE_OUT_BACK);
    eases[3] = nka_ease(NKA_EASE_OUT_ELASTIC);
    eases[4] = nka_ease(NKA_EASE_OUT_BOUNCE);
    eases[5] = nka_ease_spring(1, 180, 18, 0);
    eases[6] = nka_ease_steps(6, 0);
    eases[7] = nka_ease_bezier(0.68f, -0.55f, 0.27f, 1.55f);
    for (i = 0; i < 8; ++i) {
        float y = 26 + (float)i * 33, x0 = 170, x1 = (float)fb_w - 80;
        float k = nka_tween_float(anim, nka_id(names[i]), 0, target, 1.4f, eases[i], NKA_POLICY_CROSSFADE, 0);
        float x = x0 + (x1 - x0) * k;
        text(out, font, 28, y - 10, names[i], dim);
        nk_stroke_line(out, x0, y, x1, y, 2, line);
        nk_fill_circle(out, nk_rect(x - 8, y - 8, 16, 16), i & 1 ? copper : cyan);
    }
    nk_end(&ctx);
}

static void hover(float time)
{
    static const char *labels[] = { "Home", "Search", "Library", "Profile" };
    static float size[4] = { 1, 1, 1, 1 };
    const float pi = 3.14159265f;
    struct nk_command_buffer *out = canvas();
    struct nk_vec2 mouse;
    float sweep = fmodf(time, 4.0f) / 4.0f;
    int i;
    mouse.x = 40 + ((float)fb_w - 80) * (0.5f - 0.5f * cosf(sweep * 2 * pi));
    mouse.y = (float)fb_h * 0.5f + 34 * sinf(sweep * 4 * pi);
    for (i = 0; i < 4; ++i) {
        float w = 120, h = 64, cx = (float)fb_w * ((float)i + 0.5f) / 4, cy = (float)fb_h * 0.5f;
        struct nk_rect r = nk_rect(cx - w / 2, cy - h / 2, w, h);
        int over = mouse.x >= r.x && mouse.x <= r.x + r.w && mouse.y >= r.y && mouse.y <= r.y + r.h;
        struct nk_colorf fill;
        if (over) nka_animate(anim, 1, 1.12f, 180, &size[i], NKA_EASE_OUT_CUBIC);
        else nka_animate(anim, 1.12f, 1, 260, &size[i], NKA_EASE_OUT_CUBIC);
        fill = nka_tween_color(anim, nka_id(labels[i]), 1, nk_color_cf(over ? cyan : panel), 0.25f,
                               nka_ease(NKA_EASE_OUT_CUBIC), NKA_POLICY_CROSSFADE, NKA_COL_OKLAB, nk_color_cf(panel));
        w *= size[i];
        h *= size[i];
        nk_fill_rect(out, nk_rect(cx - w / 2, cy - h / 2, w, h), 12 * size[i], nk_rgb_cf(fill));
        text(out, bold, cx - text_width(bold, labels[i]) / 2, cy - 11, labels[i], over ? ground : ink);
    }
    text(out, small, 24, (float)fb_h - 30, "nka_animate(a, 1, 1.12f, 180, &size, NKA_EASE_OUT_CUBIC)", dim);
    nk_fill_circle(out, nk_rect(mouse.x - 6, mouse.y - 6, 12, 12), ink);
    nk_end(&ctx);
}

static struct nk_colorf rgb(float r, float g, float b)
{
    struct nk_colorf c;
    c.r = r; c.g = g; c.b = b; c.a = 1;
    return c;
}

static void colors(float time)
{
    static const char *names[] = { "sRGB", "linear", "HSV", "OKLAB", "OKLCH" };
    static const int spaces[] = { NKA_COL_SRGB, NKA_COL_SRGB_LINEAR, NKA_COL_HSV, NKA_COL_OKLAB, NKA_COL_OKLCH };
    struct nk_colorf from = rgb(0.12f, 0.42f, 1.0f), to = rgb(1.0f, 0.82f, 0.12f);
    struct nk_command_buffer *out = canvas();
    float k = nka_tween_float(anim, nka_id("blend"), 0, fmodf(time, 4.8f) < 2.4f ? 1.0f : 0.0f, 1.6f,
                              nka_ease(NKA_EASE_IN_OUT_CUBIC), NKA_POLICY_CROSSFADE, 0);
    int i, s;
    for (i = 0; i < 5; ++i) {
        float y = 22 + (float)i * 46, x0 = 110, w = 400;
        text(out, font, 24, y + 4, names[i], dim);
        for (s = 0; s < 100; ++s)
            nk_fill_rect(out, nk_rect(x0 + w * (float)s / 100, y, w / 100 + 1, 28), 0,
                         nk_rgb_cf(nka_color_blend(from, to, (float)s / 99, spaces[i])));
        nk_stroke_circle(out, nk_rect(x0 + w * k - 7, y + 7, 14, 14), 2, ink);
        nk_fill_rect(out, nk_rect(x0 + w + 30, y - 2, 64, 32), 8, nk_rgb_cf(nka_color_blend(from, to, k, spaces[i])));
    }
    nk_end(&ctx);
}

static void paths(float time)
{
    nk_hash loop = nka_id("loop"), wave = nka_id("wave");
    struct nka_text_path_opts opts = nka_text_path_opts_default();
    struct nk_command_buffer *out = canvas();
    struct nk_vec2 p, prev;
    float angle, progress;
    int i, lap = (int)(time / 3.0f);
    nka_path_begin(anim, loop, nk_vec2(40, 230));
    nka_path_cubic_to(anim, nk_vec2(60, 40), nk_vec2(250, 30), nk_vec2(270, 130));
    nka_path_catmull_to(anim, nk_vec2(200, 200), 0.5f);
    nka_path_catmull_to(anim, nk_vec2(300, 250), 0.5f);
    nka_path_end(anim);
    if (!nka_path_has_arc_lut(anim, loop)) nka_path_build_arc_lut(anim, loop, 128);
    nka_path_begin(anim, wave, nk_vec2(340, 170));
    nka_path_cubic_to(anim, nk_vec2(420, 60), nk_vec2(500, 280), nk_vec2(620, 150));
    nka_path_end(anim);
    for (i = 0, prev = nka_path_evaluate(anim, loop, 0); i <= 120; ++i, prev = p) {
        p = nka_path_evaluate(anim, loop, (float)i / 120);
        if (i) nk_stroke_line(out, prev.x, prev.y, p.x, p.y, 2, line);
    }
    for (i = 0, prev = nka_path_evaluate(anim, wave, 0); i <= 120; ++i, prev = p) {
        p = nka_path_evaluate(anim, wave, (float)i / 120);
        if (i) nk_stroke_line(out, prev.x, prev.y, p.x, p.y, 1, line);
    }
    p = nka_tween_path(anim, 1, (nk_hash)lap, loop, 2.4f, nka_ease(NKA_EASE_IN_OUT_CUBIC), NKA_POLICY_CROSSFADE);
    angle = nka_tween_path_angle(anim, 2, (nk_hash)lap, loop, 2.4f, nka_ease(NKA_EASE_IN_OUT_CUBIC), NKA_POLICY_CROSSFADE);
    nk_stroke_line(out, p.x, p.y, p.x + 22 * cosf(angle), p.y + 22 * sinf(angle), 3, copper);
    nk_fill_circle(out, nk_rect(p.x - 8, p.y - 8, 16, 16), cyan);
    progress = fmodf(time, 3.0f) / 2.2f;
    opts.color = ink;
    opts.align = NKA_TEXT_ALIGN_CENTER;
    opts.font = &bold->handle;
    nka_text_path_animated(anim, &ctx, wave, "text follows paths too", progress, &opts);
    nk_end(&ctx);
}

static void stagger(float time)
{
    static const char *names[] = { "fade", "slide up", "bounce", "wave", "typewriter", "scale" };
    static const int effects[] = { NKA_TEXT_FX_FADE, NKA_TEXT_FX_SLIDE_UP, NKA_TEXT_FX_BOUNCE,
                                   NKA_TEXT_FX_WAVE, NKA_TEXT_FX_TYPEWRITER, NKA_TEXT_FX_SCALE };
    struct nka_text_stagger_opts opts = nka_text_stagger_opts_default();
    struct nk_command_buffer *out = canvas();
    const char *line_text = "Text arrives glyph by glyph";
    float progress = fmodf(time, 4.5f) / 3.0f;
    int i;
    opts.char_delay = 0.035f;
    opts.char_duration = 0.45f;
    opts.effect_intensity = 14;
    opts.color = ink;
    opts.font = &bold->handle;
    for (i = 0; i < 6; ++i) {
        float y = 24 + (float)i * 42;
        text(out, small, 24, y + 4, names[i], dim);
        opts.effect = effects[i];
        opts.pos = nk_vec2(130, y);
        nka_text_stagger(anim, &ctx, line_text, effects[i] == NKA_TEXT_FX_WAVE ? fmodf(time, 4.5f) / 4.5f
                                                                              : (progress < 1 ? progress : 1), &opts);
    }
    nk_end(&ctx);
}

static void styles(float time)
{
    static int registered, check = 1, option = 1;
    static float slider = 0.6f;
    static nk_size progress = 64;
    nk_hash dark = nka_id("dark"), light = nka_id("light");
    if (!registered) {
        struct nk_color table[NK_COLOR_COUNT];
        int i;
        nka_style_register_current(anim, &ctx, dark);
        for (i = 0; i < NK_COLOR_COUNT; ++i) table[i] = nk_rgb(236, 238, 242);
        table[NK_COLOR_TEXT] = nk_rgb(32, 36, 44);
        table[NK_COLOR_WINDOW] = nk_rgb(250, 250, 252);
        table[NK_COLOR_BORDER] = nk_rgb(214, 218, 226);
        table[NK_COLOR_BUTTON] = nk_rgb(226, 230, 238);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgb(214, 218, 228);
        table[NK_COLOR_SLIDER] = nk_rgb(214, 218, 226);
        table[NK_COLOR_SLIDER_CURSOR] = copper;
        table[NK_COLOR_TOGGLE_CURSOR] = copper;
        table[NK_COLOR_SELECT_ACTIVE] = copper;
        table[NK_COLOR_PROPERTY] = nk_rgb(226, 230, 238);
        nka_style_register_table(anim, &ctx, light, table);
        ctx.style.slider.cursor_normal = nk_style_item_color(cyan);
        ctx.style.checkbox.cursor_normal = nk_style_item_color(cyan);
        ctx.style.option.cursor_normal = nk_style_item_color(cyan);
        nka_style_register_current(anim, &ctx, dark);
        registered = 1;
    }
    nka_style_tween(anim, &ctx, 1, fmodf(time, 5.0f) < 2.5f ? light : dark, 0.9f,
                    nka_ease(NKA_EASE_IN_OUT_CUBIC), NKA_COL_OKLAB);
    ctx.style.window.padding = nk_vec2(28, 22);
    ctx.style.window.spacing = nk_vec2(12, 12);
    if (nk_begin(&ctx, "style", nk_rect(0, 0, (float)fb_w, (float)fb_h), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(&ctx, 26, 1);
        nk_label(&ctx, "nka_style_tween blends whole Nuklear styles", NK_TEXT_LEFT);
        nk_layout_row_dynamic(&ctx, 28, 2);
        nk_checkbox_label(&ctx, "Animate", &check);
        nk_slider_float(&ctx, 0, &slider, 1, 0.01f);
        nk_option_label(&ctx, "Color space: OKLAB", option);
        nk_progress(&ctx, &progress, 100, NK_FIXED);
        nk_layout_row_dynamic(&ctx, 36, 3);
        nk_button_label(&ctx, "Cancel");
        nk_button_label(&ctx, "Apply");
        nk_button_label(&ctx, "OK");
    }
    nk_end(&ctx);
}

static struct nk_rect cell(struct nk_command_buffer *out, int i, const char *name)
{
    float w = ((float)fb_w - 16 * 4) / 3, h = ((float)fb_h - 16 * 3) / 2;
    struct nk_rect r = nk_rect(16 + (float)(i % 3) * (w + 16), 16 + (float)(i / 3) * (h + 16), w, h);
    nk_fill_rect(out, r, 10, panel);
    text(out, small, r.x + 12, r.y + 8, name, dim);
    return r;
}

static void procedural(float time)
{
    static struct nk_vec2 trail[24];
    static int trail_n;
    const float pi = 3.14159265f;
    struct nk_command_buffer *out = canvas();
    struct nk_rect r;
    struct nk_vec2 c, v;
    struct nka_gradient g;
    struct nka_transform x;
    int i;

    r = cell(out, 0, "oscillate");
    c = nk_vec2(r.x + r.w / 2, r.y + r.h / 2 + 8);
    for (i = 0; i < 5; ++i) {
        float y = nka_oscillate(anim, nka_id_mix(nka_id("osc"), (nk_hash)i), 26, 0.7f, NKA_WAVE_SINE, (float)i * 0.12f);
        nk_fill_circle(out, nk_rect(c.x - 52 + (float)i * 26 - 7, c.y + y - 7, 14, 14), cyan);
    }

    r = cell(out, 1, "wiggle");
    c = nk_vec2(r.x + r.w / 2, r.y + r.h / 2 + 8);
    v = nka_wiggle_vec2(anim, nka_id("wiggle"), nk_vec2(42, 28), 1.4f);
    nk_fill_circle(out, nk_rect(c.x + v.x - 9, c.y + v.y - 9, 18, 18), copper);

    r = cell(out, 2, "shake");
    c = nk_vec2(r.x + r.w / 2, r.y + r.h / 2 + 8);
    if (fmodf(time, 1.5f) < DT) nka_trigger_shake(anim, nka_id("shake"));
    v = nka_shake_vec2(anim, nka_id("shake"), nk_vec2(12, 5), 28, 0.55f);
    nk_fill_rect(out, nk_rect(c.x - 44 + v.x, c.y - 18 + v.y, 88, 36), 8, line);
    text(out, small, c.x - 20 + v.x, c.y - 9 + v.y, "wrong", ink);

    r = cell(out, 3, "noise");
    c = nk_vec2(r.x + r.w / 2, r.y + r.h / 2 + 8);
    v = nka_noise_channel_vec2(anim, nka_id("noise"), nk_vec2(0.9f, 0.7f), nk_vec2(170, 100), 0);
    trail[trail_n++ % 24] = nk_vec2(c.x + v.x, c.y + v.y);
    for (i = (trail_n < 24 ? trail_n : 24) - 1; i >= 0; --i) {
        struct nk_vec2 p = trail[(trail_n - 1 - i) % 24];
        struct nk_color k = cyan;
        k.a = (nk_byte)(255 - i * 10);
        nk_fill_circle(out, nk_rect(p.x - 6 + (float)i * 0.15f, p.y - 6 + (float)i * 0.15f,
                                    12 - (float)i * 0.3f, 12 - (float)i * 0.3f), k);
    }

    r = cell(out, 4, "gradient");
    {
        struct nka_gradient a = nka_gradient_three_color(nk_color_cf(cyan), rgb(0.95f, 0.95f, 0.98f), nk_color_cf(copper));
        struct nka_gradient b = nka_gradient_two_color(rgb(0.36f, 0.20f, 0.62f), rgb(1.0f, 0.78f, 0.25f));
        g = nka_tween_gradient(anim, nka_id("gradient"), 0, fmodf(time, 4.0f) < 2.0f ? &a : &b, 1.2f,
                               nka_ease(NKA_EASE_IN_OUT_CUBIC), NKA_POLICY_CROSSFADE, NKA_COL_OKLAB);
        for (i = 0; i < 64; ++i)
            nk_fill_rect(out, nk_rect(r.x + 16 + (r.w - 32) * (float)i / 64, r.y + 44, (r.w - 32) / 64 + 1, 56), 0,
                         nk_rgb_cf(nka_gradient_sample(&g, (float)i / 63, NKA_COL_OKLAB)));
    }

    r = cell(out, 5, "transform");
    c = nk_vec2(r.x + r.w / 2, r.y + r.h / 2 + 8);
    x = nka_tween_transform(anim, nka_id("square"), 0,
                            fmodf(time, 3.0f) < 1.5f ? nka_transform(c, 0, nk_vec2(1, 1))
                                                     : nka_transform(c, pi / 2, nk_vec2(1.5f, 0.8f)),
                            1.1f, nka_ease(NKA_EASE_OUT_BACK), NKA_POLICY_CROSSFADE, NKA_ROTATION_CW);
    {
        float pts[8];
        static const float corner[4][2] = { { -22, -22 }, { 22, -22 }, { 22, 22 }, { -22, 22 } };
        for (i = 0; i < 4; ++i) {
            struct nk_vec2 p = nka_transform_apply(x, nk_vec2(corner[i][0], corner[i][1]));
            pts[i * 2] = p.x;
            pts[i * 2 + 1] = p.y;
        }
        nk_fill_polygon(out, pts, 4, cyan);
    }
    nk_end(&ctx);
}

static void timeline(float time)
{
    static int made;
    static float started = -1;
    nk_hash clip = nka_id("tile"), scale = nka_id("scale"), alpha = nka_id("alpha"), lift = nka_id("lift");
    struct nka_stagger_grid_opts grid = nka_stagger_grid_opts_default();
    struct nk_command_buffer *out;
    int i;
    if (!made) {
        nka_clip_begin(anim, clip);
        nka_clip_key_float(anim, scale, 0, 0.4f, NKA_EASE_OUT_BACK, 0);
        nka_clip_key_float(anim, scale, 0.6f, 1, NKA_EASE_LINEAR, 0);
        nka_clip_key_float(anim, alpha, 0, 0, NKA_EASE_OUT_CUBIC, 0);
        nka_clip_key_float(anim, alpha, 0.35f, 1, NKA_EASE_LINEAR, 0);
        nka_clip_key_float(anim, lift, 0, 26, NKA_EASE_OUT_CUBIC, 0);
        nka_clip_key_float(anim, lift, 0.5f, 0, NKA_EASE_LINEAR, 0);
        nka_clip_end(anim);
        made = 1;
    }
    grid.cols = 6;
    grid.rows = 2;
    grid.from = NKA_STAGGER_CENTER;
    grid.delay = 0.09f;
    if (started < 0 || time - started >= 3.0f) {
        for (i = 0; i < 12; ++i) nka_play_with_delay(anim, clip, (nk_hash)(100 + i), nka_stagger_grid_delay_index(anim, i, &grid));
        started = time;
    }
    nk_style_push_vec2(&ctx, &ctx.style.window.padding, nk_vec2(16, 12));
    nk_begin(&ctx, "clip", nk_rect(0, 0, (float)fb_w, (float)fb_h), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND);
    out = nk_window_get_canvas(&ctx);
    for (i = 0; i < 12; ++i) {
        float s = 1, k = 1, y = 0, cx = 16 + 52 + (float)(i % 6) * 101, cy = 52 + (float)(i / 6) * 80;
        struct nk_color c = i % 2 ? copper : cyan;
        nka_instance_get_float(anim, (nk_hash)(100 + i), scale, &s);
        nka_instance_get_float(anim, (nk_hash)(100 + i), alpha, &k);
        nka_instance_get_float(anim, (nk_hash)(100 + i), lift, &y);
        c.a = (nk_byte)(255 * (k < 0 ? 0 : (k > 1 ? 1 : k)));
        nk_fill_rect(out, nk_rect(cx - 44 * s, cy + y - 30 * s, 88 * s, 60 * s), 10 * s, c);
    }
    nk_layout_row_dynamic(&ctx, 150, 1);
    nk_spacer(&ctx);
    nka_show_debug_timeline(anim, &ctx, 100 + 2);
    nk_end(&ctx);
    nk_style_pop_vec2(&ctx);
}

struct scene { const char *name; int w, h; float seconds; void (*draw)(float time); };

static const struct scene scenes[] = {
    { "easing", 640, 290, 4.4f, easing },
    { "hover", 640, 220, 4.0f, hover },
    { "colors", 640, 250, 4.8f, colors },
    { "paths", 640, 290, 3.0f, paths },
    { "stagger", 640, 280, 4.5f, stagger },
    { "styles", 640, 230, 5.0f, styles },
    { "procedural", 640, 300, 6.0f, procedural },
    { "timeline", 640, 330, 3.0f, timeline },
};

int main(int argc, char **argv)
{
    const struct scene *s = 0;
    unsigned int header[4];
    FILE *f;
    int i, frames;
    for (i = 0; argc == 3 && i < (int)(sizeof scenes / sizeof *scenes); ++i)
        if (!strcmp(argv[1], scenes[i].name)) s = &scenes[i];
    if (!s) {
        fprintf(stderr, "usage: clips <scene> <out.raw>; scenes:");
        for (i = 0; i < (int)(sizeof scenes / sizeof *scenes); ++i) fprintf(stderr, " %s", scenes[i].name);
        fprintf(stderr, "\n");
        return 2;
    }
    setup(s->w, s->h);
    f = fopen(argv[2], "wb");
    if (!f) return 1;
    header[0] = 0x464b414e; /* NAKF */
    header[1] = (unsigned int)s->w;
    header[2] = (unsigned int)s->h;
    header[3] = (unsigned int)(DT * 1000 + 0.5f);
    fwrite(header, sizeof header, 1, f);
    frames = (int)(s->seconds / DT + 0.5f);
    for (i = 0; i < frames; ++i) {
        nka_update(anim, i ? DT : 0);
        nk_input_begin(&ctx);
        nk_input_end(&ctx);
        s->draw((float)i * DT);
        render(ground);
        fwrite(fb, (size_t)fb_w * (size_t)fb_h * 4, 1, f);
    }
    fclose(f);
    nka_destroy(anim);
    nk_free(&ctx);
    free(fb);
    free(tex);
    return 0;
}
