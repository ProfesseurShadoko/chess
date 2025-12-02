

#include "positionBase.hpp"
#include "move.hpp"
#include <random>
#include <stdexcept>




// -------------------- //
// !-- FEN notation --! //
// -------------------- //

Piece PositionBase::getPieceOnBoardUI(Square square, const BoardUI& boardUI) const {
    if (square < 0 || square >= 64) throw std::out_of_range("Square out of range");
    int row = square / 8;
    int col = square % 8;
    char pieceChar = boardUI.board[row][col];
    return makePiece(pieceChar);
}

void PositionBase::setPieceOnBoardUI(Square square, Piece piece, BoardUI& boardUI) const {
    if (square < 0 || square >= 64) throw std::out_of_range("Square out of range");
    int row = square / 8;
    int col = square % 8;
    boardUI.board[row][col] = getCharFromPiece(piece);
}

void PositionBase::fromFEN(const std::string& fen) {
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

std::string PositionBase::toFEN() const {
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

uint32_t PositionBase::getNewCastlingRights(const Move& move) const {
    uint32_t newRights = castlingRights;

    // if the king has moved
    if (getFigure(move.getPiece()) == Figure::KING) {
        if (getColor(move.getPiece()) == Color::WHITE) {
            newRights &= 0b0011; // remove white castling rights
        } else {
            newRights &= 0b1100; // remove black castling rights
        }
    }

    // if a piece has moved away from a1 or took something on a1
    if (move.getFrom() == 0 || move.getTo() == 0) {
        newRights &= 0b1011; // remove white queenside castling rights
    }
    if (move.getFrom() == 7 || move.getTo() == 7) {
        newRights &= 0b0111; // remove white kingside castling rights
    }
    if (move.getFrom() == 56 || move.getTo() == 56) {
        newRights &= 0b1110; // remove black queenside castling rights
    }
    if (move.getFrom() == 63 || move.getTo() == 63) {
        newRights &= 0b1101; // remove black kingside castling rights
    }

    return newRights;
}

Square PositionBase::getNewEnPassantSquare(const Move& move) const {
    Square newSquare = 64; // reset en passant square
    if (move.isDoubleAdvance()) {
        newSquare = move.getEnPassantSquare();
    }
    return newSquare;
}

void PositionBase::updatePieceCount(const Move& move) {
    // Move is a capture
    if (move.isCapture()) {
        Color color = getColor(move.getCapture());
        Figure figure = getFigure(move.getCapture());
        pieceCount[static_cast<uint32_t>(color) >> 3][static_cast<uint32_t>(figure)]--;
    }

    // Move is a promotion
    if (move.isPromotion()) {
        Color color = getColor(move.getPiece());
        Figure figure = getFigure(move.getPromotion());
        // remove 1 pawn
        pieceCount[static_cast<uint32_t>(color) >> 3][static_cast<uint32_t>(Figure::PAWN)]--;
        // add the promoted piece
        pieceCount[static_cast<uint32_t>(color) >> 3][static_cast<uint32_t>(figure)]++;
    }

    // Move is enPassant
    if (move.isEnPassant()) {
        Color color = getColor(move.getPiece());
        // remove 1 pawn for opponent
        pieceCount[static_cast<uint32_t>(~color) >> 3][static_cast<uint32_t>(Figure::PAWN)]--;
    }
}

void PositionBase::restorePieceCount(const Move& move) {
    // Move was capture
    if (move.isCapture()) {
        Color color = getColor(move.getCapture());
        Figure figure = getFigure(move.getCapture());
        pieceCount[static_cast<uint32_t>(color) >> 3][static_cast<uint32_t>(figure)]++; // put back the piece
    }

    if (move.isPromotion()) {
        Color color = getColor(move.getPiece());
        Figure figure = getFigure(move.getPromotion());
        // put back 1 pawn
        pieceCount[static_cast<uint32_t>(color) >> 3][static_cast<uint32_t>(Figure::PAWN)]++;
        // remove the promoted piece
        pieceCount[static_cast<uint32_t>(color) >> 3][static_cast<uint32_t>(figure)]--;
    }

    if (move.isEnPassant()) {
        Color color = getColor(move.getPiece());
        // put back 1 pawn for opponent
        pieceCount[static_cast<uint32_t>(~color) >> 3][static_cast<uint32_t>(Figure::PAWN)]++;
    }
}


void PositionBase::play(const Move& move, bool definitive) {
    // first, handle everything that is not position representation
    stateHistory.push_back(UndoInfo(castlingRights, enPassantSquare, halfmoveClock));
    addToPositionHistory(zobristKey); // store the hash of the position before
    moveHistory.push_back(move); // add the move to the history
    updateHash(move); // update the hash with the move

    // clear cache when possible
    if (definitive) {
        // we won't do unplay, so we can clear some of the history
        stateHistory.clear(); // clear the state history since we won't unplay
        moveHistory.clear(); // clear the move history since we won't unplay
        if (move.isCapture() || getFigure(move.getPiece()) == Figure::PAWN) {
            positionHistory.clear(); // wipe out the position history // we can't do it if not definitive because the unplay method deosn't restore past history
        }
    }

    // update fullmoveClock
    if (activeColor == Color::BLACK) {
        fullmoveClock++;
    }

    // update halfmoveClock
    if (move.isCapture() || getFigure(move.getPiece()) == Figure::PAWN) {
        halfmoveClock = 0; // reset halfmove clock on capture or pawn move
    } else {
        halfmoveClock++;
    }

    // update castling rights
    // if the king has moved
    // if the king has moved
    if (getFigure(move.getPiece()) == Figure::KING) {
        if (activeColor == Color::WHITE) {
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
    activeColor = ~activeColor;

    playOnPosition(move);
}

void PositionBase::unplay() {
    // let's pop out the last move
    Move lastMove = moveHistory.back();
    moveHistory.pop_back();

    // let's pop out the undo info
    UndoInfo lastUndoInfo = stateHistory.back();
    stateHistory.pop_back();

    // restore UndoInfo
    castlingRights = lastUndoInfo.castlingRights;
    enPassantSquare = lastUndoInfo.enPassantSquare;
    halfmoveClock = lastUndoInfo.halfmoveClock;

    // restore the hash
    restoreHash(lastMove); // restore the hash from the last move by reapplying XOR
    removeFromPositionHistory(zobristKey); // remove the last position from the history

    // update draw by repetition flag
    _isDrawByRepetition = false; // reset the flag // indeed, if previous position was already daw by repetition, we wouldn't have bothered looking further!

    // switch active color
    activeColor = ~activeColor;
    // update fullmoveClock
    if (activeColor == Color::WHITE) {
        fullmoveClock--; // we are going back one move, so we decrease the fullmove
    }

    // restore the position
    unplayOnPosition(lastMove);
}



// ----------------------- //
// !-- Zobrist Hashing --! //
// ----------------------- //

// let's initialize the static zobrist tables
uint64_t PositionBase::pieceKeys[2][6][64] = {};
uint64_t PositionBase::castlingKeys[16] = {};
uint64_t PositionBase::enPassantKeys[8] = {};
uint64_t PositionBase::activeColorKey = 0;

void PositionBase::initializeHashTables() {
    std::mt19937_64 rng(std::random_device{}());
    
    for (int color = 0; color < 2; ++color) {
        for (int piece = 0; piece < 6; ++piece) {
            for (int square = 0; square < 64; ++square) {
                pieceKeys[color][piece][square] = rng();
            }
        }
    }

    for (int i = 0; i < 16; ++i) {
        castlingKeys[i] = rng();
    }

    for (int i = 0; i < 8; ++i) {
        enPassantKeys[i] = rng();
    }

    activeColorKey = rng();
    PositionBase::hashInitialized = true;
}

void PositionBase::initializeHash() {
    if (!hashInitialized) {
        initializeHashTables();
    }
    zobristKey = 0;

    // 1) Hash based on pieces on the board
    for (Square square = 0; square < 64; ++square) {
        Piece piece = getPieceAt(square);
        if (getFigure(piece) != Figure::EMPTY) {
            // color: WHITE = 0, BLACK = 8 --> index 0 or 1
            uint32_t colorIndex = static_cast<uint32_t>(getColor(piece)) >> 3; // divide by 8
            uint32_t figureIndex = static_cast<uint32_t>(getFigure(piece)) - 1; // EMPTY is 0
            zobristKey ^= pieceKeys[colorIndex][figureIndex][square];
        }
    }

    // 2) Hash based on castling rights
    zobristKey ^= castlingKeys[castlingRights]; // 0 <= castlingRights < 16 since 4 bit integer

    // 3) Hash based on en passant square
    if (enPassantSquare < 64) { // valid en passant square
        uint32_t column = getCol(enPassantSquare);
        zobristKey ^= enPassantKeys[column];
    }

    // 4) Hash based on active color
    if (activeColor == Color::BLACK) {
        zobristKey ^= activeColorKey;
    }
}

/**
 * /!\ Must be called before applying the move!!
 */
void PositionBase::updateHash(const Move& move) {
    // 1) Unpack Move Info
    const Square from = move.getFrom();
    const Square to = move.getTo();
    const Piece piece = move.getPiece();
    const Piece captured = move.getCapture();
    const Piece promotion = move.getPromotion();

    // 2) Get new castling rights and en passant square
    uint32_t newCastlingRights = getNewCastlingRights(move);
    Square newEnPassantSquare = getNewEnPassantSquare(move);

    // 3) Piece movement
    zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(piece)) >> 3][static_cast<uint32_t>(getFigure(piece)) - 1][from];
    zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(piece)) >> 3][static_cast<uint32_t>(getFigure(piece)) - 1][to];

    // 4) Captured piece
    if (getFigure(captured) != Figure::EMPTY) {
        zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(captured)) >> 3][static_cast<uint32_t>(getFigure(captured)) - 1][to];
    }

    // 5) Move is enPassant --> remove the pawn that was captured
    if (move.isEnPassant()) {
        Square enPassantSquare = move.getEnPassantSquare();
        Piece enPassantPiece = makePiece(~getColor(piece), Figure::PAWN);
        zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(enPassantPiece)) >> 3][static_cast<uint32_t>(getFigure(enPassantPiece)) - 1][enPassantSquare];
    }

    // 6) Promotion --> pawn becomes ew piece
    if (move.isPromotion()) {
        // remove the pawn
        zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(piece)) >> 3][static_cast<uint32_t>(Figure::PAWN) - 1][to];
        // add the new piece
        zobristKey ^= pieceKeys[static_cast<uint32_t>(getColor(promotion)) >> 3][static_cast<uint32_t>(getFigure(promotion)) - 1][to];
    }

    // 7) Update castling rights
    if (castlingRights != newCastlingRights) {
        zobristKey ^= castlingKeys[castlingRights];
        zobristKey ^= castlingKeys[newCastlingRights];
    }

    // 8) Update en passant square
    if (enPassantSquare != newEnPassantSquare) {
        if (enPassantSquare < 64) { // valid en passant square
            uint32_t column = getCol(enPassantSquare);
            zobristKey ^= enPassantKeys[column];
        }
        if (newEnPassantSquare < 64) { // valid en passant square
            uint32_t column = getCol(newEnPassantSquare);
            zobristKey ^= enPassantKeys[column];
        }
    }

    // 9) Update active color
    zobristKey ^= activeColorKey; // switch to black
}





/**
 * Displays the current position by using the board UI. Prints
 * also different information about the position (fen, hash, isDrawByRepetition, ...)
 */
void PositionBase::display() {

    // create the board UI
    Message::mute();
    std::string fen = toFEN();
    BoardUI board;
    board.fromFEN(fen);
    Message::unmute();

    // 1. display information
    Message("Current position:");
    Message::tab();
    Message("Meta");
    Message::tab();
    Message::print("Active Color: " + std::string(activeColor == Color::WHITE ? "White" : "Black"));
    std::string castlingStr;
    if (castlingRights & 0b1000) castlingStr += "K";
    if (castlingRights & 0b0100) castlingStr += "Q";
    if (castlingRights & 0b0010) castlingStr += "k";
    if (castlingRights & 0b0001) castlingStr += "q";
    Message::print("Castling Rights: " + castlingStr);
    
    Message::print("En Passant Square: " + (enPassantSquare < 64 ? 
        getStringFromSquare(enPassantSquare) : "-"));
    Message::print("Halfmove Clock: " + std::to_string(halfmoveClock));
    Message::print("Fullmove Clock: " + std::to_string(fullmoveClock));
    Message::print("FEN: " + fen);
    Message::print("Zobrist Hash: " + std::to_string(zobristKey));
    Message::untab();

    // 2. Display draw rules and all
    Message("Draw Rules:");
    Message::tab();
    Message::print("Draw by Repetition: " + std::string(isDrawByRepetition() ? "Yes" : "No"));
    Message::print("Draw by 50-move rule: " + std::string(isDrawBy50Moves() ? "Yes" : "No"));
    Message::print("Draw by insufficient material: " + std::string(isDrawByInsufficientMaterial() ? "Yes" : "No"));
    Message::print("Piece Count:");
    Message::tab();
    Message::print("White Pawns: " + std::to_string(pieceCount[0][static_cast<uint32_t>(Figure::PAWN)]));
    Message::print("White Knights: " + std::to_string(pieceCount[0][static_cast<uint32_t>(Figure::KNIGHT)]));
    Message::print("White Bishops: " + std::to_string(pieceCount[0][static_cast<uint32_t>(Figure::BISHOP)]));
    Message::print("White Rooks: " + std::to_string(pieceCount[0][static_cast<uint32_t>(Figure::ROOK)]));
    Message::print("White Queens: " + std::to_string(pieceCount[0][static_cast<uint32_t>(Figure::QUEEN)])); 
    Message::print("White Kings: " + std::to_string(pieceCount[0][static_cast<uint32_t>(Figure::KING)]));
    Message::print("Black Pawns: " + std::to_string(pieceCount[1][static_cast<uint32_t>(Figure::PAWN)]));
    Message::print("Black Knights: " + std::to_string(pieceCount[1][static_cast<uint32_t>(Figure::KNIGHT)]));
    Message::print("Black Bishops: " + std::to_string(pieceCount[1][static_cast<uint32_t>(Figure::BISHOP)]));
    Message::print("Black Rooks: " + std::to_string(pieceCount[1][static_cast<uint32_t>(Figure::ROOK)]));
    Message::print("Black Queens: " + std::to_string(pieceCount[1][static_cast<uint32_t>(Figure::QUEEN)])); 
    Message::print("Black Kings: " + std::to_string(pieceCount[1][static_cast<uint32_t>(Figure::KING)]));
    Message::untab();
    Message::print("Position History: " + std::to_string(positionHistory.size()) + " positions stored.");
    Message::print("Move History: " + std::to_string(moveHistory.size()) + " moves in history.");

    std::vector<Move> legalMoves = getLegalMoves();
    Message::print("Legal Moves: " + std::to_string(legalMoves.size()) + " moves available.");
    // 5. Mark pieces that can move
    for (const Move& move : legalMoves) {
        Square from = move.getFrom();
        board.mark(getStringFromSquare(from), 0); // mark in one color (green)
    }
    // 6. Mark where they can go
    for (const Move& move : legalMoves) {
        Square to = move.getTo();
        board.mark(getStringFromSquare(to), 2); // mark in another color (cyan)
    }

    board.display();

}