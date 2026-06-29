/**
 * @file app.cpp
 * @brief 应用主类实现
 *
 * App 类整合了终端、事件循环、屏幕和 Widget 树。
 * run() 方法实现了主循环：
 * 1. 处理输入事件
 * 2. 触发 tick 回调
 * 3. 渲染 Widget 树到屏幕缓冲区
 * 4. 刷新差异到终端
 * 5. 帧率控制（~60 FPS）
 */

#include "meowtui/app.h"

#include <chrono>
#include <thread>
#include <vector>

namespace meowtui {

// ============================================================================
// 构造与析构
// ============================================================================

App::App() = default;

App::~App() {
    shutdown();
}

// ============================================================================
// 初始化
// ============================================================================

bool App::init() {
    // 初始化终端
    if (!Terminal::instance().initialize()) {
        return false;
    }

    // 初始化屏幕缓冲区
    screen_.resize(Terminal::instance().width(), Terminal::instance().height());

    // 设置根 Widget 尺寸为全屏
    root_.set_rect(Rect{0, 0, Terminal::instance().width(), Terminal::instance().height()});

    return true;
}

// ============================================================================
// 主循环
// ============================================================================

/**
 * 主事件循环：
 * 1. 计算帧延迟（帧率控制）
 * 2. 非阻塞轮询事件
 * 3. 处理事件（CTRL_C/CTRL_Q 退出、RESIZE 调整尺寸等）
 * 4. 调用 tick 回调
 * 5. 渲染一帧
 * 6. 等待剩余帧时间
 *
 * 退出条件：
 * - 用户按 Ctrl+C 或 Ctrl+Q
 * - 用户按 ESC
 * - stop() 被调用
 */
void App::run() {
    if (!Terminal::instance().is_initialized()) return;

    running_ = true;
    loop_.reset_quit();

    // 帧时间（微秒）
    auto frame_duration = std::chrono::microseconds(1000000 / fps_);

    while (running_) {
        auto frame_start = std::chrono::steady_clock::now();

        // ── 轮询事件 ────────────────────────────────────────────────────
        Event ev = loop_.poll();

        // 处理事件
        switch (ev.type) {
            case EventType::QUIT:
                running_ = false;
                continue;

            case EventType::KEY:
                // Ctrl+C / Ctrl+Q → 退出
                if (ev.key == Key::CTRL_C || ev.key == Key::CTRL_Q) {
                    running_ = false;
                    continue;
                }

                // Tab → 切换焦点
                if (ev.key == Key::TAB) {
                    focus_next();
                    break;
                }

                // 将事件分发给 Widget 树
                root_.handle_event(ev);
                break;

            case EventType::RESIZE:
                Terminal::instance().refresh_size();
                screen_.resize(Terminal::instance().width(), Terminal::instance().height());
                root_.set_rect(Rect{0, 0, Terminal::instance().width(), Terminal::instance().height()});
                break;

            case EventType::TICK:
                // 无事件时触发 tick
                break;

            default:
                break;
        }

        // ── 全局 tick 回调 ──────────────────────────────────────────────
        if (tick_cb_) {
            tick_cb_(ev);
        }

        // ── 渲染一帧 ────────────────────────────────────────────────────
        render_frame();

        // ── 帧率控制 ────────────────────────────────────────────────────
        auto elapsed = std::chrono::steady_clock::now() - frame_start;
        if (elapsed < frame_duration) {
            auto sleep_time = frame_duration - elapsed;
            // 使用 select 的空闲时间进行等待，保持非阻塞
            std::this_thread::sleep_for(sleep_time);
        }
    }

    // 清理
    shutdown();
}

// ============================================================================
// 停止与清理
// ============================================================================

void App::stop() {
    running_ = false;
    loop_.quit();
}

void App::shutdown() {
    Terminal::instance().shutdown();
}

// ============================================================================
// 渲染
// ============================================================================

/**
 * 每帧渲染流程：
 * 1. 清空后缓冲区
 * 2. 通知终端尺寸已变化（重新获取）
 * 3. 递归渲染 Widget 树
 * 4. 将差异刷新到屏幕
 */
void App::render_frame() {
    // 清空屏幕
    screen_.clear();

    // 检查终端尺寸是否变化
    Terminal::instance().refresh_size();
    if (Terminal::instance().width() != screen_.width() ||
        Terminal::instance().height() != screen_.height()) {
        screen_.resize(Terminal::instance().width(), Terminal::instance().height());
        root_.set_rect(Rect{0, 0, Terminal::instance().width(), Terminal::instance().height()});
    }

    // 渲染 Widget 树
    root_.render(screen_);

    // 隐藏终端光标（Widget 可自行显示光标，如 TextBox）
    Terminal::instance().hide_cursor();

    // 刷新到终端（差异输出）
    screen_.flush();
}

/**
 * 手动触发一帧渲染
 */
void App::refresh() {
    render_frame();
}

// ============================================================================
// 焦点管理
// ============================================================================

/**
 * 递归收集所有可聚焦的 Widget
 *
 * 深度优先遍历 Widget 树，收集所有 focusable()==true 且可见的 Widget。
 */
static void collect_focusable(Widget* widget,
                              std::vector<Widget*>& out) {
    if (!widget || !widget->visible()) return;
    if (widget->focusable()) {
        out.push_back(widget);
    }
    for (size_t i = 0; i < widget->children().size(); ++i) {
        collect_focusable(widget->child_at(i), out);
    }
}

/**
 * 将焦点切换到下一个可聚焦的 Widget
 *
 * 遍历所有可聚焦 Widget，找到当前获得焦点的 Widget，
 * 然后将焦点移到下一个 Widget（循环）。
 */
void App::focus_next() {
    std::vector<Widget*> focusable;
    collect_focusable(&root_, focusable);

    if (focusable.empty()) return;

    // 找到当前拥有焦点的 Widget
    int current = -1;
    for (size_t i = 0; i < focusable.size(); ++i) {
        if (focusable[i]->is_focused()) {
            current = static_cast<int>(i);
            break;
        }
    }

    // 移除当前 Widget 的焦点
    if (current >= 0 && current < static_cast<int>(focusable.size())) {
        focusable[current]->set_focused(false);
    }

    // 将焦点移到下一个 Widget
    int next = (current + 1) % static_cast<int>(focusable.size());
    focusable[next]->set_focused(true);
}

/**
 * 将焦点切换到上一个可聚焦的 Widget
 */
void App::focus_prev() {
    std::vector<Widget*> focusable;
    collect_focusable(&root_, focusable);

    if (focusable.empty()) return;

    int current = -1;
    for (size_t i = 0; i < focusable.size(); ++i) {
        if (focusable[i]->is_focused()) {
            current = static_cast<int>(i);
            break;
        }
    }

    if (current >= 0 && current < static_cast<int>(focusable.size())) {
        focusable[current]->set_focused(false);
    }

    // 上一个 Widget（逆向循环）
    int prev = (current - 1 + static_cast<int>(focusable.size())) %
               static_cast<int>(focusable.size());
    focusable[prev]->set_focused(true);
}

} // namespace meowtui
