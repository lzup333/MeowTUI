/**
 * @file progressbar.cpp
 * @brief 进度条组件实现
 *
 * ProgressBar 以视觉化方式展示 0~100 的进度百分比。
 * 使用 [#####·····] 的样式，并显示百分比数字。
 */

#include "meowtui/progressbar.h"
#include <algorithm>

namespace meowtui {

// ============================================================================
// 构造
// ============================================================================

ProgressBar::ProgressBar(const std::string& text)
    : Widget(text) {}

// ============================================================================
// 设置进度
// ============================================================================

void ProgressBar::set_progress(int p) {
    progress_ = std::max(0, std::min(100, p));
}

// ============================================================================
// 渲染
// ============================================================================

/**
 * 进度条外观：
 * 标签 [████████████░░░░░░░░░] 50%
 *
 * 进度条的填充宽度 = (矩形宽度 - 2 - 5) * progress_ / 100
 * 其中 2 是边框 []，5 是百分比数字 " 100%"
 */
void ProgressBar::render(Screen& screen) {
    if (!visible_ || rect_.w <= 0 || rect_.h <= 0) return;

    Row y = rect_.y;

    // 如果没有标签，直接显示进度条
    Col current_x = rect_.x;
    std::string label_prefix;
    if (!text_.empty()) {
        label_prefix = text_ + " ";
    }

    // 显示标签
    if (!label_prefix.empty()) {
        screen.write_str(current_x, y, label_prefix, COLOR_WHITE, COLOR_BLACK);
        current_x += static_cast<Col>(label_prefix.size());
    }

    // 计算进度条可用的宽度（边框2 + 百分比数字最多4字符 + 空格）
    int bar_width = rect_.w - static_cast<int>(label_prefix.size()) - 6;
    if (bar_width < 2) bar_width = 2;

    // 左边框
    screen.set_cell(current_x, y, "[", COLOR_WHITE, COLOR_BLACK);
    current_x++;

    // 填充区域
    int fill_count = (bar_width - 2) * progress_ / 100;
    for (int i = 0; i < bar_width - 2; ++i) {
        std::string ch(1, (i < fill_count) ? fill_char_ : '.');
        Color fg = (i < fill_count) ? static_cast<Color>(fill_color_) : static_cast<Color>(COLOR_WHITE);
        screen.set_cell(current_x + i, y, ch, fg, COLOR_BLACK);
    }
    current_x += bar_width - 2;

    // 右边框
    screen.set_cell(current_x, y, "]", COLOR_WHITE, COLOR_BLACK);
    current_x++;

    // 百分比数字
    std::string pct = std::to_string(progress_) + "%";
    screen.write_str(current_x, y, pct, COLOR_GREEN, COLOR_BLACK, true);

    // 渲染子 Widget
    Widget::render(screen);
}

} // namespace meowtui
