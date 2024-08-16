#include "logger.h"

#include <glm/vec4.hpp>

#include <iostream>
#include <ostream>
#include <iomanip>
#include <deque>
#include <vector> 
#include <memory>
#include <cstdarg>
#include <fstream>

Logger app_logger;

Logger::~Logger(){
    flush_queue();
    for(std::shared_ptr<std::ofstream> sink_ref : sinks){
        sink_ref->close();
    }
}

bool ge_logger_init(){
    app_logger.add_file_sink("test.log");

    log_info("Initialized logger successfully!");
    return true;
}

void Logger::debug(const std::string& message, std::string filemeta){
    log(Logger::DEBUG, message, filemeta);
}
void Logger::info(const std::string& message, std::string filemeta){
    log(Logger::INFO, message, filemeta);
}
void Logger::warn(const std::string& message, std::string filemeta){
    log(Logger::WARN, message, filemeta);
}
void Logger::error(const std::string& message, std::string filemeta){
    log(Logger::ERR, message, filemeta);
}
void Logger::severe(const std::string& message, std::string filemeta){
    log(Logger::SEVERE, message, filemeta);
}

void Logger::log(const std::string& level, const std::string& message, std::string filemeta){
    std::string log_line = make_timestamp() + level.data() + " \t" + filemeta + " \t" + message + "\n";
    add_to_queue(log_line);
}

void Logger::add_file_sink(const std::string& filename){
    auto sink_reference = std::make_shared<std::ofstream>(filename);
    sinks.push_back(sink_reference);
}

std::string Logger::make_timestamp(){
    std::time_t time_now = std::time(nullptr);
    std::tm tm = *std::localtime(&time_now);
    std::stringstream ret_val;
    ret_val << std::put_time(&tm, TIMESTAMP_FORMAT.data());
    return ret_val.str();
} 

void Logger::add_to_queue(const std::string & formatted_message){
    if( is_enabled ){
        logging_queue.push_back(formatted_message);
//        flush_queue();
    }
}

void Logger::flush_queue(){
    // TODO: add Mutex for eventual thread-safety;
    for(std::shared_ptr<std::ofstream> sink_ref : sinks){
        std::ofstream& sink = *sink_ref;
        for(std::string& log_item : logging_queue){
            sink << log_item;
        }
    }
    for(std::string& log_item : logging_queue){
        if( use_cout ) std::cout << log_item;
        if( use_cerr ) std::cerr << log_item;
    }
    logging_queue.clear();
}

void Logger::clear_queue(){
    logging_queue.clear();
}

std::string mkstr(glm::vec4 v){
    return "[" + std::to_string(v[0]) + "  " + std::to_string(v[1]) + "  " + std::to_string(v[2]) + "  " + std::to_string(v[3]) + "]";
}

std::string direct_sprintf(const char * format, ...){
    char buf[4096] = {0};
    va_list argptr;
    va_start(argptr, format);    
    vsnprintf(buf, 4095, format, argptr);
    va_end(argptr);
    
    buf[4095] = '\0';
    std::string message(buf);
    return message;
}


