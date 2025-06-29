#pragma once

#include "colored_string.hpp"
#include "mutable.hpp"


/**
 * @brief A class to print messages with different colors and prefixes. Creating the object will print the message right away (unless the class is muted).
 * 
 * Usage:
 * ```cpp
 * Message("This is an info message");
 * Message("This is a success message", "#");
 * Message("This is a warning message", "?");
 * Message("This is an error message", "!");
 * Message("This is a sent message", ">");
 * Message("This is a received message", "<");
 * ```
 */
class Message : public MutableClass {
    private:
        std::string msg;
        std::string typ;
    
    public:
        Message(const std::string msg, std::string typ = "i") {
            this->msg = msg;
            std::string prefix;

            if (typ == "i") {
                prefix = cstr("[i] ").cyan();
            } else if (typ == "#") {
                prefix = cstr("[#] ").green();
            } else if (typ == "!") {
                prefix = cstr("[!] ").red();
            } else if (typ == "?") {
                prefix = cstr("[?] ").yellow();
            } else if (typ == ">") {
                prefix = cstr("[>] ").green();
            } else if (typ == "<") {
                prefix = cstr("[<] ").yellow();
            } else {
                throw std::invalid_argument("Invalid typ argument. Must be Must be one of '#', '!', '?', 'i'.");
            }

            print(prefix + this->msg);
        }
    };