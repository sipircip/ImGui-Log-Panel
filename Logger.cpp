#include "Logger.h"
#include <chrono>

Logger::Logger(size_t max_items) : max_(max_items) {}

uint64_t Logger::now_ms()
{
    using namespace std::chrono;
    return (uint64_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void Logger::push(LogLevel lvl, std::string cat, std::string msg)
{
    const uint64_t now = now_ms();
    std::scoped_lock lk(m_);
    if (items_.size() >= max_) items_.pop_front();
    items_.push_back({ lvl, std::move(cat), std::move(msg), now });
    dirty_ = true;
}

void Logger::snapshot(std::vector<LogItem>& out)
{
    std::scoped_lock lk(m_);
    out.assign(items_.begin(), items_.end());
    dirty_ = false;
}

bool Logger::dirty() const
{
    return dirty_.load();
}


Logger& AppLogger()
{
    static Logger g_logger(10000);
    return g_logger;
}
