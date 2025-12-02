

#include "positionStructureBase.hpp"
#include <vector>
#include "move.hpp"



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
        std::vector<Square> getOccupiedSquares() const override {
            std::vector<Square> occupied;
            for (Square square = 0; square < 64; ++square) {
                if (getFigure(boardArray[square]) != Figure::EMPTY) {
                    occupied.push_back(square);
                }
            }
            return occupied;
        }
        void playOnPosition(const Move& move) override {
            Piece movingPiece = getPieceAt(move.getFrom());
            

};