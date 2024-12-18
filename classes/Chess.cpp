#include "Chess.h"

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

void Chess::FENtoBoard(std::string FEN_string)
{
    std::istringstream fenStream(FEN_string); //split out the board position of the FEN string
    std::string board_representation;         //other parts (ie. castling) can be handled later on
    fenStream >> board_representation;

    int row = 0;
    int col = 0;

    for(char c : board_representation) //main board set up
    {
        if(c == '/') //transition to next row of the board
        {
            row++;
            col = 0;
        }

        else if (isdigit(c)) //if c is a digit, then there are that many empty spaces in a row on the board
        {
            col += c - '0';
        }

        else
        {
            Bit* piece = nullptr; //create piece for player
            bool isWhite = isupper(c); //determine whether the piece is white or black
            int GameTag; 
            switch(tolower(c)) //determine the appropriate biece and set it
            {
                case 'p': 
                    piece = PieceForPlayer(isWhite ? 0 : 1, Pawn); 
                    GameTag = Pawn;
                    break;
                case 'r': 
                    piece = PieceForPlayer(isWhite ? 0 : 1, Rook); 
                    GameTag = Rook;
                    piece->movedFromStart(); //set to true to set up castling later
                    break;
                case 'n': 
                    piece = PieceForPlayer(isWhite ? 0 : 1, Knight);
                    GameTag = Knight; 
                    break;
                case 'b': 
                    piece = PieceForPlayer(isWhite? 0 : 1, Bishop);
                    GameTag = Bishop; 
                    break;
                case 'q': 
                    piece = PieceForPlayer(isWhite ? 0 : 1, Queen);
                    GameTag = Queen; 
                    break;
                case 'k': 
                    piece = PieceForPlayer(isWhite ? 0 : 1, King); 
                    GameTag = King;
                    piece->movedFromStart();
                    break;
            }

            if(piece != nullptr)
            {
                int boardRow = 7 - row; //flips the board position so black is on top, white on bottom
                piece->setPosition(_grid[boardRow][col].getPosition());
                piece->setParent(&_grid[boardRow][col]);
                piece->setGameTag(GameTag);
                _grid[boardRow][col].setBit(piece);
            }

            col++;

        }

    }

    //process the remaining parts of the string if they exist
    std::string active_player; //player to go next
    std::string castle_availability; //which castles are possible
    std::string en_passant; //where en passant is available
    std::string half_move_clock; //number of moves made since last pawn advance or piece capture
    std::string full_move_number; //number of turns completed in the game

    //split the remaining fen string into its seperate parts
    fenStream >> active_player >> castle_availability >> en_passant >> half_move_clock >> full_move_number;

    //set active player
    if(active_player == "w" || active_player == "b" )
    {
        if(active_player == "w") //white turn 
        {
            //Turns not implemented yet
        }
        
        else //black turn
        {

        }

        //castle availability
        for(char C : castle_availability)
        {
            switch(C)
            {
                case 'K': //white king side 
                    _grid[0][4].bit()->notMoved(); 
                    _grid[0][7].bit()->notMoved(); 
                    break;
                
                case 'Q': //white queen side
                    _grid[0][4].bit()->notMoved();
                    _grid[0][0].bit()->notMoved(); 
                    break;

                case 'k': //black king side
                    _grid[7][4].bit()->notMoved();
                    _grid[7][7].bit()->notMoved();
                    break;
                
                case 'q': //black queen side
                    _grid[7][4].bit()->notMoved();
                    _grid[7][0].bit()->notMoved();
                    break;

                case '-':
                    break;
            }
        }

        //en passant targets
        int column_index = -1;
        int row_index = -1;
        for(char c : en_passant) //parse en passant target to get row and column
        {
            if(c == '-')
            {
                continue;
            }

            if(isalpha(c))
            {
                column_index = c - 'a';
            }

            else if (isdigit(c))
            {
                row_index = c - '1';
            }
        }

        //set passantable pawn
        if(row_index == 3) //white pawn
        {
            set_passant(_grid[4][column_index].bit());
        }

        if(row_index == 6) //black pawn
        {
            set_passant(_grid[5][column_index].bit());
        }

        //half move clock
        set_half_moves( stoi (half_move_clock) );

        //full move number
        setCurrentTurnNo( stoi(full_move_number) + 1 ); //sets current turn number to full move number + 1 
                                                        //full moves is how many moves have been made
    }                                                   //so the current turn should be one more than that
    
    return;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
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

    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"); 
    
    startGame();
}

//
// about the only thing we need to actually fill out for tic-tac-toe
//
bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    return true;
}


bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    //initial position of piece
    int starting_column = src.getPosition().x / 64; //divide by 64 to get index from 1-8
    int starting_row = 10 - (src.getPosition().y /64); //divide by 64, then subtract the result from 10 to get 1-8 index

    //destination for piece
    int destination_column = dst.getPosition().x / 64;
    int  destination_row = 10 - (dst.getPosition().y /64);


    if(starting_column == destination_column && starting_row == destination_row) //can set pieces back down in their original spot
    {
        return true;
    }
    if(bit.gameTag() == Pawn)
    {
        //white pawn can move forward one 
        if (destination_row == starting_row + 1 && destination_column == starting_column)
        {
            if(!dst.empty() || bit.getOwner()->playerNumber() == 1) //cant move to a non-empty spot
            {
                return false;
            }

            if(destination_row == 8) //queen promotion
            {
                if(ImGui::IsMouseReleased(0))
                {
                    Bit *promoted_queen = PieceForPlayer(bit.getOwner()->playerNumber(), Queen); //create a queen
                    bit.~Bit(); //destroy the pawn

                    promoted_queen->setPosition(dst.getPosition());
                    promoted_queen->setGameTag(Queen);
                    dst.setBit(promoted_queen);
                }

            }

            if(ImGui::IsMouseReleased(0))
            {
                clear_passant();
            }
            return true;
        }

        //white pawn can move forward 2 in the starting row
        if(starting_row == 2 && bit.getOwner()->playerNumber() == 0)
        {
            if(destination_row == starting_row + 2 && destination_column == starting_column)
            {
                if(!dst.empty() ||!_grid[destination_row - 2][destination_column-1].empty()) //cant move to an occupied spot or jump a piece to move forward 2
                {
                    return false;
                }

                if(ImGui::IsMouseReleased(0))
                {
                    set_passant(&bit); //pawn is now passantable
                }
                return true;
            }
        }

        //black pawn can move forward one
        if (destination_row == starting_row - 1 && destination_column == starting_column)
        {
            if(!dst.empty() || bit.getOwner()->playerNumber() == 0) //cant move to a non-empty spot
            {
                return false;
            }

            if(destination_row == 1) //queen promotion
            {
                if(ImGui::IsMouseReleased(0))
                {
                    Bit *promoted_queen = PieceForPlayer(bit.getOwner()->playerNumber(), Queen); //create a queen
                    bit.~Bit(); //destroy the pawn

                    promoted_queen->setPosition(dst.getPosition());
                    promoted_queen->setGameTag(Queen);
                    dst.setBit(promoted_queen);
                }

            }

            if(ImGui::IsMouseReleased(0))
            {
                clear_passant();
            }
            return true;
        }

        //black pawn can move forward 2 from starting row
        if(starting_row == 7 && bit.getOwner()->playerNumber() == 1)
        {
            if(destination_row == starting_row - 2 && destination_column == starting_column)
            {
                if(!dst.empty() || !_grid[destination_row][destination_column - 1].empty())
                {
                    return false;
                }

                if(ImGui::IsMouseReleased(0))
                {
                    set_passant(&bit); //pawn is now passantable
                }
                return true;
            }
        }

        //pawn can capture
        if(starting_column + 1 == destination_column || starting_column - 1 == destination_column) // capturing can happen either left of right
        {
            //white pawn
            if(starting_row + 1 == destination_row) //capture has to happen one row above
            {

                if( (dst.bit() != nullptr && dst.bit()->getOwner()->playerNumber() == 1 ) ) //if pawn is white and piece to capture is black
                {
                    if(ImGui::IsMouseReleased(0))
                    {
                        dst.destroyBit();
                        clear_passant(); 

                        if(destination_row == 8) // queen promotion
                        {
                            Bit *promoted_queen = PieceForPlayer(bit.getOwner()->playerNumber(), Queen); //create a queen
                            bit.~Bit(); //destroy the pawn

                            promoted_queen->setPosition(dst.getPosition());
                            promoted_queen->setGameTag(Queen);
                            dst.setBit(promoted_queen);
                        }
                        
                    }
                    return true;
                }

                if(_grid[starting_row - 1][destination_column - 1].bit() == passant_pawn() && bit.getOwner()->playerNumber() == 0) // en passant
                {
                    if(ImGui::IsMouseReleased(0))
                    {
                        _grid[starting_row - 1][destination_column - 1].destroyBit();
                        clear_passant();
                    }
                    return true;
                }
            }

            //black pawn
            if(starting_row - 1 == destination_row) //capture has to happen one row below
            {
                if(dst.bit() != nullptr && dst.bit()->getOwner()->playerNumber() == 0)
                {
                    if(ImGui::IsMouseReleased(0))
                    {
                        dst.destroyBit();
                        clear_passant();

                        if(destination_row == 1) // queen promotion
                        {
                            Bit *promoted_queen = PieceForPlayer(bit.getOwner()->playerNumber(), Queen); //create a queen
                            bit.~Bit(); //destroy the pawn

                            promoted_queen->setPosition(dst.getPosition());
                            promoted_queen->setGameTag(Queen);
                            dst.setBit(promoted_queen);
                        }
                    }
                    return true;
                }

                if(_grid[starting_row - 1][destination_column - 1].bit() == passant_pawn() && bit.getOwner()->playerNumber() == 1) // en passaant
                {
                    if(ImGui::IsMouseReleased(0))
                    {
                        _grid[starting_row - 1][destination_column - 1].destroyBit();
                        clear_passant();
                    }
                    return true;
                }
            }
            return false;
        }
    }

    if(bit.gameTag() == Rook)
    {
        if(starting_column == destination_column || starting_row == destination_row) //rooks can move vertically or horizontally
        {
            if(starting_column == destination_column) //vertical movement
            {
                if(starting_row < destination_row) //upwards movement
                {
                    int row_index_of_blocking_piece = 9; //set to an out of bounds value initially

                    for(int row_index = starting_row; row_index < destination_row; row_index++) //look for a piece in the path to the destination
                    {
                        if(_grid[row_index - 1][destination_column-1].bit()!= nullptr) //checks if a piece is present
                        {
                            if(starting_row == row_index) //ensures that the rook itself isnt considered a piece thats in the way
                            {
                                continue;
                            }
                            row_index_of_blocking_piece = row_index;
                        }
                    }
                    if(destination_row > row_index_of_blocking_piece) //if the destination is above the blocking piece, dont let it move past
                    {
                        return false;
                    }
                }

                if(starting_row > destination_row) //downwards movement
                {
                    int row_index_of_blocking_piece = -1;

                    for(int row_index = starting_row; row_index > destination_row; row_index-- )
                    {
                        if(_grid[row_index - 1][destination_column-1].bit()!= nullptr) //checks if a piece is present
                        {
                            if(starting_row == row_index) //ensures that the rook itself isnt considered a piece thats in the way
                            {
                                continue;
                            }
                            row_index_of_blocking_piece = row_index;
                        }
                    }
                    if(destination_row < row_index_of_blocking_piece)
                    {
                        return false;
                    }
                }
            }

            

            if(starting_row == destination_row) //horizontal movement
            {
                if(starting_column < destination_column) //moving right 
                {
                    int column_index_of_blocking_piece = 9;

                    for(int col_index = starting_column; col_index < destination_column; col_index++)
                    {
                        if(_grid[destination_row-1][col_index-1].bit()!=nullptr) //check for present piece of same color
                        {
                            if(starting_column == col_index) //esnures taht the rook itself isnt considered in the way
                            {
                                continue;
                            }
                            column_index_of_blocking_piece = col_index;
                        }
                    }
                    if(destination_column > column_index_of_blocking_piece)
                    {
                        return false;
                    }

                }

                if(starting_column > destination_column) //moving left
                {
                    int column_index_of_blocking_piece = 0;

                    for(int col_index = starting_column; col_index > destination_column; col_index--)
                    {
                        if(_grid[destination_row-1][col_index-1].bit()!=nullptr) //check for present piece of same color
                        {
                            if(starting_column == col_index) //esnures taht the rook itself isnt considered in the way
                            {
                                continue;
                            }
                            column_index_of_blocking_piece = col_index;
                        }
                    }
                    if(destination_column < column_index_of_blocking_piece)
                    {
                        return false;
                    }

                }
            }
        }
    if(starting_row == destination_row || starting_column == destination_column)
    {
        if(ImGui::IsMouseReleased(0))
        {
            if(dst.bit() != nullptr && dst.bit()->getOwner()->playerNumber() != bit.getOwner()->playerNumber())
            {
                dst.destroyBit();
            }
            bit.movedFromStart();

            clear_passant();
        }
        return true;
    }
    return false;
    }

    if(bit.gameTag() == Bishop)
    {
        int row_difference = abs(destination_row - starting_row);
        int column_difference = abs(destination_column - starting_column);

        if(row_difference == column_difference) //if the move is diagonal
        {
            int row_step = (destination_row > starting_row) ? 1 : -1; //direction of row movement
            int col_step = (destination_column > starting_column) ? 1 : -1; //direction of column movement

            int row_index_of_blocking_piece = 9; //set to an out of bounds value
            int column_index_of_blocking_piece = 9; //set to an out of bounds value 

            int current_row = starting_row + row_step;
            int current_column = starting_column + col_step;

            while(current_row != destination_row && current_column != destination_column) //check all squares along diagonal path
            {
                if(_grid[current_row - 1][current_column -1].bit() != nullptr) //check if a piece is present
                {
                    //hold index of blocking piece
                    row_index_of_blocking_piece = current_row;
                    column_index_of_blocking_piece = current_column;
                    break; 
                }

                current_column += col_step;
                current_row += row_step;
            }

            if (row_index_of_blocking_piece != 9 && column_index_of_blocking_piece != 9 && //only checks for blocking pieces if one is found
            ((destination_row > row_index_of_blocking_piece && row_step > 0) ||            //if a blocking piece is found, this ensures that 
             (destination_row < row_index_of_blocking_piece && row_step < 0) ||            //the bishop cannot jump pieces along any diagonal
             (destination_column > column_index_of_blocking_piece && col_step > 0) ||
             (destination_column < column_index_of_blocking_piece && col_step < 0)))
            {
                return false;
            }

            if(ImGui::IsMouseReleased(0))
            {
                if(dst.bit() != nullptr && dst.bit()->getOwner()->playerNumber() != bit.getOwner()->playerNumber())
                {
                dst.destroyBit();
                } 

                clear_passant();
            }
            return true;
        }
        return false;
    }

    if(bit.gameTag() == Knight)
    {
        int row_difference = abs(destination_row - starting_row);
        int column_difference = abs(destination_column - starting_column);

        if( (row_difference == 2 && column_difference == 1) || //l shaped movement checks
            (row_difference == 1 && column_difference == 2) )
        {
            if(ImGui::IsMouseReleased(0))
            {
                if(dst.bit() != nullptr && dst.bit()->getOwner()->playerNumber() != bit.getOwner()->playerNumber())
                {
                    dst.destroyBit();
                }

                clear_passant();
            }
            return true;
        }
        return false;
    }

    if(bit.gameTag() == King)
    {
        int row_difference = abs(destination_row - starting_row);
        int column_difference = abs(destination_column - starting_column);

        if(row_difference <= 1 && column_difference <= 1) //only can move one square in any direction
        {
            if(ImGui::IsMouseReleased(0))
            {
                if(dst.bit() != nullptr && dst.bit()->getOwner()->playerNumber() != bit.getOwner()->playerNumber())
                {
                    dst.destroyBit();
                }
                bit.movedFromStart();

                clear_passant();
            }
            return true;
        
        }

        //castling 
        if(starting_row == destination_row && column_difference == 2) //valid castling squares for king
        {
            if(!bit.hasMovedFromStart()) //cannot castle if king has moved
            {
                if(destination_column - starting_column > 0) //castling to right
                {
                    if(!_grid[starting_row - 1][7].bit()->hasMovedFromStart()) //if the rook to castle to hasnt moved
                    {
                        if(ImGui::IsMouseReleased(0))
                        {
                            Bit *rook_to_castle = new Bit(*_grid[starting_row - 1][7].bit()); //create a new rook to castle with

                            _grid[starting_row - 1][7].destroyBit(); //remove the old rook

                            rook_to_castle->setPosition(_grid[starting_row - 1][5].getPosition());
                            _grid[starting_row - 1][5].setBit(rook_to_castle);

                            rook_to_castle->movedFromStart();
                            bit.movedFromStart();

                            clear_passant();
                        }
                        return true;
                    }
                }

                if(destination_column - starting_column < 0) //castling to left
                {
                    if(!_grid[starting_row - 1][0].bit()->hasMovedFromStart()) //if the rook to castle to hasnt moved
                    {
                        if(ImGui::IsMouseReleased(0))
                        { 
                            Bit *rook_to_castle = new Bit(*_grid[starting_row - 1][0].bit()); //create a new rook to castle with

                            _grid[starting_row - 1][0].destroyBit(); //remove the old rook

                            rook_to_castle->setPosition(_grid[starting_row - 1][3].getPosition());
                            _grid[starting_row - 1][3].setBit(rook_to_castle);

                            rook_to_castle->movedFromStart();
                            bit.movedFromStart();

                            clear_passant();
                        }
                        return true;
                    }
                }
            }
        }
        return false;
    }

    if(bit.gameTag() == Queen)
{
    int row_difference = abs(destination_row - starting_row);
    int column_difference = abs(destination_column - starting_column);

    // Queen can move either like a rook (same row or column) or like a bishop (diagonally)
    if((starting_row == destination_row || starting_column == destination_column) || // rook movement
       (row_difference == column_difference)) // bishop movement
    {
        if(starting_column == destination_column) // vertical movement 
        {
            if(starting_row < destination_row) // upwards movement
            {
                int row_index_of_blocking_piece = 9;

                for(int row_index = starting_row; row_index < destination_row; row_index++)
                {
                    if(_grid[row_index - 1][destination_column-1].bit() != nullptr)
                    {
                        if(starting_row == row_index)
                        {
                            continue;
                        }
                        row_index_of_blocking_piece = row_index;
                    }
                }
                if(destination_row > row_index_of_blocking_piece)
                {
                    return false;
                }
            }

            if(starting_row > destination_row) // downwards movement
            {
                int row_index_of_blocking_piece = -1;

                for(int row_index = starting_row; row_index > destination_row; row_index--)
                {
                    if(_grid[row_index - 1][destination_column-1].bit() != nullptr)
                    {
                        if(starting_row == row_index)
                        {
                            continue;
                        }
                        row_index_of_blocking_piece = row_index;
                    }
                }
                if(destination_row < row_index_of_blocking_piece)
                {
                    return false;
                }
            }
        }

        if(starting_row == destination_row) // horizontal movement 
        {
            if(starting_column < destination_column) // moving right
            {
                int column_index_of_blocking_piece = 9;

                for(int col_index = starting_column; col_index < destination_column; col_index++)
                {
                    if(_grid[destination_row-1][col_index-1].bit() != nullptr)
                    {
                        if(starting_column == col_index)
                        {
                            continue;
                        }
                        column_index_of_blocking_piece = col_index;
                    }
                }
                if(destination_column > column_index_of_blocking_piece)
                {
                    return false;
                }
            }

            if(starting_column > destination_column) // moving left
            {
                int column_index_of_blocking_piece = 0;

                for(int col_index = starting_column; col_index > destination_column; col_index--)
                {
                    if(_grid[destination_row-1][col_index-1].bit() != nullptr)
                    {
                        if(starting_column == col_index)
                        {
                            continue;
                        }
                        column_index_of_blocking_piece = col_index;
                    }
                }
                if(destination_column < column_index_of_blocking_piece)
                {
                    return false;
                }
            }
        }

        if(row_difference == column_difference) // diagonal movement 
        {
            int row_step = (destination_row > starting_row) ? 1 : -1;
            int col_step = (destination_column > starting_column) ? 1 : -1;

            int row_index_of_blocking_piece = 9;
            int column_index_of_blocking_piece = 9;

            int current_row = starting_row + row_step;
            int current_column = starting_column + col_step;

            while(current_row != destination_row && current_column != destination_column)
            {
                if(_grid[current_row - 1][current_column - 1].bit() != nullptr)
                {
                    row_index_of_blocking_piece = current_row;
                    column_index_of_blocking_piece = current_column;
                    break;
                }

                current_column += col_step;
                current_row += row_step;
            }

            if (row_index_of_blocking_piece != 9 && column_index_of_blocking_piece != 9 &&
                ((destination_row > row_index_of_blocking_piece && row_step > 0) ||
                 (destination_row < row_index_of_blocking_piece && row_step < 0) ||
                 (destination_column > column_index_of_blocking_piece && col_step > 0) ||
                 (destination_column < column_index_of_blocking_piece && col_step < 0)))
            {
                return false;
            }
        }

        if(ImGui::IsMouseReleased(0))
        { 
            if(dst.bit() != nullptr && dst.bit()->getOwner()->playerNumber() != bit.getOwner()->playerNumber()) //capture piece if there is one
            {
                dst.destroyBit();
            }

            clear_passant();
        }
        return true;
    }
    return false;
}
    return false;
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) 
{
    return;
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}

Player* Chess::checkForWinner()
{
    // check to see if either player has won
    return nullptr;
}

bool Chess::checkForDraw()
{
    // check to see if the board is full
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
                _grid[y][x].setBit(nullptr);
            }
        }
    }
}


//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
}

