#ifndef MEOWTUI_APP_H
#define MEOWTUI_APP_H

/**
 * @file app.h
 * @brief 应用主类 —— 整合所有模块的应用入口
 *
 * App 类是使用 MeowTUI 库的入口点，整合了：
 * - 终端管理（Terminal）
 * - 事件循环（EventLoop）
 * - 屏幕渲染（Screen）
 * - Widget 树管理
 *
 * 典型用法：
 * @code
 * App app;
 * app.init();
 *
 * auto& root = app.root();
 * root.add_child(std::make_unique<Label>("Hello, MeowTUI!"));
 *
 * app.run();
 * @endcode
 */

#include "terminal.h"
#include "event.h"
#include "screen.h"
#include "widget.h"

#include <memory>

namespace meowtui {

/**
 * @brief 应用主类
 *
 * 封装了整个 TUI 应用程序的生命周期：
 * 1. init() —— 初始化终端和事件循环
 * 2. run()  —— 进入事件循环，处理事件并渲染
 * 3. shutdown() —— 清理资源，恢复终端
 */
class App {
public:
    App();
    ~App();

    // 禁止拷贝
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    // ── 生命周期 ────────────────────────────────────────────────────────

    /**
     * @brief 初始化应用（终端、事件循环、屏幕）
     *
     * @return true  初始化成功
     * @return false 初始化失败
     */
    bool init();

    /**
     * @brief 进入主事件循环
     *
     * 以 ~60 FPS 的速率循环处理事件和渲染。
     * 按 Ctrl+C 或 Ctrl+Q 退出循环。
     */
    void run();

    /**
     * @brief 停止事件循环
     */
    void stop();

    /**
     * @brief 清理资源，恢复终端设置
     */
    void shutdown();

    // ── 访问器 ──────────────────────────────────────────────────────────

    /** 获取根 Widget（所有其他 Widget 的父容器） */
    Widget& root() { return root_; }

    /** 获取终端引用 */
    Terminal& terminal() { return Terminal::instance(); }

    /** 获取屏幕引用 */
    Screen& screen() { return screen_; }

    /** 获取事件循环引用 */
    EventLoop& event_loop() { return loop_; }

    // ── 渲染控制 ────────────────────────────────────────────────────────

    /**
     * @brief 请求立即刷新屏幕
     *
     * 在非主循环场景下手动触发渲染。
     */
    void refresh();

    /**
     * @brief 设置帧率上限（每秒刷新次数）
     *
     * @param fps  每秒最大帧数，默认 60
     */
    void set_fps(int fps) { fps_ = fps; }

    /** 获取当前帧率 */
    int fps() const { return fps_; }

    // ── 事件回调 ────────────────────────────────────────────────────────

    /**
     * @brief 设置全局 tick 回调
     *
     * tick 回调在每次渲染前调用，适合执行动画更新等周期性任务。
     *
     * @param cb  回调函数
     */
    void set_tick_callback(EventCallback cb) { tick_cb_ = std::move(cb); }

    // ── 焦点管理 ────────────────────────────────────────────────────────

    /**
     * @brief 将焦点切换到下一个可聚焦的 Widget
     *
     * 深度优先遍历 Widget 树，找到下一个 focusable()==true 的 Widget。
     */
    void focus_next();

    /**
     * @brief 将焦点切换到上一个可聚焦的 Widget
     */
    void focus_prev();

private:
    EventLoop loop_;            ///< 事件循环
    Screen screen_;             ///< 屏幕缓冲区
    Widget root_;               ///< 根容器 Widget

    bool running_ = false;      ///< 是否正在运行
    int  fps_     = 60;         ///< 目标帧率

    EventCallback tick_cb_;     ///< 每帧 tick 回调

    /**
     * @brief 执行一帧渲染
     *
     * 包括：清空屏幕缓冲区、渲染 Widget 树、刷新到终端。
     */
    void render_frame();
};

} // namespace meowtui

#endif // MEOWTUI_APP_H
