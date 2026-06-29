#ifndef MEOWTUI_LISTBOX_H
#define MEOWTUI_LISTBOX_H

/**
 * @file listbox.h
 * @brief 列表选择框组件 —— 可滚动的选项列表
 *
 * ListBox 提供一个可滚动的选择列表，用户可以通过
 * 方向键（↑/↓）在列表中导航，通过 Enter 选择当前项。
 */

#include "widget.h"
#include <vector>

namespace meowtui {

/**
 * @brief 列表项选中回调类型
 *
 * @param index  选中项的索引（从0开始）
 * @param item   选中项的文本
 */
using ListCallback = std::function<void(size_t index, const std::string& item)>;

/**
 * @brief 列表选择框组件
 *
 * 当选项数量超过显示区域时自动滚动。
 * 支持键盘导航（↑/↓/PgUp/PgDn/Home/End）。
 */
class ListBox : public Widget {
public:
    /**
     * @brief 构造列表选择框
     *
     * @param items   选项列表
     */
    explicit ListBox(const std::vector<std::string>& items = {});

    /** 可以获得焦点 */
    bool focusable() const override { return !disabled(); }

    /** 设置选项列表 */
    void set_items(const std::vector<std::string>& items);

    /** 获取选项列表 */
    const std::vector<std::string>& items() const { return items_; }

    /** 获取当前选中项的索引 */
    size_t selected_index() const { return selected_; }

    /** 获取当前选中项的文本 */
    std::string selected_item() const;

    /**
     * @brief 设置选中回调
     *
     * @param cb  回调函数，参数为 (index, item_text)
     */
    void set_callback(ListCallback cb) { callback_ = std::move(cb); }

    /**
     * @brief 设置选中索引
     *
     * @param index  新的选中索引
     */
    void set_selected(size_t index);

    void render(Screen& screen) override;
    bool handle_event(const Event& ev) override;

private:
    std::vector<std::string> items_;    ///< 所有选项
    size_t selected_       = 0;         ///< 当前选中索引
    size_t scroll_offset_  = 0;         ///< 滚动偏移

    ListCallback callback_;             ///< 选中回调
};

} // namespace meowtui

#endif // MEOWTUI_LISTBOX_H
