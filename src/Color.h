#ifndef COLOR_H
#define COLOR_H

struct Color {
	static const Color kWhite;

	float r, g, b, a;

	Color() {}

	Color(float r, float g, float b, float a) :
		r(r), g(g), b(b), a(a) {}

	Color operator*(const float s) {
		Color c = {r * s, g * s, b * s, a * s};
		return c;
	}

	Color& operator*=(const float s) {
		r *= s;
		g *= s;
		b *= s;
		a *= s;
		return *this;
	}
};

#endif
