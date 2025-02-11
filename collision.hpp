#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include "common.hpp"

// Function naming priority
// point, line, circle, rect, capsule

struct AABB;
struct Circle;
struct Line;
struct Capsule;


bool point_in_circle(const vec2& p, const Circle& c);
bool point_in_rect(const vec2& p, const AABB& bb);

// bool line_line_coll(const Line& l1, const Line& l2, vec2* inter);
// bool line_circle_coll(const Line& l, const Circle& c, vec2* inter);
// bool line_rect_coll(const Line& l, const AABB& bb, vec2* inter);
// bool line_capsule_coll(const Line& l, const Capsule& c);

bool circle_circle_coll(const Circle& c1, const Circle& c2);
// bool circle_rect_coll(const Circle& c, const AABB& bb);

bool rect_rect_coll(const AABB& a, const AABB& b);




struct AABB {
	vec2 A, B; //min and max values
	
	//assumes that values are sorted, call correct() if not
	AABB(vec2 a = {0,0}, vec2 b = {0,0}) : A(a), B(b) {}
	AABB(float ax, float ay, float bx, float by) : A(ax, ay), B(bx, by) {}
	
	//makes sure that A holds min and B holds max
	AABB& correct(void);
	
	//offset operators
	AABB operator+(const vec2& off) const;
	AABB operator-(const vec2& off) const;
	AABB& operator+=(const vec2& off);
	AABB& operator-=(const vec2& off);
	//scaling operators
	AABB operator*(const vec2& s) const;
	AABB operator/(const vec2& s) const;
	AABB& operator*=(const vec2& s);
	AABB& operator/=(const vec2& s);
	//append size
	AABB operator+(const float& d) const;
	AABB operator+(const AABB& bb) const;
	
	bool intersects(const AABB& other) const {
		return rect_rect_coll(*this, other);
	}
	bool inside(const AABB& other) const {
		return A.x >= other.A.x && B.x <= other.B.x && A.y >= other.A.y && B.y <= other.B.y;
	}
};

struct Circle {
	vec2 pos;
	float r;
	
	Circle(vec2 pos = 0, float r = 0) : pos(pos), r(r) {}
	operator float() const { return r; }
	
	AABB GetAABB(void) const;
};

struct Line {
	vec2 A, B;
	
	Line(vec2 a = {0,0}, vec2 b = {0,0}) : A(a), B(b) {}
	
	AABB GetAABB(void) const;
};


struct GridLocation {
	uint16_t x, y;
	uint8_t dx : 1, dy : 1;
	int8_t depth;
	
	GridLocation(void) : x(0), y(0), dx(0), dy(0), depth(-1) {}
	GridLocation(uint16_t x, uint16_t y, uint8_t dx, uint8_t dy, int8_t depth) : x(x), y(y), dx(dx), dy(dy), depth(depth) {}
	//calculates deepest fit for normalized AABB (0 -> 1)
	GridLocation(AABB bb, int max_depth);
	
	bool IsInvalid(void) const;
	GridLocation& LowerDepth(void);
	//scales local 0,0 -> 1,1 AABB into global AABB given by space
	AABB GetAABB(AABB space) const;
};

std::tuple<int,int,int,int> GetGridBounds(AABB bb, int depth);

//implements an NxN grid over an AABB 
template <typename key, int depth>
class Grid {
public:
	const AABB bounds;
	
private:
	std::array<std::vector<std::pair<std::unordered_set<key>, bool>>, depth+1> grid;
	std::unordered_map<key, GridLocation> registry;
	
	AABB within_bounds(AABB bb) const {
		const vec2 delta = bounds.B - bounds.A;
		return (bb - bounds.A) / delta;
	}
	
	void MarkEmpty(int x, int y, int d) {
		//check subcells
		if (d < depth) {
			const int w = 2 << d;
			for (int i = 2*y; i <= 2*y+1; i++)
				for (int j = 2*x; j <= 2*x+1; j++)
					if (grid[d+1][i*w+j].second == true)
						return;
		}
		//free to mark this empty if it is
		const int w = 1 << d;
		const int o = y * w + x;
		if (!grid[d][o].first.empty()) return;
		grid[d][o].second = false;
		// std::cout << "marking " << x << ", " << y << " | " << d << " as empty" << std::endl;
		if (d <= 0) return;
		MarkEmpty(x/2, y/2, d-1); //recursively iterate
	}
	
	void MarkFilled(int x, int y, int d) {
		do {
			const int o = y * (1 << d) + x;
			if (grid[d][o].second) return;
			grid[d][o].second = true;
			// std::cout << "marking " << x << ", " << y << " | " << d << " as filled" << std::endl;
			x /= 2, y /= 2;
			d--;
		} while (d >= 0);
	}
	
public:
	bool isFilled(int x, int y, int d) const {
		const int w = 1 << d;
		if (x < 0 || y < 0 || d < 0 || x >= w || y >= w || d > depth) return false;
		return grid[d][y * w + x].second;
	}
	
	AABB GetAABB(int x, int y, int d) const {
		const int w = 1 << d;
		AABB bb(
			{static_cast<float>(x) / w, static_cast<float>(y) / w},
			{static_cast<float>(x + 1) / w, static_cast<float>(y + 1) / w}
		);
		return bb * (bounds.B - bounds.A) + bounds.A;
	}
	
	Grid(AABB bb) : bounds(bb) {
		for (int i = 0; i <= depth; i++) {
			grid[i].resize(1 << (2*i));
		}
	}
	
	void Clear(void) {
		registry.clear();
		for (auto v : grid)
			for (auto set : v) {
				set.first.clear();
				set.second = false;
			}
	}
	
	GridLocation GetLocation(const key id) const {
		auto it = registry.find(id);
		if (it == registry.end()) return GridLocation();
		return it->second;
	}
	
	//removes id from grid
	void Remove(const key id) {
		GridLocation loc = GetLocation(id);
		if (loc.IsInvalid()) return;
		registry.erase(id);
		for (int y = loc.y; y <= loc.y + loc.dy; y++)
			for (int x = loc.x; x <= loc.x + loc.dx; x++) {
				const int o = y * (1 << loc.depth) + x;
				grid[loc.depth][o].first.erase(id);
				if (grid[loc.depth][o].first.empty())
					MarkEmpty(x, y, loc.depth);
			}
	}
	
	GridLocation GetInsertLocation(const AABB& bb) const {
		return GridLocation(within_bounds(bb), depth);
	}
	
	//if key exists, moves its bounding box
	void Insert(const key id, const AABB& bb) {
		GridLocation loc = GetInsertLocation(bb);
		if (loc.IsInvalid()) loc = GridLocation(0,0,0,0,0);
		Remove(id);
		//insert into grid
		registry[id] = loc;
		for (int y = loc.y; y <= loc.y + loc.dy; y++)
			for (int x = loc.x; x <= loc.x + loc.dx; x++) {
				const int o = y * (1 << loc.depth) + x;
				grid[loc.depth][o].first.insert(id);
				MarkFilled(x, y, loc.depth);
			}
	}
	
	void GetInside(std::unordered_set<key>& found, const std::tuple<int,int,int,int>& b, int x, int y, int d) const {
		const int o = y * (1 << d) + x;
		if (grid[d][o].second == false) return;
		for (key id : grid[d][o].first)
			found.insert(id);
		if (d >= depth) return;
		//check subnodes
		const int t = depth - (d+1);
		const int sx = std::max(std::get<0>(b) >> t, 2*x);
		const int sy = std::max(std::get<1>(b) >> t, 2*y);
		const int ex = std::min(std::get<2>(b) >> t, 2*x+1);
		const int ey = std::min(std::get<3>(b) >> t, 2*y+1);
		for (int i = sy; i <= ey; i++)
			for (int j = sx; j <= ex; j++)
				GetInside(found, b, j, i, d+1);
	}
	
	//returns list of ids whose bounding box collides with given one
	std::unordered_set<key> GetInside(AABB bb) const {
		std::unordered_set<key> found;
		bb = within_bounds(bb);
		const auto b = GetGridBounds(bb, depth);
		GetInside(found, b, 0, 0, 0);
		return found;
	}
};
