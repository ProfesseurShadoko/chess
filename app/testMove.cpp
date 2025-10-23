#include <tintoretto.hpp>
#include "move.hpp"
#include <string>


std::string str(char c) {
    return std::string() + c;
}

int main() {
    Message("Testing file move.hpp");
    
    // ---------------------- //
    // !-- Test MakePiece --! //
    // ---------------------- //

    Test test("makePiece");
    Piece piece = makePiece('k');
    Piece true_piece = makePiece(Color::BLACK, Figure::KING);

    test.check("Color is black", getColor(piece) == Color::BLACK);
    test.check("Piece is King", getFigure(piece) == Figure::KING);
    test.check("Piece is black King", piece == true_piece);
    test.check("Piece character is 'k'", getCharFromPiece(piece)=='k');
    test.check("Oponent color is white", ~getColor(piece) == Color::WHITE);
    test.complete();



    // -------------------- //
    // !-- Test Squares --! //
    // -------------------- //

    Square square = makeSquare("c2"); 
    test = Test("Square");
    test.check("Row is 1", getRow(square)==1);
    test.check("Column is 2", getCol(square)==2);
    test.check("Square is white", getSquareColor(square) == Color::WHITE);
    test.check("String is c2", getStringFromSquare(square) == "c2");
    test.complete();


    // ----------------- //
    // !-- Test Move --! //
    // ----------------- //

    Square e2 = makeSquare("e2");
    Square e4 = makeSquare("e4");
    Piece wp = makePiece('P');

    Move move(e2, e4, wp);
    test = Test("Move e2-e4");
    test.check("Initial Square is e2", move.getFrom() == e2);
    test.check("Final square us e4", move.getTo() == e4);
    test.check("Piece is white pawn", move.getPiece() == wp);
    test.check("Piece captured is empty", move.getCapture() == makePiece('.'));
    test.check("Piece promoted is empty", move.getPromotion() == makePiece('.'));
    test.check(
        "Move isn't catpure, promotion, en passant, castle",
        ~(
            move.isCapture() || move.isPromotion() || move.isCastle() || move.isEnPassant()
        )
    );
    test.check("Move is double advance", move.isDoubleAdvance());
    test.check("En passant square is e3", move.getEnPassantSquare() == makeSquare("e3"));
    test.check("Move is 'e2e4'", move.toString() == "e2e4");


    return 0;
}