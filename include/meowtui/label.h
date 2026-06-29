#ifndef MEOWTUI_LABEL_H
#define MEOWTUI_LABEL_H

/**
 * @file label.h
 * @brief 标签组件 —— 显示静态文本
 *
 * Label 是最简单的 Widget，用于显示一行或多行文本。
 * 支持对齐方式（左对齐、居中、右对齐）。
 */

#include "widget.h"

namespace meowtui {

/**
 * @brief 文本标签组件
 *
 * 用于在界面上显示静态文字。不可获得焦点。
 */
class Label : public Widget {
public:
    /**
     * @brief 构造标签
     *
     * @param text   显示的文本
     * @param align  对齐方式
     */
    explicit Label(const std::string& text = "",
                   Align align = Align::LEFT);

    /** 设置对齐方式 */
    void set_align(Align a) { align_ = a; }

    /** 获取对齐方式 */
    Align align() const { return align_; }

    /** Label 不可获得焦点 */
    bool focusable() const override { return false; }

    void render(Screen& screen) override;

private:
    Align align_ = Align::LEFT;
};

} // namespace meowtui

#endif // MEOWTUI_LABEL_H
