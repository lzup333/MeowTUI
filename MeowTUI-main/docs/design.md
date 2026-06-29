# MeowTUI 设计文档

## 概述

MeowTUI 是一个轻量级的 C++ 终端用户界面（TUI）库，采用**结构化、非阻塞**的设计理念，提供简洁易用的 API 来构建终端应用程序。

### 设计目标

- **易用性** —— API 直观简洁，学习成本低
- **无阻塞** —— 事件循环基于 `select()` 非阻塞 I/O
- **结构清晰** —— 采用经典的组合模式（Composite Pattern）
- **标准规范** —— 符合 C++17 标准，良好的代码风格
- **轻量级** —— 零外部依赖，仅使用 POSIX 标准 API
- **易于维护** —— 模块化设计，中文注释覆盖率 > 40%

---

## 架构总览

```
┌─────────────────────────────────────────────────────┐
│                     App (应用主类)                    │
│  ┌──────────┐  ┌──────────┐  ┌──────────────────┐  │
│  │ Terminal │  │EventLoop │  │ Screen + Widgets │  │
│  │ 终端管理  │  │ 事件循环  │  │ 渲染 + UI组件   │  │
│  └──────────┘  └──────────┘  └──────────────────┘  │
└─────────────────────────────────────────────────────┘
```

### 模块划分

| 模块 | 文件 | 职责 |
|------|------|------|
| **common** | `common.h` | 公共类型、枚举、常量定义 |
| **rect** | `rect.h` | 矩形区域类，布局的几何基础 |
| **terminal** | `terminal.h/.cpp` | 终端原始模式、ANSI 控制、尺寸查询 |
| **event** | `event.h/.cpp` | 非阻塞事件读取与解析 |
| **screen** | `screen.h/.cpp` | 双缓冲渲染引擎 |
| **widget** | `widget.h/.cpp` | Widget 基类与组合模式实现 |
| **app** | `app.h/.cpp` | 应用主类，整合所有模块 |
| **label** | `label.h/.cpp` | 文本标签组件 |
| **button** | `button.h/.cpp` | 按钮组件 |
| **textbox** | `textbox.h/.cpp` | 文本框组件 |
| **progressbar** | `progressbar.h/.cpp` | 进度条组件 |
| **listbox** | `listbox.h/.cpp` | 列表选择框组件 |

---

## 核心设计模式

### 1. 组合模式（Composite Pattern）

Widget 树采用组合模式：

```
App
 └── Widget (root)
      ├── Label
      ├── Button
      │    └── ... (子 Widget)
      ├── TextBox
      └── ListBox
           └── ... (子 Widget)
```

- 每个 Widget 可以有多个子 Widget
- 渲染时递归遍历整棵树
- 事件处理也是递归分发

### 2. 双缓冲渲染（Double Buffering）

Screen 类实现了经典的双缓冲机制：

- **后缓冲区（back buffer）**：当前帧正在构建的内容
- **前缓冲区（front buffer）**：上一帧已经显示的内容
- **差异刷新（delta flush）**：只将变化的部分输出到终端

```
帧 N 开始 → 清空 back → 绘制 Widget → 比较 back/front → 输出差异 → 交换缓冲区
```

### 3. 非阻塞事件循环

EventLoop 使用 `select()` 系统调用实现非阻塞输入：

```
poll() 调用
  ↓
select(STDIN, timeout=0) → 有输入？→ 读取并解析 → 返回 Event
                      ↓ 无输入
                  返回 TICK 事件
```

---

## 数据流

### 初始化流程

```
App::init()
  ↓
Terminal::initialize() → 原始模式 + 备用屏幕
  ↓
Screen::resize()      → 分配缓冲区
  ↓
Widget::set_rect()    → 设置根 Widget 尺寸
```

### 主循环流程

```
App::run()
  ↓
┌─ EventLoop::poll() ←──────────────┐
│    ↓                               │
│  Event 分发                         │
│    ↓                               │
│  ESC/Ctrl+C/Ctrl+Q → exit          │
│    ↓                               │
│  Widget::handle_event(ev)          │
│    ↓                               │
│  tick_callback(ev)                 │
│    ↓                               │
│  render_frame():                   │
│    Screen::clear()                 │
│    Widget::render(screen)          │
│    Screen::flush() (差异输出)       │
│    ↓                               │
│  帧率控制 (sleep) ──────────────────┘
```

---

## Widget 生命周期

1. **构造** —— 设置文本、位置、尺寸
2. **添加到父容器** —— `parent->add_child(std::move(child))`
3. **渲染** —— `render()` 方法被递归调用
4. **事件处理** —— `handle_event()` 接收并响应事件
5. **焦点管理** —— `set_focused()` / `focusable()`
6. **析构** —— 自动从父容器移除

---

## 颜色系统

MeowTUI 支持标准 16 色 ANSI 颜色：

| 颜色常量 | 值 | 说明 |
|----------|-----|------|
| `COLOR_BLACK` | 0 | 黑色 |
| `COLOR_RED` | 1 | 红色 |
| `COLOR_GREEN` | 2 | 绿色 |
| `COLOR_YELLOW` | 3 | 黄色 |
| `COLOR_BLUE` | 4 | 蓝色 |
| `COLOR_MAGENTA` | 5 | 紫色 |
| `COLOR_CYAN` | 6 | 青色 |
| `COLOR_WHITE` | 7 | 白色 |
| `COLOR_BRIGHT_*` | 8-15 | 亮色版本 |

---

## 文件结构

```
MeowTUI/
├── CMakeLists.txt           # CMake 构建配置
├── README.md                # 项目说明
├── include/
│   └── meowtui/
│       ├── common.h         # 公共类型与常量
│       ├── rect.h           # 矩形区域
│       ├── terminal.h       # 终端控制
│       ├── event.h          # 事件系统
│       ├── screen.h         # 双缓冲屏幕
│       ├── widget.h         # Widget 基类
│       ├── app.h            # 应用主类
│       ├── label.h          # 标签
│       ├── button.h         # 按钮
│       ├── textbox.h        # 文本框
│       ├── progressbar.h    # 进度条
│       └── listbox.h        # 列表选择框
├── src/
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
├── examples/
│   ├── minimal.cpp          # 最小示例
│   └── demo.cpp             # 综合示例
└── docs/
    ├── design.md            # 设计文档（本文）
    ├── guide.md             # 使用指南
    └── api.md               # API 参考
```
