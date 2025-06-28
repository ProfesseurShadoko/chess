#include "boardUI.hpp"
#include <sstream>
#include <tintoretto.hpp> // for cstr
#include <algorithm>    // for std::remove


const std::string BoardUI::startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// !-- Position Initialization --! //
BoardUI::BoardUI() {
    clearBoard();
    Message("Empty board initialized");
}

void BoardUI::clearBoard() {
    board.resize(8, std::vector<char>(8, '.')); // Initialize an 8x8 board with all squares empty (' ')
}

void BoardUI::setPositionFromFENsubstring(const std::string& fen) {
    // Basic fen structure: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
    
    int column = 0, row = 0, i = 0; // i is index in the string
    while (i < fen.size() && row < 8) {
        char c = fen[i];

        if (c == ' ') throw std::invalid_argument("Invalid FEN: there should be no space in the position part of the fen");

        if (c == '/') {
            // move to next rank
            if (column != 8) throw std::invalid_argument("Invalid FEN: row does not have 8 columns");
            column = 0;
            row++;
        } else if (std::isdigit(c)) {
            // empty squares
            int empty = c - '0';
            if (column + empty > 8) throw std::invalid_argument("Invalid FEN: too many empty squares in row");
            column += empty;
        } else {
            if (column >= 8) throw std::invalid_argument("Invalid FEN: row has more than 8 columns");
            const std::string pieces = "rnbqkpRNBQKP";
            if (pieces.find(c) == std::string::npos) throw std::invalid_argument("Invalid FEN: unknown piece '" + std::string(1, c) + "'");
            board[7-row][column] = c; // Place the piece on the board // fen order is upside down :(
            column++;
        }
        i++;
    }

    if (column != 8 || row != 7) throw std::invalid_argument("Invalid FEN: not enough ranks or columns"); 
}

// !-- String Parsing --! //
void BoardUI::fromFEN(const std::string& fen) {
    clearBoard(); // Clear the board before setting a new position

    // first let's cut the fen around spaces
    std::istringstream iss(fen);
    std::vector<std::string> parts;
    std::string tok;
    while (iss >> tok) parts.push_back(tok);
 
    // !-- Position --! //
    setPositionFromFENsubstring(parts[0]); // The first part is the position

    // Set activeColor, castlingRights, enPassantTarget, halfMoveClock, fullMoveClock from FEN
    // FEN: [0]=position [1]=activeColor [2]=castling [3]=enPassant [4]=halfmove [5]=fullmove

    // Active color
    if (parts.size() > 1 && (parts[1] == "w" || parts[1] == "b")) {
        activeColor = parts[1][0];
    } else {
        activeColor = 'w'; // default
    }

    // Castling rights
    if (parts.size() > 2) {
        castlingRights = parts[2];
    } else {
        castlingRights = "KQkq";
    }

    // En passant target
    if (parts.size() > 3) {
        enPassantTarget = parts[3];
    } else {
        enPassantTarget = "-";
    }

    // Halfmove clock
    if (parts.size() > 4) {
        halfMoveClock = std::stoi(parts[4]);
    } else {
        halfMoveClock = 0;
    }

    // Fullmove clock
    if (parts.size() > 5) {
        fullMoveClock = std::stoi(parts[5]);
    } else {
        fullMoveClock = 1;
    }

    Message("Board initialized from: " + cstr(fen).cyan());
}

std::string BoardUI::toFEN() const {
    std::ostringstream fen;

    // Piece placement
    for (int row = 7; row >= 0; row--) {
        int empty = 0;
        for (int col = 0; col < 8; col++) {
            char piece = board[row][col];
            if (piece == '.') {
                empty++;
            } else {
                if (empty > 0) {
                    fen << empty;
                    empty = 0;
                }
                fen << piece;
            }
        }
        if (empty > 0) fen << empty;
        if (row != 0) fen << '/';
    }

    // The rest
    fen << ' ' << activeColor;
    fen << ' ' << castlingRights;
    fen << ' ' << enPassantTarget;
    fen << ' ' << halfMoveClock;
    fen << ' ' << fullMoveClock;

    return fen.str();
}


// !-- Move --! //
void BoardUI::play(const std::string& move, bool isRookForCastle) {
    // Check the format
    if (move.length() != 4 && move.length() != 5) throw std::invalid_argument("Move must have 4 or 5 characters");
    if (!std::islower(move[0]) || move[0] < 'a' || move[0] > 'h') throw std::invalid_argument("First character must be a lowercase letter between 'a' and 'h'");
    if (move[1] < '1' || move[1] > '8') throw std::invalid_argument("Second character must be a digit between '1' and '8'");
    if (!std::islower(move[2]) || move[2] < 'a' || move[2] > 'h') throw std::invalid_argument("Third character must be a lowercase letter between 'a' and 'h'");
    if (move[3] < '1' || move[3] > '8') throw std::invalid_argument("Fourth character must be a digit between '1' and '8'");

    
    Message("Play: " + cstr(move).cyan());

    // get piece from intial square
    int from_col = move[0] - 'a';
    int from_row = move[1] - '1';
    int to_col = move[2] - 'a';
    int to_row = move[3] - '1';
    char piece = board[from_row][from_col];
    if (piece == '.') throw std::invalid_argument("No piece on the source square");

    // if there is promotion, check that piece is q,r,b or n
    if (move.length() == 5) {
        char promo = move[4];
        if (promo != 'q' && promo != 'r' && promo != 'n' && promo != 'b') throw std::invalid_argument("Fifth character (promotion piece) must be one of 'q', 'r', 'n', or 'b'");
        // change piece right away
        board[from_row][from_col] = promo;

        // do we need to make it uppercase?
        if (std::isupper(piece)) {
            promo = std::toupper(promo);
        }

        piece = promo;
        Message::print("[promotion to " + std::string(1, promo) + "]");
    }

    if (isRookForCastle) {
        board[from_row][from_col] = '.';
        board[to_row][to_col] = piece;
        return;
    }

    // -------------------- //
    // !-- Castle Logic --! //
    // -------------------- //

    // if piece is king, move the rook as well
    if (piece == 'k' || piece == 'K') {
        if (move == "e1g1") {
            play("h1f1", true);
            Message::print("[castle]");
        }
        
        if (piece == 'k') {
            // Black king moved, disable both black castling rights
            // Remove 'k' and 'q' from castlingRights if present
            castlingRights.erase(std::remove(castlingRights.begin(), castlingRights.end(), 'k'), castlingRights.end());
            castlingRights.erase(std::remove(castlingRights.begin(), castlingRights.end(), 'q'), castlingRights.end());
        }

        if (piece == 'K') {
            castlingRights.erase(std::remove(castlingRights.begin(), castlingRights.end(), 'K'), castlingRights.end());
            castlingRights.erase(std::remove(castlingRights.begin(), castlingRights.end(), 'Q'), castlingRights.end());
        }

        Message::print("[castleRights updated]");
    }

    // if one of the squares is h8 h1 a8 a1 remove castleingRights
    if (move.find("a1") != std::string::npos) {
        castlingRights.erase(std::remove(castlingRights.begin(), castlingRights.end(), 'Q'), castlingRights.end());
        Message::print("[castleRights updated]");
    }
    if (move.find("h1") != std::string::npos) {
        castlingRights.erase(std::remove(castlingRights.begin(), castlingRights.end(), 'K'), castlingRights.end());
        Message::print("[castleRights updated]");
    }
    if (move.find("a8") != std::string::npos) {
        castlingRights.erase(std::remove(castlingRights.begin(), castlingRights.end(), 'q'), castlingRights.end());
        Message::print("[castleRights updated]");
    }
    if (move.find("h8") != std::string::npos) {
        castlingRights.erase(std::remove(castlingRights.begin(), castlingRights.end(), 'k'), castlingRights.end());
        Message::print("[castleRights updated]");
    }

    // if castle rights are empty "", replace by "-"
    if (castlingRights.empty()) castlingRights = "-";

    // ------------------------ //
    // !-- En Passant Logic --! //
    // ------------------------ //

    // reset enPassant, enPassant corresponds to last move
    enPassantTarget = '-';
    // remove piece taken en passant
    if (piece == 'p' || piece == 'P') {
        // if pawn takes (goes diagonal)
        if (from_col != to_col) {
            // check if there is nothing in target square
            if (board[to_row][to_col] == '.') {
                // remove en passant pawn
                board[from_row][to_col] = '.';
                Message::print("[enPassant]");
            }

        }

        // if pawn goes 2 squares up or down
        if (abs(from_row - to_row) == 2) {
            int enPassantRow = (from_row + to_row) / 2;
            int enPassantCol = from_col;

            // update enPassant
            enPassantTarget = std::string(1, 'a' + enPassantCol) + std::to_string(enPassantRow + 1);
            Message::print("[enPassant enabled]");
        }
    } 

    

    // ------------------ //
    // !-- TurnsLogic --! //
    // ------------------ //

    // update activeColor
    if (std::isupper(piece)) {
        if (activeColor == 'b') Message("Active color does not correspond to moved piece!", "?");
        activeColor = 'b';
    } else {
        if (activeColor == 'w') Message("Active color does not correspond to moved piece!", "?");
        activeColor = 'w';
    }

    // update halfMoveClock // TODO: but set it to zero if capture or pawn!
    if (move.length() == 5) halfMoveClock = -1; // a pawn just promoted // we set -1 because we add 1 later
    if (piece == 'p' || piece == 'P') halfMoveClock = -1; // a pawn just promoted
    if (board[to_row][to_col] != '.') halfMoveClock = -1; // a capture is happening // note that is this was en passant, the previous check for pawn passed already
    halfMoveClock += 1;

    // update fullMoveClock
    if (activeColor == 'w') fullMoveClock += 1; // black has just played!

    // move piece
    board[from_row][from_col] = '.';
    board[to_row][to_col] = piece;

}



const std::vector<std::vector<char>>& BoardUI::getBoard() const {
    return board;
}


// !-- Display --! //
std::string BoardUI::letterToPiece(char letter) {
    std::string empty  = " ";
    std::string BK     = cstr("\u265A").blue(); // ♔ --> codes are switched here, this is actually code for white king
    std::string BQ     = cstr("\u265B").blue(); // ♕
    std::string BR     = cstr("\u265C").blue(); // ♖
    std::string BB     = cstr("\u265D").blue(); // ♗
    std::string BN     = cstr("\u265E").blue(); // ♘  
    std::string BP     = cstr("\u265F").blue(); // ♙

    std::string WK     = cstr("\u265A").yellow(); // ♚
    std::string WQ     = cstr("\u265B").yellow(); // ♛
    std::string WR     = cstr("\u265C").yellow(); // ♜
    std::string WB     = cstr("\u265D").yellow(); // ♝
    std::string WN     = cstr("\u265E").yellow(); // ♞
    std::string WP     = cstr("\u265F").yellow(); // ♟

    switch (letter) {
        case 'K': return WK;
        case 'Q': return WQ;
        case 'R': return WR;
        case 'B': return WB;
        case 'N': return WN;
        case 'P': return WP;

        case 'k': return BK;
        case 'q': return BQ;
        case 'r': return BR;
        case 'b': return BB;
        case 'n': return BN;
        case 'p': return BP;

        case '.':
        default:  return empty;
    }
}

const std::string BoardUI::toString() const {
    std::string out = "\nFEN: " + toFEN() + "\n";
    // Define the separation between two rows
    const std::string hborder = "   +----+----+----+----+----+----+----+----+";

    // Add the top border
    out += hborder + "\n";
    
    // Let's do row 8 down to 1
    std::string row_string;
    std::string square_string;
    for (int row = 7; row >= 0; row--) {
        // Add the row number
        row_string = std::to_string(row + 1) + ". |";

        for (int col = 0; col < 8; col++) {
            square_string = BoardUI::letterToPiece(board[row][col]);
            row_string += " " + square_string + "  |";
        }
        out += row_string + "\n";
        out += hborder + "\n";
    }
    // Add the column labels
    out += "     a.   b.   c.   d.   e.   f.   g.   h.\n";
    return out;
}
