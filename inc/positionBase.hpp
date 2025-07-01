
#ifndef POSITIONBASE_HPP
#define POSITIONBASE_HPP

#include "move.hpp"
#include <vector>



class PositionBase {
    protected:
        // !-- Variables --! //
        Color activeColor = Color::WHITE;
        uint32_t castlingRights = 0b1111; // order: KQkq
        Square enPassantSquare = 64; // 64 is an invalid square, so it means no en passant
        uint32_t halfmoveClock = 0;
        uint32_t fullmoveClock = 1;
        std::vector<UndoInfo> undoHistory;
        std::vector<uint64_t> positionHistoryHash; // history of positions as hashes for repetition detection

        uint64_t zobristKey = 0;
        /**
         * Here is everything I (don't) understand about this. Goal is to be able to compare two positions.
         * When well implemented, this key can be modified back an fourth, in place, by play / unplay methods.
         * Thi is faster than storing / copying the key.
         * 
         * This key will be used for transposition table and for repetition detection, with someting like a 
         * dictionnary. A smart choice of dictionnary structure will b needed at some point, but we will get 
         * there later. For our position history, a linear search might be faster than a dictionnary look up.
         */

        virtual Piece getPieceAt(Square square) const = 0;



        // --------------------- //
        // !-- Play & Unplay --! //
        // --------------------- //

        virtual uint32_t getNewCastlingRights(const Move& move) const;
        virtual Square getNewEnPassantSquare(const Move& move) const;

        // ----------------------- //
        // !-- Zobrist Hashing --! //
        // ----------------------- //

        uint64_t getZobristKey() const {
            return zobristKey;
        }

        virtual void initializeHash();
        virtual void updateHash(const Move& move);
        virtual void restoreHash(const Move& move) {updateHash(move);} // updateHash is an involution.
        
        // Zobriest tables
        static uint64_t pieceKeys[2 /*colors*/][6 /*figures*/][64 /*squares*/];
        static uint64_t castlingKeys[16 /*castling rights*/]; // 2^4bits
        static uint64_t enPassantKeys[8 /*columns*/];
        static uint64_t activeColorKey;
        static inline bool hashInitialized = false; // to avoid re-initializing the hash tables multiple times
        static void initializeHashTables();
};


#endif