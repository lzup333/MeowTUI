#ifndef MEOWTUI_BUTTON_H
#define MEOWTUI_BUTTON_H

/**
 * @file button.h
 * @brief 按钮组件 —— 可点击的可交互控件
 *
 * Button 是基础交互组件，支持：
 * - 键盘操作（Enter/Space 触发点击）
 * - 焦点高亮
 * - 按下反馈
 * - 点击回调（std::function）
 */

#include "widget.h"

namespace meowtui {

/**
 * @brief 按钮点击回调类型
 */
using ButtonCallback = std::function<void()>;

/**
 * @brief 按钮组件
 *
 * 可获取焦点，当用户按下 Enter 或 Space 时触发回调。
 * 获得焦点时会以高亮样式显示。
 */
class Button : public Widget {
public:
    /**
     * @brief 构造按钮
     *
     * @param text    按钮上显示的文本
     * @param cb      点击回调函数（可选）
     */
    explicit Button(const std::string& text = "",
                    ButtonCallback cb = nullptr);

    /** 设置点击回调 */
    void set_callback(ButtonCallback cb) { callback_ = std::move(cb); }

    /** 按钮可以获得焦点 */
    bool focusable() const override { return !disabled(); }

    void render(Screen& screen) override;
    bool handle_event(const Event& ev) override;

private:
    ButtonCallback callback_;   ///< 点击时调用的回调函数
    bool pressed_ = false;      ///< 按钮是否处于按下状态
};

} // namespace meowtui

#endif // MEOWTUI_BUTTON_H
