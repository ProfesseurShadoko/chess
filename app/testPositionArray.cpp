

#include <tintoretto.hpp>
#include "move.hpp"
#include "boardUI.hpp"
#include <string>
#include "positionAsArray.hpp"
#include "memory"

int main() {
    Message("Testing file posStructAsArray.hpp");
    BoardUI board = BoardUI();
    board.fromFEN(BoardUI::startpos);

    // --------------------------- //
    // !-- Test initialization --! //
    // --------------------------- //

    PositionAsArray position;
    position.display();

    Move move = Move(
        makeSquare("e2"),
        makeSquare("e4"),
        makePiece(Color::WHITE, Figure::PAWN)
    );

    position.play(move, false);
    position.display();


    // -------------------------- //
    // !-- Test play / unplay --! //
    // -------------------------- //

    Test test("Play / Unplay moves on PositionAsArray");
    std::string fen = "rnbqkbnr/ppp1pppp/8/8/1PpP4/8/P3PPPP/RNBQKBNR b KQkq b3 0 3";
    //board.clearBoard();
    //board.fromFEN(fen);
    //board.display();
    position.fromFEN(fen);
    position.display();
}