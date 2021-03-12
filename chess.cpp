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

const int WHITE = 1;
const int BLACK = 0;

typedef struct asset_s{
	doge_image_t* image;
	int width;
	int height;
} asset;

typedef struct piece{
	int type;
	int color;
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
		}
		//draw pawn row
		else if(y == 1 || y == 6){
			for(int x = 0; x < 8; x++){
				board[x][y] = piece_make(PAWN, piececol);
			}
		}else{
			//create nullptr in empty spots
			for(int x = 0; x < 8; x++){
				board[x][y] = nullptr;
			}
		}
	}

	//mass declaration for graphics related variables
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
				if(!selected && !mouse_clicked){
					//select piece that is left clicked
					selected_x = click_x;
					selected_y = click_y;
					selected = board[selected_x][selected_y];
				} else if(selected && !mouse_clicked && (click_x != selected_x || click_y != selected_y)){
					//if clicking on new tile, move on click
					piece_free(board[click_x][click_y]);
					board[click_x][click_y] = selected;
					board[selected_x][selected_y] = nullptr;
					selected = nullptr;
				}
				//if piece selected and mouse was released
				else if(selected && mouse_clicked){
					if(release_x != selected_x || release_y != selected_y){
						//if mouse on another piece's tile, take piece
						piece_free(board[release_x][release_y]);
						board[release_x][release_y] = selected;
						board[selected_x][selected_y] = nullptr;
						selected = nullptr;
					} else
					//if clicking on same piece, deselect
					if(selected && !mouse_clicked && release_x == selected_x && release_y == selected_y){
						board[click_x][click_y] = board[selected_x][selected_y];
						board[selected_x][selected_y] = nullptr;
						selected = nullptr;
					}
				}
				mouse_clicked = doge_window_mousepressed(window, DOGE_MOUSE_BUTTON_LEFT);
			}
		} else if(mouse_clicked && !doge_window_mousepressed(window, DOGE_MOUSE_BUTTON_LEFT)){
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
					doge_draw_image(piecevisual[board[x][7 - y] -> type][board[x][7 - y] -> color] -> image, x * tile_size, y * tile_size, tile_size, tile_size);
				}
			}
		}
		//if mouse held down and piece selected
		if(mouse_clicked && selected){
			//set color to tile behind selected piece
			if((selected_x + selected_y) % 2){
				doge_setcolor(0.5, 0.8, 0.5);
			} else {
				doge_setcolor(0.2, 0.6, 0.2);
			}
			//redraw tile over selected piece
			doge_fill_rectangle(selected_x * tile_size, (7 - selected_y) * tile_size, tile_size, tile_size);
			doge_setcolor(1, 1, 1);
			//draw selected piece at cursor to give illusion of holding piece
			doge_draw_image(piecevisual[selected -> type][selected -> color] -> image, mouse_x - tile_size / 2, mouse_y - tile_size / 2, tile_size, tile_size);
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