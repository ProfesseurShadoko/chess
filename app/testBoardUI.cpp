#include "boardUI.hpp"
#include <tintoretto.hpp>

int main() {
    Test move_test("Testing Moves");
    bool passed = true;
    try {
        BoardUI board;
        board.fromFEN(BoardUI::startpos);
        std::cout << board << std::endl;

        // let's play a full game as see what happens!
        board.play("e2e4");
        std::cout << board << std::endl;

        board.play("e7e5");
        std::cout << board << std::endl;

        board.play("g1f3");
        std::cout << board << std::endl;

        board.play("b8c6");
        std::cout << board << std::endl;

        board.play("f1c4");
        std::cout << board << std::endl;

        board.play("g8f6");
        std::cout << board << std::endl;

        board.play("b1c3");
        std::cout << board << std::endl;

        board.play("a7a5");
        std::cout << board << std::endl;

        board.play("e1g1");
        std::cout << board << std::endl;

        board.play("h7h6");
        std::cout << board << std::endl;

        board.play("d2d4");
        std::cout << board << std::endl;

        board.play("h8h7");
        std::cout << board << std::endl;

        board.play("d4e5");
        std::cout << board << std::endl;

        board.play("d7d5");
        std::cout << board << std::endl;

        board.play("e5d6");
        std::cout << board << std::endl;

        board.play("a5a4");
        std::cout << board << std::endl;

        board.play("d6c7");
        std::cout << board << std::endl;

        board.play("a4a3");
        std::cout << board << std::endl;

        board.play("c7d8r");
        std::cout << board << std::endl;

        board.play("e8e7");
        std::cout << board << std::endl;

        board.play("d8c8");
        std::cout << board << std::endl;

        board.play("c6a5");
        std::cout << board << std::endl;

        board.play("d1d8"); // check mate!
        std::cout << board << std::endl;

    } catch (const std::exception& e) {
        passed = false;
    }
    move_test.complete(passed);


    Test fen_test("Testing FEN in-out");
    std::string fen = "r1RQ1b2/1p2kppr/5n1p/n7/2B1P3/p1N2N2/PPP2PPP/R1B2RK1 b - - 2 12";
    BoardUI board2;
    board2.fromFEN(fen);
    std::string fen2 = board2.toFEN();
    fen_test.complete(fen == fen2);

    Message::print("Looking at board constructed from previous fen:");
    std::cout << board2 << std::endl;

    Test mark_test("Testing Marking Squares");
    BoardUI board3;
    board3.fromFEN(BoardUI::startpos);
    board3.mark("e4", 0); // green
    board3.mark("e7", 1); // red
    board3.mark("f3", 2); // cyan
    board3.mark("f2", 3); // purple
    std::cout << board3 << std::endl;
    mark_test.complete(true);

}