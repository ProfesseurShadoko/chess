

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


    // ---------------- //
    // !-- Test FEN --! //
    // ---------------- //

    Test test("FEN conversion for PositionAsArray");
    std::string fen1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    PositionAsArray position;
    position.fromFEN(fen1);
    position.getLegalMoves(); // this function might move stuff, check that is doesn't
    test.check("FEN 1", position.toFEN() == fen1);
    position.fromFEN(position.toFEN()); // reset position
    test.check("FEN 1 after fromFEN(toFEN)", position.toFEN() == fen1);
    
    
    std::string fen2 = "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1";
    position.fromFEN(fen2);
    board.fromFEN(fen2);
    board.display();
    test.check("FEN 2", position.toFEN() == fen2);
    position.getLegalMoves(); // this function might move stuff, check that is doesn't
    position.fromFEN(position.toFEN()); // reset position
    test.check("FEN 2 after fromFEN(toFEN) and getLegalMoves", position.toFEN() == fen2);
    board.fromFEN(position.toFEN());
    board.display();


    test.complete();



    // ------------------ //
    // !-- Test Moves --! //
    // ------------------ //

    test = Test("Move and meta updates for PositionAsArray");
    // TODO


    // ------------------- //
    // !-- Test Castle --! //
    // ------------------- //


    std::string fencastle = "r3k2r/8/8/8/8/8/8/R3K2R w KQ - 0 1";
    position.fromFEN(fencastle);

    test = Test("White Castle moves");
    Move whiteKingSideCastle = Move(makeSquare("e1"), makeSquare("g1"), makePiece(Color::WHITE, Figure::KING));
    Move whiteQueenSideCastle = Move(makeSquare("e1"), makeSquare("c1"), makePiece(Color::WHITE, Figure::KING));

    // 1. King-side castle
    position.play(whiteKingSideCastle, false);
    position.getLegalMoves(); // this function might move stuff, check that is doesn't
    std::string fenAfterWhiteKingSideCastle = "r3k2r/8/8/8/8/8/8/R4RK1 b - - 1 1";
    test.check("White king-side castle FEN", position.toFEN(), fenAfterWhiteKingSideCastle);
    //position.display();

    position.unplay();
    position.getLegalMoves(); // this function might move stuff, check that is doesn't
    //position.display();
    test.check("White king-side castle unplay FEN", position.toFEN(), fencastle);

    // 2. Queen-side castle
    position.play(whiteQueenSideCastle, false);
    position.getLegalMoves(); // this function might move stuff, check that is doesn't
    std::string fenAfterWhiteQueenSideCastle = "r3k2r/8/8/8/8/8/8/2KR3R b - - 1 1";
    test.check("White queen-side castle FEN", position.toFEN(), fenAfterWhiteQueenSideCastle);
    //position.display();   
    position.unplay();
    position.getLegalMoves(); // this function might move stuff, check that is doesn't
    //position.display();
    test.check("White queen-side castle unplay FEN", position.toFEN(), fencastle);
    test.complete();


    // -------------------------- //
    // !-- Test play / unplay --! //
    // -------------------------- //

    test = Test("Play / Unplay moves on PositionAsArray");
    std::string fen = "3rk2r/p2nb1pp/1p6/1Pp1pQ2/PqBn1p1P/2NP4/2P2PPB/R3K2R w KQk c6 0 18";
    // see https://lichess.org/analysis/fromPosition/3rk2r/p2nb1pp/1p6/1Pp1pQ2/PqBn1p1P/2NP4/2P2PPB/R3K2R_w_KQk_c6_0_18
    board.fromFEN(fen);
    board.display();
    
    position.fromFEN(fen);
    position.getLegalMoves(); // this function might move stuff, check that is doesn't
    //position.display();
    test.check("FEN before moves", position.toFEN(), fen);

    // 1. En passant
    std::string fenAfterEnPassant = "3rk2r/p2nb1pp/1pP5/4pQ2/PqBn1p1P/2NP4/2P2PPB/R3K2R b KQk - 0 18";
    Move enPassant = Move(makeSquare("b5"), makeSquare("c6"), makePiece('P'), makePiece(' '));
    position.play(enPassant, false);
    position.getLegalMoves(); // this function might move stuff, check that is doesn't
    test.check("FEN after en passant", position.toFEN(), fenAfterEnPassant);
    position.unplay();
    position.getLegalMoves();
    test.check("FEN after unplay en passant", position.toFEN(), fen);

    // 2. Pawn double advance
    Move doubleAdvance = Move(makeSquare("g2"), makeSquare("g4"), makePiece('P'));
    std::string fenAfterDoubleAdvance = "3rk2r/p2nb1pp/1p6/1Pp1pQ2/PqBn1pPP/2NP4/2P2P1B/R3K2R b KQk g3 0 18";
    position.play(doubleAdvance, false);
    position.getLegalMoves();
    test.check("FEN after pawn double advance", position.toFEN(), fenAfterDoubleAdvance);
    position.unplay();
    position.getLegalMoves();
    test.check("FEN after unplay pawn double advance", position.toFEN(), fen);
    // black enPassant
    position.play(doubleAdvance, false);
    std::string fenAfterBlackEnPassant = "3rk2r/p2nb1pp/1p6/1Pp1pQ2/PqBn3P/2NP2p1/2P2P1B/R3K2R w KQk - 0 19";
    Move blackEnPassant = Move(makeSquare("f4"), makeSquare("g3"), makePiece('p'));
    position.play(blackEnPassant,false);
    position.getLegalMoves();
    test.check("FEN after black en passant", position.toFEN(), fenAfterBlackEnPassant);
    board.fromFEN(position.toFEN());
    board.display();

    position.unplay();
    position.getLegalMoves();
    test.check("FEN after unplay black en passant", position.toFEN(), fenAfterDoubleAdvance);


    
    test.complete();
    


    return 0;
}