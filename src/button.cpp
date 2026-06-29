/**
 * @file button.cpp
 * @brief 按钮组件实现
 *
 * Button 是基本交互组件，外观为 [ 按钮文本 ] 样式。
 * 获得焦点时使用高亮色（反转/粗体）显示。
 * 按下 Enter 或 Space 时触发回调函数。
 */

#include "meowtui/button.h"

namespace meowtui {

// ============================================================================
// 构造
// ============================================================================

Button::Button(const std::string& text, ButtonCallback cb)
    : Widget(text), callback_(std::move(cb)) {}

// ============================================================================
// 渲染
// ============================================================================

/**
 * 按钮外观：
 * 普通状态：  [ 按钮文本 ]
 * 焦点状态：  < 按钮文本 >  （高亮色）
 * 按下状态：  [ 按钮文本 ]  （反转色）
 */
void Button::render(Screen& screen) {
    if (!visible_ || rect_.w <= 0 || rect_.h <= 0) return;

    bool active = focused_ || pressed_;

    // 绘制按钮边框：使用 < > 或 [ ]
    char left_bracket  = active ? '<' : '[';
    char right_bracket = active ? '>' : ']';

    // 计算文本在按钮内部的偏移
    std::string display_text = " " + text_ + " ";
    int text_w = static_cast<int>(display_text.size());
    if (text_w > rect_.w - 2) {
        text_w = rect_.w - 2;
        display_text = display_text.substr(0, static_cast<size_t>(text_w));
    }

    // 居中显示
    Col start_x = rect_.x + (rect_.w - text_w - 2) / 2;
    Row y = rect_.y;

    // 焦点/按下时的颜色
    Color fg = active ? COLOR_BLACK : COLOR_WHITE;
    Color bg = active ? COLOR_CYAN  : COLOR_BLACK;

    // 绘制左括号
    screen.set_cell(start_x, y, std::string(1, left_bracket), fg, bg, active);
    // 绘制文本
    screen.write_str(start_x + 1, y, display_text, fg, bg, active);
    // 绘制右括号
    screen.set_cell(start_x + 1 + text_w, y, std::string(1, right_bracket), fg, bg, active);

    // 用空格填充按钮剩余区域（防止之前的内容残留）
    int remaining = rect_.w - (text_w + 2);
    if (remaining > 0) {
        screen.fill_rect({start_x + 2 + text_w, y, remaining, 1},
                         " ", fg, bg, false);
    }

    // 渲染子 Widget
    Widget::render(screen);
}

// ============================================================================
// 事件处理
// ============================================================================

/**
 * 按钮响应 Enter 和 Space 键。
 * 按下时设置 pressed_ 标志，释放时触发回调。
 */
bool Button::handle_event(const Event& ev) {
    if (!visible_ || disabled()) return false;

    if (ev.type == EventType::KEY && focused_) {
        // Enter 或 Space → 触发点击回调
        if (ev.key == Key::ENTER || ev.ch == ' ') {
            pressed_ = true;
            if (callback_) {
                callback_();
            }
            return true;
        }
    } else {
        // 不是焦点状态时，重置 pressed 状态
        pressed_ = false;
    }

    // 如果未处理，分发给子 Widget
    return Widget::handle_event(ev);
}

} // namespace meowtui
