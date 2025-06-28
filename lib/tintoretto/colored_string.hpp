#pragma once

#include <string>

/**
 * @brief Class to color a string in the terminal. Call ColoredString("hop")::green()
 * and then print it to get a green "hop" in the terminal. All functions return type string.
 * 
 * 
*/
class ColoredString {
    private:
        std::string str;
        static inline const std::string _green = "\033[92m";
        static inline const std::string _red = "\033[91m";
        static inline const std::string _yellow = "\033[93m";
        static inline const std::string _blue = "\033[94m";
        static inline const std::string _purple = "\033[95m";
        static inline const std::string _cyan = "\033[96m";
        static inline const std::string _reset = "\033[0m";

    public:
        ColoredString(const std::string str) : str(str) {};

        std::string green() {return _green + str + _reset;}
        std::string red() {return _red + str + _reset;}
        std::string yellow() {return _yellow + str + _reset;};
        std::string blue() {return _blue + str + _reset;};
        std::string purple() {return _purple + str + _reset;};
        std::string cyan() {return _cyan + str + _reset;};

};

/**
 * @brief Function to create a ColoredString object from a string
 * 
 * Usage:
 * 
 * >>> std::cout << cstr("hop").green() << std::endl;
 */
inline ColoredString cstr(const std::string str) { // inline because function is declared and defined in the same file
    return ColoredString(str);
}

/**
 * @brief Function to create a ColoredString object from a char
 * 
 * Usage:
 * 
 * >>> std::cout << cstr('a').blue() << std::endl;
 */
inline ColoredString cstr(char c) {
    return ColoredString(std::string(1, c)); // what does 1 mean? It means we create a string of length 1 with the character c
}

/**
 * @brief Function to create a ColoredString object from a string
 * 
 * Usage:
 * 
 * >>> std::cout << cstr(7).red() << std::endl;
 */
inline ColoredString cstr(int i) {
    return ColoredString(std::to_string(i));
}

/**
 * @brief Function to create a ColoredString object from a string
 * 
 * Usage:
 * 
 * >>> std::cout << cstr(3.14).cyan() << std::endl;
 */
inline ColoredString cstr(double d) {
    return ColoredString(std::to_string(d));
}