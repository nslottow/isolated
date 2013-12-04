#include "DebugFont.h"
#include "Vector.h"
#include <soil/SOIL.h>
#include <iostream>

using namespace std;

shared_ptr<DebugFont> gDebugFont;

DebugFont::DebugFont() {
	mTexture = SOIL_load_OGL_texture("data/DebugFont7x9.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y | SOIL_FLAG_MULTIPLY_ALPHA);
	if (!mTexture) {
		cout << "Failed to load debug font" << endl;
	}

	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

DebugFont::~DebugFont() {
	glDeleteTextures(1, &mTexture);
}

Vec2 getStringDimensions(const char* str) {
	int glyphsX = 0;
	int glyphsY = 1;

	for (; *str; ++str) {
		if (*str == '\t') {
			glyphsX += DebugFont::kSpacesPerTab;
		} else if (*str == '\n') {
			++glyphsY;
		} else {
			++glyphsX;
		}
	}

	return Vec2((float)glyphsX * DebugFont::kGlyphWidth, (float)glyphsY * DebugFont::kGlyphHeight);
}

void DebugFont::renderString(const char* str, bool centerHorizontal) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	if (centerHorizontal) {
		auto dimensions = getStringDimensions(str);
		glPushMatrix();
		glTranslatef(-dimensions.x / 2.f, 0.f, 0.f);
	}

	// Render each character as a quad
	glBegin(GL_QUADS);

	float currentX = 0.f;
	float currentY = 0.f; // TODO: make this a parameter

	for (; *str; ++str) {
		if (*str == '\t') {
			// Render as 4 spaces
			currentX += kGlyphWidth * kSpacesPerTab;
			continue;
		} else if (*str == '\n') {
			// Move to beginning of next line
			currentX = 0.f;
			currentY -= kGlyphHeight;
			continue;
		} else if (*str > '~' || *str < '!') {
			// Render as space
			currentX += kGlyphWidth;
			continue;
		}

		int glyphIndex = *str - '!';
		
		int glyphX = glyphIndex % kGlyphsX;
		int glyphY = glyphIndex / kGlyphsX;
		float s1 = (float)(glyphX * kGlyphWidth) / kTextureWidth;
		float s2 = (float)((glyphX + 1) * kGlyphWidth) / kTextureWidth;
		float t1 = (float)(glyphY * kGlyphHeight) / kTextureHeight;
		float t2 = (float)((glyphY + 1) * kGlyphHeight) / kTextureHeight;

		glTexCoord2f(s1, t1); glVertex2f(currentX, currentY);
		glTexCoord2f(s2, t1); glVertex2f(currentX + kGlyphWidth, currentY);
		glTexCoord2f(s2, t2); glVertex2f(currentX + kGlyphWidth, currentY + kGlyphHeight);
		glTexCoord2f(s1, t2); glVertex2f(currentX, currentY + kGlyphHeight);

		currentX += kGlyphWidth;
	}
	
	glEnd();
	
	if (centerHorizontal) {
		glPopMatrix();
	}

	glDisable(GL_TEXTURE_2D);
}