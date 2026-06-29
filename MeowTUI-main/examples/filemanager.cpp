/**
 * @file filemanager.cpp
 * @brief TUI 文件管理器示例 —— 展示 MeowTUI 综合应用
 *
 * 这个示例实现了一个双面板的文件管理器，包含：
 * - 左侧文件列表（可导航）
 * - 右侧文件内容预览
 * - 顶部路径栏
 * - 底部操作提示栏
 * - 目录导航（进入子目录、返回父目录）
 *
 * 编译：
 *   mkdir build && cd build
 *   cmake ..
 *   make meowtui_filemanager
 *   ./meowtui_filemanager
 *
 * 按键操作：
 *   ↑/↓        —— 上下移动选中项
 *   Enter      —— 进入目录 / 查看文件
 *   Backspace  —— 返回上级目录
 *   Tab        —— 切换焦点面板
 *   Ctrl+C/Q   —— 退出
 *   ~          —— 跳转到 home 目录
 *   /          —— 跳转到根目录
 */

#include "meowtui/app.h"
#include "meowtui/label.h"
#include "meowtui/listbox.h"
#include "meowtui/textbox.h"
#include "meowtui/button.h"

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>

// ============================================================================
// 文件系统工具（跨平台兼容 POSIX）
// ============================================================================

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <cstring>

/**
 * @brief 获取用户 home 目录
 */
static std::string get_home_dir() {
    const char* home = getenv("HOME");
    if (home) return home;
    struct passwd* pw = getpwuid(getuid());
    if (pw) return pw->pw_dir;
    return "/";
}

/**
 * @brief 获取文件类型字符标识
 */
static char get_file_type_char(mode_t mode) {
    if (S_ISDIR(mode))  return 'D';
    if (S_ISLNK(mode))  return 'L';
    if (S_ISREG(mode))  return ' ';
    if (S_ISFIFO(mode)) return 'p';
    if (S_ISSOCK(mode)) return 's';
    if (S_ISCHR(mode))  return 'c';
    if (S_ISBLK(mode))  return 'b';
    return '?';
}

/**
 * @brief 获取文件大小格式化字符串
 */
static std::string format_file_size(off_t size) {
    const char* units[] = {"B", "K", "M", "G", "T"};
    int unit_idx = 0;
    double s = static_cast<double>(size);

    while (s >= 1024.0 && unit_idx < 4) {
        s /= 1024.0;
        unit_idx++;
    }

    if (unit_idx == 0) {
        return std::to_string(size) + "B";
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f%s", s, units[unit_idx]);
    return buf;
}

/**
 * @brief 目录条目结构
 */
struct DirEntry {
    std::string name;       ///< 文件名
    std::string path;       ///< 完整路径
    bool        is_dir;     ///< 是否为目录
    off_t       size;       ///< 文件大小
    mode_t      mode;       ///< 文件权限
};

/**
 * @brief 扫描目录，返回排序后的条目列表
 */
static std::vector<DirEntry> scan_directory(const std::string& dir_path) {
    std::vector<DirEntry> entries;

    DIR* dir = opendir(dir_path.c_str());
    if (!dir) return entries;

    struct dirent* dent;
    while ((dent = readdir(dir)) != nullptr) {
        std::string name = dent->d_name;

        // 跳过 . 和 ..
        if (name == ".") continue;

        std::string full_path = dir_path + "/" + name;
        struct stat st;
        if (stat(full_path.c_str(), &st) != 0) continue;

        entries.push_back({
            name,
            full_path,
            S_ISDIR(st.st_mode),
            st.st_size,
            st.st_mode
        });
    }
    closedir(dir);

    // 排序：目录在前，文件在后，各自按字母序
    std::sort(entries.begin(), entries.end(),
              [](const DirEntry& a, const DirEntry& b) {
                  if (a.is_dir != b.is_dir) return a.is_dir > b.is_dir;
                  return a.name < b.name;
              });

    return entries;
}

/**
 * @brief 读取文件前若干行用于预览
 */
static std::string read_file_preview(const std::string& file_path, int max_lines = 50) {
    std::ifstream file(file_path);
    if (!file.is_open()) return "(无法打开文件)";

    std::ostringstream oss;
    std::string line;
    int line_count = 0;

    while (std::getline(file, line) && line_count < max_lines) {
        oss << line << '\n';
        line_count++;
    }

    std::string result = oss.str();
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    if (!file.eof()) {
        result += "\n... (文件截断，仅显示前" + std::to_string(max_lines) + "行)";
    }

    return result;
}

// ============================================================================
// 文件管理器应用
// ============================================================================

/**
 * @brief 主文件管理器类
 *
 * 管理整个文件管理器的状态和 UI 布局。
 */
class FileManager {
public:
    FileManager() = default;

    /**
     * @brief 初始化并运行文件管理器
     */
    int run() {
        meowtui::App app;

        if (!app.init()) {
            return 1;
        }

        app_ = &app;
        current_path_ = get_home_dir();
        refresh_file_list();

        // 构建 UI
        build_ui(app);

        // 进入主循环
        app.run();
        return 0;
    }

    /**
     * @brief 刷新文件列表（重新扫描当前目录）
     */
    void refresh_file_list() {
        entries_ = scan_directory(current_path_);

        // 添加 ".." 条目（除非已经是根目录）
        entries_.insert(entries_.begin(), DirEntry{
            "..",  // 显示为 ..
            get_parent_path(current_path_),
            true,
            0,
            0
        });
    }

    /**
     * @brief 获取父目录路径
     */
    static std::string get_parent_path(const std::string& path) {
        if (path == "/") return "/";
        size_t pos = path.rfind('/');
        if (pos == 0) return "/";
        return path.substr(0, pos);
    }

    /**
     * @brief 获取当前目录的显示名称
     */
    static std::string get_dir_name(const std::string& path) {
        if (path == "/") return "/ (根目录)";
        size_t pos = path.rfind('/');
        return path.substr(pos + 1);
    }

    // ── 回调函数（由 UI 组件调用） ─────────────────────────────────────

    /**
     * @brief 进入选中的目录或查看文件
     */
    void on_enter_directory(size_t index, const std::string& item) {
        if (index >= entries_.size()) return;

        const auto& entry = entries_[index];

        if (entry.is_dir) {
            // 进入目录
            if (entry.name == ".." && current_path_ == "/") return;
            current_path_ = entry.path;
            refresh_file_list();
            update_file_list_ui();
            update_path_label();
            update_preview();
        } else {
            // 预览文件
            update_preview();
        }
    }

    /**
     * @brief 返回上级目录
     */
    void on_go_up() {
        if (current_path_ == "/") return;
        current_path_ = get_parent_path(current_path_);
        refresh_file_list();
        update_file_list_ui();
        update_path_label();
        update_preview();
    }

    /**
     * @brief 跳转到指定路径
     */
    void on_navigate_to(const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            current_path_ = path;
            refresh_file_list();
            update_file_list_ui();
            update_path_label();
            update_preview();
        }
    }

    void on_selection_changed(size_t index, const std::string& item) {
        if (index < entries_.size() && !entries_[index].is_dir) {
            update_preview();
        }
    }

    void update_preview() {
        if (!preview_label_) return;

        const auto& lb = file_list_;
        if (!lb) return;

        size_t sel = lb->selected_index();
        if (sel >= entries_.size()) return;

        const auto& entry = entries_[sel];
        if (entry.is_dir) {
            // 显示目录信息：子目录数和文件数
            int dir_count = 0, file_count = 0;
            for (const auto& e : entries_) {
                if (e.name == "..") continue;
                if (e.is_dir) dir_count++;
                else file_count++;
            }
            preview_label_->set_text(
                "  目录: " + current_path_ + "\n\n"
                "  子目录: " + std::to_string(dir_count) + "\n"
                "  文件数: " + std::to_string(file_count)
            );
        } else {
            // 显示文件预览
            std::string preview = read_file_preview(entry.path, 100);
            preview_label_->set_text(preview);
        }
    }

    // ── UI 构建与更新 ──────────────────────────────────────────────────

    void build_ui(meowtui::App& app) {
        int sw = app.terminal().width();
        int sh = app.terminal().height();

        // ── 顶部标题栏 ──────────────────────────────────────────────────
        {
            auto title = std::make_unique<meowtui::Label>(
                " MeowTUI File Manager ", meowtui::Align::LEFT);
            title->set_rect({0, 0, sw, 1});
            title->set_id("title_bar");
            // 标题栏使用反转色
            title_ = title.get();
            app.root().add_child(std::move(title));
        }

        // ── 路径显示栏 ──────────────────────────────────────────────────
        {
            auto path_label = std::make_unique<meowtui::Label>(
                " " + current_path_, meowtui::Align::LEFT);
            path_label->set_rect({0, 1, sw, 1});
            path_label->set_id("path_bar");
            path_label_ = path_label.get();
            app.root().add_child(std::move(path_label));
        }

        // ── 左侧文件列表 ────────────────────────────────────────────────
        int list_width = sw / 3;
        int content_height = sh - 4;  // 减去标题、路径、状态栏和分割线

        {
            // 构建文件名字符串列表供 ListBox 使用
            std::vector<std::string> file_names;
            for (const auto& e : entries_) {
                std::string display = e.name;
                if (e.is_dir && e.name != "..") {
                    display += "/";
                }
                file_names.push_back(display);
            }

            auto listbox = std::make_unique<meowtui::ListBox>(file_names);
            listbox->set_rect({0, 2, list_width, content_height});
            listbox->set_id("file_list");

            // 保存指针
            file_list_ = listbox.get();

            // 设置选择回调
            listbox->set_callback([this](size_t index, const std::string& item) {
                (void)item;
                this->on_enter_directory(index, item);
            });

            app.root().add_child(std::move(listbox));

            // 分割线：在列表右侧画一条竖线
            // 用一个 Label 填充 '|' 字符来实现
            auto separator = std::make_unique<meowtui::Label>("", meowtui::Align::LEFT);
            separator->set_id("separator");
            // 不实际使用 Label 渲染，但保留位置
            separator->set_rect({list_width, 2, 1, content_height});
            separator->set_visible(false);
            app.root().add_child(std::move(separator));
        }

        // ── 右侧预览区 ──────────────────────────────────────────────────
        int preview_x = list_width + 1;
        int preview_w = sw - preview_x;

        {
            auto preview = std::make_unique<meowtui::Label>("", meowtui::Align::LEFT);
            preview->set_rect({preview_x, 2, preview_w, content_height});
            preview->set_id("preview");
            preview_label_ = preview.get();
            app.root().add_child(std::move(preview));

            // 初始预览
            update_preview();
        }

        // ── 底部状态栏 ──────────────────────────────────────────────────
        {
            std::string hint =
                "  ↑↓: 导航  Enter: 进入/查看  "
                "Backspace: 返回上级  Tab: 切换焦点  "
                "~: Home  /: 根目录  Ctrl+C/Q: 退出";
            auto status = std::make_unique<meowtui::Label>(hint, meowtui::Align::LEFT);
            status->set_rect({0, sh - 1, sw, 1});
            status->set_id("status_bar");
            status_bar_ = status.get();
            app.root().add_child(std::move(status));
        }

        // ── 设置 tick 回调（更新路径和状态显示） ────────────────────────
        app.set_tick_callback([this](const meowtui::Event& ev) {
            if (ev.type == meowtui::EventType::KEY) {
                // 处理全局快捷键
                if (ev.key == meowtui::Key::BACKSPACE) {
                    this->on_go_up();
                }
                // '~' 跳转到 home
                if (ev.ch == '~') {
                    this->on_navigate_to(get_home_dir());
                }
                // '/' 跳转到根目录
                if (ev.ch == '/') {
                    this->on_navigate_to("/");
                }
            }
            if (ev.type == meowtui::EventType::RESIZE) {
                // 终端尺寸变化时重新布局
                this->relayout();
            }
        });
    }

    /**
     * @brief 更新文件列表 UI（重新设置 ListBox 的选项）
     */
    void update_file_list_ui() {
        if (!file_list_) return;

        std::vector<std::string> file_names;
        for (const auto& e : entries_) {
            std::string display = e.name;
            if (e.is_dir && e.name != "..") {
                display += "/";
            }
            file_names.push_back(display);
        }
        file_list_->set_items(file_names);
    }

    /**
     * @brief 更新路径标签
     */
    void update_path_label() {
        if (path_label_) {
            path_label_->set_text(" " + current_path_);
        }
    }

    /**
     * @brief 终端尺寸变化时重新布局
     */
    void relayout() {
        if (!app_) return;
        int sw = app_->terminal().width();
        int sh = app_->terminal().height();
        int list_width = sw / 3;
        int content_height = sh - 4;

        // 更新各组件位置
        if (title_) title_->set_rect({0, 0, sw, 1});
        if (path_label_) path_label_->set_rect({0, 1, sw, 1});
        if (file_list_) file_list_->set_rect({0, 2, list_width, content_height});
        if (status_bar_) status_bar_->set_rect({0, sh - 1, sw, 1});

        if (preview_label_) {
            preview_label_->set_rect({list_width + 1, 2, sw - list_width - 1, content_height});
        }
    }

private:
    meowtui::App* app_ = nullptr;           ///< App 实例指针
    std::string current_path_;              ///< 当前路径
    std::vector<DirEntry> entries_;         ///< 当前目录条目

    // UI 组件指针
    meowtui::Label*    title_        = nullptr;
    meowtui::Label*    path_label_   = nullptr;
    meowtui::ListBox*  file_list_    = nullptr;
    meowtui::Label*    preview_label_= nullptr;
    meowtui::Label*    status_bar_   = nullptr;
};

// ============================================================================
// 程序入口
// ============================================================================

int main() {
    FileManager fm;
    return fm.run();
}
