#ifndef MEOWTUI_TERMINAL_H
#define MEOWTUI_TERMINAL_H

/**
 * @file terminal.h
 * @brief 终端底层操作封装
 *
 * Terminal 类负责与终端直接交互，包括：
 * - 原始模式（raw mode）的设置与恢复
 * - 光标隐藏/显示
 * - ANSI 转义序列的输出
 * - 终端尺寸的获取
 * - 备用屏幕（alternate screen）切换
 *
 * 这是一个封装良好的底层模块，上层代码无需关心终端细节。
 */

#include "common.h"

#include <cstdio>
#include <memory>

namespace meowtui {

/**
 * @brief 终端控制类（单例，不可拷贝）
 *
 * 封装了所有与 POSIX 终端相关的底层操作。
 * 构造时进入原始模式 + 备用屏幕，析构时自动恢复。
 */
class Terminal {
public:
    /// 获取单例实例
    static Terminal& instance();

    /// 禁止拷贝
    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

    /// 析构时自动恢复终端设置
    ~Terminal();

    // ── 尺寸查询 ────────────────────────────────────────────────────────

    /** 获取终端宽度（列数） */
    int width() const { return width_; }

    /** 获取终端高度（行数） */
    int height() const { return height_; }

    /** 刷新缓存的终端尺寸 */
    void refresh_size();

    // ── 光标操作 ────────────────────────────────────────────────────────

    /** 隐藏光标 */
    void hide_cursor();

    /** 显示光标 */
    void show_cursor();

    /** 将光标移动到指定位置 */
    void move_cursor(Col col, Row row);

    // ── 屏幕输出 ────────────────────────────────────────────────────────

    /** 清除整个屏幕 */
    void clear_screen();

    /** 清除从光标位置到行尾的内容 */
    void clear_to_eol();

    /** 设置前景色（16色） */
    void set_fg(Color c);

    /** 设置背景色（16色） */
    void set_bg(Color c);

    /** 重置颜色属性 */
    void reset_color();

    /** 启用粗体 */
    void set_bold(bool bold);

    /** 启用反转（反白） */
    void set_reverse(bool rev);

    /** 写入原始字符串到终端（无缓冲追加） */
    void write(const std::string& s);

    /** 写入并立即刷新 */
    void writeln(const std::string& s);

    /** 刷新输出缓冲区 */
    void flush();

    /** 判断终端是否已初始化 */
    bool is_initialized() const { return initialized_; }

    // ── 备用屏幕 ────────────────────────────────────────────────────────

    /** 进入备用屏幕（保存主屏幕内容） */
    void enter_alt_screen();

    /** 退出备用屏幕（恢复主屏幕内容） */
    void exit_alt_screen();

    /** 启动终端（原始模式 + 备用屏幕）。返回 false 表示失败 */
    bool initialize();

    /** 停止终端，恢复原始设置 */
    void shutdown();

private:
    Terminal() = default;

    /** 设置终端为原始模式（无缓冲、无回显、即时输入） */
    bool set_raw_mode(bool enable);

    int width_  = 0;
    int height_ = 0;
    bool initialized_ = false;
    bool alt_screen_  = false;

    /** stdout 的文件描述符，用于底层写入 */
    int fd_ = -1;

    /** 保存的原始终端属性，用于退出时恢复 */
    void* old_termios_ = nullptr;  // 实际类型为 struct termios*，避免前向声明问题
};

} // namespace meowtui

#endif // MEOWTUI_TERMINAL_H
