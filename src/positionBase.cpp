

#include "positionBase.hpp"
#include "move.hpp"
#include <random>
#include <stdexcept>



// --------------------- //
// !-- Play & Unplay --! //
// --------------------- //

uint32_t PositionBase::getNewCastlingRights(const Move& move) const {
    uint32_t newRights = castlingRights;

    // if the king has moved
    if (getFigure(move.getPiece()) == Figure::KING) {
        if (getColor(move.getPiece()) == Color::WHITE) {
            newRights &= 0b0011; // remove white castling rights
        } else {
            newRights &= 0b1100; // remove black castling rights
        }
    }

    // if a piece has moved away from a1 or took something on a1
    if (move.getFrom() == 0 || move.getTo() == 0) {
        newRights &= 0b1011; // remove white queenside castling rights
    }
    if (move.getFrom() == 7 || move.getTo() == 7) {
        newRights &= 0b0111; // remove white kingside castling rights
    }
    if (move.getFrom() == 56 || move.getTo() == 56) {
        newRights &= 0b1110; // remove black queenside castling rights
    }
    if (move.getFrom() == 63 || move.getTo() == 63) {
        newRights &= 0b0001; // remove black kingside castling rights
    }

    return newRights;
}

Square PositionBase::getNewEnPassantSquare(const Move& move) const {
    Square newSquare = 64; // reset en passant square
    if (move.isDoubleAdvance()) {
        newSquare = move.getEnPassantSquare();
    }
    return newSquare;
}



// ----------------------- //
// !-- Zobrist Hashing --! //
// ----------------------- //

// let's initialize the static zobrist tables
uint64_t PositionBase::pieceKeys[2][6][64] = {};
uint64_t PositionBase::castlingKeys[16] = {};
uint64_t PositionBase::enPassantKeys[8] = {};
uint64_t PositionBase::activeColorKey = 0;

void PositionBase::initializeHashTables() {
    std::mt19937_64 rng(std::random_device{}());
    
    for (int color = 0; color < 2; ++color) {
        for (int piece = 0; piece < 6; ++piece) {
            for (int square = 0; square < 64; ++square) {
                pieceKeys[color][piece][square] = rng();
            }
        }
    }

    for (int i = 0; i < 16; ++i) {
        castlingKeys[i] = rng();
    }

    for (int i = 0; i < 8; ++i) {
        enPassantKeys[i] = rng();
    }

    activeColorKey = rng();
    PositionBase::hashInitialized = true;
}

void PositionBase::initializeHash() {
    if (!hashInitialized) {
        initializeHashTables();
    }
    zobristKey = 0;

    // 1) Hash based on pieces on the board
    for (Square square = 0; square < 64; ++square) {
        Piece piece = getPieceAt(square);
        if (getFigure(piece) != Figure::EMPTY) {
            // color: WHITE = 0, BLACK = 8 --> index 0 or 1
            uint32_t colorIndex = static_cast<uint32_t>(getColor(piece)) >> 3; // divide by 8
            uint32_t figureIndex = static_cast<uint32_t>(getFigure(piece)) - 1; // EMPTY is 0
            zobristKey ^= pieceKeys[colorIndex][figureIndex][square];
        }
    }

    // 2) Hash based on castling rights
    zobristKey ^= castlingKeys[castlingRights]; // 0 <= castlingRights < 16 since 4 bit integer

    // 3) Hash based on en passant square
    if (enPassantSquare < 64) { // valid en passant square
        uint32_t column = getCol(enPassantSquare);
        zobristKey ^= enPassantKeys[column];
    }

    // 4) Hash based on active color
    if (activeColor == Color::BLACK) {
        zobristKey ^= activeColorKey;
    }
}

/**
 * /!\ Must be called before applying the move!!
 */
void PositionBase::updateHash(const Move& move) {
    // 1) Unpack Move Info
    const Square from = move.getFrom();
    const Square to = move.getTo();
    const Piece piece = move.getPiece();
    const Piece captured = move.getCapture();
    const Piece promotion = move.getPromotion();

    // 2) Get new castling rights and en passant square
    uint32_t newCastlingRights = getNewCastlingRights(move);
    Square newEnPassantSquare = getNewEnPassantSquare(move);

    // 3) Piece movement
    zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(piece)) >> 3][static_cast<uint32_t>(getFigure(piece)) - 1][from];
    zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(piece)) >> 3][static_cast<uint32_t>(getFigure(piece)) - 1][to];

    // 4) Captured piece
    if (getFigure(captured) != Figure::EMPTY) {
        zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(captured)) >> 3][static_cast<uint32_t>(getFigure(captured)) - 1][to];
    }

    // 5) Move is enPassant --> remove the pawn that was captured
    if (move.isEnPassant()) {
        Square enPassantSquare = move.getEnPassantSquare();
        Piece enPassantPiece = makePiece(getColor(piece), Figure::PAWN);
        zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(enPassantPiece)) >> 3][static_cast<uint32_t>(getFigure(enPassantPiece)) - 1][enPassantSquare];
    }

    // 6) Promotion --> pawn becomes ew piece
    if (move.isPromotion()) {
        // remove the pawn
        zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(piece)) >> 3][static_cast<uint32_t>(Figure::PAWN) - 1][to];
        // add the new piece
        zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(promotion)) >> 3][static_cast<uint32_t>(getFigure(promotion)) - 1][to];
    }

    // 7) Update castling rights
    if (castlingRights != newCastlingRights) {
        zobristKey ^= castlingKeys[castlingRights];
        zobristKey ^= castlingKeys[newCastlingRights];
    }

    // 8) Update en passant square
    if (enPassantSquare != newEnPassantSquare) {
        if (enPassantSquare < 64) { // valid en passant square
            uint32_t column = getCol(enPassantSquare);
            zobristKey ^= enPassantKeys[column];
        }
        if (newEnPassantSquare < 64) { // valid en passant square
            uint32_t column = getCol(newEnPassantSquare);
            zobristKey ^= enPassantKeys[column];
        }
    }

    // 9) Update active color
    zobristKey ^= activeColorKey; // switch to black
}


