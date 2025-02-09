#pragma once
#include <memory>
#include <unordered_map>
#include <raylib.h>
#include "common.hpp"
#include "collision.hpp"


struct Viewport {
	float x, y;
	float zoom;
	int w, h;
	
	AABB GetAABB(void) const;
};

//game textures
const int TEX_AMOUNT = 256;
extern Texture2D tex[TEX_AMOUNT];
extern float tex_scalars[TEX_AMOUNT];
enum texture_id {
	TEXTURE_AMBIENT, TEXTURE_ATTRACTOR, TEXTURE_PLAYER, TEXTURE_SENTIENT
};

bool InitTextures(void);

struct debug_log {
	static constexpr int LOG_SIZE = 4096;
	char dat[LOG_SIZE];
	int size;
	
	debug_log(void) : size(0) {}
	void clear(void) { dat[0] = '\0', size = 0; }
	void append(const char* format, ...);
	const char* get(void) const { return dat; }
};

struct sim_params {
	debug_log& log;
	bool show_colliders : 1;
	bool show_grid : 1;
	bool show_grid_colliders : 1;
	
	sim_params(debug_log& log)
	: log(log), show_colliders(false), show_grid(false), show_grid_colliders(false)
	{}
};


struct MoteAction {
	vec2 split_dir; //must be normalized
	float split_amount; //~ 0 - 1
	
	//set functions
	void Split(vec2 dir, float amount) {
		split_dir = dir;
		if (amount < 0.01) amount = 0.01;
		if (amount > 0.5) amount = 0.5;
		split_amount = amount;
	}
	
	//get functions
	bool IsSplitting() const { return split_amount > 0; }
	
	MoteAction() : split_dir(0), split_amount(-1) {}
};

constexpr float MIN_RADIUS = 0.001;
constexpr float SPLIT_COOLDOWN = 0.1;

class Mote;
using MotePtr = std::shared_ptr<Mote>;

class Mote {
public:
	vec2 pos, vel;
	float radius;
	float time_offset;
	float split_cooldown;
	
	Mote(vec2 pos, float r) : pos(pos), vel(0), radius(r), split_cooldown(SPLIT_COOLDOWN) {
		time_offset = rand_float(0, 256);
	}
	
	AABB GetAABB() const { return Circle(pos, radius).GetAABB(); }
	Circle GetCircle() const { return Circle(pos, radius); }
	
	virtual MoteAction Update(const float& dt);
	virtual void CollideSurface(const vec2& normal, const float& dist);
	virtual void CollideMote(Mote* m);
	
	virtual void Render(const Viewport& view, sim_params& param, const float time) const;
};


class Game {
private:
	static constexpr int GRID_DEPTH = 6;
	std::unordered_map<uint64_t, MotePtr> motes;
	Grid<uint64_t, GRID_DEPTH> grid;
	uint64_t next_id;
	
	void RenderGridRecursive(const Viewport& view, int x, int y, int d) const;
	
public:
	AABB bounds;
	
	Game();
	
	uint64_t AddMote(MotePtr m);
	MotePtr GetMote(uint64_t id) const;
	void RemoveMote(uint64_t id);
	
	bool CheckSurface(const MotePtr m, vec2& norm, float& dist);
	
	void Update(const float& dt);
	
	void Render(const Viewport& view, sim_params& param, const float time) const;
};
