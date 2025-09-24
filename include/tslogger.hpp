// C++
#pragma once
#include <string>
#include <cstdarg>

namespace TSLog {

enum class Level { Trace=0, Debug, Info, Warn, Error, Fatal };

struct Config {
    std::string path = "logs/app.log";
    Level min_level = Level::Info;
    bool also_stderr = true;
    bool rotate_daily = false; // reservado para futuro
};

void init(const Config& cfg);
void init(const std::string& path, Level min_level = Level::Info, bool also_stderr = true);

void log(Level level, const char* fmt, ...);
void logv(Level level, const char* fmt, va_list args);

void trace(const char* fmt, ...);
void debug(const char* fmt, ...);
void info (const char* fmt, ...);
void warn (const char* fmt, ...);
void error(const char* fmt, ...);
void fatal(const char* fmt, ...);

void flush();
void close();

// Util
Level level_from_env();        // lê TSLOG_LEVEL=TRACE|DEBUG|INFO|...
const char* level_name(Level); // "INFO", etc.

} // namespace TSLog