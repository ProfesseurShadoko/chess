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

        // Background “highlight” (bright variants)
        static inline const std::string _h_black  = "\033[40m";
        static inline const std::string _h_red    = "\033[41m";
        static inline const std::string _h_green  = "\033[42m";
        static inline const std::string _h_yellow = "\033[43m";
        static inline const std::string _h_blue   = "\033[44m";
        static inline const std::string _h_purple = "\033[45m";
        static inline const std::string _h_cyan   = "\033[46m";
        static inline const std::string _h_white  = "\033[47m";

    public:
        ColoredString(const std::string str) : str(str) {};

        // Foreground color methods
        ColoredString green()   const { return ColoredString(_green + str); }
        ColoredString red()     const { return ColoredString(_red + str); }
        ColoredString yellow()  const { return ColoredString(_yellow + str); }
        ColoredString blue()    const { return ColoredString(_blue + str); }
        ColoredString purple()  const { return ColoredString(_purple + str); }
        ColoredString cyan()    const { return ColoredString(_cyan + str); }

        // Background highlight methods
        ColoredString h_black()  const { return ColoredString(_h_black + str); }
        ColoredString h_red()    const { return ColoredString(_h_red + str); }
        ColoredString h_green()  const { return ColoredString(_h_green + str); }
        ColoredString h_yellow() const { return ColoredString(_h_yellow + str); }
        ColoredString h_blue()   const { return ColoredString(_h_blue + str); }
        ColoredString h_purple() const { return ColoredString(_h_purple + str); }
        ColoredString h_cyan()   const { return ColoredString(_h_cyan + str); }
        ColoredString h_white()  const { return ColoredString(_h_white + str); }
    
        // Append reset and return final string
        std::string toString() const {
            return str + _reset;
        }

        // Concatenation with std::string returns std::string
        friend std::string operator+(const ColoredString& lhs, const std::string& rhs) {
            return lhs.toString() + rhs;
        }
        friend std::string operator+(const std::string& lhs, const ColoredString& rhs) {
            return lhs + rhs.toString();
        }
        // Concatenation with another ColoredString returns std::string
        friend std::string operator+(const ColoredString& lhs, const ColoredString& rhs) {
            return lhs.toString() + rhs.toString();
        }

        // Stream insertion
        friend std::ostream& operator<<(std::ostream& os, const ColoredString& cs) {
            os << cs.toString();
            return os;
        }

        // Implicit conversion to std::string
        operator std::string() const {
            return toString();
        }


    private:
        static ColoredString cstr(const std::string& str) {
            return ColoredString(str);
        }


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