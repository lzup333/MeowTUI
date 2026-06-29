cmake_minimum_required(VERSION 3.16)
project(MeowTUI VERSION 0.1.0 LANGUAGES CXX)

# ============================================================================
# MeowTUI —— 轻量级 C++ 终端 UI 库
# 简洁 · 易用 · 无阻塞
# ============================================================================

# ── 编译选项 ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 根据构建类型设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -g -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -O2")
endif()

# ── 库源文件 ──────────────────────────────────────────────────────────────
set(MEOWTUI_SOURCES
    src/terminal.cpp
    src/event.cpp
    src/screen.cpp
    src/widget.cpp
    src/label.cpp
    src/button.cpp
    src/textbox.cpp
    src/progressbar.cpp
    src/listbox.cpp
    src/app.cpp
)

set(MEOWTUI_HEADERS
    include/meowtui/common.h
    include/meowtui/terminal.h
    include/meowtui/event.h
    include/meowtui/screen.h
    include/meowtui/rect.h
    include/meowtui/widget.h
    include/meowtui/label.h
    include/meowtui/button.h
    include/meowtui/textbox.h
    include/meowtui/progressbar.h
    include/meowtui/listbox.h
    include/meowtui/app.h
)

# ── 构建静态库 ────────────────────────────────────────────────────────────
add_library(meowtui STATIC ${MEOWTUI_SOURCES} ${MEOWTUI_HEADERS})
target_include_directories(meowtui PUBLIC include)


