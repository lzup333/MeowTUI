#ifndef MEOWTUI_SCREEN_H
#define MEOWTUI_SCREEN_H

/**
 * @file screen.h
 * @brief 屏幕缓冲区 —— 双缓冲渲染引擎
 *
 * Screen 实现了典型的双缓冲（double-buffer）渲染机制：
 * - 前缓冲区（front buffer）：当前屏幕上显示的内容
 * - 后缓冲区（back buffer）：正在构建的新一帧内容
 *
 * 每次 flush() 时，只将差异部分（delta）输出到终端，
 * 极大减少 ANSI 转义序列的输出量，提升刷新性能。
 */

#include "common.h"
#include "rect.h"

#include <string>
#include <vector>

namespace meowtui {

/**
 * @brief 屏幕字符单元
 *
 * 表示屏幕上单个字符位置的全部信息。
 * 使用 std::string 存储字符以支持 UTF-8 多字节（如中文）。
 */
struct Cell {
    std::string ch = " ";   ///< 显示的字符（支持 UTF-8 多字节）
    Color   fg  = 7;         ///< 前景色
    Color   bg  = 0;         ///< 背景色
    bool    bold = false;    ///< 是否粗体

    bool operator==(const Cell& o) const {
        return ch == o.ch && fg == o.fg && bg == o.bg && bold == o.bold;
    }
    bool operator!=(const Cell& o) const { return !(*this == o); }
};

/**
 * @brief 双缓冲屏幕
 *
 * 管理一个二维 Cell 数组，支持区域写入、清空和差异刷新。
 */
class Screen {
public:
    Screen() = default;
    ~Screen() = default;

    // 禁止拷贝
    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    /**
     * @brief 初始化屏幕缓冲区
     *
     * @param w  宽度（列数）
     * @param h  高度（行数）
     */
    void resize(int w, int h);

    /** 获取宽度 */
    int width() const { return width_; }

    /** 获取高度 */
    int height() const { return height_; }

    // ── 绘图操作 ────────────────────────────────────────────────────────

    /**
     * @brief 在指定位置写入一个带属性的字符
     *
     * 支持 UTF-8 多字节字符（如中文）。
     *
     * @param col    列坐标
     * @param row    行坐标
     * @param ch     字符（可以是多字节 UTF-8 字符串）
     * @param fg     前景色
     * @param bg     背景色
     * @param bold   是否粗体
     */
    void set_cell(Col col, Row row, const std::string& ch,
                  Color fg = COLOR_WHITE,
                  Color bg = COLOR_BLACK,
                  bool bold = false);

    /**
     * @brief 在指定位置写入字符串
     *
     * 支持 UTF-8 多字节字符。每个字符占据一列，
     * 但多字节字符的内部字节不会溢出到相邻列。
     *
     * @param col    起始列
     * @param row    行
     * @param text   要写入的文本
     * @param fg     前景色
     * @param bg     背景色
     * @param bold   是否粗体
     */
    void write_str(Col col, Row row, const std::string& text,
                   Color fg = COLOR_WHITE,
                   Color bg = COLOR_BLACK,
                   bool bold = false);

    /**
     * @brief 填充一个矩形区域
     *
     * @param rect   区域
     * @param ch     填充字符
     * @param fg     前景色
     * @param bg     背景色
     * @param bold   是否粗体
     */
    void fill_rect(const Rect& rect, const std::string& ch,
                   Color fg = COLOR_WHITE,
                   Color bg = COLOR_BLACK,
                   bool bold = false);

    /**
     * @brief 在矩形内绘制边框
     *
     * 使用 +-+| 等 ASCII 字符绘制边框。
     * 如需更美观的边框，可自行使用 UTF-8 字符绘制。
     *
     * @param rect   边框区域
     * @param fg     前景色
     * @param bg     背景色
     * @param bold   是否粗体
     */
    void draw_border(const Rect& rect,
                     Color fg = COLOR_WHITE,
                     Color bg = COLOR_BLACK,
                     bool bold = false);

    /**
     * @brief 清除所有缓冲区（用空格填充）
     */
    void clear();

    /**
     * @brief 将后缓冲区的内容刷新到终端（只输出差异部分）
     *
     * 这是核心渲染函数，通过比较前后缓冲区差异，最小化终端输出。
     */
    void flush();

    // ── 裁剪 ────────────────────────────────────────────────────────────

    /**
     * @brief 设置裁剪区域
     *
     * 后续所有 set_cell/write_str 超出此区域的部分将被忽略。
     * 默认裁剪区域为整个屏幕。
     */
    void set_clip_rect(const Rect& rect) { clip_ = rect; }

    /** 获取当前裁剪区域 */
    Rect clip_rect() const { return clip_; }

    /** 重置裁剪区域为全屏 */
    void reset_clip_rect() { clip_ = {0, 0, width_, height_}; }

private:
    int width_  = 0;
    int height_ = 0;
    Rect clip_;

    /// 后缓冲区（当前正在构建的帧）
    std::vector<Cell> back_;
    /// 前缓冲区（上一帧已显示的内容）
    std::vector<Cell> front_;

    /** 将二维坐标转换为一维索引 */
    int index(Col col, Row row) const {
        return row * width_ + col;
    }

    /** 检查坐标是否在有效范围内 */
    bool in_range(Col col, Row row) const {
        return col >= 0 && col < width_ && row >= 0 && row < height_;
    }

    /** 检查坐标是否在裁剪区域内 */
    bool in_clip(Col col, Row row) const {
        return clip_.contains(col, row);
    }
};

} // namespace meowtui

#endif // MEOWTUI_SCREEN_H
