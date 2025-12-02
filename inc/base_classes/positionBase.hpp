
#ifndef POSITIONBASE_HPP
#define POSITIONBASE_HPP

#include "move.hpp"
#include <vector>
#include "boardUI.hpp"

#include <unordered_map>




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

    public:
        /**
         * @brief Creates a position base object by initializing it through the reset() method, 
         * that sets the position to the standard chess starting position (through fromFEN).
         */
        PositionBase() {
            // reset(); // if you call this here, setPieceAt is called before the child class is constructed!! Bad!! Error with pure virtual function.
        }

        virtual ~PositionBase() = default;

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

        void reset() {
            fromFEN(BoardUI::startpos);
            initializeHash();
        }

        /**
         * @brief Get the piece on a specific square from a BoardUI object.
         * 
         * Necessary to convert position from FEN and communicate with BoardUI.
         * 
         * @param square The square index (0-63).
         * @param boardUI The BoardUI object to get the piece from.
         */
        Piece getPieceOnBoardUI(Square square, const BoardUI& boardUI) const;

        /**
         * @brief Set the piece on a specific square in a BoardUI object.
         * 
         * Necessary to convert position to FEN and communicate with BoardUI.
         * 
         * @param square The square index (0-63).
         * @param piece The piece to set.
         * @param boardUI The BoardUI object to set the piece in.
         */
        void setPieceOnBoardUI(Square square, Piece piece, BoardUI& boardUI) const;

        /**
         * @brief Sets the position from a FEN string.
         * 
         * Carefull, of course this doesn't allow to build back position history, moves, Zobrist key. This should be only called at the beginning of a game.
         * 
         * @param fen The FEN string representing the chess position.
         * @throws std::invalid_argument if the FEN string is invalid.
         */
        void fromFEN(const std::string& fen);

        /**
         * @brief Converts the position to a FEN string.
         * 
         * @return The FEN string representing the chess position.
         */
        std::string toFEN() const;

        // --------------------- //
        // !-- Play & Unplay --! //
        // --------------------- //

    protected:
        uint32_t getNewCastlingRights(const Move& move) const;
        Square getNewEnPassantSquare(const Move& move) const;
        void updatePieceCount(const Move& move);
        void restorePieceCount(const Move& move);

    
        /**
         * @brief Changes nothing but the piece representation.
         */
        virtual void playOnPosition(const Move& move) = 0;
        virtual void unplayOnPosition(const Move& move) = 0;

    public:
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

        bool isDrawByInsufficientMaterial() const {
            return false; // TODO: implement this!!
        };

        Square getEnPassantSquare() const {
            return enPassantSquare;
        }

        Color getActiveColor() const {
            return activeColor;
        }

        /**
         * @brief Get the castling rights as a vector of booleans.
         * @return A vector of booleans representing castling rights in the order: White king side, White queen side, Black king side, Black queen side.
         */
        std::vector<bool> getCastlingRights() const {
            return {
                (castlingRights & 0b1000) != 0, // White king side
                (castlingRights & 0b0100) != 0, // White queen side
                (castlingRights & 0b0010) != 0, // Black king side
                (castlingRights & 0b0001) != 0  // Black queen side
            };
        }

        
        







        // ----------------------- //
        // !-- Move Generation --! //
        // ----------------------- //

        /**
         * @brief Generates all legal moves for the current position.
         * @return A vector of legal moves.
         */
        virtual std::vector<Move> getLegalMoves() = 0;


        // ------------------ //
        // !-- Evaluation --! //
        // ------------------ //

        /**
         * @brief Get all pieces and corresponding squares on the board.
         * 
         * Pure virtual function (depends on the underlying structure). Used for evaluation function.
         */
        virtual std::vector<SquarePiece> getPieces() const = 0; // for the evaluation function


        // ----------------------- //
        // !-- Zobrist Hashing --! //
        // ----------------------- //

    protected:
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

    public:
        uint64_t getHash() const {
            return zobristKey;
        }

        void initializeHash();
        void updateHash(const Move& move);
        void restoreHash(const Move& move) {updateHash(move);} // updateHash is an involution.
    
    protected:
        // Zobriest tables
        static uint64_t pieceKeys[2 /*colors*/][6 /*figures*/][64 /*squares*/];
        static uint64_t castlingKeys[16 /*castling rights*/]; // 2^4bits
        static uint64_t enPassantKeys[8 /*columns*/];
        static uint64_t activeColorKey;
        static inline bool hashInitialized = false; // to avoid re-initializing the hash tables multiple times
        static void initializeHashTables();

    // ---------- //
    // !-- UI --! //
    // ---------- //

    public:
        void display();
};




// TODO: add StructuredPositionBase that inherits from PositionBase
// and has a structurea assigned to it (array, bitboard, etc) and implements 
// virtual functions by passing on to the structure.
// only get legal moves would remain virtual in StructuredPositionBase
#include "positionStructureBase.hpp"
#include "moveGeneratorBase.hpp"


template <typename PositionStructureType, typename MoveGeneratorType>
class PositionBaseT : public PositionBase {

    protected:
        PositionStructureType structure; // the underlying structure
        MoveGeneratorType moveGenerator; // the move generator
    
    public:
        PositionBaseT() {
            reset();
        }
    
    // ----------------- //
    // !-- Interface --! //
    // ----------------- //

    public:
        Piece getPieceAt(Square square) const override {
            return structure.getPieceAt(square);
        }
        void setPieceAt(Square square, Piece piece) override {
            structure.setPieceAt(square, piece);
        }
        std::vector<SquarePiece> getPieces() const override {
            return structure.getPieces();
        }
        void playOnPosition(const Move& move) override {
            structure.playOnPosition(move);
            moveGenerator.play(move, structure);
        }
        void unplayOnPosition(const Move& move) override {
            structure.unplayOnPosition(move);
            moveGenerator.unplay(move, structure);
        }

        std::vector<Move> getLegalMoves() override {
            return moveGenerator.getLegalMoves(structure, getActiveColor(), getEnPassantSquare(), getCastlingRights());
        }

        
    // ------------ //
    // !-- Copy --! //
    // ------------ //

        virtual ~PositionBaseT() = default;
};



#endif