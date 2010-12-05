//------------------------------------------------------------------------------
//
//	graphics.h
// 
//------------------------------------------------------------------------------

#ifndef __GRAPHICS__
#define __GRAPHICS__

#include <GLUT/glut.h>

#define DEFAULT_WINDOW_WIDTH 960
#define DEFAULT_WINDOW_HEIGHT 600

#define TEXT_BUFFER_SIZE 512
#define GLUT_FONT GLUT_BITMAP_HELVETICA_18


//-----------------------------------------------------------------------------
// Class definitions
//-----------------------------------------------------------------------------

class GLUT
{

public:

	// Constructor
	GLUT( int argc, char ** argv );
	
	void drawRect( float x, float y, float length, float height, const float rgba[4] );

	void print( float x, float y, const char * text, const float rgba[4] );

//	static void print( float x, float y, char* text, float r, float g, float b, float a );


private: 


	// Callback function invoked to draw the client area
	static void displayFunc();

	// Callback from GLUT
	static void idleFunc();

	// Called when window size changes
	static void reshapeFunc( GLsizei w, GLsizei h );

	// Handles key events
	static void keyboardFunc( unsigned char key, int x, int y );
	static void keyboardUpFunc( unsigned char key, int x, int y );
	
	static void specialFunc( int key, int x, int y );
	static void specialUpFunc( int key, int x, int y );

};


#endif 

