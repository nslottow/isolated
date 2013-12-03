#ifndef DEBUG_CONSOLE_H
#define DEBUG_CONSOLE_H

#include "Color.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

#define DEBUG_CONSOLE_RENDER_METHOD_QUADS

class DebugFont;

enum DebugConsoleNavigationCharacters {
	DEBUG_CONSOLE_HISTORY_PREVIOUS = -1,
	DEBUG_CONSOLE_HISTORY_NEXT = -2,
	DEBUG_CONSOLE_CURSOR_PREVIOUS = -3,
	DEBUG_CONSOLE_CURSOR_NEXT = -4,
	DEBUG_CONSOLE_PAGE_UP = -5,
	DEBUG_CONSOLE_PAGE_DOWN = -6
};

class AbstractDebugConsole {
protected:
	int mWidth;
	int mHeight;
	int mScreenWidth;
	int mScreenHeight;
	int mFontScale;
	int mGlyphWidth;
	int mGlyphHeight;
	int mGlyphsPerLine;
	int mNumLines;
	int mSelectionStart;
	int mSelectionEnd;
	int mInputPosition;
	int mInputLength;
	char *mTextBuffer;
	char *mInputBuffer;
	Color mTextColor;
	Color mBackgroundColor;
	Color mCursorColor;
	
	// TODO: History
	bool mOpen;

public:
	AbstractDebugConsole(const AbstractDebugConsole&) = delete;
	AbstractDebugConsole(AbstractDebugConsole&&) = delete;

	/**
	 * Note: The rendering context must be initialized before instantiating a console
	 */
	AbstractDebugConsole(int width, int height, int screenWidth, int screenHeight, int fontScale);

	virtual ~AbstractDebugConsole();

	bool isOpen() const { return mOpen; }
	void setOpen(bool open) { mOpen = open; }
	void toggleOpen() { mOpen = !mOpen; }

	/** Renders the console if it is open */
	virtual void render() = 0;

	/** Overload this method to handle commands entered into the console */
	virtual void onCommand(const char* command) {}

	/** Input the specified character into the text buffer */
	void onCharacter(char ch);

	/**
	 * Handles input of characters used for navigation around the console
	 * Navigation characters:
	 *   DEBUG_CONSOLE_HISTORY_PREVIOUS => Go to previous line in history
	 *   DEBUG_CONSOLE_HISTORY_NEXT => Go to next line in history
	 *   DEBUG_CONSOLE_CURSOR_PREVIOUS => Increment cursor
	 *   DEBUG_CONSOLE_CURSOR_NEXT => Decrement cursor
	 *   DEBUG_CONSOLE_PAGE_UP => Scroll the window
	 */
	void onNavigationCharacter(int ch);

	/** Prints text to the console using printf syntax */
	void print(const char* format, ...);

	/** Selects the region of text described by a mouse click and drag start and end position */
	void selectText(float x1, float y1, float x2, float y2);

	/** Copies the selected text to the operating system's clipboard */
	void copySelectionToClipboard();
};

extern std::shared_ptr<AbstractDebugConsole> gConsole;

#if defined(DEBUG_CONSOLE_RENDER_METHOD_QUADS)

/**
 * Uses immediate mode rendering
 */
class DebugConsole : public AbstractDebugConsole {
private:
	float mRenderPosX;
	float mRenderPosY;

	void renderChar(char ch);

public:
	DebugConsole(int width, int height, int screenWidth, int screenHeight, int fontScale = 1) :
		AbstractDebugConsole(width, height, screenWidth, screenHeight, fontScale) {}

	void render() override;
};

#elif defined(DEBUG_CONSOLE_RENDER_METHOD_VERTEX_ARRAYS)

/**
 * Uses vertex arrays for keeping track of text
 * Takes up ~256KB of memory
 */
class DebugConsole : public AbstractDebugConsole {
private:
	struct Vertex {
		float x, y;
		float t, s;
	};

	struct TextLine {
		U32 length;
		const char* text;
		Vertex* vertices;
	};

	TextLine* mLines;
	Vertex* mVertices;
};

#elif defined(DEBUG_CONSOLE_RENDER_METHOD_RENDER_TO_TEXTURE)

/**
* Renders text to a texture to render entire console by using one quad
*/
class DebugConsole : public AbstractDebugConsole {
};

#endif // DEBUG_CONSOLE_RENDER_METHOD

#endif
