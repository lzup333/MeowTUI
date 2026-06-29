#ifndef MEOWTUI_COMMON_H
#define MEOWTUI_COMMON_H

/**
 * @file common.h
 * @brief MeowTUI 公共类型与常量定义
 *
 * 本文件定义了库中全局使用的类型别名、枚举和常量。
 * 所有公共头文件都应包含此文件。
 */

#include <cstdint>
#include <string>
#include <functional>

namespace meowtui {

// ============================================================================
// 基础类型别名
// ============================================================================

/// 屏幕坐标（列，水平方向）
using Col = int;

/// 屏幕坐标（行，垂直方向）
using Row = int;

/// 颜色值（16 色索引或 RGB 组合标志）
using Color = uint8_t;

// ============================================================================
// 颜色常量
// ============================================================================

/// 标准 16 色（终端 ANSI 颜色码）
enum ColorName : Color {
    COLOR_BLACK       = 0,
    COLOR_RED         = 1,
    COLOR_GREEN       = 2,
    COLOR_YELLOW      = 3,
    COLOR_BLUE        = 4,
    COLOR_MAGENTA     = 5,
    COLOR_CYAN        = 6,
    COLOR_WHITE       = 7,
    COLOR_BRIGHT_BLACK   = 8,
    COLOR_BRIGHT_RED     = 9,
    COLOR_BRIGHT_GREEN   = 10,
    COLOR_BRIGHT_YELLOW  = 11,
    COLOR_BRIGHT_BLUE    = 12,
    COLOR_BRIGHT_MAGENTA = 13,
    COLOR_BRIGHT_CYAN    = 14,
    COLOR_BRIGHT_WHITE   = 15,
};

/**
 * @brief 在 256 色模式中启用前景色
 * @note 与 ColorName 组合使用
 */
constexpr Color FG_MASK = 0x10;

/**
 * @brief 在 256 色模式中启用背景色
 * @note 与 ColorName 组合使用
 */
constexpr Color BG_MASK = 0x20;

// ============================================================================
// 按键枚举
// ============================================================================

/**
 * @brief 特殊功能键码
 *
 * 用于 Event 结构体中表示无法直接映射为字符的按键。
 */
enum class Key : uint32_t {
    NONE      = 0,
    TAB       = 9,
    ENTER     = 13,
    ESCAPE    = 27,
    BACKSPACE = 127,

    // 功能键
    F1        = 1001,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    // 方向键
    UP        = 2001,
    DOWN,
    LEFT,
    RIGHT,

    // 编辑键
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,
    INSERT,
    DELETE,

    // 修饰组合
    CTRL_A    = 3001,
    CTRL_C,
    CTRL_D,
    CTRL_E,
    CTRL_F,
    CTRL_K,
    CTRL_L,
    CTRL_N,
    CTRL_P,
    CTRL_Q,
    CTRL_U,
    CTRL_W,
    CTRL_X,
    CTRL_Z,
};

/**
 * @brief 事件类型枚举
 */
enum class EventType {
    NONE,       ///< 无事件
    KEY,        ///< 按键事件
    RESIZE,     ///< 终端尺寸变化
    TICK,       ///< 定时心跳（用于动画/刷新）
    QUIT,       ///< 退出信号
};

/**
 * @brief 对齐方式
 */
enum class Align {
    LEFT,
    CENTER,
    RIGHT,
};

/**
 * @brief 布局方向
 */
enum class Direction {
    HORIZONTAL,
    VERTICAL,
};

} // namespace meowtui

#endif // MEOWTUI_COMMON_H
