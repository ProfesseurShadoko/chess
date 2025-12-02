
#ifndef POSITION_STRUCTURE_BASE_HPP
#define POSITION_STRUCTURE_BASE_HPP

#include "move.hpp"
#include <vector>

class PositionStructureBase {
    public:
        virtual Piece getPieceAt(Square square) const = 0;
        virtual void setPieceAt(Square square, Piece piece) = 0;
        
        virtual std::vector<SquarePiece> getPieces() const; // for the evaluation function
        virtual void playOnPosition(const Move& move);
        virtual void unplayOnPosition(const Move& move);
};

#endif // POSITION_STRUCTURE_HPP