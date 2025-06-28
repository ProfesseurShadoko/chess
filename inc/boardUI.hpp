
#ifndef BOARDUI_HPP // this bit of logic will be equivalent to #pragma once
#define BOARDUI_HPP

#include <string>
#include <vector>
#include <stdexcept>

/**
 * @class BoardUI
 * @brief Parses and stores a chessboard position from FEN. This is basically a wrapper to modify FENs, to hanlde them in an out.
 * 
 * @details This class takes a FEN string
 * and fills a 2D vector with the pieces (using characters to represent each piece).
 * An other class will be responsible for displaying the board on screen.
 */
class BoardUI {
    private:
        std::vector<std::vector<char>> board; // 8x8 array to store the board state
        char activeColor;
        std::string castlingRights;
        std::string enPassantTarget;
        int halfMoveClock; // number of moves since last capture / pawn
        int fullMoveClock;

        /**
         * @brief Initilizes the board to an empty state.
         */
        void clearBoard(); // Helper function to clear the board

        /**
         * @brief Parse and store position from FEN substring (rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR), so the first part after splitting aroung spaces.
         * @param[in] fen The FEN string representing the chess position.
         * @throws std::invalid_argument if the FEN string is invalid.
         */
        void setPositionFromFENsubstring(const std::string& fen_substring);

        static std::string letterToPiece(char letter);

    public:

        static const std::string startpos;

        /**
         * @brief Construct an empty board (all squares '.')
         */
        BoardUI();

        /**
         * @brief Parse and store position from FEN string.
         * @param[in] fen The FEN string representing the chess position.
         * @throws std::invalid_argument if the FEN string is invalid.
         */
        void fromFEN(const std::string& fen);

        /**
         * @brief returns a FEN string from current position
         */
        std::string toFEN() const;

        /**
         * @brief modifies in place the board. Doesn't check the validity of the move.
         * @param[in] move The stirng representing the move. For instance e1g1 (castle king-side) or d7d8n for knight promotion.
         * @param[in] ignoreUpdate False by default, just moves the piece and doesn't update active color or castle rights or anything.
         * 
         */
        void play(const std::string& move, bool isRookForCastle = false);

        /**
         * @brief  Get the 8×8 array of piece codes.
         * @return A const reference to the board array.
         */
        const std::vector<std::vector<char>>& getBoard() const;

        /**
         * @brief Returns for the console the string representation of the board.
         */
        const std::string toString() const;

        /**
         * @brief Overload of the << operator
         */
        friend std::ostream& operator<<(std::ostream& os, const BoardUI& boardUI) {
            os << boardUI.toString();
            return os;
        }
};

#endif