#include <doge/doge.h>
#include <doge/window.h>
#include <doge/graphics.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <random>
#include <time.h>

unsigned long nanotime(){
	timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

//get an image as an asset
struct asset_s{
	doge_image_t* image;
	int width;
	int height;
};

typedef struct asset_s asset_t;

//make entity in game with asset
struct entity_s{
	asset_t* asset;
	int x;
	int y;
	int width;
	int height;
};

typedef struct entity_s entity_t;

asset_t* asset_load(const char* filename){
	doge_image_t* image;

	image = doge_image_load(filename);
	//if fail to load image, return null
	if(!image){
		printf("Failed loading img\n");
		return nullptr;
	}
	asset_t* asset;
	asset = (asset_t*)malloc(sizeof(asset_t));
	//if failed to allocate memory, return nullptr
	if(!asset){
		doge_image_free(image);
		printf("Failed to malloc\n");
		return nullptr;
	}
	asset -> image = image;
	asset -> width = doge_image_width(image);
	asset -> height = doge_image_height(image);

	return asset;
}

void asset_free(asset_t* asset){
	doge_image_free(asset -> image);

	free(asset);
}

//function to make entities
entity_t* entity_create(asset_t* asset, int width, int height){
	entity_t* entity = (entity_t*)malloc(sizeof(entity_t));
	entity -> asset = asset;
	entity -> width = width;
	entity -> height = height;
	entity -> x = 0;
	entity -> y = 0;

	return entity;
}

void entity_free(entity_t* entity){
	free(entity);
	//entity is free, so free
}
//function to draw entity
void entity_draw(entity_t* entity){
	doge_draw_image(entity -> asset -> image, entity -> x, entity -> y, entity -> width, entity -> height);
}
//function to check if point is inside rectangle
int point_in_rect(entity_t* rect, int x, int y){
	if(x >= rect -> x && x <= rect -> x + rect -> width){
		if(y >= rect -> y && y <= rect -> y + rect -> height){
			return 1;
		}
	}
	return 0;
}
//uses point_in_rect to check if any corner of rectangle is inside another rectangle
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
	asset_t* spaceship_asset = asset_load("spaceship.png");

	if(!spaceship_asset){
		printf("Could not load asset\n");
		return -1;
	}

	asset_t* projectile_asset = asset_load("projectile.png");

	if(!projectile_asset){
		printf("Could not load asset\n");
		return -1;
	}

	asset_t* alien_asset = asset_load("vqrus.png");

	if(!alien_asset){
		printf("Could not load asset\n");
		return -1;
	}

	entity_t* spaceship;
	spaceship = entity_create(spaceship_asset, 100, 100);

	if(!spaceship){
		printf("Failed to allocate memory for entity\n");

		return -1;
	}


	const int numProjectiles = 128;
	entity_t* projectiles[numProjectiles];
	//set all projectiles to null
	for(int x = 0; x < numProjectiles; x++){
		projectiles[x] = nullptr;
	}

	const int numAliens = 10;

	entity_t* aliens[numAliens];
	//set all aliens to null
	for(int x = 0; x < numAliens; x++){
		aliens[x] = nullptr;
	}

	unsigned long last_tick = nanotime();
	unsigned long current_time;

	const int tps = 60;

	int cooldown = 0;

	int aliencooldown = 0;

	while(!doge_window_shouldclose(window)){
		//shouldtick = nanosecondspassed * tickspersecond >= 1000000000

		current_time = nanotime();

		if((current_time - last_tick) * tps >= 1000000000){
			last_tick = current_time;

			//move ship if WASD pressed
			if(doge_window_keypressed(window, DOGE_KEY_W)){
				spaceship -> y -= 10;
			}
			if(doge_window_keypressed(window, DOGE_KEY_A)){
				spaceship -> x -= 10;
			}
			if(doge_window_keypressed(window, DOGE_KEY_S)){
				spaceship -> y += 10;
			}
			if(doge_window_keypressed(window, DOGE_KEY_D)){
				spaceship -> x += 10;
			}
			//Make sure ship cant go out of bounds
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

			if(cooldown)
				cooldown--;

			if(aliencooldown)
				aliencooldown--;

			//If space pressed, shoot projectile
			if(doge_window_keypressed(window, DOGE_KEY_SPACE) && !cooldown){
				for(int x = 0; x < numProjectiles; x++){
					if(!projectiles[x]){
						//make projectile after checking array for first null projectile
						projectiles[x] = entity_create(projectile_asset, 20, 100);
						//return -1 if making proj failed
						if(!projectiles[x]){
							return -1;
						}
						//make projectile at spaceship's x and y
						projectiles[x] -> x = spaceship -> x + spaceship -> width /2 - projectiles[x] -> width / 2;
						projectiles[x] -> y = spaceship -> y - projectiles[x] -> height;

						cooldown = tps / 8;

						break;
					}
				}
			}
			for(int x = 0; x < numProjectiles; x++){
				if(projectiles[x]){
					//move projectiles up
					projectiles[x] -> y -= 35;
					//free projectiles that are oob and set to null
					if(projectiles[x] -> y < -projectiles[x] -> height){
						entity_free(projectiles[x]);

						projectiles[x] = nullptr;
					}
				}
			}
			if(!aliencooldown){
				for(int i = 0; i < numAliens; i++){
					if(!aliens[i]){
						//make alien after finding null alien
						aliens[i] = entity_create(alien_asset, 100, 100);
						printf("Alien made\n");
						//if alien creation fails, return -2
						if(!aliens[i]){
							printf("Alien failed\n");
							return -2;
						}
						//give new alien random x coordinate
						aliens[i] -> x = rand() % (doge_window_width(window) - aliens[i] -> width);
						aliens[i] -> y = 0;

						aliencooldown = 20;
						break;
					}
				}
			}
			for(int i = 0; i < numAliens; i++){
				//move aliens down
				if(aliens[i]){
					aliens[i] -> y += 5;
					//if alien reaches bottom, game over
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
							//if alien and proj collide, free both and set back to null
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

	asset_free(spaceship_asset);
	asset_free(projectile_asset);
	asset_free(alien_asset);

	return 0;
}