//------------------------------------------------------------------------------
//
//	interface.h
// 
//------------------------------------------------------------------------------

#ifndef __KEYBOARD__
#define __KEYBOARD__

#define KEY_HEIGHT .7f
#define KEY_WIDTH 1
#define SPACE_HEIGHT .1f
#define SPACE_WIDTH .2f


//------------------------------------------------------------------------------
//
// class Keyboard
// 
//------------------------------------------------------------------------------

class Keyboard
{

public:

	Keyboard(); // Constructor

	// Called from the GLUT keyboard callbacks
	void key( unsigned char key, bool keyDown, int mod, int x, int y);

	void drawKeyboard( float x, float y );

protected:

	// TODO: Enumerate these modes
	bool m_selectCut, m_putCut, m_deleteCut, m_exit;

	bool m_fx;

	bool m_showKeyboard;

};


class Mouse
{

public:

	Mouse();

	static void motion( int x, int y );

	static void button( int button, int state, int x, int y );

	static Cut m_noise; // For scratching sound effects

protected:

	static float m_initMouseX, m_initMouseY;

};

#endif
