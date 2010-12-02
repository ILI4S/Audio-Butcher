//------------------------------------------------------------------------------
//
//	graphics.cpp
// 
//------------------------------------------------------------------------------

#include "blade.h"
#include "board.h"
#include "kitchen.h"
#include "interface.h"
#include "graphics.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

extern Kitchen g_kitchen;
extern Keyboard g_keyboard;
extern Mouse g_mouse;

GLsizei g_width = DEFAULT_WINDOW_WIDTH;
GLsizei g_height = DEFAULT_WINDOW_HEIGHT;


//-----------------------------------------------------------------------------
// Static variable definitions
//-----------------------------------------------------------------------------

// char GLUT::m_statusTxt[ TEXT_BUFFER_SIZE ];


//-----------------------------------------------------------------------------
// GLUT::drawRect()
// 
//-----------------------------------------------------------------------------

void GLUT::drawRect( float x, float y, float length, float height, float * rgb )
{
	glColor4f( rgb[0], rgb[1], rgb[2], rgb[3] );

	glRectf( x, 			y, 
			 x + length, 	y + height ); 
}


//-----------------------------------------------------------------------------
// GLUT::displayFunc()
// Callback function invoked to draw the client area
//-----------------------------------------------------------------------------

void GLUT::displayFunc()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the color and depth buffers

//	print( 1.0f, 1.0f, m_statusTxt, 0.8f, 0.8f, 0.8f, 1.0f );

	g_kitchen.m_board.draw();

	g_keyboard.drawKeyboard( -6.35, 2 );
//	g_keyboard.drawKeyboard( -6.35, 2 );

    glutSwapBuffers(); // Swap double buffer
}


//-----------------------------------------------------------------------------
// GLUT::print()
// 
//-----------------------------------------------------------------------------

void GLUT::print( float x, float y, char* text, float r, float g, float b, float a ) 
{ 
    if ( !text || !strlen(text) ) return; 

    glColor4f( r, g, b, a ); 
    glRasterPos2f( x, y ); 
    while ( *text ) { 
        glutBitmapCharacter( GLUT_FONT, *text ); 
        text++; 
    } 
}  


//-----------------------------------------------------------------------------
// GLUT::GLUT( )
// Constructor. Initializes GLUT window
//-----------------------------------------------------------------------------


GLUT::GLUT( int argc, char ** argv )
{
    // initialize GLUT
    glutInit( &argc, argv );
    // double buffer, use rgb color, enable depth buffer
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    // initialize the window size
    glutInitWindowSize( g_width, g_height );
    // set the window postion
    glutInitWindowPosition( 100, 100 );
    // create the window
    glutCreateWindow( "Audio Butcher" );
    
    // set the idle function - called when idle
    glutIdleFunc( idleFunc );
    // set the display function - called when redrawing
    glutDisplayFunc( displayFunc );
    // set the reshape function - called when client area changes
    glutReshapeFunc( reshapeFunc );

	glutIgnoreKeyRepeat( true ); // hmm..

    glutKeyboardFunc( keyboardFunc ); // set the keyboard function - called on keyboard events
	glutKeyboardUpFunc( keyboardUpFunc );

    // set the mouse function - called on mouse stuff
	glutMotionFunc( g_mouse.motion ); 
    glutMouseFunc( g_mouse.button );

	glutSpecialFunc( specialFunc );
	glutSpecialUpFunc( specialUpFunc );

	// these blending functions don't work...
	glEnable(GL_BLEND);
	// glBlendFunc (GL_ONE, GL_ONE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);

//	strcpy( m_statusTxt, " " ); 	

}


//-----------------------------------------------------------------------------
// GLUT::reshapeFunc( )
// Called when window size changes
//-----------------------------------------------------------------------------

void GLUT::reshapeFunc( GLsizei w, GLsizei h )
{
    // save the new window size
    g_width = w; g_height = h;
    // map the view port to the client area
    glViewport( 0, 0, w, h );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity();
    // create the viewing frustum
    gluPerspective( 45.0, (GLfloat) w / (GLfloat) h, 1.0, 300.0 );
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity();
    // position the view point
    gluLookAt( 0.0f, 0.0f, 10.0f, 
			   0.0f, 0.0f, 0.0f, 
			   0.0f, 1.0f, 0.0f );
}


//-----------------------------------------------------------------------------
// GLUT::keyboardFunc( )	GLUT::keyboardUpFunc( )
// Handle key events
//-----------------------------------------------------------------------------

void GLUT::keyboardFunc( unsigned char key, int x, int y )
{
	g_keyboard.key( key, true, glutGetModifiers(), x, y );	
    
    glutPostRedisplay();
}

void GLUT::keyboardUpFunc( unsigned char key, int x, int y )
{
	g_keyboard.key( key, false, glutGetModifiers(), x, y );

    glutPostRedisplay();
}


//-----------------------------------------------------------------------------
// GLUT::specialFunc()
// 
//-----------------------------------------------------------------------------

void GLUT::specialFunc( int key, int x, int y )
{
	g_keyboard.key( key + 100, true, x, y, glutGetModifiers() );

	glutPostRedisplay();
}

void GLUT::specialUpFunc( int key, int x, int y )
{
	g_keyboard.key( key + 100, false, x, y, glutGetModifiers() );

	glutPostRedisplay();
}


//-----------------------------------------------------------------------------
// GLUT::idleFunc()
// Callback from GLUT
//-----------------------------------------------------------------------------
void GLUT::idleFunc()
{
    // render the scene
    glutPostRedisplay();
}




