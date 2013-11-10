#ifndef COLOR_H
#define COLOR_H

struct Color {
	float r, g, b, a;

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
