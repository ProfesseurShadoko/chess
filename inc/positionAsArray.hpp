

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

        std::vector<SquarePiece> getPieces() const override {
            std::vector<SquarePiece> pieces;
            for (Square square = 0; square < 64; ++square) {
                Piece piece = boardArray[square];
                if (getFigure(piece) != Figure::EMPTY) {
                    pieces.push_back((static_cast<SquarePiece>(square) << 8) | static_cast<SquarePiece>(piece));
                }
            }
            return pieces;
        }

        void playOnPosition(const Move& move) override {
            // TODO: add castle, promotion and en passant logic

            Square from = move.getFrom();
            Square to = move.getTo();
            Piece piece = move.getPiece();
            Piece promotion = move.getPromotion();

            // Move the piece
            boardArray[to] = (getFigure(promotion) != Figure::EMPTY) ? promotion : piece; // handle promotion
            boardArray[from] = makePiece(Color::WHITE, Figure::EMPTY); // empty the from square
        }

        void unplayOnPosition(const Move& move) override {
            // TODO: add castle, promotion and en passant logic

            Square from = move.getFrom();
            Square to = move.getTo();
            Piece piece = move.getPiece();
            Piece captured = move.getCapture();
            Piece promotion = move.getPromotion();

            // Restore the piece
            boardArray[from] = piece; // move back the piece
            boardArray[to] = captured; // restore captured piece or empty square
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

        bool isValidPosition(PosStructAsArray& position, Color sideToMove) const;
};


using PositionAsArray = PositionBaseT<PosStructAsArray, MoveGeneratorForArray>;