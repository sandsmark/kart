#include <SDL2/SDL.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "box.h"
#include "car.h"
#include "map.h"
#include "net.h"
#include "powerup.h"
#include "renderer.h"
#include "sound.h"
#include "vector.h"
#include "shell.h"
#include "libs/cJSON/cJSON.h"

const int SCREEN_WIDTH  = 1024;
const int SCREEN_HEIGHT = 768;
const vec2 car_start_dir = {1.0, 0.0};
static netmode_t netmode;
static unsigned long long tic = 0;
static int sockfd;
#define MAX_JSON_SIZE 2048
#define NET_REQUEST_STATE 1337
#define NUM_CLIENTS 1
struct client {
	int idx;
	int fd;
	Car *car;
	SDL_mutex *cmd_lock;
	unsigned cmd;
	SDL_Thread *thr;
};
char *json_state;
SDL_mutex *json_state_lock;
static struct client clients[NUM_CLIENTS];
SDL_atomic_t net_listen;
extern ivec2 map_starting_position;

static void render_car(SDL_Renderer *ren, Car *car)
{
	SDL_Rect target;
	target.x = car->pos.x;
	target.y = car->pos.y;
	target.w = car->width;
	target.h = car->height;
	SDL_RenderCopyEx(ren, car->texture, 0, &target, vec_angle(car_start_dir, car->direction), 0, 0);

	const int vertical_position = 5 + (POWERUPS_HEIGHT + 5) * car->id;
	target.x = 5;
	target.y = vertical_position;
	target.h = POWERUPS_HEIGHT;
	target.w = POWERUPS_HEIGHT * car->width / car->height;
        SDL_RenderCopy(ren, car->texture, 0, &target);

	if (car->powerup != POWERUP_NONE) {
		ivec2 powerup_pos;
		powerup_pos.x = target.w + 10;
		powerup_pos.y = vertical_position;
		powerup_render(ren, car->powerup, powerup_pos);
	}
}

static int server_recv_loop(void *data)
{
	struct client *me = (struct client *)data;
	char buf[64];
	ssize_t n = 0;
	while (SDL_AtomicGet(&net_listen))
	{
		n = net_recv(me->fd, buf, 64);
		if (n > 0)
		{
			unsigned cmd;
			if (sscanf(buf, "%u", &cmd) == 1)
			{
				printf("T%d: Received: %d\n", me->idx, cmd);
				if (cmd == NET_REQUEST_STATE)
				{
					if (SDL_LockMutex(json_state_lock) == 0)
					{
						net_send(me->fd, json_state);
						SDL_UnlockMutex(json_state_lock);
					}
				}
				else
				{
					if (SDL_LockMutex(me->cmd_lock) == 0)
					{
						me->cmd = cmd;
						SDL_UnlockMutex(me->cmd_lock);
					}
				}
			}
		}
		else if (n == 0)
		{
			printf("T%d: Client closed connection\n", me->idx);
			break;
		}
		else
			break;
	}
	return 0;
}

static int accpt_conn(void *data)
{
	int fd;
	SDL_atomic_t *got_client = (SDL_atomic_t*)data;
	fd = net_accept(sockfd);
	SDL_AtomicSet(got_client, 1);
	return fd;
}

int run_server(SDL_Renderer *ren)
{
	if (!map_init(ren, "map1.map")) {
		printf("unable to initialize map!\n");
		return 1;
	}
	if (!boxes_init(ren)) {
		printf("unable to initialize box!\n");
		return 1;
	}
	if (!powerups_init(ren)) {
		printf("unable to initialize powerups!\n");
		return 1;
	}

	json_state_lock = SDL_CreateMutex();
	if (json_state_lock == NULL)
	{
		printf("Failed to create mutex\n");
		return 1;
	}

	SDL_AtomicSet(&net_listen, 1);
	printf("Waiting for clients...\n");
	/* Set up each client */
	for (int i = 0; i < NUM_CLIENTS; i++)
	{
		SDL_Rect wfc_bg_target, car_target;
		SDL_Texture *wfc_bg_tex = ren_load_image(ren, "waitforclients.bmp");
		SDL_Texture *car_tex = ren_load_image_with_dims(ren, "car0.bmp", &car_target.w, &car_target.h);
		SDL_Event event;
		wfc_bg_target.x = 0;
		wfc_bg_target.y = 0;
		wfc_bg_target.w = SCREEN_WIDTH;
		wfc_bg_target.h = SCREEN_HEIGHT;
		car_target.x = SCREEN_WIDTH/2 - car_target.w/2;
		car_target.y = 280;
		float car_angle = 0;
		SDL_atomic_t got_client;
		SDL_AtomicSet(&got_client, 0);
		int clientfd;
		SDL_Thread *thr = SDL_CreateThread(accpt_conn, "Accpt conn thread", (void*)&got_client);
		if (thr == NULL)
		{
			printf("Failed to create accpt conn thread\n");
			return 1;
		}
		while (!SDL_AtomicGet(&got_client))
		{
			while (SDL_PollEvent(&event))
			{
				//If user closes the window
				if (event.type == SDL_QUIT) {
					return 0;
				}
				//If user presses any key
				if (event.type == SDL_KEYDOWN) {
					switch (event.key.keysym.sym) {
					case SDLK_ESCAPE:
						return 0;
					}
				}
			}
			SDL_RenderCopy(ren, wfc_bg_tex, 0, &wfc_bg_target);
			SDL_RenderCopyEx(ren, car_tex, 0, &car_target, car_angle, 0, 0);
			car_angle += 2*(i+1);
			if (car_angle >= 360) car_angle -= 360;
			for (int j = 0; j < NUM_CLIENTS; j++)
			{
				if (i > j)
					SDL_SetRenderDrawColor(ren, 0x00, 0xff, 0x00, 0xff);
				else
					SDL_SetRenderDrawColor(ren, 0xff, 0x00, 0x00, 0xff);
				SDL_Rect client_rect;
				client_rect.x = SCREEN_WIDTH/2 - ((NUM_CLIENTS-1)*10 + 5) + j * 20;
				client_rect.y = 378;
				client_rect.h = 10;
				client_rect.w = 10;
				SDL_RenderFillRect(ren, &client_rect);
			}
			SDL_RenderPresent(ren);
		}
		SDL_WaitThread(thr, &clientfd);
		clients[i].fd = clientfd;
		if (clients[i].fd < 0)
		{
			printf("Accept failed\n");
			return 1;
		}
		clients[i].car = calloc(1, sizeof(*(clients[i].car)));
		if (clients[i].car == NULL)
		{
			printf("Could not allocate memory for car\n");
			return 1;
		}
		/* TODO: Move car creation into own function */
		Car *car = clients[i].car;
		car->id = i;
		car->active_effects = 0;
		car->pos.x = 250;
		car->pos.y = 30 + i*20;
		car->direction.x = car_start_dir.x;
		car->direction.y = car_start_dir.y;
		char filename[10];
		sprintf(filename, "car%d.bmp", i);
		car->texture = ren_load_image_with_dims(ren, filename, &car->width, &car->height);

		clients[i].cmd_lock = SDL_CreateMutex();
		if (clients[i].cmd_lock == NULL)
		{
			printf("Failed to create mutex\n");
			return 1;
		}

		clients[i].thr = SDL_CreateThread(server_recv_loop, "Recv Client", &clients[i]);
		if (clients[i].thr == NULL)
		{
			printf("Failed to create recv thread\n");
			return 1;
		}
		printf("Num clients: %d\n", i+1);
	}
	printf("All clients connected\n");
	SDL_SetRenderDrawColor(ren, 0x0, 0x0, 0x0, 0xff);
	SDL_RenderClear(ren);

	int quit = 0;
	SDL_Event event;
	Uint32 time0 = SDL_GetTicks();

	while (!quit) {
		while (SDL_PollEvent(&event))
		{
			//If user closes the window
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			//If user presses any key
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = 1;
					break;
				}
			}
		}

		for (int i = 0; i < NUM_CLIENTS; i++)
		{
			Car *car = clients[i].car;
			if (SDL_LockMutex(clients[i].cmd_lock) == 0)
			{
				unsigned cmd = clients[i].cmd;
				if (cmd & NET_INPUT_UP)
				{
					vec2 force = car->direction;
					vec_scale(&force, 2500);
					car_apply_force(car, force);
				}
				if (cmd & NET_INPUT_DOWN)
				{
					vec2 force = car->direction;
					vec_scale(&force, -2500);
					car_apply_force(car, force);
				}
				if (cmd & NET_INPUT_LEFT)
				{
					vec_rotate(&car->direction, -3);
				}
				if (cmd & NET_INPUT_RIGHT)
				{
					vec_rotate(&car->direction, 3);
				}
				if (cmd & NET_INPUT_SPACE)
				{
					car->drift = 1;
				}
				if (cmd & NET_INPUT_RETURN)
				{
					car_use_powerup(car);
				}
				// Clear cmd
				clients[i].cmd = 0;
				SDL_UnlockMutex(clients[i].cmd_lock);
			}
		}

		map_render(ren);
		cJSON *state = cJSON_CreateObject(), *car_json;
		cJSON_AddItemToObject(state, "cars", car_json = cJSON_CreateArray());
		for (int i = 0; i < NUM_CLIENTS; i++)
		{
			for (int j = i+1; j < NUM_CLIENTS; j++)
			{
				car_collison(clients[i].car, clients[j].car);
			}
			car_move(clients[i].car);
			memset(&clients[i].car->force, 0, sizeof(clients[i].car->force));
			render_car(ren, clients[i].car);
			cJSON_AddItemToArray(car_json, car_serialize(clients[i].car));
		}
		if (SDL_LockMutex(json_state_lock) == 0)
		{
			free(json_state);
			json_state = cJSON_Print(state);
			cJSON_Minify(json_state);
			SDL_UnlockMutex(json_state_lock);
		}
		cJSON_Delete(state);

		boxes_render(ren);

		SDL_RenderPresent(ren);

		// Server increases tics
		tic++;
		Uint32 time1 = SDL_GetTicks();
		/* TODO: TIME_CONSTANT */
		if (time1 - time0 < 33)
		{
			SDL_Delay(33 - (time1-time0));
		}
		time0 = time1;
	}

	// Clean up
	SDL_AtomicSet(&net_listen, 0);
	for (int i = 0; i < NUM_CLIENTS; i++)
	{
		int t_status;
		SDL_WaitThread(clients[i].thr, &t_status);
		SDL_DestroyMutex(clients[i].cmd_lock);
		free(clients[i].car);
		net_close(clients[i].fd);
	}
	SDL_DestroyMutex(json_state_lock);
	map_destroy();
	return 0;
}


int run_client(SDL_Renderer *ren)
{
	int quit = 0;
	SDL_Event event;
	char state[MAX_JSON_SIZE];

	while (!quit) {
		while (SDL_PollEvent(&event)){
			//If user closes the window
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			//If user presses any key
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = 1;
					break;
				}
			}
		}

		const Uint8 *keystates = SDL_GetKeyboardState(NULL);
		if (keystates[SDL_SCANCODE_UP]) net_set_input(NET_INPUT_UP);
		if (keystates[SDL_SCANCODE_DOWN]) net_set_input(NET_INPUT_DOWN);
		if (keystates[SDL_SCANCODE_LEFT]) net_set_input(NET_INPUT_LEFT);
		if (keystates[SDL_SCANCODE_RIGHT]) net_set_input(NET_INPUT_RIGHT);
		if (keystates[SDL_SCANCODE_SPACE]) net_set_input(NET_INPUT_SPACE);
		if (keystates[SDL_SCANCODE_RETURN]) net_set_input(NET_INPUT_RETURN);
		net_send_input(sockfd);

		char buf[5];
		snprintf(buf, 5, "%d", NET_REQUEST_STATE);
		net_send(sockfd, buf);
		net_recv(sockfd, state, MAX_JSON_SIZE);
		printf("Recvd: %s\n", state);

		SDL_RenderPresent(ren);
	}

	// Clean up
	SDL_AtomicSet(&net_listen, 0);
	map_destroy();
	return 0;
}


int run_local(SDL_Renderer *ren)
{
	if (!map_init(ren, "map1.map")) {
		printf("unable to initialize map!\n");
		return 1;
	}
	if (!boxes_init(ren)) {
		printf("unable to initialize box!\n");
		return 1;
	}
	if (!powerups_init(ren)) {
		printf("unable to initialize powerups!\n");
		return 1;
	}

	int car_count = 2;
	Car *cars = calloc(car_count, sizeof(Car));

	// Create cars
	for (int i=0; i<car_count; i++) {
		// Initialize car
		cars[i].id = i;
		cars[i].active_effects = 0;
		cars[i].pos.x = map_starting_position.x;
		cars[i].pos.y = map_starting_position.y + i*20;
		cars[i].direction.x = car_start_dir.x;
		cars[i].direction.y = car_start_dir.y;
		cars[i].tiles_passed = 0;

		char filename[10];
		sprintf(filename, "car%d.bmp", i);
		cars[i].texture = ren_load_image_with_dims(ren, filename, &cars[i].width, &cars[i].height);
	}

	int quit = 0;
	SDL_Event event;

	while (!quit) {
		while (SDL_PollEvent(&event)){
			//If user closes the window
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			//If user presses any key
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = 1;
					break;
				}
			}
		}
		const Uint8 *keystates = SDL_GetKeyboardState(NULL);
		Car *car = &cars[0];
		if (keystates[SDL_SCANCODE_UP]) {
			vec2 force = car->direction;
			vec_scale(&force, 2500);
			car_apply_force(car, force);
		}
		if (keystates[SDL_SCANCODE_DOWN]) {
			vec2 force = car->direction;
			vec_scale(&force, -2500);
			car_apply_force(car, force);
		}
		if (keystates[SDL_SCANCODE_LEFT]) {
			vec_rotate(&car->direction, -3);
		}
		if (keystates[SDL_SCANCODE_RIGHT]) {
			vec_rotate(&car->direction, 3);
		}
		if (keystates[SDL_SCANCODE_SPACE]) {
			car->drift = 1;
		}
		if (keystates[SDL_SCANCODE_RETURN]) {
			car_use_powerup(car);
		}

		int freq = vec_length(car->velocity);
		if (freq < 10) freq = 10;
		freq = sqrt(freq);
		freq = 1500 / freq;
		sound_set_car_freq(freq);

		map_render(ren);

		for (int i=0; i<car_count; i++) {
			for (int j=i+1; j<car_count; j++)
			{
				car_collison(&cars[i], &cars[j]);
			}
			car_move(&cars[i]);
			memset(&cars[i].force, 0, sizeof(cars[i].force));
			render_car(ren, &cars[i]);
		}

		boxes_render(ren);
		shell_move();
		shell_render(ren);

		SDL_RenderPresent(ren);
	}

	// Clean up
	map_destroy();
	free(cars);
	return 0;
}

void show_menu(SDL_Renderer *ren)
{
	SDL_Texture *image = ren_load_image(ren, "startscreen.bmp");
	sound_set_type(SOUND_MENU);
	SDL_Event event;
	SDL_Rect target;
	target.x = 0;
	target.y = 0;
	target.w = SCREEN_WIDTH;
	target.h = SCREEN_HEIGHT;
	int quit = 0;
	int choice = 0;
	while (!quit) {
		while (SDL_PollEvent(&event)){
			//If user closes the window
			if (event.type == SDL_QUIT) {
				return;
			}
			//If user presses any key
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_RETURN:
					if (choice != 1) {
						quit = 1;
					}
					break;
				case SDLK_DOWN:
					choice = (choice + 1) % 3;
					break;
				case SDLK_UP:
					choice--;
					if (choice < 0) choice = 2;
					break;
				}
			}
		}


		SDL_RenderCopy(ren, image, 0, &target);

		                          //r    g     b     a
		SDL_SetRenderDrawColor(ren, 0x0, 0xff, 0xff, 0xff);
		SDL_Rect selection_rect;
		selection_rect.x = 364;
		selection_rect.y = 320 + choice * 62;
		selection_rect.h = 10;
		selection_rect.w = 10;
		SDL_RenderFillRect(ren, &selection_rect);

		// fancy useless effect
		for (int i=1; i<140; i++) {
			Uint32 t = SDL_GetTicks() / 10.0;
			SDL_Rect r;
			r.x = i * 5 + 105;
			r.y = sinf(t * ((i - 85)/500.0 + 0.02)) * 10 + 135;
			r.w = 2;
			r.h = 2;
			SDL_RenderFillRect(ren, &r);
		}

		render_string(ren, "hello world!1!!", 10, 10, 32);

		SDL_RenderPresent(ren);
	}

	//TODO: make not annoying sound
	sound_set_type(SOUND_NONE);
	SDL_SetRenderDrawColor(ren, 0x0, 0x0, 0x0, 0xff);
	SDL_RenderClear(ren);
	if (choice == 0) {
		switch (netmode)
		{
			case SERVER:
				run_server(ren);
				break;
			case CLIENT:
				run_client(ren);
				break;
			case LOCAL:
				run_local(ren);
				break;
			default:
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	printf("kartering " REVISION " launching...\n");
	srand(time(NULL));
	net_init();
	if (argc < 2)
	{
		netmode = LOCAL;
	}

	if (argc > 1) {
		if (strcmp(argv[1], "server") == 0)
		{
			if (argc != 3)
			{
				printf("Usage: %s server <port>\n", argv[0]);
				return 1;
			}
			sockfd = net_start_server(atoi(argv[2]));
			netmode = SERVER;
		}
		else if (strcmp(argv[1], "client") == 0)
		{
			if (argc != 4)
			{
				printf("Usage: %s client <address> <port>\n", argv[0]);
				return 1;
			}
			if (strcmp(argv[2], "localhost") == 0)
				sockfd = net_start_client("127.0.0.1", atoi(argv[3]));
			else
				sockfd = net_start_client(argv[2], atoi(argv[3]));
			netmode = CLIENT;
		}
		else if (strcmp(argv[1], "local") == 0)
		{
			if (argc != 2)
			{
				printf("Usage: %s local\n", argv[0]);
				return 1;
			}
			netmode = LOCAL;
		}
		else
		{
			printf("Invalid argument: %s\n", argv[1]);
			return 1;
		}
	}

	// Set up SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL init failed: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Window *win = SDL_CreateWindow("The Kartering", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (win == NULL){
		printf("SDL error creating window: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == NULL){
		printf("SDL error while creating renderer: %s\n", SDL_GetError());
		return 1;
	}


	sound_init();
	renderer_init(ren);
	show_menu(ren);

	if (netmode == SERVER || netmode == CLIENT)
		net_close(sockfd);
	net_cleanup();
	sound_destroy();
	SDL_DestroyRenderer(ren); // cleans up all textures
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
