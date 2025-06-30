
#ifndef CHESS_ENGINE_WRAPPER_HPP
#define CHESS_ENGINE_WRAPPER_HPP

#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdexcept>
#include <array>
#include <iostream>
#include <tintoretto.hpp>
#include <sstream>
#include "boardUI.hpp"


class UCIWrapper {
    protected:
        std::string enginePath;
        int to_child[2];
        int from_child[2];
        pid_t pid;

        void send(const std::string& command) {
            std::string full = command + "\n";
            ssize_t written = write(to_child[1], full.c_str(), full.size());
            if (written < 0) {
                throw std::runtime_error("Failed to write to engine.");
            }
            Message(command, ">");
        }

        std::string listen(const std::string& token) {
            char buffer[256];
            std::string line;
            std::string result;

            while (true) {
                // 1. Wait until there is data to read from the child process (stockfish for instance)
                ssize_t count = read(from_child[0], buffer, sizeof(buffer) - 1);

                if (count <= 0) { // we reached EOF (0 bits left to read) or an error occured (-1)
                    throw std::runtime_error("Failed to read from engine while listening for: " + token);
                }

                // 2. Append the read data to the line (\0 is used to terminate the string)
                buffer[count] = '\0';
                line += buffer;

                // 3. Look for complete lines (engind in '\n')
                size_t pos;
                while ((pos = line.find('\n')) != std::string::npos) {
                    std::string currentLine = line.substr(0, pos); // extract from start to first \n
                    line.erase(0, pos + 1); // remove so that next time we start from the next line

                    // 4. Look for token in line
                    if (currentLine.find(token) != std::string::npos) {
                        Message(currentLine, "<");
                        return currentLine;
                    }
                }
            }
        }

        void option(const std::string& option, const std::string& value) {
            send("setoption name " + option + " value " + value);
        }

        

    public:
        UCIWrapper(const std::string& path) {
            if (pipe(to_child) == -1 || pipe(from_child) == -1) {
                throw std::runtime_error("Failed to create pipes.");
            }

            pid = fork();
            if (pid == -1) throw std::runtime_error("Failed to fork process.");

            if (pid == 0) {
                // Child process
                close(to_child[1]); // Close unused write end
                close(from_child[0]); // Close unused read end

                dup2(to_child[0], STDIN_FILENO); // Redirect stdin to pipe
                dup2(from_child[1], STDOUT_FILENO); // Redirect stdout to pipe

                execl(enginePath.c_str(), enginePath.c_str(), nullptr);
                perror("execl failed");
                exit(EXIT_FAILURE); // If execl fails
            } else { // Parent process
                close(to_child[0]); // Close unused read end
                close(from_child[1]); // Close unused write end
            }

            Message("Engine player set (" + enginePath + ")", "#");
        }

        // necessary for human class
        UCIWrapper() : pid(-1) {
            // Mark as not connected / not running
            to_child[0] = to_child[1] = -1;
            from_child[0] = from_child[1] = -1;
        }
        ~UCIWrapper() {
            if (pid > 0) {
                quit();
            }
        }

        /**
         * @brief Sends 'uci' and expects 'uciok' response.
         */
        void handshake() {
            send("uci");
            listen("uciok");
        }

        /**
         * Enables strength limit and sets the engine's ELO rating.
         */
        void elo(int elo = -1) {
            if (elo < 0) {
                option("UCI_LimitStrength", "false");
            } else {
                option("UCI_LimitStrength", "true");
                option("UCI_Elo", std::to_string(elo));
            }
        }

        /**
         * @brief Sends 'isready' and expects 'readyok' response.
         */
        void ready() {
            send("isready");
            listen("readyok");
        }
        

        /**
         * @brief Sets the position on the board by providing initial position and list of moves.
         */
        void position(const std::string& moves) {
            send("position startpos moves " + moves);
        }

        /**
         * @brief Set position by FEN instead of moves.
         */
        void positionFEN(const std::string& fen) {
            send("position fen" + fen);
        }

        /**
         * @brief Sends 'go' command with a specified time limit and/or depth.
         */
        std::string getBestMove(int movetime = 1000, int depth = -1) {
            std::string command = "go";
            if (movetime > 0) {
                command += " movetime " + std::to_string(movetime);
            }
            if (depth > 0) {
                command += " depth " + std::to_string(depth);
            }
            send(command);
            std::string response = listen("bestmove");
            
            // extract move from response with fomat bestmove e2e4 ponder e7e5 (just split around ' ', move can be of length 4 or 5 if promotion!)
            if (response.rfind("bestmove ", 0) != 0) {
                throw std::runtime_error("Unexpected bestmove line: " + response);
            }
            std::istringstream iss(response);
            std::string tag, move;
            iss >> tag >> move;
            if (move.length() < 4 || move.length() > 5) {
                throw std::runtime_error("Invalid move format: " + move);
            }
            return move;
        }

        /**
         * @brief Exit the process
         */
        void quit() {
            send("quit");
            close(to_child[1]);
            close(from_child[0]);
            waitpid(pid, nullptr, 0);
            pid = -1; // Reset pid to indicate the process has been terminated
        }
};



class HumanWrapper : public UCIWrapper {
    public:
        HumanWrapper() {
            Message("Human player set", "#");
        }

        ~HumanWrapper() {}

        void handshake() {
            Message("uci", ">");
            Message("uciok", "<");
        }

        void elo() {
            Message("No elo limit in human mode", ">");
        }

        void ready() {
            Message("isready", ">");
            Message("readyok", "<");
        }

        void position(const std::string& moves) {
            Message("position startpos moves " + moves, ">");

            BoardUI board;
            board.fromFEN(BoardUI::startpos);
            std::istringstream iss(moves);
            std::string move;
            while (iss >> move) {
                board.play(move);
            }
            Message("Current board:");
            std::cout << board << std::endl;
        }

        void positionFEN(const std::string& fen) {
            Message("position fen " + fen, ">");
            BoardUI board;
            board.fromFEN(fen);
            Message("Current board:");
            std::cout << board << std::endl;
        }

        std::string getBestMove(int movetime = 1000, int depth = -1) {
            Message("go", ">");
            std::string move;
            std::cout << "Enter your move(ex: e1g1 for catle or h7h8d for promotion): ";
            std::cin >> move;
            return move;
        }
};


#endif