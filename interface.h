//------------------------------------------------------------------------------
//
//	interface.h
//	Audio Butcher Copyleft (C) 2010 Ilias Karim
// 
//------------------------------------------------------------------------------

#ifndef __KEYBOARD__
#define __KEYBOARD__

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

	bool m_once, m_loop;
	bool m_noSampleLooped, m_noSampleOnced;

	bool m_fx;

	bool m_showKeyboard;
	
	unsigned int m_playFromMark;

};


class Mouse
{

public:

	Mouse();

	static void motion( int x, int y );

	static void button( int button, int state, int x, int y );

	static Cut m_noise; // For scratching sound effects
	
	static void draw();

protected:

	static bool m_showMouse;
	
	static float m_x, m_y;	

	static float m_initMouseX, m_initMouseY;

	static float m_playbackSpeed;

};

#endif
