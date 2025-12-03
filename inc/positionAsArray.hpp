

#include "positionStructureBase.hpp"
#include "positionBase.hpp"
#include <vector>
#include "move.hpp"




// -------------------------- //
// !-- Position Structure --! //
// -------------------------- //


class PosStructAsArray : public PositionStructureBase {
    private:
        std::vector<Piece> boardArray; // 64 elements representing the board
    public:
        PosStructAsArray() : boardArray(64, makePiece(Color::WHITE, Figure::EMPTY)) {} // initialize empty board
        
        Piece getPieceAt(Square square) const override {
            return boardArray[square];
        }

        void setPieceAt(Square square, Piece piece) override {
            boardArray[square] = piece;
        }
};


// ----------------------- //
// !-- Move Generation --! //
// ----------------------- //

class MoveGeneratorForArray : public MoveGeneratorBase<PosStructAsArray> {
    public:
        std::vector<Move> getLegalMoves(PosStructAsArray& position,  Color sideToMove, Square enPassantSquare, std::vector<bool> castlingRights) const override {
            // 1. Generate all pseudo-legal moves
            std::vector<Move> pseudoLegalMoves = getPseudoLegalMoves(position, sideToMove, enPassantSquare, castlingRights);

            // 2. Filter out illegal moves (those that leave the king in check)
            std::vector<Move> legalMoves;
            for (const Move& move : pseudoLegalMoves) {
                if (isLegalMove(position, move)) {
                    legalMoves.push_back(move);
                }
            }

            return legalMoves;
        }

        std::vector<Move> getPseudoLegalMoves(const PosStructAsArray& position,  Color sideToMove, Square enPassantSquare, std::vector<bool> castlingRights) const;
        std::vector<Move> generateIterativeMovesForPiece(const PosStructAsArray& position, Square square, Piece piece) const;
        std::vector<int> getDirectionalOffsets(Figure figure) const;

        bool isLegalMove(PosStructAsArray& position, const Move& move) const;
        bool isInCheck(const PosStructAsArray& position, Color color) const override;
};


using PositionAsArray = PositionBaseT<PosStructAsArray, MoveGeneratorForArray>;