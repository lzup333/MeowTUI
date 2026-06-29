#ifndef MEOWTUI_WIDGET_H
#define MEOWTUI_WIDGET_H

/**
 * @file widget.h
 * @brief Widget 基类与容器 —— 所有 UI 组件的根
 *
 * MeowTUI 的 Widget 体系采用组合模式（Composite Pattern）：
 * - 每个 Widget 都可以有子 Widget（children）
 * - 每个 Widget 只有一个父 Widget（parent）
 * - 渲染时自动按父子关系递归绘制
 *
 * Widget 生命周期：
 * 1. 构造（设置位置、大小、属性）
 * 2. 添加到父容器（set_parent / add_child）
 * 3. 渲染（render -> children render）
 * 4. 事件处理（handle_event）
 * 5. 销毁（析构时自动从父容器移除）
 */

#include "common.h"
#include "rect.h"
#include "event.h"
#include "screen.h"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace meowtui {

// 前向声明
class App;

/**
 * @brief Widget 状态标志
 */
enum class WidgetState : uint8_t {
    NORMAL    = 0,     ///< 正常状态
    FOCUSED   = 1,     ///< 拥有焦点
    HOVERED   = 2,     ///< 鼠标悬停（预留）
    DISABLED  = 3,     ///< 禁用
};

/**
 * @brief 所有 UI 组件的基类
 *
 * Widget 是一个抽象基类，提供了：
 * - 位置和尺寸管理
 * - 父子关系管理
 * - 焦点管理
 * - 事件分发
 * - 渲染生命周期
 */
class Widget {
public:
    /**
     * @brief 构造 Widget
     *
     * @param text  初始文本（可选，各子类自行解释）
     */
    explicit Widget(const std::string& text = "");
    virtual ~Widget();

    // 禁止拷贝
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    // ── 父子关系 ────────────────────────────────────────────────────────

    /** 设置父 Widget */
    void set_parent(Widget* parent) { parent_ = parent; }

    /** 获取父 Widget */
    Widget* parent() const { return parent_; }

    /** 添加子 Widget */
    void add_child(std::unique_ptr<Widget> child);

    /** 获取子 Widget 列表 */
    const std::vector<std::unique_ptr<Widget>>& children() const { return children_; }

    /** 获取指定位置的子 Widget（不会转移所有权） */
    Widget* child_at(size_t index) const;

    /** 移除所有子 Widget */
    void clear_children();

    // ── 位置与尺寸 ──────────────────────────────────────────────────────

    /** 设置 Widget 的矩形区域 */
    void set_rect(const Rect& r) { rect_ = r; }

    /** 获取 Widget 的矩形区域 */
    const Rect& rect() const { return rect_; }

    /** 设置相对位置（相对于父容器） */
    void set_position(Col x, Row y) { rect_.x = x; rect_.y = y; }

    /** 设置尺寸 */
    void set_size(int w, int h) { rect_.w = w; rect_.h = h; }

    // ── 焦点管理 ────────────────────────────────────────────────────────

    /** 设置焦点状态 */
    virtual void set_focused(bool focused);

    /** 判断是否拥有焦点 */
    bool is_focused() const { return focused_; }

    /** 判断是否可以获取焦点（子类可重写） */
    virtual bool focusable() const { return false; }

    // ── 文本 ────────────────────────────────────────────────────────────

    /** 设置 Widget 文本 */
    virtual void set_text(const std::string& text) { text_ = text; }

    /** 获取 Widget 文本 */
    const std::string& text() const { return text_; }

    // ── 渲染与事件 ──────────────────────────────────────────────────────

    /**
     * @brief 渲染 Widget 到屏幕缓冲区
     *
     * 这是主要绘制方法。基类实现会遍历子 Widget 递归渲染。
     * 子类应重写此方法以实现自定义绘制。
     *
     * @param screen  目标屏幕缓冲区
     */
    virtual void render(Screen& screen);

    /**
     * @brief 处理事件
     *
     * 基类实现会将事件分发给所有子 Widget。
     * 子类可重写此方法处理特定事件。
     *
     * @param ev  事件
     * @return true  表示已处理，事件不再继续传递
     */
    virtual bool handle_event(const Event& ev);

    // ── 可见性 ──────────────────────────────────────────────────────────

    /** 设置可见性 */
    void set_visible(bool v) { visible_ = v; }

    /** 判断是否可见 */
    bool visible() const { return visible_; }

    // ── 状态 ────────────────────────────────────────────────────────────

    /** 设置 Widget 状态 */
    void set_state(WidgetState s) { state_ = s; }

    /** 获取 Widget 状态 */
    WidgetState state() const { return state_; }

    /** 判断是否被禁用 */
    bool disabled() const { return state_ == WidgetState::DISABLED; }

    // ── 标识 ────────────────────────────────────────────────────────────

    /** 设置 Widget ID（用于查找和调试） */
    void set_id(const std::string& id) { id_ = id; }

    /** 获取 Widget ID */
    const std::string& id() const { return id_; }

protected:
    std::string text_;                          ///< 显示的文本
    Rect rect_;                                 ///< 位置和尺寸
    Widget* parent_ = nullptr;                  ///< 父 Widget 指针（不拥有所有权）
    std::vector<std::unique_ptr<Widget>> children_; ///< 子 Widget
    bool focused_    = false;                   ///< 是否拥有焦点
    bool visible_    = true;                    ///< 是否可见
    WidgetState state_ = WidgetState::NORMAL;   ///< 当前状态
    std::string id_;                            ///< 标识符
};

} // namespace meowtui

#endif // MEOWTUI_WIDGET_H
