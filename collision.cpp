#include "collision.hpp"

using namespace std;


bool point_in_circle(const vec2& p, const Circle& c) {
	const vec2 d = p - c.pos;
	return d.length2() <= c * c;
}
bool point_in_rect(const vec2& p, const AABB& bb) {
	return p.x >= bb.A.x && p.y >= bb.A.y && p.x <= bb.B.x && p.y <= bb.B.y;
}

bool circle_circle_coll(const Circle& c1, const Circle& c2) {
	return point_in_circle(c1.pos, Circle(c2.pos, c1 + c2));
}

bool rect_rect_coll(const AABB& a, const AABB& b) {
	return a.B.x > b.A.x && a.B.y > b.A.y && a.A.x < b.B.x && a.A.y < b.B.y;
}


AABB& AABB::correct() {
	if (A.x > B.x) swap(A.x, B.x);
	if (A.y > B.y) swap(A.y, B.y);
	return *this;
}

AABB AABB::operator+(const vec2& off) const {
	return AABB(A + off, B + off);
}
AABB AABB::operator-(const vec2& off) const {
	return AABB(A - off, B - off);
}
AABB& AABB::operator+=(const vec2& off) {
	A += off, B += off;
	return *this;
}
AABB& AABB::operator-=(const vec2& off) {
	A -= off, B -= off;
	return *this;
}
AABB AABB::operator*(const vec2& s) const {
	return AABB(A * s, B * s);
}
AABB AABB::operator/(const vec2& s) const {
	return AABB(A / s, B / s);
}
AABB& AABB::operator*=(const vec2& s) {
	A *= s, B *= s;
	return *this;
}
AABB& AABB::operator/=(const vec2& s) {
	A /= s, B /= s;
	return *this;
}

AABB AABB::operator+(const float& d) const {
	return AABB(A - d, B + d);
}
AABB AABB::operator+(const AABB& bb) const {
	return AABB(A + bb.A, B + bb.B);
}

AABB Circle::GetAABB(void) const {
	return AABB(pos - r, pos + r);
}

AABB Line::GetAABB(void) const {
	AABB bb(A, B);
	bb.correct();
	return bb;
}


std::tuple<int,int,int,int> GetGridBounds(AABB bb, uint depth) {
	int w = 1 << depth;
	int sx = std::min(std::max(static_cast<int>(bb.A.x * w), 0), w-1);
	int sy = std::min(std::max(static_cast<int>(bb.A.y * w), 0), w-1);
	int ex = std::min(std::max(static_cast<int>(bb.B.x * w), 0), w-1);
	int ey = std::min(std::max(static_cast<int>(bb.B.y * w), 0), w-1);
	return {sx, sy, ex, ey};
}

GridLocation::GridLocation(AABB bb, int max_depth) {
	if (!bb.inside(AABB(0,0,1,1))) {
		x = 0, y = 0, dx = 0, dy = 0, depth = -1;
		return;
	}
	
	//clamp to something reasonable
	int depth = std::min(max_depth, 15);
	
	//get integer bounding box of highest depth
	auto [sx, sy, ex, ey] = GetGridBounds(bb, depth);
	
	//lower depth until possible to encode
	while ((ex - sx > 1 || ey - sy > 1) && depth > 0) {
		depth--;
		sx >>= 1;
		sy >>= 1;
		ex >>= 1;
		ey >>= 1;
	}
	
	x = sx, y = sy;
	dx = ex-sx, dy = ey-sy;
	this->depth = depth;
}

bool GridLocation::IsInvalid(void) const {
	if (depth < 0) return true;
	const int w = 1 << depth;
	if (static_cast<int>(x) + dx >= w || static_cast<int>(y) + dy >= w) return true;
	return false;
}

GridLocation& GridLocation::LowerDepth(void) {
	*this = GridLocation(x / 2, y / 2, dx && (x & 1), dy && (y & 1), depth - 1);
	return *this;
}

AABB GridLocation::GetAABB(AABB space) const {
	if (IsInvalid()) return space;
	const int w = 1 << depth;
	AABB bb(
		{static_cast<float>(x) / w, static_cast<float>(y) / w},
		{static_cast<float>(x + dx + 1) / w, static_cast<float>(y + dy + 1) / w}
	);
	return bb * (space.B - space.A) + space.A;
}
