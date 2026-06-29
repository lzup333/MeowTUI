#ifndef MEOWTUI_TEXTBOX_H
#define MEOWTUI_TEXTBOX_H

/**
 * @file textbox.h
 * @brief 文本框组件 —— 单行文本输入
 *
 * TextBox 提供基本的单行文本编辑功能：
 * - 字符输入（可打印字符）
 * - 退格删除
 * - 光标显示与移动
 * - 焦点管理
 * - 文本内容获取
 */

#include "widget.h"

namespace meowtui {

/**
 * @brief 单行文本框
 *
 * 可获取焦点，用户可在其中输入和编辑文本。
 * 获得焦点时会显示光标。
 */
class TextBox : public Widget {
public:
    /**
     * @brief 构造文本框
     *
     * @param text       初始文本
     * @param max_len    最大字符数（0 表示不限制）
     */
    explicit TextBox(const std::string& text = "",
                     size_t max_len = 0);

    /** 文本框可以获得焦点 */
    bool focusable() const override { return !disabled(); }

    /**
     * @brief 设置输入完成回调
     *
     * @param cb  回调函数，参数为当前输入的文本
     */
    void set_submit_callback(std::function<void(const std::string&)> cb) {
        submit_cb_ = std::move(cb);
    }

    /** 设置最大长度 */
    void set_max_length(size_t max) { max_length_ = max; }

    /** 获取最大长度 */
    size_t max_length() const { return max_length_; }

    void render(Screen& screen) override;
    bool handle_event(const Event& ev) override;

private:
    size_t cursor_pos_ = 0;     ///< 光标在文本中的位置
    size_t max_length_ = 0;     ///< 最大字符数（0=不限制）
    std::function<void(const std::string&)> submit_cb_;  ///< 提交回调
};

} // namespace meowtui

#endif // MEOWTUI_TEXTBOX_H
