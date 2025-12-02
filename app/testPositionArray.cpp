

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

    // we need the structuredPositionBase to test the PosStructAsArray
    using PositionAsArrayTest = PositionBaseT<PosStructAsArray, MoveGeneratorForArray>;
    
    PositionAsArrayTest position;
    position.display();

    Move move = Move(
        makeSquare("e2"),
        makeSquare("e4"),
        makePiece(Color::WHITE, Figure::PAWN)
    );

    position.play(move, false);
    position.display();
}