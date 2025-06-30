
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
    public:
        virtual ~PositionBase() = default;

        // ------------------- //
        // !-- Setup & FEN --! //
        // ------------------- //

        virtual void reset() = 0;
        virtual void fromFEN(const std::string& fen) const = 0;
        virtual std::string getFEN() const = 0;

        // --------------- //
        // !-- Getters --! //
        // --------------- //

        virtual Color getActiveColor() const = 0;
        virtual uint32_t getCastlingRights() const = 0;
        virtual Square getEnPassantSquare() const = 0;
        virtual uint32_t getHalfmoveClock() const = 0;
        virtual uint32_t getFullmoveClock() const = 0;
        virtual const std::vector<uint64_t>& getPositionHistory() const = 0;
        virtual const std::vector<Move>& getMoveHistory() const = 0;


        // ---------------------- //
        // !-- Move Execution --! //
        // ---------------------- //

        virtual void play(const Move& move) = 0;
        virtual void unplay() = 0;

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
        virtual bool isCheckmate() = 0;
        virtual bool isStalemate() = 0;
        virtual bool isDrawByRepetition() = 0;
        virtual bool isDrawBy50Moves() = 0;

        // --------------------------- //
        // !-- Transposition Table --! //
        // --------------------------- //
        virtual uint64_t getHash() const = 0;

};


 #endif