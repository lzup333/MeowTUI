#ifndef MEOWTUI_PROGRESSBAR_H
#define MEOWTUI_PROGRESSBAR_H

/**
 * @file progressbar.h
 * @brief 进度条组件 —— 显示任务进度
 *
 * ProgressBar 以视觉化方式展示 0~100 的进度百分比。
 * 支持自定义填充字符和颜色。
 */

#include "widget.h"

namespace meowtui {

/**
 * @brief 进度条组件
 *
 * 使用等宽字符填充来表示进度百分比。
 * 例如：50% 进度 -> [████████████████████░░░░░░░░░░░░░░░░░░░░░░]
 */
class ProgressBar : public Widget {
public:
    /**
     * @brief 构造进度条
     *
     * @param text   进度条标签文本
     */
    explicit ProgressBar(const std::string& text = "");

    /** 设置进度值（0~100） */
    void set_progress(int p);

    /** 获取当前进度值 */
    int progress() const { return progress_; }

    /** 设置填充字符 */
    void set_fill_char(char c) { fill_char_ = c; }

    /** 设置填充颜色 */
    void set_fill_color(Color c) { fill_color_ = c; }

    /** 不可获得焦点 */
    bool focusable() const override { return false; }

    void render(Screen& screen) override;

private:
    int   progress_   = 0;       ///< 当前进度（0~100）
    char  fill_char_  = '#';     ///< 填充字符
    Color fill_color_ = COLOR_GREEN;  ///< 填充颜色
};

} // namespace meowtui

#endif // MEOWTUI_PROGRESSBAR_H
