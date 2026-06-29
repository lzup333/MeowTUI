# MeowTUI 使用指南

## 快速开始

### 1. 编译安装

```bash
git clone <repo-url>
cd MeowTUI
mkdir build && cd build
cmake ..
make
```

编译后会在 `build/` 目录下生成：
- `libmeowtui.a` — 静态库
- `meowtui_demo` — 综合示例
- `meowtui_minimal` — 最小示例

### 2. 最小程序

```cpp
#include "meowtui/app.h"
#include "meowtui/label.h"

int main() {
    meowtui::App app;
    if (!app.init()) return 1;

    // 创建一个居中标签
    auto label = std::make_unique<meowtui::Label>(
        "Hello, MeowTUI!", meowtui::Align::CENTER);
    label->set_rect({35, 12, 20, 1});  // {x, y, w, h}
    app.root().add_child(std::move(label));

    app.run();  // 按 ESC 或 Ctrl+C/Q 退出
    return 0;
}
```

---

## 基本概念

### App（应用主类）

`App` 是所有 MeowTUI 程序的入口，整合了终端、事件循环和渲染系统。

```cpp
meowtui::App app;

// 生命周期
app.init();     // 初始化
app.run();      // 运行主循环
app.shutdown(); // 清理（析构函数自动调用）
```

### Widget（组件基类）

所有 UI 组件都继承自 `Widget` 基类。

```cpp
auto widget = std::make_unique<meowtui::Button>("Click");

// 位置和尺寸
widget->set_rect({10, 5, 20, 3});  // {x, y, width, height}
// 或分别设置
widget->set_position(10, 5);
widget->set_size(20, 3);

// 添加到父容器
app.root().add_child(std::move(widget));
```

### Rect（矩形区域）

`Rect` 描述 Widget 在屏幕上的位置和尺寸：

```cpp
meowtui::Rect r{10, 5, 30, 8};
// r.x  = 10  (列坐标)
// r.y  = 5   (行坐标)
// r.w  = 30  (宽度)
// r.h  = 8   (高度)
```

---

## 可用组件

### Label（标签）— `label.h`

显示静态文本，不可交互。

```cpp
#include "meowtui/label.h"

auto label = std::make_unique<meowtui::Label>(
    "Hello World",
    meowtui::Align::CENTER  // LEFT / CENTER / RIGHT
);
label->set_rect({0, 0, 80, 1});
```

### Button（按钮）— `button.h`

可点击的交互按钮，支持回调函数。

```cpp
#include "meowtui/button.h"

auto btn = std::make_unique<meowtui::Button>("Submit");
btn->set_rect({10, 5, 16, 1});

// 设置点击回调
btn->set_callback([]() {
    // 按钮被点击时执行
});
```

### TextBox（文本框）— `textbox.h`

单行文本输入框。

```cpp
#include "meowtui/textbox.h"

auto input = std::make_unique<meowtui::TextBox>("", 30);  // 最大30字符
input->set_rect({10, 8, 32, 1});

// 设置提交回调（Enter 时触发）
input->set_submit_callback([](const std::string& text) {
    // text 是当前输入的内容
});
```

### ProgressBar（进度条）— `progressbar.h`

可视化进度显示。

```cpp
#include "meowtui/progressbar.h"

auto bar = std::make_unique<meowtui::ProgressBar>("Loading:");
bar->set_rect({2, 10, 40, 1});
bar->set_progress(50);  // 0~100
```

### ListBox（列表选择框）— `listbox.h`

可滚动的选项列表。

```cpp
#include "meowtui/listbox.h"

auto list = std::make_unique<meowtui::ListBox>(
    std::vector<std::string>{
        "Option 1",
        "Option 2",
        "Option 3",
    }
);
list->set_rect({10, 5, 20, 8});

// 设置选择回调
list->set_callback([](size_t index, const std::string& item) {
    // index: 选中索引 (0-based)
    // item:  选中文本
});
```

---

## 焦点管理

按 `Tab` 键在可聚焦的 Widget 之间切换焦点。

```cpp
// 判断 Widget 是否可获得焦点
if (widget->focusable()) { ... }

// 手动设置焦点
widget->set_focused(true);

// 判断是否拥有焦点
if (widget->is_focused()) { ... }
```

> Button、TextBox、ListBox 默认可获得焦点。
> Label 和 ProgressBar 不可获得焦点。

---

## 事件处理

### 按键事件

```cpp
// 在 Widget 的 handle_event 中
bool handle_event(const meowtui::Event& ev) override {
    if (ev.type == meowtui::EventType::KEY) {
        if (ev.is_printable()) {
            // 可打印字符: ev.ch
        }
        switch (ev.key) {
            case meowtui::Key::UP:    /* ↑ 键 */ break;
            case meowtui::Key::ENTER: /* 回车 */ break;
            case meowtui::Key::ESCAPE: /* ESC */ break;
            // ...
        }
    }
    return false;  // 返回 true 表示事件已处理
}
```

### Tick 回调（动画）

```cpp
// 每帧渲染前调用，适合做动画
app.set_tick_callback([](const meowtui::Event& ev) {
    if (ev.type == meowtui::EventType::TICK) {
        // 更新动画状态...
    }
});
```

---

## 帧率控制

```cpp
// 设置帧率上限（默认 60）
app.set_fps(30);  // 降低帧率以减少 CPU 使用
```

---

## 键盘快捷键

| 按键 | 功能 |
|------|------|
| `ESC` | 退出程序 |
| `Ctrl+C` | 退出程序 |
| `Ctrl+Q` | 退出程序 |
| `Tab` | 切换焦点 |
| `↑/↓` | 列表导航 |
| `Enter` | 确认/提交 |
| `Backspace` | 删除字符（文本框） |
| `Home/End` | 跳到首/尾（文本框/列表） |
| `PgUp/PgDn` | 翻页（列表） |

---

## 集成到现有项目

### CMake

```cmake
# 在你的 CMakeLists.txt 中
add_subdirectory(path/to/MeowTUI)
target_link_libraries(your_app PRIVATE meowtui)
target_include_directories(your_app PRIVATE path/to/MeowTUI/include)
```

### 手动编译

```bash
g++ -std=c++17 -Ipath/to/MeowTUI/include \
    your_app.cpp -Lpath/to/MeowTUI/build -lmeowtui \
    -o your_app
```
