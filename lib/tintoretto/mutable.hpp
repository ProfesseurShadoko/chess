#pragma once

#include "colored_string.hpp"
#include <iostream>
#include <thread>


/**
 * @brief Class that handles printing statemnts, mute and unmute messages, and tabbing. Also has a sleep method.
 * 
 * Usage:
 * ```cpp
 * MutableClass::print("hello");
 * MutableClass::mute();
 * MutableClass::print("this message will not be printed");
 * MutableClass::unmute();
 * MutableClass::tab();
 * MutableClass::print("this message will be tabbed"); // prints " > this message will be tabbed"
 * MutableClass::untab();
 * ```
 */
class MutableClass {
    private:
        static inline int mute_count = 0; // since we are header only, we need this inline keyword
        static inline int tab_count = 0;

    public:
        /**
         * @brief Increment the mute_count static variable. This prevents the message from being printed.
        */
        static void mute() {
            mute_count++;
        };

        /**
         * @brief Decrement the mute_count static variable. This allows the message to be printed,
         * provided mute() method has not been called more times than unmute() method.
        */
        static void unmute() {
            mute_count--;
            mute_count = mute_count < 0 ? 0 : mute_count;
        };

        /**
         * @brief Check if the message is muted by looking at the private static variable mute_count.
         * This variable gets incremented or decremented by the mute() and unmute() methods.
         * @return true if the message is muted, false otherwise
        */
        static bool is_muted() {
            return mute_count > 0;
        };

        static void tab() {
            tab_count++;
        }

        static void untab() {
            tab_count--;
            tab_count = tab_count < 0 ? 0 : tab_count;
        }

        /**
         * @brief Create a ColoredString object from a string.
         * @return ColoredString object. This means you can use:
         * MutableCLass::print(MutableClass::cstr("hello")->green())
        */
        static ColoredString cstr(std::string str) {
            return ColoredString(str);
        };

        /**
         * @brief Print the message if it is not muted. The print includes the tabs.
        */
        static void print(std::string msg = "") {
            if (!is_muted()) {
                std::cout << tab_to_str() << msg << std::endl;
            }
        };
    
        static void sleep(int ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }

        static void par() {
            print();
        }

    protected:
        static std::string tab_to_str() {
            if (tab_count == 0) {
                return "";
            }
            std::string out = " ";
            for (int i=0; i<tab_count; i++) {
                out += ">";
            }
            return out + " ";
        }
};