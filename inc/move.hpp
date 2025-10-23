
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
#include <memory> // for std::unique_ptr


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

/**
 * @brief Creates a Piece from a Color and a Figure.
 * 
 * This function combines the color and figure into a single Piece value (an integer). Bit manipulations will
 * allow us to extract the color and figure from the Piece later.
 * 
 * @param color The color of the piece (WHITE or BLACK).
 * @param figure The figure of the piece (PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, or EMPTY).
 * @return The combined Piece value, Piece being an alias for uint32_t.
 */
inline constexpr Piece makePiece(Color color, Figure figure) {
    return static_cast<Piece>(static_cast<uint32_t>(color) | static_cast<uint32_t>(figure));
}

/**
 * @brief Creates a Piece from a character representation. The letter allows to infer the piece, 
 * and the case allows to infer the color.
 * 
 * @param c The character representation of the piece (e.g., 'P', 'N', 'B', 'R', 'Q', 'K' for white pieces, and 'p', 'n', 'b', 'r', 'q', 'k' for black pieces).
 * @return The combined Piece value.
 * @throws std::runtime_error If the character is not recognized as a valid piece.
 */
inline constexpr Piece makePiece(char c) {
    Color color = (c >= 'a' && c <= 'z') ? Color::BLACK : Color::WHITE;
    Figure figure = Figure::EMPTY;
    switch (std::tolower(c)) {
        case 'p': figure = Figure::PAWN; break;
        case 'n': figure = Figure::KNIGHT; break;
        case 'b': figure = Figure::BISHOP; break;
        case 'r': figure = Figure::ROOK; break;
        case 'q': figure = Figure::QUEEN; break;
        case 'k': figure = Figure::KING; break;
        case '.': figure = Figure::EMPTY; break; // empty square
        default: throw std::runtime_error("Unknown piece character");
    }
    return makePiece(color, figure);
}

/**
 * @brief Returns the color of a piece (uint32_t) by extracting the color bit.
 * 
 * @param piece The piece from which to extract the color.
 * @return The color of the piece (WHITE or BLACK).
 */
inline constexpr Color getColor(Piece piece) {
    return static_cast<Color>(piece & 0b1000); // 0x8 is the bit for color
}

/**
 * @brief Returns the figure of a piece (uint32_t) by extracting the figure bits.
 * 
 * @param piece The piece from which to extract the figure.
 * @return The figure of the piece (PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, or EMPTY).
 */
inline constexpr Figure getFigure(Piece piece) {
    return static_cast<Figure>(piece & 0x7); // 0x7 is the bits for figure // same as 0b0111
} // 0x7 = 0b0111 --> mask all except the last 3 bits

/**
 * @brief Returns the character representation of a piece.
 * 
 * This function converts a Piece to its character representation, using uppercase for white pieces and lowercase for black pieces.
 * 
 * @param piece The piece to convert.
 * @return The character representation of the piece.
 */
inline constexpr char getCharFromPiece(Piece piece) {
    // returns the character representation of the piece, in lowercase for black pieces
    char c = ' ';
    switch (getFigure(piece)) {
        case Figure::PAWN:   c = 'p'; break;
        case Figure::KNIGHT: c = 'n'; break;
        case Figure::BISHOP: c = 'b'; break;
        case Figure::ROOK:   c = 'r'; break;
        case Figure::QUEEN:  c = 'q'; break;
        case Figure::KING:   c = 'k'; break;
        case Figure::EMPTY:  c = '.'; break; // empty square
    }
    if (getColor(piece) == Color::WHITE) {
        c = std::toupper(c); // convert to uppercase for white pieces
    }
    return c;
}

/**
 * @brief Returns the row of a square (uint32_t) on the chessboard.
 * 
 * @param square The square index (0-63).
 * @return The row index (0-7).
 */
inline constexpr uint32_t getRow(Square square) {
    return square / 8;
}

/**
 * @brief Returns the column of a square (uint32_t) on the chessboard.
 * 
 * @param square The square index (0-63).
 * @return The column index (0-7).
 */
inline constexpr uint32_t getCol(Square square) {
    return square % 8;
}

/**
 * @brief Returns a square (an integer from 0 to 63) based on its string representation.
 * 
 * @param square The string. For instance `"c2"`.
 * @return The corresponding Square object (integer).
 */
inline const Square makeSquare(std::string square) {
    char col = square[0];
    char row = square[1];
    return (row - '1') * 8 + (col - 'a');
}

/**
 * @brief Converts a Square object (an integer from  to 0 to 63) to a string.
 * 
 * @param square The number of the square, from 0 to 63.
 * @return The corresponding string, for instance `"c2"`.
 */
inline const std::string getStringFromSquare(Square square) {
    char col = 'a' + getCol(square);
    char row = '1' + getRow(square);
    return std::string() + col + row;
}

/**
 * @brief Returns the color of a square (uint32_t) on the chessboard.
 * 
 * This function determines the color of a square based on the parities of its row and column indices.
 * 
 * @param square The square index (0-63).
 * @return The color of the square (WHITE or BLACK).
 */
inline constexpr Color getSquareColor(Square square) {
    return (getRow(square) + getCol(square)) % 2 == 0 ? Color::BLACK: Color::WHITE; // even squares are white, odd squares are black
}

/**
 * @brief Flips the color of a piece.
 * 
 * This function takes a Color and returns the opposite color. Black becomes White and vice versa.
 * 
 * @param color The color to flip (WHITE or BLACK).
 * @return The flipped color.
 */
inline constexpr Color operator~(Color color) {
    return static_cast<Color>(static_cast<uint32_t>(color) ^ 0b1000); // flip the color
}


/**
 * @brief Stores information needed to undo a move in a chess engine.
 *
 * This class keeps track of the game state before a move is played, 
 * allowing the engine to revert to the previous position. It stores:
 * 
 * - Castling rights (4 bits)
 * 
 * - En passant square (7 bits) 
 * 
 * - Halfmove clock for the 50 move rule (7 bits)
 * 
 * - A flag indicating whether the struct contains valid information
 * 
 * This assumes that Move keeps the rest of all necessary objects to restore the 
 * previous position (like captured piece, promoted piece, etc).
 * Moreover, a table keeping track of all played positions shall be dynamically updated along the way.
 * Zobrist key mustn't be saved here, as it can be reversed back by unplaying the move.
 *
 * UndoInfo doesn't implement the UndoLogic itself. This shall be done by the Position class.
 * 
 * Functions:
 * 
 * - Default constructor: Initializes an empty UndoInfo object.
 * 
 * - Parameterized constructor: Initializes with specific game state values.
 * 
 * - Method to check if the struct is empty: `isNull()` (ie if default constructor was used).
 * 
 */
struct UndoInfo {
    // information about the previous position, usefull for the unplay method!
    uint32_t castlingRights; 
    Square enPassantSquare; 
    uint32_t halfmoveClock;
    bool isEmpty;

    UndoInfo() : castlingRights(0), enPassantSquare(64), halfmoveClock(0), isEmpty(true) {} // default constructor
    UndoInfo(uint32_t castlingRights, Square enPassantSquare, uint32_t halfmoveClock)
        : castlingRights(castlingRights), enPassantSquare(enPassantSquare), halfmoveClock(halfmoveClock), isEmpty(false) {}

    bool isNull() const {return isEmpty;}
};




// ------------ //
// !-- Move --! //
// ------------ //

/**
 * @brief Represents a move in the chess game. Implements various functions to manipulate and retrieve information about the move.
 * 
 * This class is designed to contain all the necessary information about a move, so that an Engine can play it. Some of this
 * information can be used to unplay the move as well, but the state of the previous position (like en passant square, castling rights, etc)
 * need to be stored in the UndoInfo struct.
 * 
 * A Move is represented as a 32-bit integer, with the following bit allocation:
 * 
 * - First 6 bits: Origin square (0-63)
 * 
 * - Next 6 bits: Destination square (0-63)
 * 
 * - Next 4 bits: Piece being moved (0-15)
 * 
 * - Next 4 bits: Captured piece (0-15), 0 if no capture
 * 
 * - Last 4 bits: Promotion piece (0-15), 0 if no promotion
 * 
 * No UndoInfo is attached to the Move itself, as it is common to all moves for a given position, and thus 
 * shall be attached to the Position class.
 * 
 * Functions:
 * 
 * - Getters for move components: `getFrom()`, `getTo()`, `getPiece()`, `getCapture()`, `getPromotion()`
 * 
 * - Move type checks: `isPromotion()`, `isCapture()`, `isEnPassant()`, `isCastle()`, `isDoubleAdvance()`
 * 
 * - Utility functions: `getEnPassantSquare()`, `toString()`, `hash()`

 */
class Move {
    public:
        uint32_t move;

        /**
         * @brief Constructs a Move object from a 32-bit integer representation.
         * 
         * @param move The 32-bit integer representation of the move.
         */
        Move(uint32_t move) : move(move) {}

        /**
         * @brief Constructs a Move object from its components.
         * 
         * @param from The origin square (0-63).
         * @param to The destination square (0-63).
         * @param piece The piece being moved (0-15).
         * @param captured The captured piece (0-15), 0 if no capture. Defaults to 0 (WHITE | EMPTY).
         * @param promotion The promotion piece (0-15), 0 if no promotion. Defaults to 0 (WHITE | EMPTY).
         */
        Move(Square from, Square to, Piece piece, Piece captured = makePiece(Color::WHITE, Figure::EMPTY), Piece promotion = makePiece(Color::WHITE,Figure::EMPTY)) {
            move = from << 18 | to << 12 | piece << 8 | captured << 4 | promotion; // << means move up!
        }

        /**
         * @brief Retrieves the origin square of the move.
         */
        Square getFrom() const {
            // move >> 18 --> shift last bits out of the way
            // & 0x3F = 0b111111 --> select next 6
            return static_cast<Square>((move >> 18) & 0b111111);
        }

        /**
         * @brief Retrieves the destination square of the move.
         */
        Square getTo() const {
            return static_cast<Square>((move >> 12) & 0b111111);
        }

        /**
         * @brief Retrieves the piece being moved. From there, information about color and figure can be extracted:
         * 
         * - Color: `getColor(move.getPiece())`
         * 
         * - Figure: `getFigure(move.getPiece())`
         */
        Piece getPiece() const {
            return static_cast<Piece>((move >> 8) & 0b1111);
        }

        /**
         * @brief Retrieves the captured piece, if any. From there, information about color and figure can be extracted:
         * 
         * - Color: `getColor(move.getCapture())`
         * 
         * - Figure: `getFigure(move.getCapture())`
         */
        Piece getCapture() const {
            return static_cast<Piece>((move >> 4) & 0b1111);
        }

        /**
         * @brief Retrieves the promotion piece, if any. From there, information about color and figure can be extracted:
         * 
         * - Color: `getColor(move.getPromotion())`
         * 
         * - Figure: `getFigure(move.getPromotion())`
         */
        Piece getPromotion() const{
            return static_cast<Piece>(move & 0b1111);
        }

        /**
         * @brief Returns a hash of the move, which is simply its integer representation.
         */
        uint32_t hash() const {
            return move;
        }

        /**
         * @brief Checks if the move is a promotion.
         */
        bool isPromotion() const {
            return getFigure(getPromotion()) != Figure::EMPTY;
        }

        /**
         * @brief Checks if the move is a capture. Careful, this returns false for en passant moves!
         */
        bool isCapture() const {
            return getFigure(getCapture()) != Figure::EMPTY;
        }

        /**
         * @brief Checks if the move is an en passant capture.
         * 
         * Checks if the move is an en passant capture. Checks whether the pawn moves diagonally but getCapture is empty
         */
        bool isEnPassant() const {
            return getFigure(getPiece()) == Figure::PAWN && getCol(getFrom()) != getCol(getTo()) && !isCapture();
        }

        /**
         * @brief Checks if the move is a castling move.
         * 
         * Checks whether the king moves two squares horizontally.
         */
        bool isCastle() const {
            return getFigure(getPiece()) == Figure::KING && (abs(getCol(getFrom()) - getCol(getTo())) == 2);
        }

        /**
         * @brief Checks if the move is a double pawn advance.
         * 
         * Checks whether a pawn moves two squares forward from its starting position.
         */
        bool isDoubleAdvance() const {
            return getFigure(getPiece()) == Figure::PAWN && abs(getRow(getFrom()) - getRow(getTo())) == 2;
        }

        /**
         * @brief Returns the en passant square for the next move, if the move is a double pawn advance.
         * 
         * @throws std::runtime_error if the move is not a double pawn advance.
         */
        uint32_t getEnPassantSquare() const {
            if (!isDoubleAdvance()) {
                throw std::runtime_error("Move is not a double pawn advance, no en passant square available");
            }
            return (getFrom() + getTo()) / 2;
        }

        /**
         * @brief Converts the move to a string in UCI format. E.g., "e2e4", "e7e8q".
         */
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

