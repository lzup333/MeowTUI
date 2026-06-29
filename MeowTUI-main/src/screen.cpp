/**
 * @file screen.cpp
 * @brief 双缓冲屏幕实现
 *
 * 本文件实现了 Screen 类的双缓冲渲染机制。
 * 核心策略是比较前后缓冲区，只将差异部分输出到终端。
 * 这能大幅减少 ANSI 转义序列的输出量，提升渲染性能。
 */

#include "meowtui/screen.h"
#include "meowtui/terminal.h"

#include <algorithm>

namespace meowtui {

// ============================================================================
// 初始化与清空
// ============================================================================

void Screen::resize(int w, int h) {
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    width_  = w;
    height_ = h;

    size_t count = static_cast<size_t>(w) * static_cast<size_t>(h);
    back_.assign(count, Cell{});
    front_.assign(count, Cell{});

    clip_ = {0, 0, w, h};
}

void Screen::clear() {
    std::fill(back_.begin(), back_.end(), Cell{});
}

// ============================================================================
// 绘图操作
// ============================================================================

void Screen::set_cell(Col col, Row row, const std::string& ch,
                      Color fg, Color bg, bool bold) {
    if (!in_range(col, row) || !in_clip(col, row)) return;

    auto& cell = back_[index(col, row)];
    cell.ch   = ch.empty() ? " " : ch;
    cell.fg   = fg;
    cell.bg   = bg;
    cell.bold = bold;
}

void Screen::write_str(Col col, Row row, const std::string& text,
                       Color fg, Color bg, bool bold) {
    Col cx = col;
    size_t i = 0;
    while (i < text.size()) {
        if (!in_range(cx, row)) break;

        // 判断 UTF-8 字符的字节数
        unsigned char c = static_cast<unsigned char>(text[i]);
        size_t char_len = 1;
        if ((c & 0x80) == 0) {
            char_len = 1;
        } else if ((c & 0xE0) == 0xC0) {
            char_len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            char_len = 3;
        } else if ((c & 0xF8) == 0xF0) {
            char_len = 4;
        }

        std::string ch = text.substr(i, char_len);

        if (in_clip(cx, row)) {
            auto& cell = back_[index(cx, row)];
            cell.ch   = ch;
            cell.fg   = fg;
            cell.bg   = bg;
            cell.bold = bold;
        }

        i += char_len;
        cx++;
    }
}

void Screen::fill_rect(const Rect& rect, const std::string& ch,
                       Color fg, Color bg, bool bold) {
    Rect area = rect.intersect(clip_);
    if (!area.valid()) return;

    for (Row r = area.y; r < area.y + area.h; ++r) {
        for (Col c = area.x; c < area.x + area.w; ++c) {
            auto& cell = back_[index(c, r)];
            cell.ch   = ch;
            cell.fg   = fg;
            cell.bg   = bg;
            cell.bold = bold;
        }
    }
}

void Screen::draw_border(const Rect& rect,
                         Color fg, Color bg, bool bold) {
    if (!rect.valid()) return;

    Col x1 = rect.x, x2 = rect.x + rect.w - 1;
    Row y1 = rect.y, y2 = rect.y + rect.h - 1;

    set_cell(x1, y1, "+", fg, bg, bold);
    set_cell(x2, y1, "+", fg, bg, bold);
    set_cell(x1, y2, "+", fg, bg, bold);
    set_cell(x2, y2, "+", fg, bg, bold);

    for (Col c = x1 + 1; c < x2; ++c) {
        set_cell(c, y1, "-", fg, bg, bold);
        set_cell(c, y2, "-", fg, bg, bold);
    }

    for (Row r = y1 + 1; r < y2; ++r) {
        set_cell(x1, r, "|", fg, bg, bold);
        set_cell(x2, r, "|", fg, bg, bold);
    }
}

// ============================================================================
// 差异刷新 —— 核心渲染函数
// ============================================================================

/**
 * 通过逐行比较前缓冲区和后缓冲区，找出所有发生变化的单元格，
 * 然后仅将这些变化的单元格输出到终端。
 *
 * 优化策略：
 * 1. 行内连续变化合并输出
 * 2. 跳过完全未变化的整行
 * 3. 仅在需要时移动光标
 */
void Screen::flush() {
    auto& term = Terminal::instance();
    size_t count = static_cast<size_t>(width_) * static_cast<size_t>(height_);

    for (size_t i = 0; i < count; ++i) {
        if (back_[i] != front_[i]) {
            // 找到变化位置，计算坐标
            Row  row = static_cast<Row>(i / width_);
            Col  col = static_cast<Col>(i % width_);

            // 将光标移到变化位置
            term.move_cursor(col, row);

            // 尝试合并输出后续连续变化的单元格
            std::string output;
            size_t j = i;
            Color cur_fg = back_[j].fg;
            Color cur_bg = back_[j].bg;
            bool cur_bold = back_[j].bold;

            while (j < count && back_[j] != front_[j]) {
                // 检查是否需要切换颜色/样式
                if (j == i || back_[j].fg != cur_fg ||
                    back_[j].bg != cur_bg || back_[j].bold != cur_bold) {
                    // 新样式需要先输出已累积的文本
                    if (!output.empty()) {
                        term.write(output);
                        output.clear();
                    }
                    cur_fg   = back_[j].fg;
                    cur_bg   = back_[j].bg;
                    cur_bold = back_[j].bold;

                    // 应用样式
                    if (cur_bold) term.set_bold(true);
                    term.set_fg(cur_fg);
                    term.set_bg(cur_bg);
                }

                output += back_[j].ch;
                ++j;
            }

            // 输出最后累积的文本
            if (!output.empty()) {
                term.write(output);
            }

            // 重置样式
            term.reset_color();

            // 将 i 跳到已处理的最后位置
            i = j - 1;
        }
    }

    // 交换前后缓冲区
    front_.swap(back_);
}

} // namespace meowtui
