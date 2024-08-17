#ifndef LOGGER_H
#define LOGGER_H

#pragma once

#include <iostream>
#include <ostream>
#include <iomanip>
#include <deque>
#include <vector> 
#include <memory>
#include <cstdarg>


#include <glm/vec4.hpp>


typedef class Logger logger;
extern Logger app_logger;

bool ge_logger_init();

std::string direct_sprintf(const char * format, ...);

#define LOGMETA (std::string(__FILE__) + ":" + std::to_string(__LINE__))
#define log_debug(...) do{app_logger.debug(direct_sprintf(__VA_ARGS__) , (LOGMETA));}while(0)
#define log_info(...) do{app_logger.info(direct_sprintf(__VA_ARGS__) , (LOGMETA));}while(0)
#define log_warn(...) do{app_logger.warn(direct_sprintf(__VA_ARGS__) , (LOGMETA));}while(0)
#define log_error(...) do{app_logger.error(direct_sprintf(__VA_ARGS__) , (LOGMETA));}while(0)
#define log_severe(...) do{app_logger.severe(direct_sprintf(__VA_ARGS__) , (LOGMETA));}while(0)
#define log_assert(cond, ...) do{ if(!(cond)) {app_logger.severe(direct_sprintf(__VA_ARGS__) , (LOGMETA)); app_logger.flush_queue();}}while(0)

class Logger {
  public:
    ~Logger();

    void debug(const std::string& message, std::string logmeta);
    void info(const std::string& message, std::string logmeta);
    void warn(const std::string& message, std::string logmeta);
    void error(const std::string& message, std::string logmeta);
    void severe(const std::string& message, std::string logmeta);

    void log(const std::string& level, const std::string& message, std::string logmeta);

    void add_file_sink(const std::string& filename);

    std::string make_timestamp();

    const std::string DEBUG = "INFO";  
    const std::string INFO = "INFO";  
    const std::string WARN = "INFO";  
    const std::string ERR = "INFO";  
    const std::string SEVERE = "SEVERE";  

    const std::string TIMESTAMP_FORMAT = "[%Y %m %d %H:%M:%S] ";

    bool is_enabled = true;

    void flush_queue();
    void clear_queue();
  private:
    void add_to_queue(const std::string & formatted_message);


    bool use_cout = false;
    bool use_cerr = true;

    std::vector<std::shared_ptr<std::ofstream>> sinks;

    std::deque<std::string> logging_queue;
};

std::string mkstr(glm::vec4 v);

#endif /* LOGGER_H */