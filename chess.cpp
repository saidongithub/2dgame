#include <doge/doge.h>
#include <doge/window.h>
#include <doge/graphics.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <random>
#include <time.h>

const int PAWN = 0;
const int BISHOP = 1;
const int KNIGHT = 2;
const int ROOK = 3;
const int QUEEN = 4;
const int KING = 5;


const int BLACK = 0;
const int WHITE = 1;

typedef struct asset_s{
	doge_image_t* image;
	int width;
	int height;
} asset;

typedef struct piece{
	int type;
	int color;
	int castle;
} piece;

piece* piece_make(int type, int color){
	//allocate memory for new piece
	piece* newpiece = (piece*)malloc(sizeof(piece));
	if(!newpiece){
		printf("Failed to allocate memory\n");
		return nullptr;
	}

	//make a new piece
	newpiece -> type = type;
	newpiece -> color = color;
	if(type == ROOK || type == KING){
		newpiece -> castle = 1;
	} else{
		newpiece -> castle = 0;
	}
	return newpiece;
}

void piece_free(piece* piece){
	free(piece);
}

asset* asset_load(const char* filename){
	//load image
	doge_image_t* image;
	image = doge_image_load(filename);
	if(!image){
		printf("Failed loading img\n");
		return nullptr;
	}

	//get memory for new asset
	asset* newasset;
	newasset = (asset*)malloc(sizeof(asset));
	if(!newasset){
		doge_image_free(image);
		printf("Failed to malloc\n");
		return nullptr;
	}
	//make new asset
	newasset -> image = image;
	newasset -> width = doge_image_width(image);
	newasset -> height = doge_image_height(image);

	return newasset;
}

void asset_free(asset* asset){
	doge_image_free(asset -> image);

	free(asset);
}

asset* piecevisual[6][2];

piece* board[8][8];

const int tile_size = 150;

doge_image_t* piece_image(piece* piece){
	return piecevisual[piece -> type][piece -> color] -> image;
}

int pawncanmove(int color, int x1, int y1, int x2, int y2){
	//if moving one tile in y
	if((color == WHITE && y2 == y1 + 1) || (color == BLACK && y2 == y1 - 1)){
		//if x is same and no piece blocking
		if(!board[x2][y2] && x1 == x2){
			return 1;
		} else
		//if taking piece diagonally
		if(board[x2][y2] && (x2 == x1 + 1 || x2 == x1 - 1)){
			return 1;
		}
	} else
	//moving 2 from starting
	if((y1 == 1 && y2 == 3 && color == WHITE) || (y1 == 6 && y2 == 4 && color == BLACK)){
		if(!board[x2][y2] && x2 == x1){
			return 1;
		}
	}
	//en passant here
	return 0;
}

int bishopcanmove(int x1, int y1, int x2, int y2){
	//if not moving diagonally
	if(abs(x2 - x1) != abs(y2 - y1)){
		return 0;
	}
	int dx = 1;
	int dy = 1;
	if(x2 < x1){
		dx = -1;
	}
	if(y2 < y1){
		dy = -1;
	}
	//if piece obstructing path of diagonal
	for(int i = 1; i < abs(x2 - x1); i++){
		if(board[x1 + dx * i][y1 + dy * i]){
			return 0;
		}
	}
	return 1;
}

int knightcanmove(int x1, int y1, int x2, int y2){
	//if moving 2 tiles in a direction and 1 tile perpendicular
	if((abs(x2 - x1) == 2 && abs(y2 - y1) == 1) || (abs(x2-x1) == 1 && abs(y2-y1) == 2)){
		return 1;
	}
	return 0;
}

int rookcanmove(int x1, int y1, int x2, int y2){
	if(x2 != x1 && y2 != y1){
		return 0;
	}
	//if moving in more than one direction
	int dx = 0;
	int dy = 1;
	if(y2 == y1){
		dy = 0;
	} else
	if(y2 < y1){
		dy = -1;
	}
	if(x2 > x1){
		dx = 1;
	} else
	if(x2 < x1){
		dx = -1;
	}
	//if piece obstructing path of rook
	for(int i = 1; i < abs(x2-x1 + y2-y1); i++){
		if(board[x1 + dx * i][y1 + dy * i]){
			return 0;
		}
	}
	return 1;
}

int queencanmove(int x1, int y1, int x2, int y2){
	return 0;
}

int kingcanmove(int x1, int y1, int x2, int y2){
	return 0;
}

int canmove(piece* piece, int x1, int y1, int x2, int y2){
	//can't move on piece of same color
	if(board[x2][y2] && board[x2][y2] -> color == piece -> color){
		return 0;
	} else
	//pawn movement
	if(piece -> type == PAWN){
		return pawncanmove(piece -> color, x1, y1, x2, y2);
	} else
	if(piece -> type == BISHOP){
		return bishopcanmove(x1, y1, x2, y2);
	} else
	if(piece -> type == KNIGHT){
		return knightcanmove(x1, y1, x2, y2);
	} else
	if(piece -> type == ROOK){
		return rookcanmove(x1, y1, x2, y2);
	} else
	if(piece -> type == QUEEN){
		return queencanmove(x1, y1, x2, y2);
	} else
	if(piece -> type == KING){
		return kingcanmove(x1, y1, x2, y2);
	}
}

int main(){
    int error;

    error = glfwInit();

    if(!error){
        printf("Failed to initialize glfw\n");

        return -1;
    }

    doge_window_t* window;

    window = doge_window_create("Said chess", 8 * tile_size, 8 * tile_size);

    if(!window){
        printf("Failed to create window\n");

        return -1;
    }

    doge_window_makecurrentcontext(window);

    error = glewInit();

    if(error){
        printf("Failed to initialize glew\n");
        doge_window_free(window);

        return -1;
    }

	//white assets
	asset* wpawn = asset_load("whitepawn.png");
	asset* wbishop = asset_load("whitebishop.png");
	asset* wknight = asset_load("whiteknight.png");
	asset* wrook = asset_load("whiterook.png");
	asset* wqueen = asset_load("whitequeen.png");
	asset* wking = asset_load("whiteking.png");
	//black assets
	asset* bpawn = asset_load("blackpawn.png");
	asset* bbishop = asset_load("blackbishop.png");
	asset* bknight = asset_load("blackknight.png");
	asset* brook = asset_load("blackrook.png");
	asset* bqueen = asset_load("blackqueen.png");
	asset* bking = asset_load("blackking.png");
	//if assets fail, return error
	if(!(wpawn && wbishop && wknight && wrook && wqueen && wking
	  && bpawn && bbishop && bknight && brook && bqueen && bking)){
		printf("Failed to load asset\n");
		return -1;
	}

	//white pieces
	piecevisual[PAWN][BLACK] = bpawn;
	piecevisual[BISHOP][BLACK] = bbishop;
	piecevisual[KNIGHT][BLACK] = bknight;
	piecevisual[ROOK][BLACK] = brook;
	piecevisual[QUEEN][BLACK] = bqueen;
	piecevisual[KING][BLACK] = bking;
	//black pieces
	piecevisual[PAWN][WHITE] = wpawn;
	piecevisual[BISHOP][WHITE] = wbishop;
	piecevisual[KNIGHT][WHITE] = wknight;
	piecevisual[ROOK][WHITE] = wrook;
	piecevisual[QUEEN][WHITE] = wqueen;
	piecevisual[KING][WHITE] = wking;

	//initialize board
	int piececol = WHITE;
	for(int y = 0; y < 8; y++){
		if(y > 4){
			piececol = BLACK;
		}
		//draw piece row
		if(y == 0 || y == 7){
			board[0][y] = piece_make(ROOK, piececol);
			board[1][y] = piece_make(KNIGHT, piececol);
			board[2][y] = piece_make(BISHOP, piececol);
			board[3][y] = piece_make(QUEEN, piececol);
			board[4][y] = piece_make(KING, piececol);
			board[5][y] = piece_make(BISHOP, piececol);
			board[6][y] = piece_make(KNIGHT, piececol);
			board[7][y] = piece_make(ROOK, piececol);
		} else
		//draw pawn row
		if(y == 1 || y == 6){
			for(int x = 0; x < 8; x++){
				board[x][y] = piece_make(PAWN, piececol);
			}
		} else{
			//create nullptr in empty spots
			for(int x = 0; x < 8; x++){
				board[x][y] = nullptr;
			}
		}
	}

	//mass declaration of variables
	int mouse_x, mouse_y;
	int mouse_x_tile = 0;
	int mouse_y_tile = 0;

	piece* selected = nullptr;
	int selected_x = 0;
	int selected_y = 0;

	int mouse_clicked = 0;
	int click_x = 0;
	int click_y = 0;
	int release_x = 0;
	int release_y = 0;

	int firstclick = 0;

	int turn = 1;

    while(!doge_window_shouldclose(window)){
        /* clear the window */
        doge_clear();

		//set mousex and mousey to mouse's x and y position relative to window's 0,0
		doge_window_getcursorpos(window, &mouse_x, &mouse_y);
		mouse_x_tile = mouse_x / tile_size;
		mouse_y_tile = 7 - mouse_y / tile_size;

		//check if mouse is within window
		if(mouse_x >= 0 && mouse_y >= 0 && mouse_x_tile < 8 && mouse_y_tile < 8){
			//click update
			if(mouse_clicked != doge_window_mousepressed(window, DOGE_MOUSE_BUTTON_LEFT)){
				//if the mouse went from normal to pushed (was pushed)
				if(!mouse_clicked){
					click_x = mouse_x_tile;
					click_y = mouse_y_tile;
				}
				//if the mouse went from pushed to normal (was released)
				else {
					release_x = mouse_x_tile;
					release_y = mouse_y_tile;
				}
				//if no piece selected and mouse was pushed
				if(!selected && !mouse_clicked && board[click_x][click_y] && board[click_x][click_y] -> color == turn){
					//select piece that is left clicked
					selected_x = click_x;
					selected_y = click_y;
					selected = board[selected_x][selected_y];
				}else
				if(selected && board[click_x][click_y] != selected && !mouse_clicked && board[click_x][click_y] && board[click_x][click_y] -> color == turn){
					selected_x = click_x;
					selected_y = click_y;
					selected = board[selected_x][selected_y];
				}else
				if(selected && !mouse_clicked && canmove(selected, selected_x, selected_y, click_x, click_y)){
					//if clicking on new tile, move piece
					piece_free(board[click_x][click_y]);
					board[click_x][click_y] = selected;
					board[selected_x][selected_y] = nullptr;
					selected = nullptr;
					turn = 1 - turn;
				}else
				if(selected && !mouse_clicked && click_x == selected_x && click_y == selected_y){
					//if clicking selected piece and it's already clicked then deselect
					if(firstclick){
						selected = nullptr;
					}
				}
				else
				//if piece selected and mouse was released
				if(selected && mouse_clicked){
					if(canmove(selected, selected_x, selected_y, release_x, release_y)){
						//if mouse on another piece's tile, take piece
						piece_free(board[release_x][release_y]);
						board[release_x][release_y] = selected;
						board[selected_x][selected_y] = nullptr;
						selected = nullptr;
						turn = 1 - turn;
					} else
					if(!firstclick){
						firstclick = 1;
					}
				}
				mouse_clicked = doge_window_mousepressed(window, DOGE_MOUSE_BUTTON_LEFT);
			}
		} else
		if(mouse_clicked && !doge_window_mousepressed(window, DOGE_MOUSE_BUTTON_LEFT)){
			selected = nullptr;
			selected_x = -1;
			selected_y = -1;
		}

		//draws chess board
		for(int y = 0; y < 8; y++){
			for(int x = 0; x < 8; x++){
				//alternate tile colors
				if((x + y) % 2){
					doge_setcolor(0.2, 0.6, 0.2);
				} else {
					doge_setcolor(0.5, 0.8, 0.5);
				}
				//draw tiles
				doge_fill_rectangle(x * tile_size, y * tile_size, tile_size, tile_size);

				//draw pieces on board by location
				doge_setcolor(1, 1, 1);
				if(board[x][7 - y]){
					doge_draw_image(piece_image(board[x][7-y]), x * tile_size, y * tile_size, tile_size, tile_size);
				}
			}
		}
		doge_setcolor_alpha(0.3, 0.3, 0.3, 0.4);
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if(selected && canmove(selected, selected_x, selected_y, x, y)){
					doge_fill_ellipse(x * tile_size, (7 - y) * tile_size + tile_size / 4, tile_size / 2, tile_size / 2);
				}
			}
		}

		//if mouse held down and piece selected
		if(selected){
			//set color to tile behind selected piece
			if((selected_x + selected_y) % 2){
				doge_setcolor(1, 1, 0.85);
			} else {
				doge_setcolor(1, 1, 0.85);
			}
			//redraw tile over selected piece
			doge_fill_rectangle(selected_x * tile_size, (7 - selected_y) * tile_size, tile_size, tile_size);
			doge_setcolor(1, 1, 1);
			//draw selected piece at cursor to give illusion of holding piece
			if(mouse_clicked){
				doge_draw_image(piece_image(selected), mouse_x - tile_size / 2, mouse_y - tile_size / 2, tile_size, tile_size);
			} else{
				doge_draw_image(piece_image(selected), selected_x * tile_size, (7 - selected_y) * tile_size, tile_size, tile_size);
			}
		}
        /* swap the frame buffer */
        doge_window_render(window);

        /* check for keyboard, mouse, or close event */
        doge_window_poll();
    }
	//loop through board to free all pieces
	for(int x = 0; x < 8; x++){
		for(int y = 0; y < 8; y++){
			free(board[x][y]);
		}
	}
	//free all assets
	for(int type = 0; type < 6; type++){
		asset_free(piecevisual[type][0]);
		asset_free(piecevisual[type][1]);
	}
	//free doge_window
	doge_window_free(window);
    return 0;
}