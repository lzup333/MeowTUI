/**
 * @file demo.cpp
 * @brief 综合示例 —— 展示 MeowTUI 各种 Widget 的用法
 *
 * 这个示例创建了一个功能完整的 TUI 程序，包含：
 * - 标题标签
 * - 可点击按钮（带计数器）
 * - 文本输入框
 * - 进度条动画
 * - 列表选择框
 *
 * 编译：
 *   mkdir build && cd build
 *   cmake ..
 *   make meowtui_demo
 *   ./meowtui_demo
 */

#include "meowtui/app.h"
#include "meowtui/label.h"
#include "meowtui/button.h"
#include "meowtui/textbox.h"
#include "meowtui/progressbar.h"
#include "meowtui/listbox.h"
#include <memory>
#include <string>
#include <functional>

// ============================================================================
// 全局状态（示例用）
// ============================================================================

static int g_click_count = 0;           ///< 按钮点击计数
static int g_progress    = 0;            ///< 进度条值
static bool g_progress_dir = true;       ///< 进度条方向（来回摆动）

int main() {
    meowtui::App app;

    // ── 初始化 ──────────────────────────────────────────────────────────
    if (!app.init()) {
        return 1;
    }

    int sw = app.terminal().width();   // 屏幕宽度
    int sh = app.terminal().height();  // 屏幕高度

    // ── 1. 标题 ────────────────────────────────────────────────────────
    {
        auto title = std::make_unique<meowtui::Label>(
            "=== MeowTUI Demo ===", meowtui::Align::CENTER);
        title->set_rect({0, 0, sw, 1});
        title->set_id("title");
        app.root().add_child(std::move(title));
    }

    // ── 2. 按钮（带点击计数器） ────────────────────────────────────────
    {
        auto btn = std::make_unique<meowtui::Button>("Click me!");
        btn->set_rect({2, 2, 16, 1});
        btn->set_id("click_btn");

        // 创建一个标签显示点击次数（需在闭包中捕获）
        auto count_label = std::make_unique<meowtui::Label>(
            "Count: 0", meowtui::Align::LEFT);
        count_label->set_rect({20, 2, 12, 1});
        count_label->set_id("count_label");

        // 保存指针供回调使用
        meowtui::Label* count_ptr = count_label.get();

        // 设置按钮点击回调
        btn->set_callback([count_ptr]() {
            g_click_count++;
            count_ptr->set_text("Count: " + std::to_string(g_click_count));
        });

        app.root().add_child(std::move(btn));
        app.root().add_child(std::move(count_label));
    }

    // ── 3. 文本框 ──────────────────────────────────────────────────────
    {
        auto text_label = std::make_unique<meowtui::Label>(
            "Input:", meowtui::Align::LEFT);
        text_label->set_rect({2, 4, 8, 1});
        text_label->set_id("input_label");
        app.root().add_child(std::move(text_label));

        auto textbox = std::make_unique<meowtui::TextBox>("", 30);
        textbox->set_rect({12, 4, 32, 1});
        textbox->set_id("textbox");

        // 设置提交回调（Enter 时触发）
        textbox->set_submit_callback([](const std::string& text) {
            (void)text;  // 示例中仅作演示
        });

        app.root().add_child(std::move(textbox));
    }

    // ── 4. 进度条 ──────────────────────────────────────────────────────
    {
        auto progress = std::make_unique<meowtui::ProgressBar>("Loading:");
        progress->set_rect({2, 6, 40, 1});
        progress->set_id("progress");
        app.root().add_child(std::move(progress));
    }

    // ── 5. 列表选择框 ──────────────────────────────────────────────────
    {
        auto list_label = std::make_unique<meowtui::Label>(
            "Menu:", meowtui::Align::LEFT);
        list_label->set_rect({2, 8, 8, 1});
        list_label->set_id("menu_label");
        app.root().add_child(std::move(list_label));

        auto listbox = std::make_unique<meowtui::ListBox>(
            std::vector<std::string>{
                "New File",
                "Open File",
                "Save File",
                "Save As...",
                "Settings",
                "Help",
                "About MeowTUI",
                "Quit"
            }
        );
        listbox->set_rect({12, 8, 20, 8});
        listbox->set_id("menu_list");

        // 状态栏文本
        auto status = std::make_unique<meowtui::Label>(
            "Select an option...", meowtui::Align::LEFT);
        status->set_rect({2, 17, sw - 2, 1});
        status->set_id("status");

        meowtui::Label* status_ptr = status.get();

        // 设置列表选择回调
        listbox->set_callback([status_ptr](size_t index, const std::string& item) {
            status_ptr->set_text("Selected: [" + std::to_string(index) + "] " + item);
        });

        app.root().add_child(std::move(listbox));
        app.root().add_child(std::move(status));
    }

    // ── 6. 自动为第一个可聚焦 Widget（按钮）设置焦点 ───────────────────
    {
        // 通过深度遍历找到第一个可聚焦 Widget（使用 Y-combinator 风格递归）
        bool focused = false;
        std::function<void(meowtui::Widget*)> focus_first;
        focus_first = [&focused, &focus_first](meowtui::Widget* w) {
            if (focused || !w || !w->visible()) return;
            if (w->focusable()) {
                w->set_focused(true);
                focused = true;
                return;
            }
            for (size_t i = 0; i < w->children().size(); ++i) {
                focus_first(w->child_at(i));
            }
        };
        focus_first(&app.root());
    }

    // ── 7. 底部提示 ────────────────────────────────────────────────────
    {
        std::string hint = "Tab: switch focus  |  ↑↓: navigate list  "
                           "|  Enter: confirm  |  Ctrl+C/Q: quit";
        auto hint_label = std::make_unique<meowtui::Label>(hint, meowtui::Align::CENTER);
        hint_label->set_rect({0, sh - 1, sw, 1});
        hint_label->set_id("hint");
        app.root().add_child(std::move(hint_label));
    }

    // ── 8. 设置 tick 回调（动画用） ────────────────────────────────────
    app.set_tick_callback([](const meowtui::Event& ev) {
        if (ev.type == meowtui::EventType::TICK) {
            // 进度条动画：0→100→0 来回摆动
            if (g_progress_dir) {
                g_progress++;
                if (g_progress >= 100) {
                    g_progress_dir = false;
                }
            } else {
                g_progress--;
                if (g_progress <= 0) {
                    g_progress_dir = true;
                }
            }
        }
    });

    // ── 9. 进入主循环 ──────────────────────────────────────────────────
    app.run();

    return 0;
}
