/**
 * @file widget.cpp
 * @brief Widget 基类实现
 *
 * 本文件实现了 Widget 的公共行为：
 * - 父子关系管理
 * - 焦点管理
 * - 默认渲染（递归遍历子 Widget）
 * - 默认事件处理（分发给子 Widget）
 */

#include "meowtui/widget.h"

namespace meowtui {

// ============================================================================
// 构造与析构
// ============================================================================

Widget::Widget(const std::string& text)
    : text_(text) {}

Widget::~Widget() {
    // 从父 Widget 的 children 列表中移除自己
    // unique_ptr 会自动释放所有子 Widget
}

// ============================================================================
// 子 Widget 管理
// ============================================================================

void Widget::add_child(std::unique_ptr<Widget> child) {
    if (child) {
        child->set_parent(this);
        children_.push_back(std::move(child));
    }
}

Widget* Widget::child_at(size_t index) const {
    if (index < children_.size()) {
        return children_[index].get();
    }
    return nullptr;
}

void Widget::clear_children() {
    children_.clear();
}

// ============================================================================
// 焦点管理
// ============================================================================

void Widget::set_focused(bool focused) {
    focused_ = focused;
}

// ============================================================================
// 渲染
// ============================================================================

/**
 * 基类的 render() 方法：
 * 1. 裁剪区域设为 Widget 自身矩形
 * 2. 递归渲染所有可见的子 Widget
 * 3. 恢复裁剪区域
 *
 * 子类重写 render() 时应：
 * - 先绘制自身内容
 * - 然后调用 Widget::render() 来渲染子 Widget
 */
void Widget::render(Screen& screen) {
    if (!visible_) return;

    // 保存原有裁剪区域，设置新的裁剪区域为当前 Widget 的矩形
    Rect old_clip = screen.clip_rect();
    screen.set_clip_rect(rect_);

    // 递归渲染所有子 Widget
    for (auto& child : children_) {
        if (child->visible()) {
            child->render(screen);
        }
    }

    // 恢复裁剪区域
    screen.set_clip_rect(old_clip);
}

// ============================================================================
// 事件处理
// ============================================================================

/**
 * 基类的 handle_event() 方法将事件分发给子 Widget。
 * 分发顺序为：从上到下遍历子 Widget，优先让子 Widget 处理。
 * 如果某个子 Widget 处理了事件（返回 true），则停止分发。
 *
 * 子类重写 handle_event() 时应：
 * - 先处理自身逻辑
 * - 可调用 Widget::handle_event() 分发给子 Widget
 */
bool Widget::handle_event(const Event& ev) {
    if (!visible_) return false;

    // 逆序遍历（后添加的子 Widget 更"上层"）
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->visible() && (*it)->handle_event(ev)) {
            return true;
        }
    }
    return false;
}

} // namespace meowtui
