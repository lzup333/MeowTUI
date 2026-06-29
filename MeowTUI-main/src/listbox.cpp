/**
 * @file listbox.cpp
 * @brief 列表选择框组件实现
 *
 * ListBox 显示一个可滚动的选项列表。
 * 使用方向键（↑/↓）导航，Enter 确认选择。
 * 当选项超出显示区域时自动滚动。
 */

#include "meowtui/listbox.h"

namespace meowtui {

// ============================================================================
// 构造
// ============================================================================

ListBox::ListBox(const std::vector<std::string>& items)
    : Widget(""), items_(items) {}

// ============================================================================
// 选项管理
// ============================================================================

void ListBox::set_items(const std::vector<std::string>& items) {
    items_ = items;
    if (selected_ >= items_.size() && !items_.empty()) {
        selected_ = 0;
    }
    scroll_offset_ = 0;
}

std::string ListBox::selected_item() const {
    if (selected_ < items_.size()) {
        return items_[selected_];
    }
    return "";
}

void ListBox::set_selected(size_t index) {
    if (index < items_.size()) {
        selected_ = index;
    }
}

// ============================================================================
// 渲染
// ============================================================================

/**
 * 列表选择框外观：
 *
 * ┌─────────────┐
 * │ 选项 1       │
 * │>选项 2       │  ← 选中项以 > 标记
 * │ 选项 3       │
 * └─────────────┘
 *
 * 选中项高亮显示，超出区域时自动滚动。
 */
void ListBox::render(Screen& screen) {
    if (!visible_ || rect_.w <= 1 || rect_.h <= 1) return;

    // 绘制边框
    screen.draw_border(rect_, COLOR_WHITE, COLOR_BLACK, false);

    // 内容区域（去掉边框）
    Col  content_x = rect_.x + 1;
    Row  content_y = rect_.y + 1;
    int  content_w = rect_.w - 2;
    int  content_h = rect_.h - 2;

    if (content_w <= 0 || content_h <= 0) return;

    // 清空内容区域
    screen.fill_rect({content_x, content_y, content_w, content_h},
                     " ", COLOR_WHITE, COLOR_BLACK);

    // 调整滚动偏移，确保选中项可见
    if (selected_ < scroll_offset_) {
        scroll_offset_ = selected_;
    }
    if (selected_ >= scroll_offset_ + static_cast<size_t>(content_h)) {
        scroll_offset_ = selected_ - content_h + 1;
    }

    // 渲染可见的选项
    for (int i = 0; i < content_h; ++i) {
        size_t idx = scroll_offset_ + i;
        if (idx >= items_.size()) break;

        bool is_selected = (idx == selected_);

        // 选中标记
        std::string prefix = is_selected ? "> " : "  ";

        // 显示文本
        std::string display = prefix + items_[idx];
        if (static_cast<int>(display.size()) > content_w) {
            display = display.substr(0, static_cast<size_t>(content_w));
        }

        Color fg = is_selected ? COLOR_BLACK : COLOR_WHITE;
        Color bg = is_selected ? COLOR_CYAN  : COLOR_BLACK;

        screen.write_str(content_x, content_y + i, display, fg, bg, is_selected);

        // 用空格填充剩余区域
        int remain = content_w - static_cast<int>(display.size());
        if (remain > 0) {
            screen.fill_rect({content_x + static_cast<Col>(display.size()),
                              content_y + i, remain, 1},
                             " ", fg, bg);
        }
    }

    // 渲染子 Widget
    Widget::render(screen);
}

// ============================================================================
// 事件处理
// ============================================================================

/**
 * 列表选择框处理以下按键：
 * - ↑/↓：上下移动选中项
 * - PgUp/PgDn：快速翻页
 * - Home/End：跳到首/尾
 * - Enter：确认选择，触发回调
 */
bool ListBox::handle_event(const Event& ev) {
    if (!visible_ || disabled() || items_.empty()) return false;

    if (ev.type == EventType::KEY && focused_) {
        switch (ev.key) {
            case Key::UP:
                if (selected_ > 0) {
                    selected_--;
                }
                return true;

            case Key::DOWN:
                if (selected_ + 1 < items_.size()) {
                    selected_++;
                }
                return true;

            case Key::PAGE_UP: {
                int step = rect_.h - 2;  // 翻一页
                if (step < 1) step = 1;
                if (selected_ >= static_cast<size_t>(step))
                    selected_ -= step;
                else
                    selected_ = 0;
                return true;
            }

            case Key::PAGE_DOWN: {
                int step = rect_.h - 2;
                if (step < 1) step = 1;
                size_t target = selected_ + step;
                if (target < items_.size())
                    selected_ = target;
                else
                    selected_ = items_.size() - 1;
                return true;
            }

            case Key::HOME:
                selected_ = 0;
                return true;

            case Key::END:
                selected_ = items_.size() - 1;
                return true;

            case Key::ENTER:
                if (callback_) {
                    callback_(selected_, items_[selected_]);
                }
                return true;

            default:
                break;
        }
    }

    return Widget::handle_event(ev);
}

} // namespace meowtui
