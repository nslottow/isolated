#ifndef VECTOR_H

#include <cmath>

struct Vec2 {
	float x, y;

	Vec2()
		: x(0.f), y(0.f) {}

	Vec2(float s)
		: x(s), y(s) {}

	Vec2(float x, float y)
		: x(x), y(y) {}

	Vec2 operator+(const Vec2& v) const {
		return Vec2(x + v.x, y + v.y);
	}

	Vec2 operator-(const Vec2& v) const {
		return Vec2(x - v.x, y - v.y);
	}

	Vec2 operator*(const float s) const {
		return Vec2(x * s, y * s);
	}

	Vec2 operator/(const float s) const {
		return Vec2(x / s, y / s);
	}

	Vec2& operator+=(const Vec2& v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	Vec2& operator-=(const Vec2& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vec2& operator*=(const float s) {
		x *= s;
		y *= s;
		return *this;
	}

	Vec2& operator/=(const float s) {
		x /= s;
		y /= s;
		return *this;
	}

	float sqrDistance(const Vec2& a, const Vec2& b) {
		auto dx = b.x - a.x;
		auto dy = b.y - a.y;
		return dx * dx + dy * dy;
	}

	float distance(const Vec2& a, const Vec2& b) {
		return std::sqrtf(sqrDistance(a, b));
	}
};

inline Vec2 operator*(const float s, const Vec2& v) {
	return v * s;
}

inline Vec2 operator/(const float s, const Vec2& v) {
	return v / s;
}

#endif
