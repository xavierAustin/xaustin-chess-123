#pragma once
#include "Game.h"
#include "ChessSquare.h"

#include <list>

#define MAX_DEPTH 1

const int pieceSize = 64;

enum ChessPiece {
    //unnessesary notation but useful for readability
    NoPiece = 0,
    Pawn    = 1,
    Knight  = 2,
    Bishop  = 3,
    Rook    = 4,
    Queen   = 5,
    King    = 6
};

struct Move_t {
    char sX;
    char sY;
    char dX;
    char dY;
};

using MoveList = std::list<Move_t>;

//
// the main game class
//
class Chess : public Game
{
public:
    Chess();
    ~Chess();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder& holder) override;
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    bool        canBitMoveFromToPosition(int sX, int sY, int dX, int dY, int c);
    void        bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    void        bitMovedFromToPosition(int sX, int sY, int dX, int dY, int c);

    void        stopGame() override;
    BitHolder& getHolderAt(const int x, const int y) override { return _grid[y][x]; }

	void        updateAI() override;
    //change this for debug :)
    bool        gameHasAI() override { return false; }
    void        QuickPlacePeice(int x, int y, ChessPiece type, bool color);
    bool        bitFromToHelper(int sX, int sY, int type, BitHolder& dst);
    MoveList    generateMoveList(int c);
    int         getPosAttacked(int pX, int pY, int c);
    void        gridToBitboard();
    int32_t     evaluateBoard();
    void        FENtoPos(std::string fen);
    int32_t     negamax(int depth, int32_t alpha, int32_t beta, int playerColor);
private:
    Bit *       PieceForPlayer(const int playerNumber, ChessPiece piece);
    const char  bitToPieceNotation(int row, int column) const;

    ChessSquare _grid[8][8];
    //not really implemented yet
    uint64_t _bitboards[8] = {};
    int _gridNum[8][8] = {
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0}
    };
    int _pTable[8][8] = {
        { 0,  0,  0,  0,  0,  0,  0,  0},
        {50, 50, 50, 50, 50, 50, 50, 50},
        {10, 10, 20, 30, 30, 20, 10, 10},
        { 5,  5, 10, 27, 27, 10,  5,  5},
        { 0,  0,  0, 25, 25,  0,  0,  0},
        { 5, -5,-10,  0,  0,-10, -5,  5},
        { 5, 10, 10,-25,-25, 10, 10,  5},
        { 0,  0,  0,  0,  0,  0,  0,  0}
    };
    int _nTable[8][8] = {
        {-50,-40,-30,-30,-30,-30,-40,-50},
        {-40,-20,  0,  0,  0,  0,-20,-40},
        {-30,  0, 10, 15, 15, 10,  0,-30},
        {-30,  5, 15, 20, 20, 15,  5,-30},
        {-30,  0, 15, 20, 20, 15,  0,-30},
        {-30,  5, 10, 15, 15, 10,  5,-30},
        {-40,-20,  0,  5,  5,  0,-20,-40},
        {-50,-40,-20,-30,-30,-20,-40,-50},
    };
    int _bTable[8][8] = {
        {-20,-10,-10,-10,-10,-10,-10,-20},
        {-10,  0,  0,  0,  0,  0,  0,-10},
        {-10,  0,  5, 10, 10,  5,  0,-10},
        {-10,  5,  5, 10, 10,  5,  5,-10},
        {-10,  0, 10, 10, 10, 10,  0,-10},
        {-10, 10, 10, 10, 10, 10, 10,-10},
        {-10,  5,  0,  0,  0,  0,  5,-10},
        {-20,-10,-40,-10,-10,-40,-10,-20},
    };

    //value of each peice indexed w/ ChessPiece
    int _value[7] = {0,10,30,30,50,90,40000};
    
    //0 x of black king
    //1 y of black king
    //3 y of white king
    int _kingpos[4] = {4,0,4,7};

    //0b1100 Black Castle Possible (Both Ways)
    //0b1010 Queenside Castle Possible (Both Colors)
    char        _castleRights = 0b1111;
    char        _enpassant = -1;
};

