
#ifndef POSITIONBASE_HPP
#define POSITIONBASE_HPP

#include "move.hpp"
#include <vector>
#include "boardUI.hpp"

#include <unordered_map>

/**
 * TODO:
 * include unordered map and keep track of maximum repetition over game, so that we don't need
 * to call 3 repetitions each time we make a move.
 * Fix the draw functions.
 * enable more flexibility for the handling of zobriest key.
 * transfer cpp code here into cpp file
 * 
 * fix count piece with bishops black and white
 * initialize piece count with fens
 */



 /**
  * @brief Base class for representing a chess position. Contains common attributes and methods for managing the game state. Method depending
  * on the representation of the board (array, bitboard, etc) are pure virtual and must be implemented by child classes.
  * 
  * Chess position represented by:
  * 
  * - active color (who's turn it is, updated by `play` and `unplay`)
  * 
  * - castling rights (4 bits, KQkq order, updated by `play` and `unplay`)
  * 
  * - en passant square (64 if no en passant possible, updated by `play` and `unplay`)
  * 
  * - halfmove clock (number of halfmoves since last capture or pawn move, updated by `play` and `unplay`), needed for 50-move rule
  * 
  * - fullmove clock (number of full moves, incremented after black's move, updated by `play` and `unplay`)
  * 
  * - piece placement (array or bitboard representation, whatever the child class implements, updated by `playOnPosition` and `unplayOnPosition` (virtual here, 
  * but called by `play` and `unplay`))
  * 
  * - piece count (keep track of the number of pieces on the board, updated by `play` and `unplay`), needed for draw by insufficient material
  * 
  * - isDrawByRepetition (set to true whenever the position is repeated three times by `addToPositionHistory`, reset to false by `unplay`)
  * 
  * - zobristKey (used for fast position comparison and transposition table, updated by `play` and `unplay`)
  * 
  * - positionHistory (used for detecting repetitions and fast position comparison, updated by `play` and `unplay`)
  * 
  * - moveHistory and stateHistory (used for undoing moves and restoring previous states, updated by `play` and `unplay`)
  */
class PositionBase {
    protected:
        // !-- Variables --! //
        Color activeColor = Color::WHITE;
        uint32_t castlingRights = 0b1111; // order: KQkq
        Square enPassantSquare = 64; // 64 is an invalid square, so it means no en passant
        uint32_t halfmoveClock = 0;
        uint32_t fullmoveClock = 1;

        // Play / Unplat stuff
        std::vector<UndoInfo> stateHistory; // keep track of previous states for unplay many times!
        std::vector<Move> moveHistory; // with state and move we can unplay a move!
        std::unordered_map<uint64_t, uint32_t> positionHistory; // history of positions as hashes for repetition detection
        uint64_t zobristKey = 0;
        
        // Additional draw rules
        uint32_t pieceCount[2 /* colors */][8 /* 7 figures, but distinguish between white and black bishop(3). Black bishop will be 7 and white bishop 3*/];
        bool _isDrawByRepetition = false; // don't forget to put back when unplaying

        /**
         * @brief Creates a position base object by initializing it through the reset() method, 
         * that sets the position to the standard chess starting position (through fromFEN).
         */
        PositionBase() {
            reset();
        }


        /**
         * @brief Get the piece on a specific square.
         * 
         * Pure virtual function (depends on the underlying structure). Necessary to convert position to FEN and communicate with BoardUI.
         * Doesn't need to be efficient, as it is only used BoardUI communication, which isn't used inside the tree search.
         */
        virtual Piece getPieceAt(Square square) const = 0;

        /**
         * @brief Set the piece on a specific square.
         * 
         * Pure virtual function (depends on the underlying structure). Necessary to convert position from FEN and communicate with BoardUI.
         * Doesn't need to be efficient, as it is only used BoardUI communication, which isn't used inside the tree search.
         */
        virtual void setPieceAt(Square square, Piece piece) = 0;


        // -------------------- //
        // !-- FEN notation --! //
        // -------------------- //

        void reset() {fromFEN(BoardUI::startpos);}

        /**
         * @brief Get the piece on a specific square from a BoardUI object.
         * 
         * Necessary to convert position from FEN and communicate with BoardUI.
         * 
         * @param square The square index (0-63).
         * @param boardUI The BoardUI object to get the piece from.
         */
        Piece getPieceOnBoardUI(Square square, const BoardUI& boardUI) const {
            if (square < 0 || square >= 64) throw std::out_of_range("Square out of range");
            int row = square / 8;
            int col = square % 8;
            char pieceChar = boardUI.board[row][col];
            return makePiece(pieceChar);
        }

        /**
         * @brief Set the piece on a specific square in a BoardUI object.
         * 
         * Necessary to convert position to FEN and communicate with BoardUI.
         * 
         * @param square The square index (0-63).
         * @param piece The piece to set.
         * @param boardUI The BoardUI object to set the piece in.
         */
        void setPieceOnBoardUI(Square square, Piece piece, BoardUI& boardUI) const {
            if (square < 0 || square >= 64) throw std::out_of_range("Square out of range");
            int row = square / 8;
            int col = square % 8;
            boardUI.board[row][col] = getCharFromPiece(piece);
        }

        /**
         * @brief Sets the position from a FEN string.
         * 
         * Carefull, of course this doesn't allow to build back position history, moves, Zobrist key. This should be only called at the beginning of a game.
         * 
         * @param fen The FEN string representing the chess position.
         * @throws std::invalid_argument if the FEN string is invalid.
         */
        void fromFEN(const std::string& fen) {
            BoardUI board;
            board.fromFEN(fen);

            // 1) Set the variables
            activeColor = board.activeColor == 'w' ? Color::WHITE : Color::BLACK;
            castlingRights = 0;
            if (board.castlingRights.find('K') != std::string::npos) castlingRights |= 0b1000; // white kingside
            if (board.castlingRights.find('Q') != std::string::npos) castlingRights |= 0b0100; // white queenside
            if (board.castlingRights.find('k') != std::string::npos) castlingRights |= 0b0010; // black kingside
            if (board.castlingRights.find('q') != std::string::npos) castlingRights |= 0b0001; // black queenside
            enPassantSquare = board.enPassantTarget == "-" ? 64 : board.enPassantTarget[0] - 'a' + (board.enPassantTarget[1] - '1') * 8; // convert to square index, 64 is an invalid square
            halfmoveClock = board.halfMoveClock;
            fullmoveClock = board.fullMoveClock;

            // 2) reset history of positions
            stateHistory.clear();
            moveHistory.clear(); // clear the move history

            // 3) add the pieces to the position
            for (Square square = 0; square < 64; square++) {
                Piece piece = getPieceOnBoardUI(square, board);
                if (piece != makePiece(Color::WHITE, Figure::EMPTY)) {
                    setPieceAt(square, piece); // implemented by child class
                }
            }
        }

        /**
         * @brief Converts the position to a FEN string.
         * 
         * @return The FEN string representing the chess position.
         */
        std::string toFEN() const {
            BoardUI board;

            // 1) Set variabels from BoardUI
            board.activeColor = (activeColor == Color::WHITE) ? 'w' : 'b';
            board.castlingRights = "";
            if (castlingRights & 0b1000) board.castlingRights += 'K'; // white kingside
            if (castlingRights & 0b0100) board.castlingRights += 'Q'; // white queenside
            if (castlingRights & 0b0010) board.castlingRights += 'k'; // black kingside
            if (castlingRights & 0b0001) board.castlingRights += 'q'; // black queenside
            if (board.castlingRights.empty()) board.castlingRights = "-"; // if no castling rights, set to "-"
            board.enPassantTarget = (enPassantSquare == 64) ? "-" :
                std::string(1, 'a' + getCol(enPassantSquare)) + std::to_string(getRow(enPassantSquare) + 1); // convert square index to algebraic notation
            board.halfMoveClock = halfmoveClock;
            board.fullMoveClock = fullmoveClock;

            // 2) Fill the board with pieces
            for (Square square = 0; square < 64; square++) {
                Piece piece = getPieceAt(square); // implemented by child class
                setPieceOnBoardUI(square, piece, board);
            }

            // 3) Convert to FEN string
            return board.toFEN();
        }



        // --------------------- //
        // !-- Play & Unplay --! //
        // --------------------- //

        uint32_t getNewCastlingRights(const Move& move) const;
        Square getNewEnPassantSquare(const Move& move) const;
        void updatePieceCount(const Move& move);
        void restorePieceCount(const Move& move);

        /**
         * @brief Changes nothing but the piece representation.
         */
        virtual void playOnPosition(const Move& move) const = 0;
        virtual void unplayOnPosition(const Move& move) const = 0;

        /**
         * Updates state variables and calls playOnPosition to update the position.
         */
        void play(const Move& move, bool definitive);

        /**
         * Restores state variables and calls unplayOnPosition to restore the position.
         */
        void unplay();


        // ------------- //
        // !-- Rules --! //
        // ------------- //

        bool isDrawBy50Moves() const {
            return halfmoveClock >= 100; // 50 moves = 100 halfmoves
        }

        bool isDrawByRepetition() const {
            return _isDrawByRepetition;
        }

        bool isDrawByInsufficientMaterial() {
            return false;
        };







        // ----------------------- //
        // !-- Move Generation --! //
        // ----------------------- //

        virtual std::vector<Move> getLegalMoves() const = 0;


        // ----------------------- //
        // !-- Zobrist Hashing --! //
        // ----------------------- //

        void addToPositionHistory(uint64_t hash) {
            positionHistory[hash]++;

            if (positionHistory[hash] > 2) {
                _isDrawByRepetition = true; // update the draw by repetition flag
            }
        }

        void removeFromPositionHistory(uint64_t hash) {
            positionHistory[hash]--; // we don't need to check maxRepetitions, this is handled by UnfoIndo
            if (positionHistory[hash] == 0) {
                positionHistory.erase(hash); // remove the hash from the history if it is not present // otherwise we risk the dictionnary to reach 2^64 elements :(
            }
        }

        uint64_t getHash() const {
            return zobristKey;
        }

        void initializeHash();
        void updateHash(const Move& move);
        void restoreHash(const Move& move) {updateHash(move);} // updateHash is an involution.
        
        // Zobriest tables
        static uint64_t pieceKeys[2 /*colors*/][6 /*figures*/][64 /*squares*/];
        static uint64_t castlingKeys[16 /*castling rights*/]; // 2^4bits
        static uint64_t enPassantKeys[8 /*columns*/];
        static uint64_t activeColorKey;
        static inline bool hashInitialized = false; // to avoid re-initializing the hash tables multiple times
        static void initializeHashTables();
};


#endif