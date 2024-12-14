#include "Chess.h"
//#include "magicbitboards.h"
//#include "Logger.h"

const int AI_PLAYER = 1;
const int HUMAN_PLAYER = -1;

Chess::Chess()
{
}

Chess::~Chess()
{
}

//
// make a chess piece for the player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    _gameOptions.currentTurnNo = 0;
    //
    // we want white to be at the bottom of the screen so we need to reverse the board
    //
    char piece[2];
    piece[1] = 0;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            ImVec2 position((float)(pieceSize * x + pieceSize), (float)(pieceSize * (_gameOptions.rowY - y) + pieceSize));
            _grid[y][x].initHolder(position, "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            piece[0] = bitToPieceNotation(y,x);
            _grid[y][x].setNotation(piece);
        }
    }
    //debug
    //FENtoPos("8/8/8/8/3N4/8/8/8 w KQkq - 0 1");
    //FENtoPos("8/4P3/8/2pPp3/8/3p1p2/4P3/8 w KQkq c 0 1");
    //FENtoPos("8/8/8/8/3K4/8/8/8 w KQkq - 0 1");
    //FENtoPos("8/3R4/8/8/3Q4/8/1B6/8 w KQkq - 0 1");
    //FENtoPos("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    //FENtoPos("2rq1rk1/1ppb1pp1/p1n4p/2bN1B2/P7/3Q2B1/1PP2PPP/R4RK1 w - - 0 1");
    FENtoPos("r3r3/ppB3pp/8/2Q4k/1P5n/8/P1P1q1PP/R5K1 b - - 0 25");
    //FENtoPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    return;
}

void Chess::QuickPlacePeice(int x, int y, ChessPiece type, bool color){
    _grid[y][x].destroyBit();
    _gridNum[y][x] = 0;
    if (!type)
        return;
    Bit* bit = PieceForPlayer(color, type);
    bit->setPosition(_grid[y][x].getPosition());
    bit->setParent(&_grid[y][x]);
    //raise bit 0b10000000 to be a little more descrete to code i initially commented out
    bit->setGameTag(type|(color<<7));
    _grid[y][x].setBit(bit);
    _gridNum[y][x] = bit->gameTag();
    if (type == King){
        _kingpos[color*2] = x;
        _kingpos[color*2+1] = y;
    }
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFromToPosition(int sX, int sY, int dX, int dY, int c)
{
    if (!_gridNum[sY][sX])
        return false;
    //general collision check
    int capture = 0;
    if (_gridNum[dY][dX]){
        if ((_gridNum[dY][dX] > 127) == (c > 0))
            return false;
        capture = 1;
    }
    int type = _gridNum[sY][sX] & 127;
    //illegal moves that put king in check... check
    int tempType = _gridNum[dY][dX];
    //temp modify board to match attempted move
    _gridNum[dY][dX] = _gridNum[sY][sX];
    _gridNum[sY][sX] = NoPiece;
    //temp move king if selected piece is a king
    if (type == King){
        _kingpos[c*2] = dX;
        _kingpos[c*2+1] = dY;
    }
    //do check for king attacks
    bool moveIllegal = getPosAttacked(_kingpos[c*2],_kingpos[c*2+1],c);
    //reset temp changes
    _gridNum[sY][sX] = _gridNum[dY][dX];
    _gridNum[dY][dX] = tempType;
    
    if (type == King){
        _kingpos[c*2] = sX;
        _kingpos[c*2+1] = sY;
    }
    if (moveIllegal)
        return false;

    int sign = (!c) - c;
    //Pawn
    if (type == Pawn)
        //single / double
        return (sX == dX && !capture && ((dY == 3+c && sY+sign*2 == dY && !_gridNum[sY+sign][dX]) || (sY+sign == dY))) || 
        //captures
            ((capture || (dX == _enpassant && sY == 3+c)) && abs(sX-dX) == 1 && (sY+sign == dY));
    //Knight
    else if (type == Knight)
        return (abs(dY-sY) == 2 && abs(dX-sX) == 1) || (abs(dY-sY) == 1 && abs(dX-sX) == 2);
    //King
    else if (type == King && (abs(dY-sY) < 2 && abs(dX-sX) < 2) && !getPosAttacked(dX,dY,c))
        return true;
    //Queen, Rook, Bishop
    else if (type != King)
        for (int x = -1; x < 2; x ++){
            for (int y = -1; y < 2; y ++){
                if ((!y && !x) || (type == Bishop && !(x && y)) || (type == Rook && x && y))
                    continue;
                for (int i = 1; sY+y*i < 8 && sY+y*i > -1 && sX+x*i < 8 && sX+x*i > -1; i++){
                    if (sY+y*i == dY && sX+x*i == dX)
                        return true;
                    if (_gridNum[sY+y*i][sX+x*i])
                        break;
                }
            }
        }
    //Castling
    if (type != King || sX != 4 || (sY != 0 && sY != 7) || getPosAttacked(sX,sY,c))
        return false;
    if ((dX == sX + 2 && (_castleRights & 1<<(2*c)) && !_gridNum[sY][sX+1] && !getPosAttacked(sX+1,sY,c)) ||
        (dX == sX - 2 && (_castleRights & 2<<(2*c)) && !_gridNum[sY][sX-1] && !getPosAttacked(sX-1,sY,c)))
        return dY == sY && !capture;
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
     //highlight based on peice type
    int type = bit.gameTag() & 127;
    int c = bit.getOwner()->playerNumber();
    int sX = ((ChessSquare*)&src)->getColumn();
    int sY = ((ChessSquare*)&src)->getRow();

    if (bit.getOwner() != getCurrentPlayer())
        return false;

    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++){
            ((ChessSquare*)&_grid[y][x])->setMoveHighlighted(canBitMoveFromToPosition(sX,sY,x,y,c));
        }

    return true;
}

//still no bitboards? ;:(
bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    //find info on current peice
    //get "playing" color
    int c = bit.getOwner()->playerNumber();
    int type = bit.gameTag() & 127;
    //typecast that's in no way dangerous trust me
    int sX = ((ChessSquare*)&src)->getColumn();
    int sY = ((ChessSquare*)&src)->getRow();
    int dX = ((ChessSquare*)&dst)->getColumn();
    int dY = ((ChessSquare*)&dst)->getRow();

    return canBitMoveFromToPosition(sX,sY,dX,dY,c);
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) 
{
    bool c = getCurrentTurnNo()%2;
    int type = bit.gameTag();
    int sX = ((ChessSquare*)&src)->getColumn();
    int sY = ((ChessSquare*)&src)->getRow();
    int dX = ((ChessSquare*)&dst)->getColumn();
    int dY = ((ChessSquare*)&dst)->getRow();
    _gridNum[sY][sX] = 0;
    _gridNum[dY][dX] = type;
    type &= 127;
    for (int x = 0; x < 8; x++)
        for (int y = 0; y < 8; y++){
            ((ChessSquare*)&_grid[y][x])->setMoveHighlighted(0);
        }
    if ((dX == 0 || dX == 7) && (dY == 0 || dY == 7) && _castleRights)
        _castleRights &= (0b0001 << (2*(dY == 7) + (!dX))) ^ 0b1111;
    switch (type){
        case King:
            _castleRights &= 0b1100 >> 2*c;
            for (int i = -1; i < 2; i += 2)
                if (dX == 4+i*2 && ((_castleRights & 0b0001) << (2*c + !(i+1)))){
                    QuickPlacePeice(4+i,sY,Rook,c);
                    QuickPlacePeice(7*(i==1),sY,NoPiece,c);
                }
            _enpassant = -1;
            _kingpos[c*2] = dX;
            _kingpos[c*2+1] = dY;
        break;
        case Rook:
            _castleRights &= (0b0001 << (2*c + (sX==0))) ^ 0b1111;
            _enpassant = -1;
        break;
        case Pawn:
            if (dY == (!c) * 7)
                QuickPlacePeice(dX,dY,Queen,c);
            for (int i = -1; i < 2; i += 2)
                if (sX+i == _enpassant && sY == 4 - c)
                    QuickPlacePeice(sX+i,sY,NoPiece,c);
            if (sY == (1 + 5*c) && &dst == &_grid[(3 + c)][sX])
                _enpassant = sX;
            else
                _enpassant = -1;
        break;
        default:
            _enpassant = -1;
        break;
    }
    endTurn();
}

void Chess::bitMovedFromToPosition(int sX, int sY, int dX, int dY, int info) 
{
    bool c = info > 127;
    int type = info & 127;
    _gridNum[sY][sX] = 0;
    _gridNum[dY][dX] = type;
    if ((dX == 0 || dX == 7) && (dY == 0 || dY == 7) && _castleRights)
        _castleRights &= (0b0001 << (2*(dY == 7) + (!dX))) ^ 0b1111;
    switch (type){
        case King:
            for (int i = -1; i < 2; i += 2)
                if (dX == 4+i*2 && ((_castleRights & 0b0001) << (2*c + !(i+1)))){
                    _gridNum[sY][4+i] = Rook + (c * 128);
                    _gridNum[sY][7*(i==1)] = 0;
                }
            _castleRights &= 0b1100 >> 2*c;
            _enpassant = -1;
            _kingpos[c*2] = dX;
            _kingpos[c*2+1] = dY;
        break;
        case Rook:
            _castleRights &= (0b0001 << (2*c + (sX==0))) ^ 0b1111;
            _enpassant = -1;
        break;
        case Pawn:
            if (dY == (!c) * 7)
                QuickPlacePeice(dX,dY,Queen,c);
            for (int i = -1; i < 2; i += 2)
                if (sX+i == _enpassant && sY == 4 - c)
                    _gridNum[sY][sX+i] = 0;
            if (sY == (1 + 5*c) && dY == (3 + c) && dX == sX)
                _enpassant = sX;
            else
                _enpassant = -1;
        break;
        default:
            _enpassant = -1;
        break;
    }
}

int Chess::getPosAttacked(int sX, int sY, int c){
    //knight
    for (int x = -2; x < 3; x ++){
        if (sX+x < 0 || sX+x > 7 || x == 0)
            continue;
        for (int i = -1; i < 2; i += 2){
            int y = (3-abs(x))*i;
            if (sY+y < 0 || sY+y > 7)
                continue;
            if ((_gridNum[sY+y][sX+x] & 127) == Knight && ((_gridNum[sY+y][sX+x] > 127) != (c > 0)))
                return Knight;
        }
    }
    //king, queen, bishop, rook
    for (int x = -1; x < 2; x ++){
        for (int y = -1; y < 2; y ++){
            if ((!y && !x) || sY+y > 7 || sY+y < 0 || sX+x > 7 || sX+x < 0)
                continue;
            for (int i = 1; sY+y*i < 8 && sY+y*i > -1 && sX+x*i < 8 && sX+x*i > -1; i++){
                int type = _gridNum[sY+y*i][sX+x*i] & 127;
                int diffCol = ((_gridNum[sY+y*i][sX+x*i] > 127) != (c > 0));
                if (diffCol && (type == Queen || (type == King && (i < 2)) || (type == Bishop && x && y) || (type == Rook && !(x && y))))
                    return type;
                if (type)
                    break;
            }
        }
    }
    //pawn
    for (int i = -1; i < 2; i += 2){
        int temp = _gridNum[sY-c+!c][sX+i];
        if (((temp & 127) == Pawn) && (temp > 127) != (c > 0))
            return Pawn;
    }
    return NoPiece;
}

void Chess::FENtoPos(std::string fen){
    int i = 0;
    int x = 0;
    int y = 7;
    //actually clear the board
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            _gridNum[y][x] = 0;
    while (fen[i] != ' ') {
        switch(fen[i]){
            case 'P':
                QuickPlacePeice(x,y,Pawn,0);
            break;
            case 'B':
                QuickPlacePeice(x,y,Bishop,0);
            break;
            case 'N':
                QuickPlacePeice(x,y,Knight,0);
            break;
            case 'R':
                QuickPlacePeice(x,y,Rook,0);
            break;
            case 'Q':
                QuickPlacePeice(x,y,Queen,0);
            break;
            case 'K':
                QuickPlacePeice(x,y,King,0);
            break;
            case 'p':
                QuickPlacePeice(x,y,Pawn,1);
            break;
            case 'b':
                QuickPlacePeice(x,y,Bishop,1);
            break;
            case 'n':
                QuickPlacePeice(x,y,Knight,1);
            break;
            case 'r':
                QuickPlacePeice(x,y,Rook,1);
            break;
            case 'q':
                QuickPlacePeice(x,y,Queen,1);
            break;
            case 'k':
                QuickPlacePeice(x,y,King,1);
            break;
            case '/':
                y --;
                x = -1;
            break;
            default:
                x += (fen[i] - '1'); //subtract 1 from x bc its added again later
        }
        i++;
        x++;
        if (i >= fen.length())
            return;
    }
    i ++;
    if (i >= fen.length())
        return;
    if (fen[i] == 'b')
        endTurn();
    i += 2;
    if (i >= fen.length())
        return;
    _castleRights = 0;
    while (fen[i] != ' ') {
        switch(fen[i]){
            case 'q':
                _castleRights |= 0b1000;
            break;
            case 'k':
                _castleRights |= 0b0100;
            break;
            case 'Q':
                _castleRights |= 0b0010;
            break;
            case 'K':
                _castleRights |= 0b0001;
            break;
            default:
            break;
        }
        i ++;
        if (i >= fen.length())
            return;
    }
    i ++;
    if (i < fen.length())
        _enpassant = fen[i] - 'a';
    //last two feilds are ignored (i dont have win condition checks and i don't super care what turn it is)
}

void Chess::gridToBitboard(){
    for (int i = 0; i < 8; i += 1){
        _bitboards[i] = 0;
    }
    for (int x = 0; x < 8; x ++)
    for (int y = 0; y < 8; y ++){
        if (!_grid[y][x].empty()){
            Bit* temp = _grid[y][x].bit();
            int type = temp->gameTag();
            //set white pieces bitboard
            if (type & 128)
                _bitboards[NoPiece] |= 1ULL<<(y*8+x);
            type = type & 127;
            //set bitboards for each specific piece type
            if (type)
                _bitboards[type] |= 1ULL<<(y*8+x);
        }
    }
}

std::list<Move_t> Chess::generateMoveList(int c)
{
    std::list<Move_t> out = {};
    //find piece to move
    for (int sX = 0; sX < 8; sX ++)
        for (int sY = 0; sY < 8; sY ++){
            //check if legal move
            if (!_gridNum[sY][sX] || (_gridNum[sY][sX] > 127) != (c > 0))
                continue;
            //try to move piece
            for (int dX = 0; dX < 8; dX ++)
                for (int dY = 0; dY < 8; dY ++){
                    if (canBitMoveFromToPosition(sX,sY,dX,dY,c)){
                        Move_t newMove;
                        newMove.sX = sX;
                        newMove.sY = sY;
                        newMove.dX = dX;
                        newMove.dY = dY;
                        out.push_back(newMove);
                    }
                }
        }
    return out;
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}

Player* Chess::checkForWinner()
{
    int c = getCurrentPlayer()->playerNumber();
    std::list<Move_t> temp = generateMoveList(c);
    if (temp.empty() && getPosAttacked(_kingpos[(c)*2],_kingpos[(c)*2+1],c) != NoPiece)
        return getPlayerAt(!c);
    return nullptr;
}

bool Chess::checkForDraw()
{
    std::list<Move_t> temp = generateMoveList(getCurrentPlayer()->playerNumber());
    if (temp.empty())
        return true;
    return false;
}



//
// add a helper to Square so it returns out FEN chess notation in the form p for white pawn, K for black king, etc.
// this version is used from the top level board to record moves
//
const char Chess::bitToPieceNotation(int row, int column) const {
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }

    const char* wpieces = { "?PNBRQK" };
    const char* bpieces = { "?pnbrqk" };
    unsigned char notation = '0';
    Bit* bit = _grid[row][column].bit();
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag() & 127];
    } else {
        notation = '0';
    }
    return notation;
}

//
// state strings
//
std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString()
{
    std::string s;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            s += bitToPieceNotation(y, x);
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            int index = y * _gameOptions.rowX + x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit(PieceForPlayer(playerNumber - 1, Pawn));
            } else {
                _grid[y][x].destroyBit();
            }
        }
    }
}

int32_t Chess::evaluateBoard(){
    int32_t out = 0;
    for (int y = 0; y < 8; y++) 
        for (int x = 0; x < 8; x++) {
            int32_t sign = 1;
            if (_gridNum[y][x] > 127)
                sign = -1;
            int type = _gridNum[y][x] & 127;
            //reduces the value of currently contested peices
            int attackerType = getPosAttacked(x,y,sign == 1) & 127;
            if (attackerType && type != NoPiece)
                out -= sign * ((_value[type] - _value[attackerType])/3);
            int yForInd = (sign == -1) ? (7-y) : y;
            if (type == Pawn)
                out += sign * _pTable[yForInd][x];
            else if (type == Knight)
                out += sign * _nTable[yForInd][x];
            else if (type == Bishop)
                out += sign * _bTable[yForInd][x];
            out += sign * _value[type];
        }
    return out;
}

//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
    int whoAmI = getAIPlayer();
    if (getCurrentPlayer() != getPlayerAt(whoAmI) || !gameHasAI())
        return;
    int32_t score = -_value[King];
    std::list<Move_t> possibleMoves = generateMoveList(whoAmI);
    Move_t bestMove;
    for (std::list<Move_t>::iterator it = possibleMoves.begin(); it != possibleMoves.end(); ++it){
        //temp change board
        //int tempEnpassant = _enpassant;
        //int tempCastle = _castleRights;
        int temp = _gridNum[it->dY][it->dX];
        int info = _gridNum[it->sY][it->sX];
        _gridNum[it->sY][it->sX] = 0;
        _gridNum[it->dY][it->dX] = info;
        //bitMovedFromToPosition(it->sX,it->sY,it->dX,it->dY,info);
        //score pos
        int32_t tempScore = -negamax(0, -_value[King], _value[King],(!whoAmI) - (whoAmI));
        if (score <= tempScore){
            bestMove.sY = it->sY;
            bestMove.sX = it->sX;
            bestMove.dY = it->dY;
            bestMove.dX = it->dX;
            score = tempScore;
        }
        //undo temp changes
        _gridNum[it->sY][it->sX] = info;
        _gridNum[it->dY][it->dX] = temp;
        //_enpassant = tempEnpassant;
        //if (_enpassant != -1)
        //    _gridNum[3 + whoAmI][_enpassant] = Pawn;
        //if (_castleRights != tempCastle && (info & 127) == King && abs(it->sX-it->dX) == 2){
        //    _gridNum[7*!whoAmI][(it->dX == 6)*7] = _gridNum[7*!whoAmI][(it->dX == 6)*2+3];
        //    _gridNum[7*!whoAmI][(it->dX == 6)*2+3] = 0;
        //}
        //_castleRights = tempCastle;
        
    }
    int pinfo = _gridNum[bestMove.sY][bestMove.sX];
    int type = (pinfo & 127);
    QuickPlacePeice(bestMove.dX,bestMove.dY,*((ChessPiece*)&type),(pinfo > 127));
    QuickPlacePeice(bestMove.sX,bestMove.sY,NoPiece,0);
    bitMovedFromTo(*_grid[bestMove.dY][bestMove.dX].bit(),_grid[bestMove.sY][bestMove.sX],_grid[bestMove.dY][bestMove.dX]);
}

int32_t Chess::negamax(int depth, int32_t alpha, int32_t beta, int playerColor){
    int32_t score = evaluateBoard() * playerColor;
    int c = (playerColor == -1);
    //check if at terminal state
    std::list<Move_t> possibleMoves = generateMoveList(c);
    //if (possibleMoves.empty() && getPosAttacked(_kingpos[(!c)*2],_kingpos[(!c)*2+1],c))
    //    return _value[King] * playerColor;
    //possibleMoves = generateMoveList(c);
    //if (possibleMoves.empty() && getPosAttacked(_kingpos[c*2],_kingpos[c*2+1],c))
    //    return -_value[King] * playerColor;
    if (possibleMoves.empty() || depth > MAX_DEPTH)
        return score;
    //worst best move is just a guarenteed loss
    score = -_value[King];
    for (std::list<Move_t>::iterator it = possibleMoves.begin(); it != possibleMoves.end(); ++it){
        //int tempEnpassant = _enpassant;
        //int tempCastle = _castleRights;
        int temp = _gridNum[it->dY][it->dX];
        int info = _gridNum[it->sY][it->sX];
        _gridNum[it->sY][it->sX] = 0;
        _gridNum[it->dY][it->dX] = info;
        //bitMovedFromToPosition(it->sX,it->sY,it->dX,it->dY,info);
        //score pos
        score = std::max(score, -negamax(depth+1, beta, alpha,-playerColor));
        //undo temp changes
        _gridNum[it->sY][it->sX] = info;
        _gridNum[it->dY][it->dX] = temp;
        //_enpassant = tempEnpassant;
        //if (_enpassant != -1)
        //    _gridNum[3 + c][_enpassant] = Pawn;
        //if (_castleRights != tempCastle && (info & 127) == King && abs(it->sX-it->dX) == 2){
        //    _gridNum[7*!c][(it->dX == 6)*7] = _gridNum[7*!c][(it->dX == 6)*2+3];
        //    _gridNum[7*!c][(it->dX == 6)*2+3] = 0;
        //}
        //_castleRights = tempCastle;
        alpha = std::max(score,alpha);
        if (beta <= alpha)
            break;
    }
    return score;
}