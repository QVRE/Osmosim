#pragma once
#include <cmath>
#include <cstdint>


struct vec2 {
	float x, y;
	
	vec2(void) : x(0), y(0) {}
	vec2(float x, float y) : x(x), y(y) {}
	vec2(float z) : x(z), y(z) {}
	
	vec2 operator+(const vec2& other) const;
	vec2 operator-(const vec2& other) const;
	vec2 operator-(void) const;
	vec2& operator+=(const vec2& other);
	vec2& operator-=(const vec2& other);
	vec2 operator*(const vec2& other) const;
	vec2& operator*=(const vec2& other);
	vec2 operator/(const vec2& other) const;
	vec2& operator/=(const vec2& other);
	
	vec2 operator+(const float& a) const; //add value to both
	vec2 operator-(const float& a) const;
	vec2 operator*(const float& s) const; //scalar
	
	float dot(const vec2& other) const;
	vec2 normalized(void) const;
	void normalize(void);
	float length(void) const;
	float length2(void) const;
};


inline float rand_float(float a = 0, float b = 1) {
	return a + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (b-a)));
}
