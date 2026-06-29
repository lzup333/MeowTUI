# MeowTUI 快速入门指南

> 面向**仅掌握 C++ 基础**的程序员 —— 不需要了解高级 C++ 特性，也不需要了解终端编程。

## 目录

1. [MeowTUI 是什么？](#1-meowtui-是什么)
2. [你需要知道的基础概念](#2-你需要知道的基础概念)
3. [第一个程序：Hello World](#3-第一个程序-hello-world)
4. [第二步：添加按钮和交互](#4-第二步添加按钮和交互)
5. [第三步：文本输入框](#5-第三步文本输入框)
6. [第四步：列表选择](#6-第四步列表选择)
7. [第五步：进度条动画](#7-第五步进度条动画)
8. [完整示例：综合演示](#8-完整示例综合演示)
9. [常见问题](#9-常见问题)

---

## 1. MeowTUI 是什么？

MeowTUI 是一个** C++ 库**，让你可以用代码在终端（黑色命令行窗口）里画出**界面** ——
就像写网页用 HTML，写桌面程序用 Qt，写终端界面就用 MeowTUI。

### 你能用它做什么？

```
┌───── 一个典型的 MeowTUI 界面 ─────────────────┐
│  === MeowTUI Demo ===                          │
│  [ Click me! ]   Count: 5                      │
│  Input: [hello world|]                         │
│  Loading: [████████░░░░░░░░░░] 50%              │
│  Menu:                                         │
│  ┌──────────────────┐                          │
│  │ > New File       │                          │
│  │   Open File      │                          │
│  │   Save File      │                          │
│  │   Save As...     │                          │
│  └──────────────────┘                          │
│  Tab: 切换焦点  Enter: 确认  Ctrl+Q: 退出      │
└────────────────────────────────────────────────┘
```

### 核心特点

| 特点 | 说明 |
|------|------|
| **轻量** | 只有 10 个源文件，编译后 ~100KB |
| **零依赖** | 不需要装任何额外软件，有 C++ 编译器就行 |
| **非阻塞** | 程序不会"卡住"，始终流畅响应 |
| **简单** | API 直观，每个组件 2~3 行代码就能用 |

---

## 2. 你需要知道的基础概念

### 2.1 坐标系统

屏幕坐标以**左上角**为原点 `(0, 0)`：

```
(0,0) ───→ 列 (Col / x)
  │
  ↓
 行 (Row / y)
```

- **列 (Col / x)**：从左到右，相当于横坐标
- **行 (Row / y)**：从上到下，相当于纵坐标

### 2.2 矩形区域 Rect

每个组件的位置和大小用 `Rect` 描述：

```cpp
meowtui::Rect r{列坐标, 行坐标, 宽度, 高度};
// 例如：从 (10, 5) 开始，宽 30 字符，高 1 行
meowtui::Rect r{10, 5, 30, 1};
```

### 2.3 组件树

组件像文件夹一样可以**嵌套**：

```
App (应用)
 └── 根容器 (root)
      ├── 文本标签 (Label)     ← 显示文字
      ├── 按钮 (Button)        ← 可以点
      └── 容器 (Widget)        ← 装其他组件
           ├── 文本框 (TextBox)
           └── 列表 (ListBox)
```

### 2.4 焦点 (Focus)

- 只有获得焦点的组件才能接收键盘输入
- 按 **Tab** 键在组件之间切换焦点
- 按钮、文本框、列表可以获得焦点；标签、进度条不能

---

## 3. 第一个程序：Hello World

### 完整代码

```cpp
// hello.cpp
#include "meowtui/app.h"     // 应用类
#include "meowtui/label.h"   // 标签组件
#include <memory>            // std::make_unique (C++14 以上)

int main() {
    // 1. 创建应用
    meowtui::App app;

    // 2. 初始化（进入终端界面模式）
    if (!app.init()) return 1;

    // 3. 创建一个标签，显示在屏幕中央
    auto label = std::make_unique<meowtui::Label>(
        "Hello, MeowTUI!",                    // 文字
        meowtui::Align::CENTER                // 居中
    );
    label->set_rect({0, 12, 80, 1});           // {x, y, 宽, 高}

    // 4. 把标签加到根容器
    app.root().add_child(std::move(label));

    // 5. 启动程序（按 Ctrl+C 或 Ctrl+Q 退出）
    app.run();

    return 0;
}
```

### 逐行解释

| 行 | 解释 |
|----|------|
| `#include "meowtui/app.h"` | 引入应用类，这是使用 MeowTUI 的入口 |
| `#include "meowtui/label.h"` | 引入标签组件（显示静态文字） |
| `meowtui::App app;` | 创建一个应用对象。`meowtui::` 是命名空间前缀 |
| `app.init()` | 初始化终端，进入"界面模式" |
| `std::make_unique<Label>(...)` | 创建一个标签组件。`make_unique` 是 C++14 的特性，可以理解为"新建一个对象" |
| `label->set_rect(...)` | 设置标签的位置和大小 |
| `app.root().add_child(...)` | "把标签添加到根容器"。`add_child` 接受 `unique_ptr`，会自动管理内存 |
| `app.run()` | 启动事件循环，开始响应用户操作 |

### 编译运行

```bash
# 编译
g++ -std=c++17 -I./include hello.cpp -L./build -lmeowtui -o hello

# 或使用 CMake（推荐）
```

---

## 4. 第二步：添加按钮和交互

按钮可以响应用户点击，执行你指定的操作。

### 完整代码

```cpp
// button_demo.cpp
#include "meowtui/app.h"
#include "meowtui/button.h"
#include "meowtui/label.h"
#include <memory>

int main() {
    meowtui::App app;
    if (!app.init()) return 1;

    // ── 创建一个按钮 ──────────────────────────────────
    // 按钮文本为 "Click"，位置 (2, 2)，宽度 16
    auto btn = std::make_unique<meowtui::Button>("Click");
    btn->set_rect({2, 2, 16, 1});

    // ── 创建一个显示计数的标签 ─────────────────────────
    auto counter = std::make_unique<meowtui::Label>("Count: 0");
    counter->set_rect({20, 2, 12, 1});

    // 保存指针供回调使用
    meowtui::Label* counter_ptr = counter.get();

    // ── 设置按钮点击后的操作 ───────────────────────────
    // 这里用了 C++ 的 lambda 表达式（匿名函数）
    // [counter_ptr] 表示捕获 counter_ptr 变量
    int click_count = 0;
    btn->set_callback([counter_ptr, &click_count]() {
        click_count++;
        counter_ptr->set_text("Count: " + std::to_string(click_count));
    });

    // ── 添加到根容器 ──────────────────────────────────
    app.root().add_child(std::move(btn));
    app.root().add_child(std::move(counter));

    // ── 启动 ──────────────────────────────────────────
    app.run();
    return 0;
}
```

### 关键点说明

- **`set_callback(...)`**：设置按钮被点击时要执行的代码。参数是一个"回调函数"
- **Lambda 表达式 `[捕获]() { 代码 }`**：C++11 的特性，可以理解为"一段可以访问外部变量的代码块"
- **`counter_ptr`**：因为 `counter` 被 `move` 到根容器后，原来的变量就不能用了，所以需要用指针来访问

---

## 5. 第三步：文本输入框

文本框让用户输入文字。

### 完整代码

```cpp
// input_demo.cpp
#include "meowtui/app.h"
#include "meowtui/textbox.h"
#include "meowtui/label.h"
#include <memory>

int main() {
    meowtui::App app;
    if (!app.init()) return 1;

    // ── 提示文字 ──────────────────────────────────────
    auto hint = std::make_unique<meowtui::Label>("Enter your name:");
    hint->set_rect({2, 2, 20, 1});
    app.root().add_child(std::move(hint));

    // ── 文本框 ─────────────────────────────────────────
    // 参数1：初始文字（空字符串）
    // 参数2：最大字符数（20）
    auto input = std::make_unique<meowtui::TextBox>("", 20);
    input->set_rect({2, 4, 30, 1});

    // ── 设置输入完成回调（用户按 Enter 时触发） ────────
    input->set_submit_callback([](const std::string& text) {
        // text 就是用户输入的内容
        // 这里可以处理输入结果
    });

    app.root().add_child(std::move(input));

    // ── 启动后按 Tab 键让文本框获得焦点 ────────────────
    app.run();
    return 0;
}
```

### 文本框支持的按键

| 按键 | 功能 |
|------|------|
| **字母/数字/符号** | 输入字符 |
| **Backspace** | 删除光标前一个字符 |
| **Delete** | 删除光标后一个字符 |
| **← / →** | 移动光标位置 |
| **Home** | 移到开头 |
| **End** | 移到结尾 |
| **Enter** | 提交（触发回调） |
| **Ctrl+U** | 清空所有文字 |

---

## 6. 第四步：列表选择

列表组件让用户从多个选项中选一个。

```cpp
// list_demo.cpp
#include "meowtui/app.h"
#include "meowtui/listbox.h"
#include "meowtui/label.h"
#include <memory>
#include <vector>    // std::vector

int main() {
    meowtui::App app;
    if (!app.init()) return 1;

    // ── 创建列表 ──────────────────────────────────────
    // 用 std::vector<std::string> 提供选项
    auto list = std::make_unique<meowtui::ListBox>(
        std::vector<std::string>{
            "New File",       // 索引 0
            "Open File",      // 索引 1
            "Save File",      // 索引 2
            "Save As...",     // 索引 3
            "Settings",       // 索引 4
            "Quit"            // 索引 5
        }
    );
    list->set_rect({2, 2, 20, 8});
    // 第 4 个参数是 8，表示列表显示区域高 8 行（含边框）

    // ── 状态栏 ────────────────────────────────────────
    auto status = std::make_unique<meowtui::Label>("Select an option...");
    status->set_rect({2, 12, 40, 1});

    meowtui::Label* status_ptr = status.get();

    // ── 设置选择回调 ───────────────────────────────────
    // index: 选中项的编号（从 0 开始）
    // item:  选中项的文本
    list->set_callback([status_ptr](size_t index,
                                    const std::string& item) {
        status_ptr->set_text(
            "You selected [" + std::to_string(index) + "] " + item);
    });

    app.root().add_child(std::move(list));
    app.root().add_child(std::move(status));

    app.run();
    return 0;
}
```

### 列表支持的按键

| 按键 | 功能 |
|------|------|
| **↑ / ↓** | 上下移动选择 |
| **PageUp / PageDown** | 快速翻页 |
| **Home** | 跳到第一个 |
| **End** | 跳到最后一个 |
| **Enter** | 确认选择（触发回调） |

---

## 7. 第五步：进度条动画

进度条用于显示任务进度（0%~100%）。

```cpp
// progress_demo.cpp
#include "meowtui/app.h"
#include "meowtui/progressbar.h"
#include <memory>

int main() {
    meowtui::App app;
    if (!app.init()) return 1;

    // ── 创建进度条 ────────────────────────────────────
    auto bar = std::make_unique<meowtui::ProgressBar>("Loading:");
    bar->set_rect({2, 2, 40, 1});
    bar->set_progress(0);

    // 保存指针
    meowtui::ProgressBar* bar_ptr = bar.get();
    app.root().add_child(std::move(bar));

    // ── 设置动画（每帧更新进度） ──────────────────────
    // tick 回调会在每帧渲染前被调用
    int progress = 0;
    bool forward = true;
    app.set_tick_callback([bar_ptr, &progress, &forward](
                              const meowtui::Event& ev) {
        // 只在空闲时更新（没有按键事件时）
        if (ev.type == meowtui::EventType::TICK) {
            if (forward) {
                progress++;
                if (progress >= 100) forward = false;
            } else {
                progress--;
                if (progress <= 0) forward = true;
            }
            bar_ptr->set_progress(progress);
        }
    });

    app.run();
    return 0;
}
```

### set_tick_callback 的作用

- `set_tick_callback` 设置一个函数，**每帧都会自动调用**
- 适合做：动画、定时器、后台任务
- 参数 `ev` 是当前事件，可以用来判断程序是否空闲

---

## 8. 完整示例：综合演示

下面是一个综合了所有组件的完整程序。

```cpp
// full_demo.cpp
#include "meowtui/app.h"
#include "meowtui/label.h"
#include "meowtui/button.h"
#include "meowtui/textbox.h"
#include "meowtui/progressbar.h"
#include "meowtui/listbox.h"
#include <memory>
#include <vector>
#include <string>

int main() {
    meowtui::App app;
    if (!app.init()) return 1;

    int sw = app.terminal().width();   // 屏幕宽度
    int sh = app.terminal().height();  // 屏幕高度

    // ── 标题 ──────────────────────────────────────────
    auto title = std::make_unique<meowtui::Label>(
        "=== MeowTUI Demo ===",
        meowtui::Align::CENTER
    );
    title->set_rect({0, 0, sw, 1});
    app.root().add_child(std::move(title));

    // ── 按钮 ──────────────────────────────────────────
    auto btn = std::make_unique<meowtui::Button>("Click me!");
    btn->set_rect({2, 2, 16, 1});

    auto counter = std::make_unique<meowtui::Label>("Count: 0");
    counter->set_rect({20, 2, 12, 1});

    meowtui::Label* counter_ptr = counter.get();
    int click_count = 0;
    btn->set_callback([counter_ptr, &click_count]() {
        click_count++;
        counter_ptr->set_text("Count: " + std::to_string(click_count));
    });

    app.root().add_child(std::move(btn));
    app.root().add_child(std::move(counter));

    // ── 文本框 ────────────────────────────────────────
    auto input = std::make_unique<meowtui::TextBox>("", 30);
    input->set_rect({12, 4, 32, 1});

    input->set_submit_callback([](const std::string& text) {
        // 用户按 Enter 时，text 就是输入的内容
    });

    auto input_label = std::make_unique<meowtui::Label>("Input:");
    input_label->set_rect({2, 4, 8, 1});

    app.root().add_child(std::move(input_label));
    app.root().add_child(std::move(input));

    // ── 进度条 ────────────────────────────────────────
    auto progress = std::make_unique<meowtui::ProgressBar>("Loading:");
    progress->set_rect({2, 6, 40, 1});
    meowtui::ProgressBar* prog_ptr = progress.get();
    app.root().add_child(std::move(progress));

    // ── 列表 ──────────────────────────────────────────
    auto list = std::make_unique<meowtui::ListBox>(
        std::vector<std::string>{
            "New File", "Open File", "Save File",
            "Settings", "Help", "Quit"
        }
    );
    list->set_rect({12, 8, 20, 8});

    auto status = std::make_unique<meowtui::Label>("Ready");
    status->set_rect({2, 17, sw - 2, 1});
    meowtui::Label* status_ptr = status.get();

    list->set_callback([status_ptr](size_t idx, const std::string& item) {
        status_ptr->set_text("Selected: " + item);
    });

    auto menu_label = std::make_unique<meowtui::Label>("Menu:");
    menu_label->set_rect({2, 8, 8, 1});

    app.root().add_child(std::move(menu_label));
    app.root().add_child(std::move(list));
    app.root().add_child(std::move(status));

    // ── 底部提示 ──────────────────────────────────────
    auto hint = std::make_unique<meowtui::Label>(
        "Tab: switch focus  |  Ctrl+C/Q: quit",
        meowtui::Align::CENTER
    );
    hint->set_rect({0, sh - 1, sw, 1});
    app.root().add_child(std::move(hint));

    // ── 动画 ──────────────────────────────────────────
    int prog_val = 0;
    bool prog_dir = true;
    app.set_tick_callback([prog_ptr, &prog_val, &prog_dir](
                              const meowtui::Event& ev) {
        if (ev.type == meowtui::EventType::TICK) {
            if (prog_dir) {
                if (++prog_val >= 100) prog_dir = false;
            } else {
                if (--prog_val <= 0) prog_dir = true;
            }
            prog_ptr->set_progress(prog_val);
        }
    });

    // ── 启动 ──────────────────────────────────────────
    app.run();
    return 0;
}
```

### 编译运行

```bash
# 方法1：用 CMake（推荐）
# 将代码保存到 examples/full_demo.cpp
# 在 CMakeLists.txt 中添加：
#   add_executable(full_demo examples/full_demo.cpp)
#   target_link_libraries(full_demo PRIVATE meowtui)

# 方法2：直接编译
cd MeowTUI
g++ -std=c++17 -I./include full_demo.cpp -L./build -lmeowtui -o full_demo
./full_demo
```

---

## 9. 常见问题

### Q1：编译时报错 "找不到头文件"

```
fatal error: meowtui/app.h: No such file or directory
```

**解决**：编译时需要加 `-I` 参数指定头文件路径：

```bash
g++ -std=c++17 -I./include/meowtui/.. your_code.cpp ...
```

或者使用 CMake（推荐）。

### Q2：运行时报错 "无法初始化终端"

```
// 程序直接退出，没有任何显示
```

**原因**：可能当前终端不支持原始模式，或者标准输入/输出不是终端。

**解决**：在真正的终端窗口中运行，不要在 IDE 的输出面板里运行。

### Q3：程序运行后界面混乱

```
屏幕上有乱码或残留字符
```

**原因**：程序异常退出时未能正确恢复终端设置。

**解决**：在终端中运行 `reset` 命令恢复。或者关闭并重新打开终端窗口。

### Q4：为什么用 `std::make_unique`？

`std::make_unique` 是 C++14 引入的工具，用来创建对象并自动管理内存。

```cpp
// 传统方式（不推荐）：
auto label = new meowtui::Label("Hello");
// 需要手动 delete，容易忘记

// MeowTUI 方式（推荐）：
auto label = std::make_unique<meowtui::Label>("Hello");
// 不需要手动 delete，自动释放
```

使用 `add_child` 后，组件会被"移动"到父容器中，**不需要**自己管理释放。

### Q5：我可以在一个程序里创建多个窗口吗？

MeowTUI 是**单窗口**的——整个终端就是一个窗口。但你可以通过 Widget 的嵌套和布局，在屏幕上实现多区域划分。

### Q6：程序如何退出？

- 按 **Ctrl+C** 或 **Ctrl+Q**
- 或者在代码中调用 `app.stop()`

程序退出后会自动恢复终端到初始状态。

---

## 总结：最基本的代码模板

```cpp
#include "meowtui/app.h"
#include "meowtui/你需要的组件.h"
#include <memory>

int main() {
    // 1. 创建应用
    meowtui::App app;
    if (!app.init()) return 1;

    // 2. 创建组件
    auto widget = std::make_unique<meowtui::Label>("文字");
    widget->set_rect({x, y, 宽, 高});

    // 3. 添加到根容器
    app.root().add_child(std::move(widget));

    // 4. 启动
    app.run();
    return 0;
}
```

掌握了以上内容，你就可以用 MeowTUI 编写自己的终端界面程序了！😸
