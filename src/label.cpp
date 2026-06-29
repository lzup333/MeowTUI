/**
 * @file label.cpp
 * @brief 标签组件实现
 *
 * Label 是最简单的 Widget，用于在指定位置显示文本。
 * 支持左对齐、居中对齐和右对齐三种方式。
 */

#include "meowtui/label.h"

namespace meowtui {

// ============================================================================
// 构造
// ============================================================================

Label::Label(const std::string& text, Align align)
    : Widget(text), align_(align) {}

// ============================================================================
// 渲染
// ============================================================================

/**
 * 根据对齐方式计算文本的起始列，然后写入屏幕。
 *
 * 左对齐：从矩形左边缘开始
 * 居中对齐：计算文本宽度，居中放置
 * 右对齐：从矩形右边缘对齐
 */
void Label::render(Screen& screen) {
    if (!visible_ || rect_.w <= 0 || rect_.h <= 0) return;

    // 计算文本的实际显示宽度（只截取矩形宽度范围内的部分）
    int text_w = static_cast<int>(text_.size());
    if (text_w > rect_.w) text_w = rect_.w;

    // 根据对齐方式计算起始列
    Col start_x = rect_.x;
    if (align_ == Align::CENTER) {
        start_x = rect_.x + (rect_.w - text_w) / 2;
    } else if (align_ == Align::RIGHT) {
        start_x = rect_.x + rect_.w - text_w;
    }

    // 在矩形第一行写入文本
    screen.write_str(start_x, rect_.y,
                     text_.substr(0, static_cast<size_t>(text_w)),
                     COLOR_WHITE, COLOR_BLACK);

    // 继续渲染子 Widget
    Widget::render(screen);
}

} // namespace meowtui
