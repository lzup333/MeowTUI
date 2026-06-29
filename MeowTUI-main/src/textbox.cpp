/**
 * @file textbox.cpp
 * @brief 文本框组件实现
 *
 * TextBox 支持基本的单行文本编辑功能。
 * 获得焦点时显示光标并处理输入。
 * 支持退格删除和在文本末尾追加字符。
 */

#include "meowtui/textbox.h"
#include "meowtui/terminal.h"

namespace meowtui {

// ============================================================================
// 构造
// ============================================================================

TextBox::TextBox(const std::string& text, size_t max_len)
    : Widget(text), cursor_pos_(text.size()), max_length_(max_len) {}

// ============================================================================
// 渲染
// ============================================================================

/**
 * 文本框外观：
 * 无焦点： [ 当前文本内容 ]
 * 有焦点： [ 当前文本内容| ]  带光标
 *
 * 渲染时在矩形区域内绘制可编辑文本区域。
 */
void TextBox::render(Screen& screen) {
    if (!visible_ || rect_.w <= 0 || rect_.h <= 0) return;

    Row y = rect_.y;

    // 清空文本框区域
    screen.fill_rect(rect_, " ", COLOR_WHITE, focused_ ? COLOR_BLUE : COLOR_BLACK);

    // 确定可显示文本宽度（留一个字符给光标）
    int max_display = rect_.w - 1;
    if (max_display <= 0) return;

    // 如果文本太长，滚动显示
    size_t offset = 0;
    if (cursor_pos_ >= static_cast<size_t>(max_display)) {
        offset = cursor_pos_ - max_display + 1;
    }

    // 提取要显示的部分
    std::string display = text_.substr(offset, static_cast<size_t>(max_display));
    size_t vis_cursor = cursor_pos_ - offset;

    // 显示文本
    Color bg = focused_ ? COLOR_BLUE : COLOR_BLACK;
    screen.write_str(rect_.x, y, display, COLOR_WHITE, bg, focused_);

    // 如果有焦点，绘制光标
    if (focused_) {
        Col cursor_col = rect_.x + static_cast<Col>(vis_cursor);
        if (cursor_col < rect_.x + rect_.w) {
            screen.set_cell(cursor_col, y, "|", COLOR_WHITE, COLOR_CYAN, true);
            // 移动终端真实光标到文本框位置（便于用户感知焦点位置）
            Terminal::instance().move_cursor(cursor_col, y);
            Terminal::instance().show_cursor();
        }
    }

    // 渲染子 Widget
    Widget::render(screen);
}

// ============================================================================
// 事件处理
// ============================================================================

/**
 * 文本框处理以下按键：
 * - 可打印字符：追加到文本中
 * - Backspace：删除光标前一个字符
 * - Enter：触发提交回调
 * - Ctrl+U：清空文本框
 */
bool TextBox::handle_event(const Event& ev) {
    if (!visible_ || disabled()) return false;

    if (ev.type == EventType::KEY && focused_) {
        // 可打印字符输入
        if (ev.is_printable()) {
            if (max_length_ == 0 || text_.size() < max_length_) {
                text_.insert(cursor_pos_, 1, ev.ch);
                cursor_pos_++;
                return true;
            }
        }

        // 退格
        if (ev.key == Key::BACKSPACE && cursor_pos_ > 0) {
            text_.erase(cursor_pos_ - 1, 1);
            cursor_pos_--;
            return true;
        }

        // 删除键
        if (ev.key == Key::DELETE && cursor_pos_ < text_.size()) {
            text_.erase(cursor_pos_, 1);
            return true;
        }

        // 左移光标
        if (ev.key == Key::LEFT && cursor_pos_ > 0) {
            cursor_pos_--;
            return true;
        }

        // 右移光标
        if (ev.key == Key::RIGHT && cursor_pos_ < text_.size()) {
            cursor_pos_++;
            return true;
        }

        // Home / End
        if (ev.key == Key::HOME) {
            cursor_pos_ = 0;
            return true;
        }
        if (ev.key == Key::END) {
            cursor_pos_ = text_.size();
            return true;
        }

        // 提交（Enter）
        if (ev.key == Key::ENTER) {
            if (submit_cb_) {
                submit_cb_(text_);
            }
            return true;
        }

        // 清空（Ctrl+U）
        if (ev.key == Key::CTRL_U) {
            text_.clear();
            cursor_pos_ = 0;
            return true;
        }
    }

    return Widget::handle_event(ev);
}

} // namespace meowtui
