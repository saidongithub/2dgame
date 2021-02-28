#include <doge/doge.h>
#include <doge/window.h>
#include <doge/graphics.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

int main(){
	int error;

	error = glfwInit();

	if(!error){
		printf("Failed to initialize glfw\n");

		return -1;
	}

	doge_window_t* window;

	window = doge_window_create("Said", 800, 600);
	
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
	
	doge_image_t* image;
	int image_width, image_height;

	image = doge_image_load("vqrus.png");
	image_width = doge_image_width(image);
	image_height = doge_image_height(image);

	while(!doge_window_shouldclose(window)){
		/* clear the window */
		doge_clear();

		/* set the color to white */
		doge_setcolor(1.0f, 1.0f, 1.0f);
		
		/* draw a line from (0, 0) to (800, 600) */
		doge_draw_line(0, 0, 800, 600);

		if(doge_window_keypressed(window, DOGE_KEY_A)){
			printf("A pressed\n");
		}

		if(doge_window_mousepressed(window, DOGE_MOUSE_BUTTON_LEFT)){
			printf("left click pressed\n");
		}

		int x, y;

		/* give pointers to our (x,y) integers */
		doge_window_getcursorpos(window, &x, &y);

		printf("mouse position x = %d, y = %d\n", x, y);

		/* draw the image, starting at (0, 0) with (image_width, image_height) dimensions */
		doge_rotate_around(300, 400, x); /* 45 degrees */
		doge_draw_image(image, 0, 0, image_width, image_height);
		doge_transform_reset(); /* reset rotation */

		/* swap the frame buffer */
		doge_window_render(window);

		/* check for keyboard, mouse, or close event */
		doge_window_poll();
	}

	return 0;
}