#pragma once

#include "mutable.hpp"
#include <sys/resource.h>

/**
 * @brief A progress bar that shows the progress of a task.
 * 
 * Usage:
 * ```cpp
 * int N_iter = 100;
 * ProgressBar bar(N_iter);
 * for (int i=0; i<N_iter; i++) {
 *    computeStuff();
 *    bar.update();
 *    if (i==50) bar.whisper("Halfway there!"); // prints a statement without erasing the progress bar
 * }
 * ```
 */
class ProgressBar : public MutableClass {
    private:
        int length;
        int progress;
        static inline const int bar_length = 50;
        std::string previous_print = "zou!";
    
    public:
        ProgressBar(int length) {
            this->length = length;
            this->progress = 0;
            display();
        }

        void update() {
            progress++;

            if (progress <= length) display();
            
            if (progress == length) {
                std::cout << std::endl;
            }
        }

        void whisper(std::string msg="") {

            if (is_muted()) return;

            // progress bar erases stuff, hence we need to be careful

            // print one empty line to erase the line. then go back to front of line.
            std::string empty_line = "\r";
            for (size_t i=0; i<bar_length*2; i++) {
                empty_line += " ";
            }
            std::cout << empty_line << std::flush;
            std::cout << "\r" << tab_to_str() + msg << std::endl;
            std::cout << previous_print << std::flush;
        }
    

    private:
        void display() {
            if (is_muted()) return;
            
            std::string incomplete = "━";
            std::string complete = cstr("━").blue();
            int n_complete = ((progress * bar_length) / length);

            std::string bar = "";
            // fill the bar with complete times █
            for (int i = 0; i < n_complete; i++) {
                bar += complete;
            }
            // fill the bar with incomplete times █
            for (int i = n_complete; i < bar_length; i++) {
                bar += incomplete;
            }

            int progress_percent = ((progress * 100) / length);
            if (progress >= length) {
                progress_percent = 100;
            }

            std::string progress_percent_str = cstr(std::to_string(progress_percent) + "%").red();
            std::string next_print = "\r" + tab_to_str() + cstr("[%]").blue() + " Progress: " + bar + " (" + progress_percent_str + ")";

            if (next_print != previous_print) {
                previous_print = next_print;

                // let's add some information about the memory usage
                struct rusage usage;
                getrusage(RUSAGE_SELF, &usage);
                std::string memory_usage = " > " + std::to_string(usage.ru_maxrss / 1000) + " MB";

                std::cout << next_print + memory_usage << std::flush;
            }
        }

};    


