/**
 * @file event.cpp
 * @brief 非阻塞事件系统实现
 *
 * 本文件实现了基于 select() 的非阻塞输入读取。
 * 支持普通字符、功能键（方向键/Fn）、Ctrl 组合键等的解析。
 */

#include "meowtui/event.h"

#include <unistd.h>
#include <sys/select.h>
#include <cstring>
#include <cerrno>

namespace meowtui {

// ============================================================================
// 构造
// ============================================================================

EventLoop::EventLoop() {}

// ============================================================================
// 事件轮询
// ============================================================================

Event EventLoop::poll() {
    if (quit_) return Event::from_quit();
    return read_event();
}

// ============================================================================
// 非阻塞读取
// ============================================================================

/**
 * 使用 select() 实现非阻塞输入检查。
 * 如果有输入则读取并解析，否则返回 NONE 事件。
 */
Event EventLoop::read_event() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    // timeout = 0，不阻塞
    struct timeval tv = {0, 0};
    int ret = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);

    if (ret <= 0) {
        // 没有输入可用
        return Event::from_tick();
    }

    // 读取一个字节
    char buf[1];
    ssize_t n = ::read(STDIN_FILENO, buf, 1);
    if (n <= 0) {
        return Event::from_tick();
    }

    unsigned char c = static_cast<unsigned char>(buf[0]);

    // ── ESC（转义序列开始） ─────────────────────────────────────────────
    if (c == 27) {
        return read_escape_sequence();
    }

    // ── Ctrl 组合键 ─────────────────────────────────────────────────────
    if (c < 32) {
        switch (c) {
            case 1:  return Event::from_key(Key::CTRL_A);
            case 3:  return Event::from_key(Key::CTRL_C);
            case 4:  return Event::from_key(Key::CTRL_D);
            case 5:  return Event::from_key(Key::CTRL_E);
            case 6:  return Event::from_key(Key::CTRL_F);
            case 9:  return Event::from_key(Key::TAB);
            case 10: return Event::from_key(Key::ENTER);
            case 11: return Event::from_key(Key::CTRL_K);
            case 12: return Event::from_key(Key::CTRL_L);
            case 13: return Event::from_key(Key::ENTER);  // CR
            case 14: return Event::from_key(Key::CTRL_N);
            case 16: return Event::from_key(Key::CTRL_P);
            case 21: return Event::from_key(Key::CTRL_U);
            case 23: return Event::from_key(Key::CTRL_W);
            case 24: return Event::from_key(Key::CTRL_X);
            case 26: return Event::from_key(Key::CTRL_Z);
            case 27: return Event::from_key(Key::ESCAPE);
            case 127: return Event::from_key(Key::BACKSPACE);
            default: break;
        }
        // 其他控制字符作为普通 Ctrl+字母
        return Event::from_key(static_cast<Key>(3000 + c));
    }

    // ── 可打印字符 ──────────────────────────────────────────────────────
    return Event::from_char(static_cast<char>(c));
}

// ============================================================================
// 转义序列解析
// ============================================================================

/**
 * 解析以 ESC（\x1b）开头的 ANSI 转义序列。
 *
 * 支持的序列格式：
 * - ESC [ A ~ 方向键、功能键（CSI 序列）
 * - ESC O P ~ 旧式功能键（SS3 序列）
 */
Event EventLoop::read_escape_sequence() {
    // 尝试读取后续字符（非阻塞）
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv = {0, 20000};  // 20ms 等待后续字符
    int ret = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);

    if (ret <= 0) {
        // 单独的 ESC 键
        return Event::from_key(Key::ESCAPE);
    }

    // 读取第二个字节
    char buf2[1];
    ::read(STDIN_FILENO, buf2, 1);

    if (buf2[0] == '[') {
        // CSI 序列: ESC [ ...
        std::string params;
        char c;
        while (true) {
            fd_set fds2;
            FD_ZERO(&fds2);
            FD_SET(STDIN_FILENO, &fds2);
            struct timeval tv2 = {0, 5000};
            int r = select(STDIN_FILENO + 1, &fds2, nullptr, nullptr, &tv2);
            if (r <= 0) break;
            if (::read(STDIN_FILENO, &c, 1) <= 0) break;

            if (c >= 'A' && c <= 'Z') {
                // 简单序列结束符
                switch (c) {
                    case 'A': return Event::from_key(Key::UP);
                    case 'B': return Event::from_key(Key::DOWN);
                    case 'C': return Event::from_key(Key::RIGHT);
                    case 'D': return Event::from_key(Key::LEFT);
                    case 'H': return Event::from_key(Key::HOME);
                    case 'F': return Event::from_key(Key::END);
                    default:  break;
                }
                break;
            } else if (c == '~') {
                // 数字参数 + ~ 结尾
                int num = 0;
                try { num = std::stoi(params); } catch (...) { break; }
                switch (num) {
                    case 1:  return Event::from_key(Key::HOME);
                    case 2:  return Event::from_key(Key::INSERT);
                    case 3:  return Event::from_key(Key::DELETE);
                    case 4:  return Event::from_key(Key::END);
                    case 5:  return Event::from_key(Key::PAGE_UP);
                    case 6:  return Event::from_key(Key::PAGE_DOWN);
                    case 15: return Event::from_key(Key::F5);
                    case 17: return Event::from_key(Key::F6);
                    case 18: return Event::from_key(Key::F7);
                    case 19: return Event::from_key(Key::F8);
                    case 20: return Event::from_key(Key::F9);
                    case 21: return Event::from_key(Key::F10);
                    case 23: return Event::from_key(Key::F11);
                    case 24: return Event::from_key(Key::F12);
                    default: break;
                }
                break;
            } else if ((c >= '0' && c <= '9') || c == ';') {
                params += c;
            } else {
                break;
            }
        }
    } else if (buf2[0] == 'O') {
        // SS3 序列: ESC O ...
        char c;
        fd_set fds2;
        FD_ZERO(&fds2);
        FD_SET(STDIN_FILENO, &fds2);
        struct timeval tv2 = {0, 5000};
        int r = select(STDIN_FILENO + 1, &fds2, nullptr, nullptr, &tv2);
        if (r > 0 && ::read(STDIN_FILENO, &c, 1) > 0) {
            switch (c) {
                case 'P': return Event::from_key(Key::F1);
                case 'Q': return Event::from_key(Key::F2);
                case 'R': return Event::from_key(Key::F3);
                case 'S': return Event::from_key(Key::F4);
                case 'H': return Event::from_key(Key::HOME);
                case 'F': return Event::from_key(Key::END);
                default: break;
            }
        }
        // 未识别的 SS3 序列，当作普通键
        return Event::from_key(static_cast<Key>(2000 + buf2[0]));
    }

    // 无法识别的转义序列，返回 ESC
    return Event::from_key(Key::ESCAPE);
}

} // namespace meowtui
