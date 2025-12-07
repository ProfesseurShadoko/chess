

#include <vector>
#include "move.hpp"
#include "boardUI.hpp"
#include "positionAsArray.hpp"
#include <cmath>


// ----------------------------- //
// !-- Structure Play Unplay --! //
// ----------------------------- //


// ----------------------- //
// !-- All Lagel Moves --! //
// ----------------------- //


std::vector<int> MoveGeneratorForArray::getDirectionalOffsets(Figure figure) const {
    switch (figure) {
        case Figure::BISHOP:
            return {7, 9, -7, -9};
        case Figure::ROOK:
            return {8, -8, 1, -1};
        case Figure::QUEEN:
            return {7, 9, -7, -9, 8, -8, 1, -1};
        case Figure::KING:
            return {7, 9, -7, -9, 8, -8, 1, -1};
        case Figure::KNIGHT:
            return {15, 17, -15, -17, 10, 6, -10, -6};
        default:
            return {};
    }
}


std::vector<Move> MoveGeneratorForArray::generateIterativeMovesForPiece(const PosStructAsArray& position, Square square, Piece piece) const {
    std::vector<Move> moves;
    Figure figure = getFigure(piece);
    Color sideToMove = getColor(piece);
    std::vector<int> directions = getDirectionalOffsets(figure);
    uint32_t maxSteps = (figure == Figure::KING || figure == Figure::KNIGHT) ? 1 : 7;

    // loop over each direction, advance until stopped
    for (int direction : directions) {
        Square previousSquare = square;
        for (uint32_t step = 1; step <= maxSteps; ++step) {
            Square targetSquare = square + direction * step; // Square = uint32_t

            // 1. Check if targetSquare is on board
            if (targetSquare >= 64) break;

            // 2. Check that the row of target square is at most different by 2 columns
            int column_change = std::abs(static_cast<int>(getCol(targetSquare) - getCol(previousSquare)));
            if (column_change > 2) break;

            previousSquare = targetSquare;

            // 3. Check wether target square is occupied
            Piece targetPiece = position.getPieceAt(static_cast<Square>(targetSquare));
            if (getFigure(targetPiece) == Figure::EMPTY) {
                // Empty square, valid move
                moves.push_back(
                    Move(square, static_cast<Square>(targetSquare), piece)
                );
            } else {
                // Occupied square
                if (getColor(targetPiece) != sideToMove) {
                    // Capture move
                    moves.push_back(
                        Move(square, static_cast<Square>(targetSquare), piece, targetPiece)
                    );
                }
                break; // Stop in this direction after hitting a piece
            }
        }
    }


    return moves;
}


std::vector<Move> MoveGeneratorForArray::getPseudoLegalMoves(const PosStructAsArray& position,  Color sideToMove, Square enPassantSquare, std::vector<bool> castlingRights) const {
    std::vector<Move> moves;

    // 1. Generate pseudo-legal moves for all pieces of the side to move
    std::vector<SquarePiece> pieces = position.getPieces();
    for (const SquarePiece& squarePiece : pieces) {
        Square square = getSquareFromPair(squarePiece);
        Piece piece = getPieceFromPair(squarePiece);

        if (getColor(piece) != sideToMove) continue; // not our piece
        Figure figure = getFigure(piece);

        // Generate moves based on figure type
        switch (figure) {
            case Figure::PAWN: {
                // generate pawn moves
                uint32_t direction = (sideToMove == Color::WHITE) ? 8 : -8;
                uint32_t diagonalLeft = (sideToMove == Color::WHITE) ? 7 : -9;
                uint32_t diagonalRight = (sideToMove == Color::WHITE) ? 9 : -7;
                uint32_t startRank = (sideToMove == Color::WHITE) ? 1 : 6;

                // 1. if square in front is empty, move forward (a pawn has always a square in front of it, otherwise it would have promoted)
                if (getFigure(position.getPieceAt(square + (sideToMove == Color::WHITE ? 8 : -8))) == Figure::EMPTY) {
                    // if we do not reach promotion rank
                    if (getRow(square + direction) != (sideToMove == Color::WHITE ? 7 : 0)) {
                        moves.push_back(
                            Move(square, square + direction, piece)
                        );
                    } else {
                        // promotion moves
                        moves.push_back(
                            Move(square, square + direction, piece, makePiece(Color::WHITE, Figure::EMPTY), makePiece(sideToMove, Figure::QUEEN))
                        );
                        moves.push_back(
                            Move(square, square + direction, piece, makePiece(Color::WHITE, Figure::EMPTY), makePiece(sideToMove, Figure::ROOK))
                        );
                        moves.push_back(
                            Move(square, square + direction, piece, makePiece(Color::WHITE, Figure::EMPTY), makePiece(sideToMove, Figure::BISHOP))
                        );
                        moves.push_back(
                            Move(square, square + direction, piece, makePiece(Color::WHITE, Figure::EMPTY), makePiece(sideToMove, Figure::KNIGHT))
                        );
                    }
                }
                    
                // 2. if diagonal squares contain opponent piece, capture
                if (square % 8 != 0) { // not on a-file
                    Piece targetPiece = position.getPieceAt(square + diagonalLeft);
                    if (getFigure(targetPiece) != Figure::EMPTY && getColor(targetPiece) != sideToMove) {
                        moves.push_back(
                            Move(square, square + diagonalLeft, piece, targetPiece)
                        );
                    }
                }
                if (square % 8 != 7) { // not on h-file
                    Piece targetPiece = position.getPieceAt(square + diagonalRight);
                    if (getFigure(targetPiece) != Figure::EMPTY && getColor(targetPiece) != sideToMove) {
                        moves.push_back(
                            Move(square, square + diagonalRight, piece, targetPiece)
                        );
                    }
                }

                // 3. if on starting rank and two squares in front are empty, double advance
                if (getRow(square) == startRank) {
                    if (getFigure(position.getPieceAt(square + direction)) == Figure::EMPTY &&
                        getFigure(position.getPieceAt(square + 2 * direction)) == Figure::EMPTY) {
                        moves.push_back(
                            Move(square, square + 2 * direction, piece)
                        );
                    }
                }
                break;
            }

            case Figure::KNIGHT:
            case Figure::BISHOP:
            case Figure::ROOK:
            case Figure::QUEEN:
            case Figure::KING:
                for (const Move& move : generateIterativeMovesForPiece(position, square, piece)) {
                    moves.push_back(move);
                }
                break;
            default:
                break;
        }
    }

    // 2. Add en passant
    if (enPassantSquare < 64) {
        // generate en passant captures
        // check wether there are pawns in the corrsponding squares
        int direction = (sideToMove == Color::WHITE) ? -8 : 8; // wether the pawns move up or down
        Square leftSquare = enPassantSquare + 1; // change column but same row
        Square rightSquare = enPassantSquare - 1; // change column but same row
        leftSquare += direction;
        rightSquare += direction;
        // check with columns that we do not wrap around the board
        if (std::abs(static_cast<int>(getCol(leftSquare) - getCol(enPassantSquare))) == 1) {
            Piece leftPiece = position.getPieceAt(leftSquare);
            if (getFigure(leftPiece) == Figure::PAWN && getColor(leftPiece) == sideToMove) {
                moves.push_back(
                    Move(leftSquare, enPassantSquare, leftPiece, makePiece(Color::WHITE, Figure::EMPTY))
                );
            }
        }
        if (std::abs(static_cast<int>(getCol(rightSquare) - getCol(enPassantSquare))) == 1) {
            Piece rightPiece = position.getPieceAt(rightSquare);
            if (getFigure(rightPiece) == Figure::PAWN && getColor(rightPiece) == sideToMove) {
                moves.push_back(
                    Move(rightSquare, enPassantSquare, rightPiece, makePiece(Color::WHITE, Figure::EMPTY))
                );
            }
        }
    }

    // 3. Add castling (check wether allowed, and wether king movement of 1 square instread of castle is allowed)
    if (sideToMove == Color::WHITE) {
        // white 
        if (castlingRights[0]) {
            // check that squares between e1 and h1 are empty
            if (getFigure(position.getPieceAt(makeSquare("f1"))) == Figure::EMPTY &&
                getFigure(position.getPieceAt(makeSquare("g1"))) == Figure::EMPTY) {
                moves.push_back(
                    Move(makeSquare("e1"), makeSquare("g1"), makePiece(Color::WHITE, Figure::KING))
                );
                }
        }
        if (castlingRights[1]) {
            // check that squares between e1 and a1 are empty
            if (getFigure(position.getPieceAt(makeSquare("d1"))) == Figure::EMPTY &&
                getFigure(position.getPieceAt(makeSquare("c1"))) == Figure::EMPTY &&
                getFigure(position.getPieceAt(makeSquare("b1"))) == Figure::EMPTY) {
                moves.push_back(
                    Move(makeSquare("e1"), makeSquare("c1"), makePiece(Color::WHITE, Figure::KING))
                );
            }
        }
    } else {
        // black castling
        if (castlingRights[2]) {
            // check that squares between e8 and h8 are empty
            if (getFigure(position.getPieceAt(makeSquare("f8"))) == Figure::EMPTY &&
                getFigure(position.getPieceAt(makeSquare("g8"))) == Figure::EMPTY) {
                moves.push_back(
                    Move(makeSquare("e8"), makeSquare("g8"), makePiece(Color::BLACK, Figure::KING))
                );
            }
        }
        if (castlingRights[3]) {
            // check that squares between e8 and a8 are empty
            if (getFigure(position.getPieceAt(makeSquare("d8"))) == Figure::EMPTY &&
                getFigure(position.getPieceAt(makeSquare("c8"))) == Figure::EMPTY &&
                getFigure(position.getPieceAt(makeSquare("b8"))) == Figure::EMPTY) {
                moves.push_back(
                    Move(makeSquare("e8"), makeSquare("c8"), makePiece(Color::BLACK, Figure::KING))
                );
            }
        }
    }

    // 4. Return the list of pseudo-legal moves
    return moves;
}





// --------------------- //
// !-- Is Legal Move --! //
// --------------------- //

bool MoveGeneratorForArray::isLegalMove(PosStructAsArray& position, const Move& move) const {
    position.playOnPosition(move);
    bool isValid = !isInCheck(position, getColor(move.getPiece())); // if white just played, white king should not be in check
    position.unplayOnPosition(move);
    return isValid;
}


bool MoveGeneratorForArray::isInCheck(const PosStructAsArray& position, Color color) const {
    // Check wether color king is in check

    // 1. Generate all pseudo-legal moves for opposite color
    std::vector<Move> pseudoLegalMoves = getPseudoLegalMoves(position, ~color, 64, {false, false, false, false}); // no en passant square, no castling rights needed for this

    // 2. Find wether any of these moves capture the king
    for (const Move& move : pseudoLegalMoves) {
        if (getFigure(move.getCapture()) == Figure::KING) {
            return true; // king is in check
        }
    }
    return false;
}
