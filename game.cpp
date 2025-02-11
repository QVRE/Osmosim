#include "game.hpp"
#include "collision.hpp"
#include "common.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <raylib.h>
#include <cstdlib>
#include <vector>


void debug_log::append(const char* format, ...) {
	if (size >= LOG_SIZE) return;
	
	va_list args;
	va_start(args, format);
	int written = std::vsnprintf(dat + size, LOG_SIZE - size, format, args);
	va_end(args);
	
	if (written > 0) {
		size += static_cast<size_t>(written);
		if (size > LOG_SIZE) size = LOG_SIZE;
	}
}


Texture2D tex[TEX_AMOUNT];
float tex_scalars[TEX_AMOUNT];

#define LOAD_TEX(file, en, scale) \
	tex[en] = LoadTexture(file);\
	SetTextureFilter(tex[en], TEXTURE_FILTER_BILINEAR);\
	tex_scalars[en] = scale

bool InitTextures(void) {
	LOAD_TEX("Textures/spore.png", TEXTURE_AMBIENT, 1.37);
	LOAD_TEX("Textures/spore3.png", TEXTURE_ATTRACTOR, 1.31);
	return true;
}


AABB Viewport::GetAABB(void) const {
	const float z = 0.5 / zoom;
	return AABB(
		x - w * z, y - h * z,
		x + w * z, y + h * z
	);
}

std::pair<float, float> Viewport::ToScreen(float x, float y) const {
	return {
		(x - this->x) * zoom + w * 0.5,
		(y - this->y) * zoom + h * 0.5
	};
}


uint64_t Game::AddMote(MotePtr m) {
	motes[next_id] = m;
	grid.Insert(next_id, m->GetAABB());
	if (m->IsAttractor()) attractors[next_id] = m;
	return next_id++;
}

MotePtr Game::GetMote(uint64_t id) const {
	//get iterator of element
	auto it = motes.find(id);
	//mote does not exist
	if (it == motes.end()) return nullptr;
	
	return it->second;
}

void Game::RemoveMote(uint64_t id) {
	motes.erase(id);
	grid.Remove(id);
	attractors.erase(id);
	// std::cout << "Removed " << id << std::endl;
}

void RenderAABB(const Viewport& view, AABB bb) {
	auto [Ax, Ay] = view.ToScreen(bb.A.x, bb.A.y);
	auto [Bx, By] = view.ToScreen(bb.B.x, bb.B.y);
	DrawRectangleLines(Ax, Ay, Bx - Ax, By - Ay, WHITE);
}

void RenderAABBFilled(const Viewport& view, AABB bb, Color clr) {
	auto [Ax, Ay] = view.ToScreen(bb.A.x, bb.A.y);
	auto [Bx, By] = view.ToScreen(bb.B.x, bb.B.y);
	DrawRectangleV({Ax, Ay}, {Bx - Ax, By - Ay}, clr);
}

void Game::RenderGridRecursive(const Viewport& view, int x, int y, int d) const {
	if (!grid.isFilled(x, y, d)) return;
	float inten = 1. - powf(1.3, -d);
	Color clr = {static_cast<uint8_t>(inten * 255), 0, 0, 255};
	RenderAABBFilled(view, grid.GetAABB(x, y, d), clr);
	RenderGridRecursive(view, 2*x, 2*y, d+1);
	RenderGridRecursive(view, 2*x+1, 2*y, d+1);
	RenderGridRecursive(view, 2*x, 2*y+1, d+1);
	RenderGridRecursive(view, 2*x+1, 2*y+1, d+1);
}

float total_area = 0;

bool sort_motes(const std::pair<Mote*, uint64_t>& a, const std::pair<Mote*, uint64_t>& b) {
	return a.first->radius < b.first->radius;
}

void Game::Render(const Viewport& view, sim_params& param, const float time) const {
	//get camera's bounding box
	const AABB bb = view.GetAABB();
	//get motes
	std::unordered_set<uint64_t> visible = grid.GetInside(bb);
	std::vector<std::pair<Mote*, uint64_t>> m;
	for (uint64_t id : visible) m.push_back({GetMote(id).get(), id});
	std::sort(m.begin(), m.end(), sort_motes);
	
	//render grid
	if (param.show_grid) RenderGridRecursive(view, 0, 0, 0);
	
	for (const auto& r : m) {
		if (param.show_grid_colliders) RenderAABB(view, grid.GetLocation(r.second).GetAABB(bounds));
		r.first->Render(view, param, time);
	}
}

// bool Game::CheckSurface(const MotePtr m, vec2& norm, float& dist) {
// 	int x = 0, y = 0;
// 	float dx = 0, dy = 0;
	
// 	const AABB bb = m->GetAABB();
// 	if (bb.B.x > bounds.B.x) x--, dx -= bb.B.x - bounds.B.x;
// 	if (bb.A.x < bounds.A.x) x++, dx -= bb.A.x - bounds.A.x;
// 	if (bb.B.y > bounds.B.y) y--, dy -= bb.B.y - bounds.B.y;
// 	if (bb.A.y < bounds.A.y) y++, dy -= bb.A.y - bounds.A.y;
	
// 	if (x != 0 || y != 0) {
// 		norm = vec2(dx, dy).normalized();
// 		dist = vec2(dx, dy).length();
// 		return true;
// 	}
// 	return false;
// }
bool Game::CheckSurface(const MotePtr m, vec2& norm, float& dist) {
	const float r = std::min(bounds.B.x, bounds.B.y);
	const float len = m->pos.length();
	const float d = len + m->radius - r;
	if (d > 0) {
		norm = -m->pos / len;
		dist = d;
		return true;
	}
	return false;
}

void Game::Update(const sim_params& param, const float& dt) {
	std::vector<uint64_t> mote_list;
	mote_list.reserve(motes.size());
	for (const auto& [id, m] : motes)
		mote_list.push_back(id);
	
	for (size_t i = 0; i < mote_list.size(); i++) {
		const uint64_t id = mote_list[i];
		const MotePtr m = GetMote(id);
		if (m == nullptr) continue;
		
		for (const auto& [a_id, a] : attractors)
			if (id != a_id)
				m->AttractTowards(a, dt);
		
		const MoteAction act = m->Update(param, dt);
		vec2 norm;
		float dist;
		if (CheckSurface(m, norm, dist))
			m->CollideSurface(norm, dist);
		
		//evaluate actions
		if (act.IsSplitting()) {
			const float a = m->radius * m->radius;
			const float r1 = sqrtf(a * (1 - act.split_amount));
			const float r2 = sqrtf(a * act.split_amount);
			
			if (r2 > MIN_RADIUS) {
				Mote new_m(m->pos + act.split_dir * (r1 + r2), r2);
				m->radius = r1;
				
				//calculate velocities
				new_m.vel = m->vel + act.split_dir * SPLIT_VELOCITY;
				m->vel -= act.split_dir * SPLIT_VELOCITY * (act.split_amount / (1 - act.split_amount));
				
				MotePtr nm = std::make_shared<Mote>(new_m);
				mote_list.push_back(AddMote(nm));
			}
		}
		
		//check for collisions
		std::unordered_set<uint64_t> coll = grid.GetInside(m->GetAABB());
		for (uint64_t other : coll) {
			if (other == id) continue;
			if (m->radius <= 0) break;
			MotePtr mo = GetMote(other);
			if (circle_circle_coll(m->GetCircle(), mo->GetCircle())) {
				m->CollideMote(mo.get());
				if (mo->radius <= 0) {
					RemoveMote(other);
				}
			}
		}
		if (m->radius < MIN_RADIUS) {
			RemoveMote(id);
			continue;
		}
		grid.Insert(id, m->GetAABB());
	}
	
	total_area = 0;
	for (const auto& [id, m] : motes)
		total_area += m->radius * m->radius;
}

Game::Game(const AABB bb) : next_id(1), grid(bb), bounds(bb) {
	AttractorMote m(vec2(0, 0), 1.5);
	m.vel = {0,0};
	uint64_t m_id = AddMote(std::make_shared<AttractorMote>(m));
}


// Mote functions

MoteAction Mote::Update(const sim_params& param, const float& dt) {
	MoteAction act;
	
	pos += vel * dt;
	// vel = vel * powf(0.9, dt);
	
	if (split_cooldown > 0) {
		split_cooldown -= dt;
	} else if (param.allow_splitting) {
		float split_k = fminf(powf(radius / this->GetCriticalRadius(), 8), 1.);
		const float split_chance = 1. - powf(1 - split_k, dt);
		if (rand_float() <= split_chance) {
			const float q = rand_float(0, 2*M_PI);
			act.Split(vec2(cos(q), sin(q)), 0.25);
			split_cooldown = SPLIT_COOLDOWN;
		}
	}
	return act;
}

void Mote::CollideSurface(const vec2& normal, const float& dist) {
	pos += normal * dist;
	if (vel.dot(normal) >= 0) return;
	vel -= normal * 2 * vel.dot(normal);
	// vel = vel * 0.9;
}

void Mote::CollideMote(Mote* m) {
	if (m->radius > radius) {
		m->CollideMote(this);
		return;
	}
	
	const float old_a = radius * radius;
	const float d = (pos - m->pos).length(); //distance
	const float h = d * 0.5;
	const float a = m->radius * m->radius + old_a;
	const float q = sqrt(0.5*a - h*h);
	
	//calculate new radii
	radius = h + q;
	m->radius = h - q;
	if (m->radius < MIN_RADIUS) {
		m->radius = -1;
		radius = sqrt(a);
	}
	
	//momentum transfer
	const float k = old_a / (radius * radius);
	vel = vel * k + m->vel * (1-k);
}

void Mote::AttractTowards(const MotePtr& m, const float& dt) {
	const float gravity = GRAVITY_CONSTANT * dt;
	const vec2 delta = m->pos - pos;
	const float dist2 = delta.length2();
	const vec2 dir = delta.normalized() * gravity / dist2;
	
	vel += dir * (m->radius * m->radius);
	m->vel -= dir * (radius * radius);
}

void RenderCircleTex(const float x, const float y, const float r, const float rot, const texture_id id) {
	const Texture2D tx = tex[id];
	const float scale = tex_scalars[id];
	const Rectangle src = {0, 0, (float)tx.width, (float)tx.height};
	const Rectangle dst = {x, y, r*2*scale, r*2*scale};
	const Vector2 origin = {r*scale, r*scale};
	
	DrawTexturePro(tx, src, dst, origin, rot, WHITE);
}

void Mote::RenderExtra(const float& x, const float& y, const float& r, const sim_params& param) const {
	if (param.show_colliders) DrawCircleLines(static_cast<int>(round(x)), static_cast<int>(round(y)), r, WHITE);
}

void Mote::Render(const Viewport& view, sim_params& param, const float time) const {
	//screen coordinates
	const auto [x, y] = view.ToScreen(pos.x, pos.y);
	const float r = radius * view.zoom;
	
	RenderCircleTex(x, y, r, 0, TEXTURE_AMBIENT);
	RenderExtra(x, y, r, param);
}

void AttractorMote::Render(const Viewport& view, sim_params& param, const float time) const {
	//screen coordinates
	const auto [x, y] = view.ToScreen(pos.x, pos.y);
	const float r = radius * view.zoom;
	
	RenderCircleTex(x, y, r, 0, TEXTURE_ATTRACTOR);
	RenderExtra(x, y, r, param);
}
