#ifndef MEOWTUI_RECT_H
#define MEOWTUI_RECT_H

/**
 * @file rect.h
 * @brief 矩形区域类 —— 描述 Widget 在屏幕上的位置和尺寸
 *
 * Rect 是整个布局系统的几何基础，所有 Widget 的定位和大小
 * 都通过 Rect 来表示。
 */

#include "common.h"

namespace meowtui {

/**
 * @brief 矩形区域
 *
 * 表示屏幕上的一块矩形区域，包含位置（col, row）和尺寸（width, height）。
 */
struct Rect {
    Col x = 0;      ///< 左上角列坐标
    Row y = 0;      ///< 左上角行坐标
    int w = 0;      ///< 宽度（列数）
    int h = 0;      ///< 高度（行数）

    Rect() = default;

    /**
     * @brief 构造矩形
     * @param x_  列坐标
     * @param y_  行坐标
     * @param w_  宽度
     * @param h_  高度
     */
    Rect(Col x_, Row y_, int w_, int h_)
        : x(x_), y(y_), w(w_), h(h_) {}

    /**
     * @brief 判断点 (cx, cy) 是否在矩形内部
     */
    bool contains(Col cx, Row cy) const {
        return cx >= x && cx < x + w && cy >= y && cy < y + h;
    }

    /**
     * @brief 返回裁剪后的交集矩形
     */
    Rect intersect(const Rect& other) const {
        Col nx = std::max(x, other.x);
        Row ny = std::max(y, other.y);
        int nw = std::min(x + w, other.x + other.w) - nx;
        int nh = std::min(y + h, other.y + other.h) - ny;
        if (nw < 0) nw = 0;
        if (nh < 0) nh = 0;
        return {nx, ny, nw, nh};
    }

    /** 判断矩形是否有效（宽高都 > 0） */
    bool valid() const { return w > 0 && h > 0; }
};

} // namespace meowtui

#endif // MEOWTUI_RECT_H
