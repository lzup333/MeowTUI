/**
 * @file minimal.cpp
 * @brief 极简示例 —— 展示 MeowTUI 的最基本用法
 *
 * 这个示例演示了如何使用 MeowTUI 创建一个
 * 最简单的 TUI 程序：显示一个带边框的"Hello World"标签。
 *
 * 编译：
 *   mkdir build && cd build
 *   cmake ..
 *   make meowtui_minimal
 *   ./meowtui_minimal
 */

#include "meowtui/app.h"
#include "meowtui/label.h"
#include <memory>

int main() {
    // ── 1. 创建应用实例 ────────────────────────────────────────────────
    meowtui::App app;

    // ── 2. 初始化（终端进入原始模式 + 备用屏幕） ────────────────────────
    if (!app.init()) {
        return 1;
    }

    // ── 3. 创建 UI 组件 ────────────────────────────────────────────────
    // 获取屏幕尺寸
    int sw = app.terminal().width();
    int sh = app.terminal().height();

    // 创建一个居中显示的标签
    auto label = std::make_unique<meowtui::Label>("Hello, MeowTUI! ^_^",
                                                  meowtui::Align::CENTER);
    label->set_rect({sw / 2 - 10, sh / 2 - 1, 20, 1});

    // 将标签添加到根 Widget
    app.root().add_child(std::move(label));

    // ── 4. 启动事件循环 ────────────────────────────────────────────────
    // 按 ESC 或 Ctrl+C/Q 退出
    app.run();

    return 0;
}
