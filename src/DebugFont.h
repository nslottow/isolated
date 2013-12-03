#ifndef DEBUG_FONT_H
#define DEBUG_FONT_H

#include <GLFW/glfw3.h>
#include <memory>

class DebugFont {
private:
	GLuint mTexture;

public:
	static const int kTextureWidth = 128;
	static const int kTextureHeight = 64;
	static const int kGlyphWidth = 7;
	static const int kGlyphHeight = 9;
	static const int kGlyphsX = 18;
	static const int kGlyphsY = 6;
	static const int kSpacesPerTab = 4;

	DebugFont();

	~DebugFont();

	void renderString(const char* str, bool centered = false);
};

extern std::shared_ptr<DebugFont> gDebugFont;

#endif