// C++
#include "tslogger.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <filesystem>
#include <iomanip>
#include <cctype>
#include <ctime>
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace TSLog {

namespace {
std::mutex g_mutex;
std::FILE* g_file = nullptr;
std::atomic<bool> g_inited{false};
Config g_cfg{};
}

static std::string now_timestamp() {
    using clock = std::chrono::system_clock;
    auto tp = clock::now();
    auto t = clock::to_time_t(tp);
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count() % 1000000;
    std::tm tm_buf{};
    localtime_r(&t, &tm_buf);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setw(6) << std::setfill('0') << micros;
    return oss.str();
}

const char* level_name(Level lv) {
    switch (lv) {
        case Level::Trace: return "TRACE";
        case Level::Debug: return "DEBUG";
        case Level::Info:  return "INFO";
        case Level::Warn:  return "WARN";
        case Level::Error: return "ERROR";
        case Level::Fatal: return "FATAL";
        default: return "INFO";
    }
}

static bool ensure_parent_dir(const std::string& path) {
    try {
        auto p = std::filesystem::path(path).parent_path();
        if (!p.empty() && !std::filesystem::exists(p)) {
            std::filesystem::create_directories(p);
        }
        return true;
    } catch (...) { return false; }
}

Level level_from_env() {
    const char* env = std::getenv("TSLOG_LEVEL");
    if (!env) return Level::Info;
    std::string s(env);
    for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if (s == "TRACE") return Level::Trace;
    if (s == "DEBUG") return Level::Debug;
    if (s == "INFO")  return Level::Info;
    if (s == "WARN" || s == "WARNING") return Level::Warn;
    if (s == "ERROR") return Level::Error;
    if (s == "FATAL") return Level::Fatal;
    return Level::Info;
}

void init(const Config& cfg) {
    std::scoped_lock lk(g_mutex);
    if (g_inited.load()) return;
    g_cfg = cfg;
    if (!ensure_parent_dir(cfg.path)) {
        std::fprintf(stderr, "[TSLog] Falha ao criar diretório do log: %s\n", cfg.path.c_str());
    }
    g_file = std::fopen(cfg.path.c_str(), "a");
    if (!g_file) {
        std::fprintf(stderr, "[TSLog] Falha ao abrir log em %s: %s\n", cfg.path.c_str(), std::strerror(errno));
    } else {
        std::setvbuf(g_file, nullptr, _IOLBF, 0); // line-buffered
        std::fprintf(g_file, "[%s] %-5s tid=%zu TSLog iniciado (min=%s)\n",
                     now_timestamp().c_str(), "INFO",
                     std::hash<std::thread::id>{}(std::this_thread::get_id()),
                     level_name(cfg.min_level));
        std::fflush(g_file);
    }
    g_inited.store(true);
}

void init(const std::string& path, Level min_level, bool also_stderr) {
    Config cfg;
    cfg.path = path;
    cfg.min_level = min_level;
    cfg.also_stderr = also_stderr;
    init(cfg);
}

static void vlog_impl(Level level, const char* fmt, va_list ap) {
    if (!g_inited.load()) {
        Config auto_cfg;
        auto_cfg.min_level = level_from_env();
        init(auto_cfg);
    }
    if (level < g_cfg.min_level) return;

    va_list ap_copy;
    va_copy(ap_copy, ap);
    int needed = std::vsnprintf(nullptr, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (needed < 0) return;

    std::vector<char> buf(static_cast<size_t>(needed) + 1);
    std::vsnprintf(buf.data(), buf.size(), fmt, ap);

    const auto ts = now_timestamp();
    const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

    std::scoped_lock lk(g_mutex);
    if (g_file) {
        std::fprintf(g_file, "[%s] %-5s tid=%zu %s\n", ts.c_str(), level_name(level), tid, buf.data());
        if (level >= Level::Error) std::fflush(g_file);
    }
    if (g_cfg.also_stderr) {
        std::fprintf(stderr, "[%s] %-5s tid=%zu %s\n", ts.c_str(), level_name(level), tid, buf.data());
        if (level >= Level::Error) std::fflush(stderr);
    }
}

void log(Level level, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlog_impl(level, fmt, ap);
    va_end(ap);
}

void logv(Level level, const char* fmt, va_list args) {
    vlog_impl(level, fmt, args);
}

#define TSLOG_MAKE_FUN(name, LV) \
    void name(const char* fmt, ...) { \
        va_list ap; va_start(ap, fmt); vlog_impl(LV, fmt, ap); va_end(ap); \
    }

TSLOG_MAKE_FUN(trace, Level::Trace)
TSLOG_MAKE_FUN(debug, Level::Debug)
TSLOG_MAKE_FUN(info , Level::Info)
TSLOG_MAKE_FUN(warn , Level::Warn)
TSLOG_MAKE_FUN(error, Level::Error)
TSLOG_MAKE_FUN(fatal, Level::Fatal)

void flush() {
    std::scoped_lock lk(g_mutex);
    if (g_file) std::fflush(g_file);
    std::fflush(stderr);
}

void close() {
    std::scoped_lock lk(g_mutex);
    if (g_file) {
        std::fprintf(g_file, "[%s] %-5s tid=%zu TSLog finalizado\n",
                     now_timestamp().c_str(), "INFO",
                     std::hash<std::thread::id>{}(std::this_thread::get_id()));
        std::fflush(g_file);
        std::fclose(g_file);
        g_file = nullptr;
    }
    g_inited.store(false);
}

} // namespace TSLog