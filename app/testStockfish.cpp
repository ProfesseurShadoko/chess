#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <tintoretto.hpp>


void sendCommand(int write_fd, const std::string& command) {
    std::string fullCommand = command + "\n";
    write(write_fd, fullCommand.c_str(), fullCommand.size());
    Message(command, ">");
}

std::string readResponse(int read_fd, const std::string& token) {
    char buffer[256];
    std::string line;
    std::string lastMatchingLine;

    while (true) {
        ssize_t count = read(read_fd, buffer, sizeof(buffer) - 1);
        if (count > 0) {
            buffer[count] = '\0';
            line += buffer;

            size_t pos;
            while ((pos = line.find('\n')) != std::string::npos) {
                std::string currentLine = line.substr(0, pos);
                line.erase(0, pos + 1);

                if (currentLine.find(token) != std::string::npos) {
                    lastMatchingLine = currentLine;
                    Message(lastMatchingLine, "<");
                    return lastMatchingLine;
                }
            }
        } else {
            break;  // EOF or error
        }
    }

    throw std::runtime_error("Token '" + token + "' not found in Stockfish output.");
}

int main() {
    int to_child[2];   // Parent writes → child reads
    int from_child[2]; // Child writes → parent reads

    if (pipe(to_child) == -1 || pipe(from_child) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process
        dup2(to_child[0], STDIN_FILENO);
        dup2(from_child[1], STDOUT_FILENO);
        dup2(from_child[1], STDERR_FILENO);  // Optional: capture stderr too

        // Close unused pipe ends
        close(to_child[1]);
        close(from_child[0]);

        // Execute Stockfish binary (adjust path as needed)
        execl("../bin/stockfish", "stockfish", nullptr);

        // If execl fails:
        perror("execl");
        _exit(1);
    } else {
        // Parent process
        close(to_child[0]);
        close(from_child[1]);

        // Handshake with UCI
        sendCommand(to_child[1], "uci");
        readResponse(from_child[0], "uciok");

        // Set ELO
        sendCommand(to_child[1], "setoption name UCI_LimitStrength value true");
        sendCommand(to_child[1], "setoption name UCI_Elo value 1350");

        // Wait for ready
        sendCommand(to_child[1], "isready");
        readResponse(from_child[0], "readyok");

        // Position and go
        sendCommand(to_child[1], "position startpos");
        sendCommand(to_child[1], "go movetime 1000");
        readResponse(from_child[0], "bestmove");

        // Quit Stockfish
        sendCommand(to_child[1], "quit");

        // Cleanup
        close(to_child[1]);
        close(from_child[0]);

        waitpid(pid, nullptr, 0);
    }

    return 0;
}