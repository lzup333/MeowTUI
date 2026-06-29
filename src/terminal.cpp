/**
 * @file terminal.cpp
 * @brief 终端底层操作实现
 *
 * 本文件实现了与 POSIX 终端的直接交互。
 * 使用 termios 设置原始模式，通过 write() 输出 ANSI 转义序列。
 */

#include "meowtui/terminal.h"

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstring>
#include <stdexcept>

namespace meowtui {

// ============================================================================
// 单例
// ============================================================================

Terminal& Terminal::instance() {
    static Terminal inst;
    return inst;
}

// ============================================================================
// 析构 —— 自动恢复终端
// ============================================================================

Terminal::~Terminal() {
    if (initialized_) {
        shutdown();
    }
    if (old_termios_) {
        delete static_cast<struct termios*>(old_termios_);
    }
}

// ============================================================================
// 初始化与关闭
// ============================================================================

bool Terminal::initialize() {
    if (initialized_) return true;

    fd_ = STDOUT_FILENO;
    if (!isatty(fd_)) {
        // 不是终端，仍然尝试工作
    }

    // 保存当前终端属性
    old_termios_ = new struct termios();
    if (tcgetattr(STDIN_FILENO, static_cast<struct termios*>(old_termios_)) != 0) {
        delete static_cast<struct termios*>(old_termios_);
        old_termios_ = nullptr;
        return false;
    }

    // 启用原始模式
    if (!set_raw_mode(true)) {
        return false;
    }

    // 获取初始尺寸
    refresh_size();

    // 进入备用屏幕
    enter_alt_screen();

    // 隐藏光标
    hide_cursor();

    // 清屏
    clear_screen();
    flush();

    initialized_ = true;
    return true;
}

void Terminal::shutdown() {
    if (!initialized_) return;

    // 显示光标
    show_cursor();

    // 退出备用屏幕
    exit_alt_screen();

    // 重置文本属性
    reset_color();
    flush();

    // 恢复原始终端属性
    set_raw_mode(false);

    initialized_ = false;
}

// ============================================================================
// 原始模式
// ============================================================================

bool Terminal::set_raw_mode(bool enable) {
    if (!old_termios_) return false;

    auto* term = static_cast<struct termios*>(old_termios_);

    if (enable) {
        // 设置原始模式
        struct termios raw = *term;
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        raw.c_oflag &= ~(OPOST);
        raw.c_cflag |= (CS8);
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        raw.c_cc[VMIN]  = 0;  // 非阻塞读取
        raw.c_cc[VTIME] = 0;  // 不等待
        return tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0;
    } else {
        // 恢复保存的属性
        return tcsetattr(STDIN_FILENO, TCSAFLUSH, term) == 0;
    }
}

// ============================================================================
// 尺寸查询
// ============================================================================

void Terminal::refresh_size() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        width_  = ws.ws_col;
        height_ = ws.ws_row;
    } else {
        // 默认回退值
        width_  = 80;
        height_ = 24;
    }
}

// ============================================================================
// 光标操作
// ============================================================================

void Terminal::hide_cursor() {
    write("\033[?25l");
}

void Terminal::show_cursor() {
    write("\033[?25h");
}

void Terminal::move_cursor(Col col, Row row) {
    // ANSI: ESC[row;colH  —— row 和 col 从 1 开始
    write("\033[" + std::to_string(row + 1) + ";" + std::to_string(col + 1) + "H");
}

// ============================================================================
// 屏幕输出
// ============================================================================

void Terminal::clear_screen() {
    write("\033[2J");
    move_cursor(0, 0);
}

void Terminal::clear_to_eol() {
    write("\033[K");
}

void Terminal::set_fg(Color c) {
    // 16 色模式
    if (c < 16) {
        write("\033[38;5;" + std::to_string(c) + "m");
    }
}

void Terminal::set_bg(Color c) {
    if (c < 16) {
        write("\033[48;5;" + std::to_string(c) + "m");
    }
}

void Terminal::reset_color() {
    write("\033[0m");
}

void Terminal::set_bold(bool bold) {
    if (bold)
        write("\033[1m");
    else
        write("\033[22m");
}

void Terminal::set_reverse(bool rev) {
    if (rev)
        write("\033[7m");
    else
        write("\033[27m");
}

void Terminal::write(const std::string& s) {
    ::write(fd_, s.data(), s.size());
}

void Terminal::writeln(const std::string& s) {
    write(s);
    write("\n");
    flush();
}

void Terminal::flush() {
    fsync(fd_);
}

// ============================================================================
// 备用屏幕
// ============================================================================

void Terminal::enter_alt_screen() {
    write("\033[?1049h");
    alt_screen_ = true;
}

void Terminal::exit_alt_screen() {
    write("\033[?1049l");
    alt_screen_ = false;
}

} // namespace meowtui
