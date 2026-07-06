#pragma once

#include <lvgl.h>
#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace lv
{

    class StyleWrapper
    {
        static_assert(std::is_trivially_copyable_v<lv_style_t>,
                      "lv_style_t is no longer trivially copyable. "
                      "See Style class @note for fix strategy: use lv_style_copy() or "
                      "iterate properties with lv_style_get_prop()/lv_style_set_prop().");

    private:
        lv_style_t *m_style;

    public:
        ~StyleWrapper() noexcept
        {
        }

        StyleWrapper(lv_style_t *style) noexcept
        {
            m_style = style;
        }

        /// Get pointer to underlying style (for LVGL API)
        [[nodiscard]] lv_style_t *get() noexcept { return m_style; }
        [[nodiscard]] const lv_style_t *get() const noexcept { return m_style; }

        /// Implicit conversion for C API interop
        [[nodiscard]] operator lv_style_t *() noexcept { return m_style; }
        [[nodiscard]] operator const lv_style_t *() const noexcept { return m_style; }

        /// Reset style to empty state
        StyleWrapper &reset() noexcept
        {
            lv_style_reset(m_style);
            lv_style_init(m_style);
            return *this;
        }

        // ==================== Background ====================

        StyleWrapper &bg_color(lv_color_t color) noexcept
        {
            lv_style_set_bg_color(m_style, color);
            return *this;
        }

        StyleWrapper &bg_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_bg_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &bg_grad_color(lv_color_t color) noexcept
        {
            lv_style_set_bg_grad_color(m_style, color);
            return *this;
        }

        StyleWrapper &bg_grad_dir(lv_grad_dir_t dir) noexcept
        {
            lv_style_set_bg_grad_dir(m_style, dir);
            return *this;
        }

        StyleWrapper &bg_main_stop(int32_t stop) noexcept
        {
            lv_style_set_bg_main_stop(m_style, stop);
            return *this;
        }

        StyleWrapper &bg_grad_stop(int32_t stop) noexcept
        {
            lv_style_set_bg_grad_stop(m_style, stop);
            return *this;
        }

        // ==================== Border ====================

        StyleWrapper &border_color(lv_color_t color) noexcept
        {
            lv_style_set_border_color(m_style, color);
            return *this;
        }

        StyleWrapper &border_width(int32_t width) noexcept
        {
            lv_style_set_border_width(m_style, width);
            return *this;
        }

        StyleWrapper &border_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_border_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &border_side(lv_border_side_t side) noexcept
        {
            lv_style_set_border_side(m_style, side);
            return *this;
        }

        // ==================== Outline ====================

        StyleWrapper &outline_color(lv_color_t color) noexcept
        {
            lv_style_set_outline_color(m_style, color);
            return *this;
        }

        StyleWrapper &outline_width(int32_t width) noexcept
        {
            lv_style_set_outline_width(m_style, width);
            return *this;
        }

        StyleWrapper &outline_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_outline_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &outline_pad(int32_t pad) noexcept
        {
            lv_style_set_outline_pad(m_style, pad);
            return *this;
        }

        // ==================== Shadow ====================

        StyleWrapper &shadow_color(lv_color_t color) noexcept
        {
            lv_style_set_shadow_color(m_style, color);
            return *this;
        }

        StyleWrapper &shadow_width(int32_t width) noexcept
        {
            lv_style_set_shadow_width(m_style, width);
            return *this;
        }

        StyleWrapper &shadow_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_shadow_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &shadow_offset(int32_t x, int32_t y) noexcept
        {
            lv_style_set_shadow_offset_x(m_style, x);
            lv_style_set_shadow_offset_y(m_style, y);
            return *this;
        }

        StyleWrapper &shadow_spread(int32_t spread) noexcept
        {
            lv_style_set_shadow_spread(m_style, spread);
            return *this;
        }

        // ==================== Padding ====================

        StyleWrapper &pad_all(int32_t pad) noexcept
        {
            lv_style_set_pad_all(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_top(int32_t pad) noexcept
        {
            lv_style_set_pad_top(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_bottom(int32_t pad) noexcept
        {
            lv_style_set_pad_bottom(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_left(int32_t pad) noexcept
        {
            lv_style_set_pad_left(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_right(int32_t pad) noexcept
        {
            lv_style_set_pad_right(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_hor(int32_t pad) noexcept
        {
            lv_style_set_pad_hor(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_ver(int32_t pad) noexcept
        {
            lv_style_set_pad_ver(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_row(int32_t pad) noexcept
        {
            lv_style_set_pad_row(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_column(int32_t pad) noexcept
        {
            lv_style_set_pad_column(m_style, pad);
            return *this;
        }

        StyleWrapper &pad_gap(int32_t pad) noexcept
        {
            lv_style_set_pad_row(m_style, pad);
            lv_style_set_pad_column(m_style, pad);
            return *this;
        }

        // ==================== Margin ====================

        StyleWrapper &margin_all(int32_t margin) noexcept
        {
            lv_style_set_margin_top(m_style, margin);
            lv_style_set_margin_bottom(m_style, margin);
            lv_style_set_margin_left(m_style, margin);
            lv_style_set_margin_right(m_style, margin);
            return *this;
        }

        StyleWrapper &margin_top(int32_t margin) noexcept
        {
            lv_style_set_margin_top(m_style, margin);
            return *this;
        }

        StyleWrapper &margin_bottom(int32_t margin) noexcept
        {
            lv_style_set_margin_bottom(m_style, margin);
            return *this;
        }

        StyleWrapper &margin_left(int32_t margin) noexcept
        {
            lv_style_set_margin_left(m_style, margin);
            return *this;
        }

        StyleWrapper &margin_right(int32_t margin) noexcept
        {
            lv_style_set_margin_right(m_style, margin);
            return *this;
        }

        // ==================== Size ====================

        StyleWrapper &width(int32_t w) noexcept
        {
            lv_style_set_width(m_style, w);
            return *this;
        }

        StyleWrapper &min_width(int32_t w) noexcept
        {
            lv_style_set_min_width(m_style, w);
            return *this;
        }

        StyleWrapper &max_width(int32_t w) noexcept
        {
            lv_style_set_max_width(m_style, w);
            return *this;
        }

        StyleWrapper &height(int32_t h) noexcept
        {
            lv_style_set_height(m_style, h);
            return *this;
        }

        StyleWrapper &min_height(int32_t h) noexcept
        {
            lv_style_set_min_height(m_style, h);
            return *this;
        }

        StyleWrapper &max_height(int32_t h) noexcept
        {
            lv_style_set_max_height(m_style, h);
            return *this;
        }

        // ==================== Appearance ====================

        StyleWrapper &radius(int32_t r) noexcept
        {
            lv_style_set_radius(m_style, r);
            return *this;
        }

        StyleWrapper &opa(lv_opa_t opa) noexcept
        {
            lv_style_set_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &clip_corner(bool clip) noexcept
        {
            lv_style_set_clip_corner(m_style, clip);
            return *this;
        }

        StyleWrapper &blend_mode(lv_blend_mode_t mode) noexcept
        {
            lv_style_set_blend_mode(m_style, mode);
            return *this;
        }

        // ==================== Text ====================

        StyleWrapper &text_color(lv_color_t color) noexcept
        {
            lv_style_set_text_color(m_style, color);
            return *this;
        }

        StyleWrapper &text_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_text_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &text_font(const lv_font_t *font) noexcept
        {
            lv_style_set_text_font(m_style, font);
            return *this;
        }

        StyleWrapper &text_letter_space(int32_t space) noexcept
        {
            lv_style_set_text_letter_space(m_style, space);
            return *this;
        }

        StyleWrapper &text_line_space(int32_t space) noexcept
        {
            lv_style_set_text_line_space(m_style, space);
            return *this;
        }

        StyleWrapper &text_decor(lv_text_decor_t decor) noexcept
        {
            lv_style_set_text_decor(m_style, decor);
            return *this;
        }

        StyleWrapper &text_align(lv_text_align_t align) noexcept
        {
            lv_style_set_text_align(m_style, align);
            return *this;
        }

        // ==================== Image ====================

        StyleWrapper &image_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_image_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &image_recolor(lv_color_t color) noexcept
        {
            lv_style_set_image_recolor(m_style, color);
            return *this;
        }

        StyleWrapper &image_recolor_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_image_recolor_opa(m_style, opa);
            return *this;
        }

        // ==================== Line ====================

        StyleWrapper &line_color(lv_color_t color) noexcept
        {
            lv_style_set_line_color(m_style, color);
            return *this;
        }

        StyleWrapper &line_width(int32_t width) noexcept
        {
            lv_style_set_line_width(m_style, width);
            return *this;
        }

        StyleWrapper &line_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_line_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &line_rounded(bool rounded) noexcept
        {
            lv_style_set_line_rounded(m_style, rounded);
            return *this;
        }

        // ==================== Arc ====================

        StyleWrapper &arc_color(lv_color_t color) noexcept
        {
            lv_style_set_arc_color(m_style, color);
            return *this;
        }

        StyleWrapper &arc_width(int32_t width) noexcept
        {
            lv_style_set_arc_width(m_style, width);
            return *this;
        }

        StyleWrapper &arc_opa(lv_opa_t opa) noexcept
        {
            lv_style_set_arc_opa(m_style, opa);
            return *this;
        }

        StyleWrapper &arc_rounded(bool rounded) noexcept
        {
            lv_style_set_arc_rounded(m_style, rounded);
            return *this;
        }

        // ==================== Transform ====================

        StyleWrapper &transform_width(int32_t width) noexcept
        {
            lv_style_set_transform_width(m_style, width);
            return *this;
        }

        StyleWrapper &transform_height(int32_t height) noexcept
        {
            lv_style_set_transform_height(m_style, height);
            return *this;
        }

        StyleWrapper &transform_scale(int32_t scale) noexcept
        {
            lv_style_set_transform_scale(m_style, scale);
            return *this;
        }

        StyleWrapper &transform_scale_x(int32_t scale) noexcept
        {
            lv_style_set_transform_scale_x(m_style, scale);
            return *this;
        }

        StyleWrapper &transform_scale_y(int32_t scale) noexcept
        {
            lv_style_set_transform_scale_y(m_style, scale);
            return *this;
        }

        StyleWrapper &transform_rotation(int32_t angle) noexcept
        {
            lv_style_set_transform_rotation(m_style, angle);
            return *this;
        }

        StyleWrapper &transform_pivot(int32_t x, int32_t y) noexcept
        {
            lv_style_set_transform_pivot_x(m_style, x);
            lv_style_set_transform_pivot_y(m_style, y);
            return *this;
        }

        // ==================== Layout ====================

        StyleWrapper &layout(uint16_t layout) noexcept
        {
            lv_style_set_layout(m_style, layout);
            return *this;
        }

#if LV_USE_FLEX
        StyleWrapper &flex_flow(lv_flex_flow_t flow) noexcept
        {
            lv_style_set_flex_flow(m_style, flow);
            return *this;
        }

        StyleWrapper &flex_main_place(lv_flex_align_t place) noexcept
        {
            lv_style_set_flex_main_place(m_style, place);
            return *this;
        }

        StyleWrapper &flex_cross_place(lv_flex_align_t place) noexcept
        {
            lv_style_set_flex_cross_place(m_style, place);
            return *this;
        }

        StyleWrapper &flex_track_place(lv_flex_align_t place) noexcept
        {
            lv_style_set_flex_track_place(m_style, place);
            return *this;
        }

        StyleWrapper &flex_grow(uint8_t grow) noexcept
        {
            lv_style_set_flex_grow(m_style, grow);
            return *this;
        }
#endif // LV_USE_FLEX

        // ==================== Alignment ====================

        StyleWrapper &align(lv_align_t align) noexcept
        {
            lv_style_set_align(m_style, align);
            return *this;
        }

        // ==================== Transform ====================

        StyleWrapper &translate_x(int32_t x) noexcept
        {
            lv_style_set_translate_x(m_style, x);
            return *this;
        }

        StyleWrapper &translate_y(int32_t y) noexcept
        {
            lv_style_set_translate_y(m_style, y);
            return *this;
        }
    };

    // Static assertion to verify Style wraps lv_style_t without overhead
    static_assert(sizeof(Style) == sizeof(lv_style_t),
                  "Style must be exactly lv_style_t-sized for zero overhead");

} // namespace lv
