#include "common.hpp"

vec2 vec2::operator+(const vec2& other) const {
	return vec2(x + other.x, y + other.y);
}
vec2 vec2::operator-(const vec2& other) const {
	return vec2(x - other.x, y - other.y);
}
vec2 vec2::operator-(void) const {
	return vec2(-x, -y);
}
vec2& vec2::operator+=(const vec2& other) {
	x += other.x, y += other.y;
	return *this;
}
vec2& vec2::operator-=(const vec2& other) {
	x -= other.x, y -= other.y;
	return *this;
}
vec2 vec2::operator*(const vec2& other) const {
	return vec2(x * other.x, y * other.y);
}
vec2& vec2::operator*=(const vec2& other) {
	x *= other.x, y *= other.y;
	return *this;
}
vec2 vec2::operator/(const vec2& other) const {
	return vec2(x / other.x, y / other.y);
}
vec2& vec2::operator/=(const vec2& other) {
	x /= other.x, y /= other.y;
	return *this;
}

vec2 vec2::operator+(const float& a) const {
	return vec2(x + a, y + a);
}
vec2 vec2::operator-(const float& a) const {
	return vec2(x - a, y - a);
}
vec2 vec2::operator*(const float& s) const {
	return vec2(x * s, y * s);
}


float vec2::dot(const vec2& other) const {
	return x * other.x + y * other.y;
}
vec2 vec2::normalized(void) const {
	return *this * (1. / sqrtf(length2()));
}
void vec2::normalize(void) {
	*this = this->normalized();
}
float vec2::length(void) const {
	return sqrtf(x*x + y*y);
}
float vec2::length2(void) const {
	return x*x + y*y;
}
