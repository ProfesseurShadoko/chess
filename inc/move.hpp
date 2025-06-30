
#ifndef MOVEBASE_HPP
#define MOVEBASE_HPP

/**
 * Store all the information necessary for a move,
 * with helpers to update fen info
 */

#include <cstdint>
#include <cstdlib> // for abs
#include <stdexcept> // for std::runtime_error
#include <string>


// ----------------------- //
// !-- Pieces & Colors --! //
// ----------------------- //

enum class Color : uint32_t { // uint is better for bit operations
    WHITE = 0,
    BLACK = 8,
};

enum class Figure : uint32_t {
    EMPTY = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,
};

using Square = uint32_t; // 0-63 for 64 squares on the board
using Piece = uint32_t; // sum of color and figure, e.g., WHITE | PAWN

inline constexpr Piece makePiece(Color color, Figure figure) {
    return static_cast<Piece>(static_cast<uint32_t>(color) | static_cast<uint32_t>(figure));
}

inline constexpr Color getColor(Piece piece) {
    return static_cast<Color>(piece & 0x8); // 0x8 is the bit for color
}

inline constexpr Figure getFigure(Piece piece) {
    return static_cast<Figure>(piece & 0x7); // 0x7 is the bits for figure
} // 0x7 = 0b0111 --> mask all except the last 3 bits

inline constexpr uint32_t getRow(Square square) {
    return square / 8;
}

inline constexpr uint32_t getCol(Square square) {
    return square % 8;
}



// ------------ //
// !-- Move --! //
// ------------ //

class Move {
    public:
        // First bits: origin square (6 bits)
        // Next bits: destination square (6 bits)
        // Next bits: piece being moved (4 bits)
        // Next bits: captured piece (4 bits)
        // Next bits: promotion piece (4 bits)
        uint32_t move;

        // TODO: replace everything below by bit operations
        Move(uint32_t move) : move(move) {}

        Move(Square from, Square to, Piece piece, Piece captured = makePiece(Color::WHITE, Figure::EMPTY), Piece promotion = makePiece(Color::WHITE,Figure::EMPTY)) {
            move = from << 18 | to << 12 | piece << 8 | captured << 4 | promotion; // << means move up!
        }

        Square getFrom() const {
            // move >> 18 --> shift last bits out of the way
            // & 0x3F = 0b111111 --> select next 6
            return static_cast<Square>((move >> 18) & 0b111111);
        }

        Square getTo() const {
            return static_cast<Square>((move >> 12) & 0b111111);
        }

        Piece getPiece() const {
            return static_cast<Piece>((move >> 8) & 0b1111);
        }

        Piece getCapture() const {
            return static_cast<Piece>((move >> 4) & 0b1111);
        }

        Piece getPromotion() const{
            return static_cast<Piece>(move & 0b1111);
        }

        uint32_t hash() const {
            return move;
        }

        bool isPromotion() const {
            return getFigure(getPromotion()) != Figure::EMPTY;
        }

        /**
         * This returns false for enPassant
         */
        bool isCapture() const {
            return getFigure(getCapture()) != Figure::EMPTY;
        }

        /**
         * If pawn moves diagonally but getCapture is empty
         */
        bool isEnPassant() const {
            return getFigure(getPiece()) == Figure::PAWN && getCol(getFrom()) != getCol(getTo()) && !isCapture();
        }

        bool isCastle() const {
            return getFigure(getPiece()) == Figure::KING && (abs(getCol(getFrom()) - getCol(getTo())) == 2);
        }

        bool isDoubleAdvance() const {
            return getFigure(getPiece()) == Figure::PAWN && abs(getRow(getFrom()) - getRow(getTo())) == 2;
        }

        uint32_t getEnPassantSquare() const {
            if (!isEnPassant()) {
                throw std::runtime_error("Move is not an en passant move");
            }
            return (getFrom() + getTo()) / 2;
        }

        std::string toString() {
            // Returns move in UCI format, e.g., "e2e4", "e7e8q"
            auto squareToStr = [](Square sq) -> std::string {
                char file = 'a' + getCol(sq);
                char rank = '1' + getRow(sq);
                return std::string{file, rank};
            };

            std::string str = squareToStr(getFrom()) + squareToStr(getTo());
            if (isPromotion()) {
                Figure promo = getFigure(getPromotion());
                char promoChar = 'q'; // default
                switch (promo) {
                    case Figure::KNIGHT: promoChar = 'n'; break;
                    case Figure::BISHOP: promoChar = 'b'; break;
                    case Figure::ROOK:   promoChar = 'r'; break;
                    case Figure::QUEEN:  promoChar = 'q'; break;
                    default: break;
                }
                str += promoChar;
            }
            return str;
        }
};

#endif

