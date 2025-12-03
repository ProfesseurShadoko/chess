
#include "positionStructureBase.hpp"
#include "move.hpp"


void PositionStructureBase::playOnPosition(const Move& move) {
    Square from = move.getFrom();
    Square to = move.getTo();
    Piece piece = move.getPiece();
    Piece promotion = move.getPromotion();

    // Move the piece
    setPieceAt(to, move.isPromotion() ? promotion : piece); // handle promotion
    setPieceAt(from, makePiece(Color::WHITE, Figure::EMPTY)); // empty the from square

    // If this was castle, move the rook accordingly
    if (move.isCastle()) {
        // the target square of the rook is the mean between from and to
        Square rookTo = (from + to) / 2;
        // rook is same row as king, and column depends on side
        int rookRow = getRow(from);
        Square rookFrom;
        if (getCol(to) > getCol(from)) {
            // king-side castle
            rookFrom = rookRow * 8 + 7; // h-file
        } else {
            // queen-side castle
            rookFrom = rookRow * 8 + 0; // a-file
        }
        playOnPosition(
            Move(rookFrom, rookTo, getPieceAt(rookFrom))
        );
    }

    // if en Passant, remove the captured pawn
    if (move.isEnPassant()) {
        // the pawn to remove is one square up (or down) from the to square
        Square pawnSquare = (getColor(piece) == Color::WHITE) ? to - 8 : to + 8;
        setPieceAt(pawnSquare, makePiece(Color::WHITE, Figure::EMPTY));
    }
}


void PositionStructureBase::unplayOnPosition(const Move& move) {
    Square from = move.getFrom(); // Castle: where the king comes from => e1 or e8
    Square to = move.getTo(); // Castle: where the king went to => g1/c1 or g8/c8 depending on side
    Piece piece = move.getPiece();
    Piece captured = move.getCapture();
    Piece promotion = move.getPromotion();

    // Restore the piece
    setPieceAt(from, piece); // move back the piece
    setPieceAt(to, captured); // restore captured piece or empty square

    // Was the move castle? if so, we must put back the rook
    if (move.isCastle()) {
        // the target square of the rook is the mean between from and to
        Square rookTo = (from + to) / 2;
        // where was the rook originally? a or h file? if king went to the left (towards a file, hence col(to) < col(from)), rook was on a file, else h file
        int rookRow = getRow(from);
        Square rookFrom;
        if (getCol(to) > getCol(from)) { // (white) king goes right -> king side
            // king-side castle
            rookFrom = rookRow * 8 + 7; // h-file
        } else {
            // queen-side castle
            rookFrom = rookRow * 8 + 0; // a-file
        }
        unplayOnPosition(
            Move(rookFrom, rookTo, getPieceAt(rookTo))
        );
    }

    // Was the move en Passant? if so, we must restore the captured pawn
    if (move.isEnPassant()) {
        // the pawn to restore is one square up (or down) from the to square
        Square pawnSquare = (getColor(piece) == Color::WHITE) ? to - 8 : to + 8;
        setPieceAt(pawnSquare, makePiece(~getColor(piece), Figure::PAWN));
    }
}



std::vector<SquarePiece> PositionStructureBase::getPieces() const {
    std::vector<SquarePiece> pieces;
    for (Square square = 0; square < 64; ++square) {
        Piece piece = getPieceAt(square);
        if (getFigure(piece) != Figure::EMPTY) {
            pieces.push_back((static_cast<SquarePiece>(square) << 8) | static_cast<SquarePiece>(piece));
        }
    }
    return pieces;
}