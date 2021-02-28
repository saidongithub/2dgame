#include <doge/doge.h>
#include <doge/window.h>
#include <doge/graphics.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <random>
#include <time.h>

struct asset_s{
	doge_image_t* image;
	int width;
	int height;
};

typedef struct asset_s asset_t;

struct entity_s{
	asset_t asset;
	int x;
	int y;
	int width;
	int height;
};

typedef struct entity_s entity_t;

entity_t* entity_create(const char* filename, int width, int height){
	doge_image_t* image;

	image = doge_image_load(filename);

	if(!image){
		printf("Failed loading img\n");
		return nullptr;
	}
	entity_t* entity;
	entity = (entity_t*)malloc(sizeof(entity_t));
	if(!entity){
		doge_image_free(image);
		printf("Failed to malloc\n");
		return nullptr;
	}
	entity -> asset.image = image;
	entity -> asset.width = doge_image_width(image);
	entity -> asset.height = doge_image_height(image);
	entity -> width = width;
	entity -> height = height;
	entity -> x = 0;
	entity -> y = 0;

	return entity;
}

void entity_free(entity_t* entity){
	doge_image_free(entity -> asset.image);
	free(entity);
	//os declares entity as free, so free
}

void entity_draw(entity_t* entity){
	doge_draw_image(entity -> asset.image, entity -> x, entity -> y, entity -> width, entity -> height);
}

int point_in_rect(entity_t* rect, int x, int y){
	if(x >= rect -> x && x <= rect -> x + rect -> width){
		if(y >= rect -> y && y <= rect -> y + rect -> height){
			return 1;
		}
	}
	return 0;
}

int collides(entity_t* proj, entity_t* alien){
	if(point_in_rect(alien, proj -> x, proj -> y)){
		return 1;
	}
	if(point_in_rect(alien, proj -> x + proj -> width, proj -> y)){
		return 1;
	}
	if(point_in_rect(alien, proj -> x, proj -> y + proj -> height)){
		return 1;
	}
	if(point_in_rect(alien, proj -> x + proj -> width, proj -> y + proj -> height)){
		return 1;
	}
	return 0;
}

int main(){
	srand(clock());

	int error;

	error = glfwInit();

	if(!error){
		printf("Failed to initialize glfw\n");

		return -1;
	}

	doge_window_t* window;

	window = doge_window_create("Said", 1000, 1000);

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

	entity_t* spaceship;
	spaceship = entity_create("spaceship.png", 100, 100);


	const int numProjectiles = 128;
	entity_t* projectiles[numProjectiles];

	for(int x = 0; x < numProjectiles; x++){
		projectiles[x] = nullptr;
	}

	const int numAliens = 10;

	entity_t* aliens[numAliens];

	for(int x = 0; x < numAliens; x++){
		aliens[x] = nullptr;
	}

	while(!doge_window_shouldclose(window)){
		if(doge_window_keypressed(window, DOGE_KEY_W)){
			spaceship -> y -= 5;
		}
		if(doge_window_keypressed(window, DOGE_KEY_A)){
			spaceship -> x -= 5;
		}
		if(doge_window_keypressed(window, DOGE_KEY_S)){
			spaceship -> y += 5;
		}
		if(doge_window_keypressed(window, DOGE_KEY_D)){
			spaceship -> x += 5;
		}
		if(spaceship -> x + spaceship -> width > doge_window_width(window)){
			spaceship -> x = doge_window_width(window) - spaceship -> width;
		}
		if(spaceship -> x < 0){
			spaceship -> x = 0;
		}
		if(spaceship -> y + spaceship -> height > doge_window_height(window)){
			spaceship -> y = doge_window_height(window) - spaceship -> height;
		}
		if(spaceship -> y < 0){
			spaceship -> y = 0;
		}
		if(doge_window_keypressed(window, DOGE_KEY_SPACE)){
			for(int x = 0; x < numProjectiles; x++){
				if(!projectiles[x]){
					projectiles[x] = entity_create("projectile.png", 20, 100);

					if(!projectiles[x]){
						return -1;
					}
					projectiles[x] -> x = spaceship -> x + spaceship -> width /2 - projectiles[x] -> width / 2;
					projectiles[x] -> y = spaceship -> y - projectiles[x] -> height;
					break;
				}
			}
		}
		for(int x = 0; x < numProjectiles; x++){
			if(projectiles[x]){
				projectiles[x] -> y -= 10;

				if(projectiles[x] -> y < -projectiles[x] -> height){
					entity_free(projectiles[x]);

					projectiles[x] = nullptr;
				}
			}
		}

		for(int i = 0; i < numAliens; i++){
			if(!aliens[i]){
				aliens[i] = entity_create("vqrus.png", 100, 100);
				printf("Alien made\n");

				if(!aliens[i]){
					printf("Alien failed\n");
					return -2;
				}
				aliens[i] -> x = rand() % (doge_window_width(window) - aliens[i] -> width);
				aliens[i] -> y = 0;
			}
		}
		for(int i = 0; i < numAliens; i++){
			if(aliens[i]){
				aliens[i] -> y += 1;

				if(aliens[i] -> y > doge_window_height(window) - aliens[i] -> height){
					printf("Game over\n");
					return 0;
				}
			}
		}
		for(int i = 0; i < numProjectiles; i++){
			if(projectiles[i]){
				for(int x = 0; x < numAliens; x++){
					if(aliens[x]){
						//check collision of proj[i] and alien[x]
						if(collides(projectiles[i], aliens[x])){
							entity_free(aliens[x]);
							entity_free(projectiles[i]);

							aliens[x] = nullptr;
							projectiles[i] = nullptr;
							break;
						}
					}
				}
			}
		}
		/* clear the window */
		doge_clear();

		entity_draw(spaceship);

		for(int i = 0; i < numProjectiles; i++){
			if(projectiles[i]){
				entity_draw(projectiles[i]);
			}
		}
		for(int i = 0; i < numAliens; i++){
			if(aliens[i]){
				entity_draw(aliens[i]);
			}
		}
		/* swap the frame buffer */
		doge_window_render(window);

		/* check for keyboard, mouse, or close event */
		doge_window_poll();
	}

	return 0;
}