#pragma once

#include "mutable.hpp"

#include <string>
#include <chrono>
#include <stdexcept>

/**
 * @brief A class to measure the time taken for a task to complete, and print it nicely in the console
 * 
 * Usage:
 * ```cpp
 * Task task("Computing Stuff"); // prints in console and starts the timer // increase by one tab the print() method also
 * computeStuff();
 * task.complete(); // prints in console the time taken
 * long long time = task.getTimeNs(); // returns the time taken in nanoseconds
 * ```
 */
class Task: public MutableClass {
    private:
        std::string msg;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
        bool isTest = false;
    protected:
        std::string prefix = cstr("[T] ").purple();
    
    public:
        Task(std::string msg) {
            this->msg = msg;
            start();
        }

    private:
        /**
         * Display the task message, and then start the timer
         */
        void start() {
            print(
                prefix + msg
            );
            tab();
            start_time = std::chrono::high_resolution_clock::now();
        }
    public:
        /**
         * Stop the timer and display the time taken
         */
        void complete() {
            // stop the clock
            end_time = std::chrono::high_resolution_clock::now();

            // transform time taken into hh:mm:ss.ms
            long long time_ns = getTimeNs();
            int time_ms = time_ns / 1000000;
            int time_s = time_ms / 1000;
            int time_m = time_s / 60;
            int time_h = time_m / 60;
            time_ms = time_ms % 1000;
            time_s = time_s % 60;
            time_m = time_m % 60;

            std::string time_str = "";
            if (time_h < 10) time_str += "0";
            time_str += std::to_string(time_h) + ":";
            if (time_m < 10) time_str += "0";
            time_str += std::to_string(time_m) + ":";
            if (time_s < 10) time_str += "0";
            time_str += std::to_string(time_s) + ".";
            if (time_ms < 100) time_str += "0";
            if (time_ms < 10) time_str += "0";
            time_str += std::to_string(time_ms);

            untab();
            // print out the result
            print(
                prefix + msg + cstr(" (" + time_str + ")").purple()
            );
        }

        long long getTimeNs() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
        };


};


class Test: public Task {

    bool isTest = true;

    public:
    Test(std::string msg) : Task(msg) {};

    /**
     * Completes the task, showing whether the test is passed or not
     */
    void complete(bool condition) {
        if (condition) {
            prefix += cstr("<PASS> ").green();
        }
        else {
            prefix += cstr("<FAIL> ").red();
        }
        Task::complete();
    } 
};