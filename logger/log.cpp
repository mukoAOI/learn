#include "log.h"

Log::Log() {
    line_count_ = 0;
    is_async_ = false;
    write_thread_ = nullptr;
    deque_ = nullptr;
    to_day_ = 0;
    fp_ = nullptr;
}

Log::~Log() {
    if (write_thread_ && write_thread_->joinable()) {
        while(!deque_->empty()) {
            deque_->flush();
        };
        deque_->close();
        write_thread_->join();
    }
    if (fp_) {
        std::lock_guard<std::mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

bool Log::is_open() {
    return is_open_;
}

int Log::get_level() {
    std::lock_guard<std::mutex> locker(mtx_);
    return level_;
}

void Log::set_level(int level) {
    std::lock_guard<std::mutex> locker(mtx_);
    level_ = level;
}

void Log::init(int level = 1, const char* path, const char* suffix, int max_queue_capacity) {
    is_open_ = true;
    level_ = level;
    if (max_queue_capacity > 0) {
        is_async_ = true;
        if (!deque_) {
            std::unique_ptr<BlockDeque<std::string>> new_deque(new BlockDeque<std::string>);
            deque_ = std::move(new_deque);
            std::unique_ptr<std::thread> new_thread(new std::thread(flush_log_thread));
            write_thread_ = std::move(new_thread);
        } 
    } else {
        is_async_ = false;
    }
    
    line_count_ = 0;
    
    time_t timer = time(nullptr);
    struct tm* sys_time = localtime(&timer);
    struct tm t = *sys_time;

    path_ = path;
    suffix_ = suffix;

    char file_name[LOG_NAME_LEN] = {0};
    snprintf(file_name, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);

    to_day_ = t.tm_mday;

    {
        std::lock_guard<std::mutex> locker(mtx_);
        buff_.RetrieveAll();
        if (fp_) {
            flush();
            fclose(fp_);
        }

        fp_ = fopen(file_name, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(file_name, "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t t_sec = now.tv_sec;
    struct tm *sys_time = localtime(&t_sec);
    struct tm t = *sys_time;
    va_list valist;

    if (to_day_ != t.tm_mday || (line_count_ && (line_count_ % MAX_LINES == 0))) {
        std::unique_lock<std::mutex> locker(mtx_);
        locker.unlock();

        char new_file[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (to_day_ != t.tm_mday) {
            snprintf(new_file, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            to_day_ = t.tm_mday;
            line_count_ = 0;
        }
        else {
            snprintf(new_file, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (line_count_ / MAX_LINES), suffix_);
        }
        
        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(new_file, "a");
        assert(fp_ != nullptr);
    }

    {
        std::unique_lock<std::mutex> locker(mtx_);
        line_count_++;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff_.HasWritten(n);
        append_log_level_title(level);
        va_start(valist, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, valist);
        va_end(valist);
        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if (is_async_ && deque_ && !deque_->full()) {
            deque_->push_back(buff_.RetrieveAllToStr());
        } else {
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll();
    }
}

void Log::append_log_level_title(int level) {
    switch(level) {
    case 0:
        buff_.Append("[debug]: ", 9);
        break;
    case 1:
        buff_.Append("[info] : ", 9);
        break;
    case 2:
        buff_.Append("[warn] : ", 9);
        break;
    case 3:
        buff_.Append("[error]: ", 9);
        break;
    default:
        buff_.Append("[info] : ", 9);
        break;
    }
}

void Log::flush() {
    if (is_async_) {
        deque_->flush();
    }
    fflush(fp_);
}

void Log::async_write() {
    std::string str = "";
    while (deque_->pop_front(str)) {
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

void Log::flush_log_thread() {
    Log::get_log()->async_write();
}

Log* Log::get_log() {
    static Log log;
    return &log;
}