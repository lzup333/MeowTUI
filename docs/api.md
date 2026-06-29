# MeowTUI API 参考文档

## 模块索引

- [common.h](#commonh)
- [rect.h](#recth)
- [terminal.h](#terminalh)
- [event.h](#eventh)
- [screen.h](#screenh)
- [widget.h](#widgeth)
- [app.h](#apph)
- [label.h](#labelh)
- [button.h](#buttonh)
- [textbox.h](#textboxh)
- [progressbar.h](#progressbarh)
- [listbox.h](#listboxh)

---

## common.h

### 类型别名

```cpp
using Col = int;   // 列坐标（水平方向）
using Row = int;   // 行坐标（垂直方向）
using Color = uint8_t;  // 颜色值
```

### 颜色常量

```cpp
enum ColorName : Color {
    COLOR_BLACK       = 0,
    COLOR_RED         = 1,
    COLOR_GREEN       = 2,
    COLOR_YELLOW      = 3,
    COLOR_BLUE        = 4,
    COLOR_MAGENTA     = 5,
    COLOR_CYAN        = 6,
    COLOR_WHITE       = 7,
    COLOR_BRIGHT_BLACK   = 8,
    COLOR_BRIGHT_RED     = 9,
    // ... 8~15 为亮色版本
};
```

### 按键枚举

```cpp
enum class Key : uint32_t {
    NONE, TAB, ENTER, ESCAPE, BACKSPACE,  // 基本键
    F1~F12,                                // 功能键
    UP, DOWN, LEFT, RIGHT,                 // 方向键
    HOME, END, PAGE_UP, PAGE_DOWN,         // 编辑键
    INSERT, DELETE,
    CTRL_A~CTRL_Z,                         // Ctrl 组合
};
```

### 事件类型

```cpp
enum class EventType { NONE, KEY, RESIZE, TICK, QUIT };
```

### 对齐方式

```cpp
enum class Align { LEFT, CENTER, RIGHT };
```

---

## rect.h

### Rect 结构体

```cpp
struct Rect {
    Col x = 0;      // 左上角列坐标
    Row y = 0;      // 左上角行坐标
    int w = 0;      // 宽度
    int h = 0;      // 高度

    Rect();
    Rect(Col x, Row y, int w, int h);

    bool contains(Col cx, Row cy) const;  // 点是否在矩形内
    Rect intersect(const Rect& other) const;  // 交集
    bool valid() const;  // 是否有效（w>0 && h>0）
};
```

---

## terminal.h

### Terminal 类（单例）

```cpp
class Terminal {
    static Terminal& instance();  // 获取单例

    int width() const;       // 终端宽度
    int height() const;      // 终端高度
    void refresh_size();     // 刷新尺寸缓存

    void hide_cursor();      // 隐藏光标
    void show_cursor();      // 显示光标
    void move_cursor(Col col, Row row);  // 移动光标

    void clear_screen();     // 清屏
    void clear_to_eol();     // 清空到行尾
    void set_fg(Color c);    // 设置前景色
    void set_bg(Color c);    // 设置背景色
    void reset_color();      // 重置颜色
    void set_bold(bool bold);  // 设置粗体
    void set_reverse(bool rev);  // 设置反转

    void write(const std::string& s);   // 写入字符串
    void writeln(const std::string& s); // 写入并换行
    void flush();             // 刷新输出

    bool initialize();        // 初始化终端
    void shutdown();          // 恢复终端
    bool is_initialized() const;
};
```

---

## event.h

### Event 结构体

```cpp
struct Event {
    EventType type = EventType::NONE;
    Key       key  = Key::NONE;
    char      ch   = 0;

    static Event from_key(Key k);
    static Event from_char(char c);
    static Event from_resize();
    static Event from_tick();
    static Event from_quit();

    bool is_printable() const;  // 是否可打印字符
};
```

### EventLoop 类

```cpp
class EventLoop {
    Event poll();            // 非阻塞轮询事件
    void quit();             // 设置退出标志
    bool should_quit() const;
    void reset_quit();
};
```

---

## screen.h

### Cell 结构体

```cpp
struct Cell {
    char  ch  = ' ';     // 字符
    Color fg  = 7;       // 前景色
    Color bg  = 0;       // 背景色
    bool  bold = false;  // 粗体
};
```

### Screen 类

```cpp
class Screen {
    void resize(int w, int h);
    int width() const;
    int height() const;

    void set_cell(Col col, Row row, char ch,
                  Color fg = COLOR_WHITE,
                  Color bg = COLOR_BLACK,
                  bool bold = false);

    void write_str(Col col, Row row, const std::string& text,
                   Color fg = COLOR_WHITE,
                   Color bg = COLOR_BLACK,
                   bool bold = false);

    void fill_rect(const Rect& rect, char ch,
                   Color fg = COLOR_WHITE,
                   Color bg = COLOR_BLACK,
                   bool bold = false);

    void draw_border(const Rect& rect,
                     Color fg = COLOR_WHITE,
                     Color bg = COLOR_BLACK,
                     bool bold = false);

    void clear();    // 清空后缓冲区
    void flush();    // 差异刷新到终端

    void set_clip_rect(const Rect& rect);
    Rect clip_rect() const;
    void reset_clip_rect();
};
```

---

## widget.h

### Widget 类（所有组件的基类）

```cpp
class Widget {
    explicit Widget(const std::string& text = "");
    virtual ~Widget();

    // 父子关系
    void set_parent(Widget* parent);
    Widget* parent() const;
    void add_child(std::unique_ptr<Widget> child);
    const auto& children() const;
    Widget* child_at(size_t index) const;
    void clear_children();

    // 位置与尺寸
    void set_rect(const Rect& r);
    const Rect& rect() const;
    void set_position(Col x, Row y);
    void set_size(int w, int h);

    // 焦点
    virtual void set_focused(bool focused);
    bool is_focused() const;
    virtual bool focusable() const;

    // 文本
    virtual void set_text(const std::string& text);
    const std::string& text() const;

    // 渲染与事件（虚函数，子类重写）
    virtual void render(Screen& screen);
    virtual bool handle_event(const Event& ev);

    // 可见性与状态
    void set_visible(bool v);
    bool visible() const;
    void set_state(WidgetState s);
    WidgetState state() const;
    bool disabled() const;

    // 标识
    void set_id(const std::string& id);
    const std::string& id() const;
};
```

---

## app.h

### App 类

```cpp
class App {
    App();
    ~App();

    bool init();       // 初始化
    void run();        // 运行主循环
    void stop();       // 停止循环
    void shutdown();   // 清理

    Widget& root();            // 根 Widget
    Terminal& terminal();      // 终端引用
    Screen& screen();          // 屏幕引用
    EventLoop& event_loop();   // 事件循环

    void refresh();            // 手动刷新
    void set_fps(int fps);     // 设置帧率
    int fps() const;

    void set_tick_callback(EventCallback cb);  // tick 回调
};
```

---

## label.h

### Label 类

```cpp
class Label : public Widget {
    explicit Label(const std::string& text = "",
                   Align align = Align::LEFT);
    void set_align(Align a);
    Align align() const;
    bool focusable() const override { return false; }
};
```

---

## button.h

### Button 类

```cpp
using ButtonCallback = std::function<void()>;

class Button : public Widget {
    explicit Button(const std::string& text = "",
                    ButtonCallback cb = nullptr);
    void set_callback(ButtonCallback cb);
    bool focusable() const override;
};
```

---

## textbox.h

### TextBox 类

```cpp
class TextBox : public Widget {
    explicit TextBox(const std::string& text = "",
                     size_t max_len = 0);
    bool focusable() const override;

    void set_submit_callback(
        std::function<void(const std::string&)> cb);
    void set_max_length(size_t max);
    size_t max_length() const;
};
```

---

## progressbar.h

### ProgressBar 类

```cpp
class ProgressBar : public Widget {
    explicit ProgressBar(const std::string& text = "");
    void set_progress(int p);       // 0~100
    int progress() const;
    void set_fill_char(char c);
    void set_fill_color(Color c);
    bool focusable() const override { return false; }
};
```

---

## listbox.h

### ListBox 类

```cpp
using ListCallback = std::function<void(
    size_t index, const std::string& item)>;

class ListBox : public Widget {
    explicit ListBox(
        const std::vector<std::string>& items = {});
    bool focusable() const override;

    void set_items(const std::vector<std::string>& items);
    const auto& items() const;
    size_t selected_index() const;
    std::string selected_item() const;

    void set_callback(ListCallback cb);
    void set_selected(size_t index);
};
```
