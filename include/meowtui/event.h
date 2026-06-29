#ifndef MEOWTUI_EVENT_H
#define MEOWTUI_EVENT_H

/**
 * @file event.h
 * @brief 事件系统 —— 非阻塞输入事件的分发与处理
 *
 * MeowTUI 的事件系统基于非阻塞 I/O 和多路复用（select/poll）实现。
 * EventLoop 负责从标准输入读取事件，并以统一的 Event 结构分发给
 * 各个 Widget 或 App。
 *
 * 核心设计原则：
 * - 绝不阻塞：每次 tick 都使用非阻塞读取
 * - 统一封装：所有输入（普通键、功能键、终端调整）都转为 Event
 * - 易于扩展：可通过 timeouts 或自定义 fd 添加额外事件源
 */

#include "common.h"
#include <chrono>

namespace meowtui {

/**
 * @brief 统一事件结构
 *
 * 所有外部输入（按键、终端尺寸变化、心跳等）都由此结构表示。
 */
struct Event {
    EventType type = EventType::NONE;   ///< 事件类型
    Key       key  = Key::NONE;         ///< 按键值（仅 KEY 类型时有效）
    char      ch   = 0;                 ///< 原始字符（仅 KEY 类型且为可打印字符时有效）

    /// 构造一个按键事件
    static Event from_key(Key k)       { return {EventType::KEY, k, 0}; }
    /// 构造一个字符事件
    static Event from_char(char c)     { return {EventType::KEY, Key::NONE, c}; }
    /// 构造一个尺寸变化事件
    static Event from_resize()         { return {EventType::RESIZE, Key::NONE, 0}; }
    /// 构造一个 tick 事件（用于定时刷新）
    static Event from_tick()           { return {EventType::TICK, Key::NONE, 0}; }
    /// 构造一个退出事件
    static Event from_quit()           { return {EventType::QUIT, Key::NONE, 0}; }

    /// 判断是否为可打印字符
    bool is_printable() const { return ch >= 32 && ch <= 126; }
};

// ============================================================================
// 事件回调类型
// ============================================================================

/// 事件处理回调函数类型
using EventCallback = std::function<void(const Event&)>;

// ============================================================================
// 事件循环 —— 非阻塞事件驱动核心
// ============================================================================

/**
 * @brief 非阻塞事件循环
 *
 * 提供基于 select() 的非阻塞输入读取。
 * 每次调用 poll() 会：
 * 1. 检查是否有输入可用（timeout=0，不阻塞）
 * 2. 如果有输入，读取并解析为 Event
 * 3. 如果没有输入，返回 NONE 事件
 *
 * 使用方式：在主循环中反复调用 poll()，然后处理返回的事件。
 */
class EventLoop {
public:
    EventLoop();
    ~EventLoop() = default;

    // 禁止拷贝
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    /**
     * @brief 非阻塞轮询事件
     *
     * @return Event  如果无事件，返回 type=NONE 的事件
     */
    Event poll();

    /**
     * @brief 设置退出标志，poll() 将返回 QUIT 事件
     */
    void quit() { quit_ = true; }

    /**
     * @brief 判断是否设置了退出标志
     */
    bool should_quit() const { return quit_; }

    /**
     * @brief 重置退出标志（用于重新进入事件循环）
     */
    void reset_quit() { quit_ = false; }

private:
    /**
     * @brief 尝试从 stdin 非阻塞读取一个事件
     *
     * 使用 select() 检查输入是否可用，只有在有数据时才读取。
     *
     * @return 解析后的事件
     */
    Event read_event();

    /**
     * @brief 读取转义序列（以 ESC 开头的序列）
     *
     * 解析常见的 ANSI 转义序列（方向键、功能键等）。
     *
     * @return 解析后的按键事件
     */
    Event read_escape_sequence();

    bool quit_ = false;

    /// 部分读取的转义序列缓冲区
    std::string seq_buf_;
};

} // namespace meowtui

#endif // MEOWTUI_EVENT_H
