
#ifndef BOARDUI_HPP // this bit of logic will be equivalent to #pragma once
#define BOARDUI_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <tintoretto.hpp>
#include <sstream>
#include <iostream>
#include <algorithm> // for std::remove

/**
 * @class BoardUI
 * @brief Parses and stores a chessboard position from FEN. This is basically a wrapper to modify FENs, to hanlde them in an out.
 * 
 * @details This class takes a FEN string
 * and fills a 2D vector with the pieces (using characters to represent each piece).
 * An other class will be responsible for displaying the board on screen.
 */
class BoardUI {
    public:
        std::vector<std::vector<char>> board; // 8x8 array to store the board state
        char activeColor;
        std::string castlingRights;
        std::string enPassantTarget;
        int halfMoveClock; // number of moves since last capture / pawn
        int fullMoveClock;

        std::vector<std::string> markedSquares; // squares that are marked for some reason, for instance to highlight a move
        std::vector<int> markedSquaresColors; // colors for marked squares (0 green, 1 red, 2 cyan, 3 purple)


        /**
         * @brief Initilizes the board to an empty state.
         */
        void clearBoard() {
            board.resize(8, std::vector<char>(8, '.')); // Initialize an 8x8 board with all squares empty (' ')
        }

        /**
         * @brief Parse and store position from FEN substring (rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR), so the first part after splitting aroung spaces.
         * @param[in] fen The FEN string representing the chess position.
         * @throws std::invalid_argument if the FEN string is invalid.
         */
        void setPositionFromFENsubstring(const std::string& fen_substring) {
            // Basic fen structure: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
    
            int column = 0, row = 0, i = 0; // i is index in the string
            while (i < fen_substring.size() && row < 8) {
                char c = fen_substring[i];

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

        // !-- Display --! //
        static std::string letterToPiece(char letter) {
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



    public:

        static inline const std::string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

        /**
         * @brief Construct an empty board (all squares '.')
         */
        BoardUI() {
            clearBoard();
            Message("Empty board initialized");
        }

        /**
         * @brief Parse and store position from FEN string.
         * @param[in] fen The FEN string representing the chess position.
         * @throws std::invalid_argument if the FEN string is invalid.
         */
        void fromFEN(const std::string& fen) {
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

        /**
         * @brief returns a FEN string from current position
         */
        std::string toFEN() const {
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

        /**
         * @brief modifies in place the board. Doesn't check the validity of the move.
         * @param[in] move The stirng representing the move. For instance e1g1 (castle king-side) or d7d8n for knight promotion.
         * @param[in] ignoreUpdate False by default, just moves the piece and doesn't update active color or castle rights or anything.
         * 
         */
        void play(const std::string& move, bool isRookForCastle = false) {
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
                if (move == "e1c1") {
                    play("a1d1", true);
                    Message::print("[castle]");
                }
                if (move == "e8g8") {
                    play("h8f8", true);
                    Message::print("[castle]");
                }
                if (move == "e8c8") {
                    play("a8d8", true);
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

            // mark arrival square and departure square in green
            unmarkAll(); // clear previous marks
            mark(std::string(1, 'a' + from_col) + std::to_string(from_row + 1), 0); // green
            mark(std::string(1, 'a' + to_col) + std::to_string(to_row + 1), 0); // green

        }

        /**
         * @brief  Get the 8×8 array of piece codes.
         * @return A const reference to the board array.
         */
        const std::vector<std::vector<char>>& getBoard() const {
            return board;
        }


        /**
         * @brief Returns for the console the string representation of the board.
         */
        const std::string toString() const {
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
                    square_string = letterToPiece(board[row][col]) + " ";

                    // let's highlight the square if it is marked
                    // find square string from row and col
                    std::string square = std::string(1, 'a' + col) + std::to_string(row + 1);
                    bool found = false;
                    for (int i = 0; i < markedSquares.size(); i++) {
                        if (markedSquares[i] == square) {
                            found = true;
                            // add color to square_string
                            switch (markedSquaresColors[i]) {
                                case 0: square_string = cstr("<").green() + square_string + cstr(">").green(); break; // green
                                case 1: square_string = cstr("<").red() + square_string + cstr(">").red(); break; // red
                                case 2: square_string = cstr("<").cyan() + square_string + cstr(">").cyan(); break; // cyan
                                case 3: square_string = cstr("<").purple() + square_string + cstr(">").purple(); break; // purple   
                                
                                default: break; // should not happen anyway
                            }
                        }
                    }
                    if (!found) square_string = " " + square_string + " "; // no color, just a space
                    row_string += square_string + "|";
                }
                out += row_string + "\n";
                out += hborder + "\n";
            }
            // Add the column labels
            out += "     a.   b.   c.   d.   e.   f.   g.   h.\n";
            return out;
        }


        /**
         * @brief Overload of the << operator
         */
        friend std::ostream& operator<<(std::ostream& os, const BoardUI& boardUI) {
            os << boardUI.toString();
            return os;
        }


        // ---------------------- //
        // !-- Marked Squares --! //
        // ---------------------- //
        void mark(const std::string& square, int color = 0) {
            // Check if square is valid
            if (square.length() != 2 || square[0] < 'a' || square[0] > 'h' || square[1] < '1' || square[1] > '8') {
                throw std::invalid_argument("Invalid square: " + square);
            }

            // Add the square to the marked squares
            markedSquares.push_back(square);
            markedSquaresColors.push_back(color);
        }

        void unmarkAll() {
            markedSquares.clear();
            markedSquaresColors.clear();
        }
};

#endif