#include "allegro5/allegro.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_primitives.h"

#include <iostream>
/*
Pi being a dream game... That's why it is hard

if i am going to click on prims to see if
pixel coords work, why not change their
colour? or move them up slightly?

moving the camera will be harder
*/

//
// data
//

typedef struct Vector
{
	float x = 0.f, y = 0.f, z = 0.f;
} vector;

struct Touch
{
	int x, y;
	
} touch;

Vector our_vertex;
Vector out_shape_size;
Vector out_coords;
Vector out_coords2;
Vector move{0.f, 0.f, 0.f};


struct Shape
{
	int n = 3;
	ALLEGRO_VERTEX *v;

} shape;

//
// functions
//

void setPerspective()
{
	ALLEGRO_TRANSFORM projection;
	ALLEGRO_DISPLAY *display =
		al_get_current_display();

	double sw = al_get_display_width(display);
	double sh = al_get_display_height(display);
	// radians
	float f = 90 * ALLEGRO_PI / 180;

	// must identity
	al_identity_transform(&projection);

	// example from projection2.c
	// 0,0,0 in centre of screen
	// super useful expression
	al_perspective_transform(&projection,
							 (-1 * sw / sh * f), f, 1,
							 (f * sw / sh), -f, 1000);

	al_use_projection_transform(&projection);
}


// them - the vertex we look at
// out - the screen coords
void transformPerspective(
Vector them,
Vector &out)
{
	ALLEGRO_TRANSFORM t2;
	al_copy_transform(&t2, al_get_current_transform());
	al_compose_transform(&t2, al_get_current_projection_transform());

	ALLEGRO_TRANSFORM t3;
	al_identity_transform(&t3);
	al_scale_transform(&t3, 0.5, -0.5);
	al_translate_transform(&t3, 0.5, 0.5);
	al_scale_transform(&t3, al_get_bitmap_width(al_get_target_bitmap()),
					   al_get_bitmap_height(al_get_target_bitmap()));


	// pass in the shape vertex
	al_transform_coordinates_3d_projective(&t2, &them.x, &them.y, &them.z);
	// x, y now contain normalized coordinates
	al_transform_coordinates(&t3, &them.x, &them.y);
	// x, y now contain pixel coordinates
	
	// move
	out.x = them.x;
	out.y = them.y;
	
	// not using transform so no reset
}

// no need
/*
void findThem(
Vector &compare)
{
	ALLEGRO_DISPLAY *display =
		al_get_current_display();
	int sw = (int)al_get_display_width(display);
	int sh = (int)al_get_display_height(display);
	compare.x = sw / 2 + compare.x * sw / 2;
	compare.y = sh / 2 - compare.y * sh / 2;
*/

bool isTouching()
{
	if(touch.x > out_coords.x &&
		touch.x < out_coords2.x &&
		touch.y < out_coords.y &&
		touch.y > out_coords2.y)
	{
		std::cout << "touch ";
		move.y += 10.f;
	}
}

void moveTri()
{
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_translate_transform_3d(
		&t, 0.f, move.y, -40.f);
	al_use_transform(&t);
	
/*
	no need either
	
	al_draw_prim(shape.v, NULL, NULL,
		0, shape.n+1,
		ALLEGRO_PRIM_TRIANGLE_LIST);
*/
}

void drawScene()
{
	setPerspective();

	al_clear_to_color(al_map_rgb(50.f, 50.f, 150.f));
	al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
	al_clear_depth_buffer(1);

	ALLEGRO_TRANSFORM camera;

	// no identity needed

	al_build_camera_transform(
		&camera,
		0, 0, 30,
		0, 0, 0,
		0, 1, 0);
	al_use_transform(&camera);
	
	// i guess this has an updated position
	// before using it
	moveTri();
	
	// find where our vertex lives
	our_vertex.x = shape.v[0].x;
	our_vertex.y = shape.v[0].y;
	our_vertex.z = shape.v[0].z;
	// out screen coords
	transformPerspective(our_vertex, out_coords);
	
	
	// then find out its size
	out_shape_size.x = shape.v[2].x;
	out_shape_size.y = shape.v[2].y;
	out_shape_size.z = shape.v[2].z;
	transformPerspective(
		out_shape_size, out_coords2);

	//moveTri();
	// doesnt update
	
	al_draw_prim(shape.v, NULL, NULL,
				 0, shape.n + 1,
				 ALLEGRO_PRIM_TRIANGLE_LIST);
}

void makeShape()
{
	shape.v = (ALLEGRO_VERTEX *)realloc(shape.v, shape.n * sizeof *shape.v);

	float depth = 0.f;
	float size = 10.f;
	float x = 0.f;

	ALLEGRO_COLOR c1 = al_map_rgb(
		100.f, 0.f, 0.f);

	shape.v[0] = {.x = x, .y = 0.f, .z = depth, .u = 20.f, .v = 20., .color = c1};

	shape.v[1] = {.x = x + size, .y = 0.f, .z = depth, .u = 20.f, .v = 20.f, .color = c1};

	shape.v[2] = {.x = x + size, .y = size, .z = depth, .u = 20.f, .v = 20.f, .color = c1};
}

void findTouch()
{
	std::cout << touch.x << "," << touch.y << " ";
}

int main(int argc, char *argv[])
{
	al_init();
	al_init_primitives_addon();
	al_install_touch_input();

	ALLEGRO_DISPLAY *display =
		al_create_display(720, 1600);
	ALLEGRO_TIMER *timer =
		al_create_timer(1.0f / 30.0f);
	ALLEGRO_EVENT_QUEUE *queue =
		al_create_event_queue();

	ALLEGRO_EVENT event;

	bool running = true;
	bool redraw = false;

	al_register_event_source(
		queue,
		al_get_timer_event_source(timer));
	al_register_event_source(
		queue,
		al_get_touch_input_event_source());

	makeShape();

	al_start_timer(timer);

	while (running)
	{
		al_wait_for_event(queue, &event);

		switch (event.type)
		{
		case ALLEGRO_EVENT_TIMER:
		{
			redraw = true;
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
		{
			running = false;
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
		{
			al_stop_timer(timer);
			redraw = false;
			al_acknowledge_drawing_halt(display);
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
		{
			al_acknowledge_drawing_resume(display);
			al_start_timer(timer);
			break;
		}
		case ALLEGRO_EVENT_TOUCH_BEGIN:
		{
			touch.x = event.touch.x;
			touch.y = event.touch.y;
			
			std::cout << "Vertex 1 at "
				<< out_coords.x
				 << "," << out_coords.y 
				 << " Vertex 2 at "
				 << out_coords2.x
				 << ","
				 << out_coords2.y
				 << " ";
	
			isTouching();
			break;
		}
		}

		//
		// draw
		//

		if (redraw &&
			al_event_queue_is_empty(queue))
		{
			drawScene();
			al_flip_display();
			redraw = false;
		}
	}

	al_destroy_display(display);

	return 0;
}