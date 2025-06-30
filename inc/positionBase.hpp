
#ifndef POSITION_BASE_HPP
#define POSITION_BASE_HPP


/**
 * Representation of the position 
 * Has intialize and play and unplay methods
 * store fen indo as well as transpositions
 */

#include "move.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include "boardUI.hpp"

/**
 * Representation of a position.
 */
class PositionBase {

    // !-- Variables --! //
    Color activeColor;

    // KQkq in this order
    uint32_t castlingRights = 0b1111;
    Square enPassantSquare = 64; // 64 is an invalid square, so it means no en passant
    uint32_t halfmoveClock = 0; // number of halfmoves since last capture
    uint32_t fullmoveClock = 1; // number of full moves, starts at 1
    std::vector<uint64_t> stateHistory; // contains the 5 variables above. used to unplay a move. // position history does not contain the current position.
    std::vector<uint64_t> positionHistoryHash; // history of positions as hashes for repetition detection // wipe out each time a pawn move is made or a capture // used for the 3 repetition rule.
    std::vector<Move> moveHistory; // history of moves played // needed to unplay

    public:
        virtual ~PositionBase() = default;

        // ------------------- //
        // !-- Setup & FEN --! //
        // ------------------- //

        virtual void reset() {
            fromFEN(BoardUI::startpos); // reset to the starting position
        }

        virtual void fromFEN(const std::string& fen) {
            // by default just set the variables above. children classes will handle position represetation.
            BoardUI board;
            board.fromFEN(fen);

            activeColor = board.activeColor == 'w' ? Color::WHITE : Color::BLACK;
            castlingRights = 0;
            if (board.castlingRights.find('K') != std::string::npos) castlingRights |= 0b1000; // white kingside
            if (board.castlingRights.find('Q') != std::string::npos) castlingRights |= 0b0100; // white queenside
            if (board.castlingRights.find('k') != std::string::npos) castlingRights |= 0b0010; // black kingside
            if (board.castlingRights.find('q') != std::string::npos) castlingRights |= 0b0001; // black queenside
            enPassantSquare = board.enPassantTarget == "-" ? 64 : board.enPassantTarget[0] - 'a' + (board.enPassantTarget[1] - '1') * 8; // convert to square index, 64 is an invalid square
            halfmoveClock = board.halfMoveClock;
            fullmoveClock = board.fullMoveClock;

            // reset history of positions
            stateHistory.clear();
            moveHistory.clear(); // clear the move history
        }

        /**
         * Fill the BoardUI object first, then use BoardUI fen functions.
         */
        virtual std::string getFEN() const = 0;
    
    protected:
        void fillBoardWithInfo(BoardUI& board) const {
            board.activeColor = (activeColor == Color::WHITE) ? 'w' : 'b';
            board.castlingRights = "";
            if (castlingRights & 0b1000) board.castlingRights += 'K'; // white kingside
            if (castlingRights & 0b0100) board.castlingRights += 'Q'; // white queenside
            if (castlingRights & 0b0010) board.castlingRights += 'k'; // black kingside
            if (castlingRights & 0b0001) board.castlingRights += 'q'; // black queenside
            if (board.castlingRights.empty()) board.castlingRights = "-"; // if no castling rights, set to "-"
            board.enPassantTarget = (enPassantSquare == 64) ? "-" : std::string(1, 'a' + getCol(enPassantSquare)) + std::to_string(getRow(enPassantSquare) + 1); // convert square index to algebraic notation*
            board.halfMoveClock = halfmoveClock;
            board.fullMoveClock = fullmoveClock;
            // fill the board with pieces --> done by children classes
        }

        // --------------- //
        // !-- Getters --! //
        // --------------- //

        Color getActiveColor() {
            return activeColor;
        };
        uint32_t getCastlingRights() {
            return castlingRights;
        };
        Square getEnPassantSquare() {
            return enPassantSquare;
        };
        uint32_t getHalfmoveClock() {
            return halfmoveClock;
        };
        uint32_t getFullmoveClock() {
            return fullmoveClock;
        };
        const std::vector<uint64_t>& getPositionHistory() {
            return stateHistory;
        };
        const std::vector<Move>& getMoveHistory() {
            return moveHistory;
        };


        // ---------------------- //
        // !-- Move Execution --! //
        // ---------------------- //

        virtual void play(const Move& move, bool definitive) {
            // add current position to history since we are going to make a move! (history only contais information about the past positions)
            stateHistory.push_back(getState());
            positionHistoryHash.push_back(getHash());

            if (definitive) { // this means we won't call unplay => no need to know the previous state! 
                stateHistory.clear();
                // we can check whether move is a capture or a pawn, and wipe out the position history
                if (move.isCapture() || getFigure(move.getPiece()) == Figure::PAWN) {
                    positionHistoryHash.clear(); // wipe out the position history // we can't do it if not definitive because otherwise the unplay method would have been painful
                }
            }

            // update fullmoveClock
            if (getActiveColor() == Color::BLACK) {
                fullmoveClock++;
            }

            // update halfmoveClock?
            if (move.isCapture() || getFigure(move.getPiece()) == Figure::PAWN) {
                halfmoveClock = 0; // reset halfmove clock on capture or pawn move
            } else {
                halfmoveClock++;
            }

            // update castling rights?
            // if the king has moved
            if (getFigure(move.getPiece()) == Figure::KING) {
                if (getActiveColor() == Color::WHITE) {
                    castlingRights &= 0b0011; // remove white castling rights
                } else {
                    castlingRights &= 0b1100; // remove black castling rights
                }
            }
            // if a piece has moved away from a1 or took something on a1
            if (move.getFrom() == 0 || move.getTo() == 0) {
                castlingRights &= 0b1011; // remove white queenside castling rights
            }
            if (move.getFrom() == 7 || move.getTo() == 7) {
                castlingRights &= 0b0111; // remove white kingside castling rights
            }
            if (move.getFrom() == 56 || move.getTo() == 56) {
                castlingRights &= 0b1110; // remove black queenside castling rights
            }
            if (move.getFrom() == 63 || move.getTo() == 63) {
                castlingRights &= 0b0001; // remove black kingside castling rights
            }

            // update en passant square
            enPassantSquare = 64; // reset en passant square
            if (move.isDoubleAdvance()) {
                enPassantSquare = move.getEnPassantSquare();
            }

            // switch active color
            activeColor = (activeColor == Color::WHITE) ? Color::BLACK : Color::WHITE;

            // add the move to history
            moveHistory.push_back(move);
        }

        /**
         * First, parent function should update its representation of the position. Then this is called.
         */
        virtual void unplay() {
            if (moveHistory.empty()) {
                throw std::runtime_error("No move to unplay");
            }
            if (stateHistory.empty()) {
                throw std::runtime_error("No state to unplay. The play method was probably called with definitive = true, so the state history was cleared.");
            }

            // let's pop out the last move
            moveHistory.pop_back();

            // restore the previous state
            uint32_t previousState = stateHistory.back();
            stateHistory.pop_back();

            // remove the hast from the position history
            positionHistoryHash.pop_back();
        }

        uint32_t getState() const {
            // First bit: active color (1 bit)
            // Next bits: castling rights (4 bits)
            // Next bits: en passant square (7 bits because 64 is invalid)
            // Next bits: halfmove clock (7 bits = 128 > 50*2)
            // Next bits: fullmove clock (8 bits = 256 moves)
            // Total: 27 bits < 32 bits
            return (
                (activeColor == Color::WHITE ? 0b0 : 0b1) << 26 | // 1 bit for active color
                (castlingRights & 0b1111) << 22 | // 4 bits for castling rights
                (enPassantSquare & 0b1111111) << 15 | // 7 bits for en passant square (0-63, 64 is invalid square, so we need 7 bits for 0-64)
                (halfmoveClock & 0b1111111) << 8 | // 6 bits for halfmove clock (0-127, 128 is more than enough for 2*50 moves)
                (fullmoveClock & 0b11111111) // 8 bits for fullmove clock (0-255, 256 is enough for a game)
            );
        }

        uint32_t fromState(uint32_t state) {
            // Extract the values from the state
            activeColor = (state >> 26) & 0b1 ? Color::BLACK : Color::WHITE;
            castlingRights = (state >> 22) & 0b1111;
            enPassantSquare = (state >> 15) & 0b1111111;
            halfmoveClock = (state >> 8) & 0b1111111;
            fullmoveClock = state & 0b11111111;
        }



        // ------------------- //
        // !-- Legal Moves --! //
        // ------------------- //
        // let's maybe store these so that we don't compute them twice,
        virtual std::vector<Move>& getPseudoLegalMoves() = 0;
        virtual std::vector<Move>& geLegalMoves() = 0;

        // ------------- //
        // !-- Rules --! //
        // ------------- //
        virtual bool isInCheck() = 0;
        virtual bool hasLegalMove() = 0;
        virtual bool isCheckmate() {
            return !hasLegalMove() & isInCheck();
        }
        virtual bool isStalemate() {
            return !hasLegalMove() & !isInCheck();
        }
        virtual bool isDrawByRepetition() = 0;
        virtual bool isDrawBy50Moves() = 0;

        
        // --------------------------- //
        // !-- Transposition Table --! //
        // --------------------------- //
        virtual uint64_t getHash() const = 0;

};


 #endif