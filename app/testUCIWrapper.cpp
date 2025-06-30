#include "UCIWrapper.hpp"


int main() {
    // Print current working directory
    Message("Current working directory: " + cstr(getcwd(nullptr, 0)).cyan());

    // let's initialize two UCI players
    UCIWrapper stockfish("../bin/stockfish");
    stockfish.handshake();
    stockfish.elo(3000);
    stockfish.ready();

    HumanWrapper human;
    human.handshake();
    human.elo();
    human.ready();

    // loop!
    int count = 0;
    std::string moves;
    while (true) {
        if (count % 2 == 1) {
            // stockfish's turn (with black pieces)
            stockfish.position(moves);
            std::string bestMove = stockfish.getBestMove(1000);
            moves += " " + bestMove;
        } else {
            // human's turn (with white pieces)
            human.position(moves);
            std::string bestMove = human.getBestMove(1000);
            moves += " " + bestMove;
        }
        count++;
    }

}