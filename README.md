# MeowTUI 🐱

> **轻量级 C++ 终端 UI 库** —— 简洁 · 易用 · 无阻塞 · 喵~

MeowTUI 是一个从零构建的 C++17 终端用户界面库，零外部依赖，仅使用 POSIX 标准 API。
采用**双缓冲渲染**、**组合模式 Widget 树**和**非阻塞事件循环**三大核心设计。

---

## ✨ 特性

- **零外部依赖** —— 仅需 C++17 编译器和 POSIX 系统
- **非阻塞 I/O** —— 基于 `select()` 的事件循环，绝不阻塞
- **双缓冲渲染** —— 只刷新变化区域，性能优秀
- **组合模式** —— Widget 树结构清晰，易于扩展
- **丰富组件** —— Label、Button、TextBox、ProgressBar、ListBox
- **中文友好** —— 40%+ 中文注释覆盖率
- **完整文档** —— 设计文档 + 使用指南 + API 参考
- **标准规范** —— 符合 C++17 标准，代码整洁有序

---

## 🚀 快速开始

```bash
git clone <repo-url>
cd MeowTUI
mkdir build && cd build
cmake ..
make

# 运行最小示例
./meowtui_minimal

# 运行综合示例
./meowtui_demo
```

### 最简单的程序

```cpp
#include "meowtui/app.h"
#include "meowtui/label.h"

int main() {
    meowtui::App app;
    if (!app.init()) return 1;

    auto label = std::make_unique<meowtui::Label>(
        "Hello, MeowTUI!", meowtui::Align::CENTER);
    label->set_rect({30, 12, 20, 1});
    app.root().add_child(std::move(label));

    app.run();  // ESC/Ctrl+C/Q 退出
    return 0;
}
```

---

## 🧩 可用组件

| 组件 | 头文件 | 说明 |
|------|--------|------|
| `Label` | `label.h` | 静态文本标签，支持对齐 |
| `Button` | `button.h` | 可点击按钮，支持回调 |
| `TextBox` | `textbox.h` | 单行文本输入框 |
| `ProgressBar` | `progressbar.h` | 进度条显示 |
| `ListBox` | `listbox.h` | 可滚动列表选择框 |

---

## 📁 项目结构

```
MeowTUI/
├── CMakeLists.txt      # CMake 构建配置
├── README.md           # 本文件
├── include/meowtui/    # 公共头文件
│   ├── common.h        # 类型与常量定义
│   ├── rect.h          # 矩形区域
│   ├── terminal.h      # 终端控制
│   ├── event.h         # 事件系统
│   ├── screen.h        # 双缓冲屏幕
│   ├── widget.h        # Widget 基类
│   ├── app.h           # 应用主类
│   ├── label.h         # 标签组件
│   ├── button.h        # 按钮组件
│   ├── textbox.h       # 文本框组件
│   ├── progressbar.h   # 进度条组件
│   └── listbox.h       # 列表选择框
├── src/                # 实现文件
│   ├── terminal.cpp
│   ├── event.cpp
│   ├── screen.cpp
│   ├── widget.cpp
│   ├── app.cpp
│   ├── label.cpp
│   ├── button.cpp
│   ├── textbox.cpp
│   ├── progressbar.cpp
│   └── listbox.cpp
├── examples/           # 示例程序
│   ├── minimal.cpp     # 最小示例
│   └── demo.cpp        # 综合示例
└── docs/               # 文档
    ├── design.md       # 设计文档
    ├── guide.md        # 使用指南
    └── api.md          # API 参考
```

---

## 📚 文档

- [设计文档](docs/design.md) — 架构设计、核心模式、数据流
- [使用指南](docs/guide.md) — 快速上手、组件用法、最佳实践
- [API 参考](docs/api.md) — 完整 API 接口说明

---

## 🏗️ 构建要求

- C++17 兼容的编译器（GCC 8+ / Clang 7+）
- CMake 3.16+
- POSIX 兼容操作系统（Linux、macOS、WSL）

---

## 📄 许可

MIT License

---

## 🐾 设计理念

> **"Keep it simple, make it work, then make it better."**

MeowTUI 不是又一个"大而全"的框架，而是一个**小而美**的工具库。
它的设计哲学是：
1. **简单胜过复杂** —— 能用 10 行解决的问题，绝不写 100 行
2. **结构胜过技巧** —— 清晰的代码结构比花哨的技巧更有价值
3. **实用胜过完美** —— 满足 80% 的需求，而不是追求 100% 的完美

## ⚠️ 声明

> **本项目由ai辅助生成**

> **目前处于开发阶段，可能出现不稳定/功能异常**
 
> **介意请勿使用**

