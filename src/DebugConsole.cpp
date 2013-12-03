#include "DebugConsole.h"
#include "DebugFont.h"
#include <iostream>

using namespace std;

shared_ptr<AbstractDebugConsole> gConsole;

static const int kHistoryNumLines = 100;
static const int kMaxLineLength = 128;

AbstractDebugConsole::AbstractDebugConsole(int width, int height, int screenWidth, int screenHeight, int fontScale) :
	mWidth(width), mHeight(height),
	mScreenWidth(screenWidth), mScreenHeight(screenHeight),
	mFontScale(fontScale),
	mGlyphWidth(DebugFont::kGlyphWidth * fontScale), mGlyphHeight(DebugFont::kGlyphHeight * fontScale),
	mGlyphsPerLine(mWidth / mGlyphWidth),
	mNumLines(mHeight / mGlyphHeight),
	mSelectionStart(0), mSelectionEnd(0),
	mInputPosition(0), mInputLength(0),
	mOpen(false)
{
	mTextBuffer = new char[(mGlyphsPerLine + 1) * mNumLines + 1]; // Add 1 per line for \n characters + 1 for null-terminator
	mTextBuffer[0] = 0;
	mInputBuffer = new char[kMaxLineLength+1];
	mInputBuffer[0] = 0;

	mTextColor = { .8f, 1.f, 1.f, 0.7f };
	mBackgroundColor = { .1f, .1f, .1f, .5f };
	mCursorColor = { .3f, .3f, .3f, .3f };
}

AbstractDebugConsole::~AbstractDebugConsole() {
	delete[] mTextBuffer;
	delete[] mInputBuffer;
}

void AbstractDebugConsole::onCharacter(char ch) {
	if (ch == '\n') {
		onCommand(mInputBuffer);

		// Clear the input buffer
		mInputBuffer[0] = 0;
		mInputPosition = 0;
	} else if (ch == 8) {
		// Backspace
		if (mInputPosition > 0) {
			for (int i = mInputPosition - 1; i < kMaxLineLength; ++i) {
				mInputBuffer[i] = mInputBuffer[i + 1];
			}
			--mInputPosition;
		}
	} else if (mInputPosition < kMaxLineLength - 1) {
		// Printable ASCII character
		mInputBuffer[mInputPosition++] = ch;
		mInputBuffer[mInputPosition] = 0;
	}
}

void AbstractDebugConsole::onNavigationCharacter(int ch) {
}

void AbstractDebugConsole::print(const char* format, ...) {

}

void AbstractDebugConsole::selectText(float x1, float y1, float x2, float y2) {

}

void AbstractDebugConsole::copySelectionToClipboard() {

}

#if defined(DEBUG_CONSOLE_RENDER_METHOD_QUADS)

void DebugConsole::render() {
	if (!mOpen) {
		return;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0., mScreenWidth, 0., mScreenHeight, -1., 1.);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.f, (float)(mScreenHeight - mHeight), 0.f);

	// Render the background
	glBegin(GL_QUADS);
	glColor4fv((GLfloat*)&mBackgroundColor);
	glVertex2i(0, 0);
	glVertex2i(mWidth, 0);
	glVertex2i(mWidth, mHeight);
	glVertex2i(0, mHeight);

	glVertex2i(0, 0);
	glVertex2i(mWidth, 0);
	glVertex2i(mWidth, mGlyphHeight);
	glVertex2i(0, mGlyphHeight);

	// Render the cursor
	int x1 = mInputPosition * mGlyphWidth;
	int x2 = x1 + mGlyphWidth;

	glColor4fv((GLfloat*)&mCursorColor);
	glVertex2i(x1, 0);
	glVertex2i(x2, 0);
	glVertex2i(x2, mGlyphHeight);
	glVertex2i(x1, mGlyphHeight);
	glEnd();

	// TODO: Render the text buffer

	// Render the input line
	glColor4fv((GLfloat*)&mTextColor);
	glScalef((float)mFontScale, (float)mFontScale, 1.f);
	// TODO: Give the debug font a size state?
	gDebugFont->renderString(mInputBuffer);

	glPopMatrix();
	glDisable(GL_BLEND);
}

#else
#error No render method defined for DebugConsole
#endif