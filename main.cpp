#include "game.hpp"
#include <cstdio>
#include <raylib.h>

using namespace std;

//defaults
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_ZOOM 50

int main(int argc, char** argv) {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Mold Osmos");
	// ToggleBorderlessWindowed();
	SetExitKey(KEY_Q);
	SetTargetFPS(60);
	
	InitTextures();
	Font font = LoadFontEx("Fonts/DroidSansMono.ttf", 32, nullptr, 0);
	
	Game g(AABB({-20,-20}, {20,20}));
	float sim_speed = 1;
	bool paused = false;
	Viewport cam = {0,0, WINDOW_ZOOM, WINDOW_WIDTH,WINDOW_HEIGHT};
	debug_log log;
	sim_params param(log);
	char param_mode = '\0';
	
	while (!WindowShouldClose()) {
		const float dt = GetFrameTime();
		
		if (IsWindowResized()) {
			cam.w = GetScreenWidth();
			cam.h = GetScreenHeight();
		}
		// User input
		const float wheel_move = GetMouseWheelMove();
		cam.zoom *= 2. / (1 + exp(-wheel_move * 0.5));
		//camera movement
		const float move_factor = 250 * dt / cam.zoom;
		if (IsKeyDown(KEY_D)) cam.x += move_factor;
		if (IsKeyDown(KEY_A)) cam.x -= move_factor;
		if (IsKeyDown(KEY_S)) cam.y += move_factor;
		if (IsKeyDown(KEY_W)) cam.y -= move_factor;
		
		#define Rel(key) IsKeyReleased(key)
		if (Rel(KEY_COMMA)) sim_speed *= 0.5;
		if (Rel(KEY_PERIOD)) sim_speed *= 2;
		if (Rel(KEY_SPACE)) paused = !paused;
		
		//parameter mode
		if (Rel(KEY_F1)) param_mode = param_mode ? '\0' : '-';
		if (param_mode && Rel(KEY_ESCAPE)) param_mode = '-';
		switch (param_mode) {
			case '-':
				if (Rel(KEY_C)) param_mode = 'c';
				else if (Rel(KEY_F)) param_mode = 'f';
				break;
			case 'c':
				if (Rel(KEY_ONE)) param.show_colliders = !param.show_colliders;
				if (Rel(KEY_TWO)) param.show_grid = !param.show_grid;
				if (Rel(KEY_THREE)) param.show_grid_colliders = !param.show_grid_colliders;
				break;
			case 'f':
				if (Rel(KEY_ONE)) param.allow_splitting = !param.allow_splitting;
				break;
		}
		
		//Simulation
		if (!paused)
			g.Update(param, dt * sim_speed);
		
		//Rendering
		BeginDrawing();
		ClearBackground({0, 20, 50, 255});
		char txt[2048];
		g.Render(cam, param, 0);
		if (param_mode) {
			log.clear();
			log.append("[%c]\n%d FPS\n\n", param_mode, GetFPS());
			DrawTextEx(font, log.get(), {10,10}, 32, 0, WHITE);
		}
		EndDrawing();
	}
	
	CloseWindow();
	
	return 0;
}
