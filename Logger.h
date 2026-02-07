#pragma once
#include "pch.h"

enum class LogLevel { Info, Warn, Error };

struct LogItem {
    LogLevel level;
    std::string category;
    std::string text;
    uint64_t ts_ms;
};

class Logger
{
public:
    explicit Logger(size_t max_items = 10000);

    void push(LogLevel lvl, std::string cat, std::string msg);
    void snapshot(std::vector<LogItem>& out); // thread-safe copy
    bool dirty() const;

private:
    static uint64_t now_ms();

private:
    mutable std::mutex m_;
    std::deque<LogItem> items_;
    size_t max_;
    std::atomic<bool> dirty_{ true };
};

// Global application logger (easy access from anywhere).
Logger& AppLogger();

#define LOGI(cat, msg) AppLogger().push(LogLevel::Info,  (cat), (msg))
#define LOGW(cat, msg) AppLogger().push(LogLevel::Warn,  (cat), (msg))
#define LOGE(cat, msg) AppLogger().push(LogLevel::Error, (cat), (msg))
