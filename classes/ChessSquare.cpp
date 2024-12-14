#include "ChessSquare.h"
#include "Chess.h"
#include "Bit.h"

void ChessSquare::initHolder(const ImVec2 &position, const char *spriteName, const int column, const int row)
{
    _column = column;
    _row = row;
    int odd = (column + row) % 2;
    ImVec4 color = odd ? ImVec4(1, 1, 1, 1.0) : ImVec4(0.96, 0.83, 0.99, 1.0);
    //ImVec4 color = odd ? ImVec4(0.31, 0.36, 0.49, 1.0) : ImVec4(0.25, 0.21, 0.43, 1.0);
    BitHolder::initHolder(position, color, spriteName);
    setSize(pieceSize, pieceSize);
}

bool ChessSquare::canDropBitAtPoint(Bit *newbit, const ImVec2 &point)
{
    //see next
    //return true;
    
    if (bit() == nullptr)
    {
        return true;
    }
    //
    // xor the gametags to see if we have opposing colors
    //
    if ((bit()->gameTag() ^ newbit->gameTag()) >= 128)
    {
        return true;
    }
    return false;
}

bool ChessSquare::dropBitAtPoint(Bit *newbit, const ImVec2 &point)
{
    //changed my mind; I mean it's still very odd but idk
    /*
    setBit(newbit);
    newbit->setParent(this);
    newbit->moveTo(getPosition());
    return true;
    */
    if (bit() == nullptr)
    {
        setBit(newbit);
        newbit->setParent(this);
        newbit->moveTo(getPosition());
        return true;
    }
    // we're taking a piece!
    if ((bit()->gameTag() ^ newbit->gameTag()) >= 128)
    {
        setBit(newbit);
        newbit->setParent(this);
        newbit->moveTo(getPosition());
        return true;
    }
    return false;
}

void ChessSquare::setMoveHighlighted(bool highlighted)
{
    int odd = (_column + _row) % 2;
    _color = odd ? ImVec4(1, 1, 1, 1.0) : ImVec4(0.96, 0.83, 0.99, 1.0);
    if (highlighted)
        _color = Lerp(_color, ImVec4(0.21, 0.89, 0.60, 1.0), 0.30);
}
