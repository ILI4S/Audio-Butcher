//------------------------------------------------------------------------------
//
//	interface.cpp
// 
//------------------------------------------------------------------------------

#include "defs.h"
#include "blade.h"
#include "board.h" 
#include "kitchen.h"
#include "interface.h"
#include "graphics.h"
#include "rgbaColors.h"

#include <string>
#include <GLUT/glut.h>
#include <iostream>

using namespace std;


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

extern Kitchen g_kitchen;
extern GLUT * g_glut;

extern GLsizei g_width;
extern GLsizei g_height;

Keyboard g_keyboard;
Mouse g_mouse;

// Maps each qwerty key to the alphabet
const unsigned int g_keyboard_table[] = { 16, 22, 4, 17, 19, 24, 20, 8, 14, 15,
										  0, 18, 3, 5, 6, 7, 9, 10, 11,
										  25, 23, 2, 21, 1, 13, 12 };


//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

unsigned char getBufferId( unsigned char key );
void drawRow( float x, float y, unsigned int n, unsigned int *n_square );
//void drawKey( float x, float y, float length, float height, unsigned int *n );


//------------------------------------------------------------------------------
//
// class Keyboard
// 
//------------------------------------------------------------------------------

Keyboard::Keyboard()
{
	m_selectCut = false;
	m_putCut = false;
	m_deleteCut = false;	
	m_showKeyboard = true;
	m_fx = false;
	m_exit = false;

	m_once = false;

	m_playFromMark = 0;
}


//------------------------------------------------------------------------------
// Keyboard::key( )
// Handles keypresses. Is called from GLUT keyboard callbacks
//------------------------------------------------------------------------------

unsigned int getMarkN( unsigned int key )
{
	if ( key == 33 ) return 1;
	if ( key == 64 ) return 2;
	if ( key == 35 ) return 3;
	if ( key == 36 ) return 4;
	if ( key == 37 ) return 5;
	if ( key == 94 ) return 6;
	if ( key == 38 ) return 7;
	if ( key == 42 ) return 8;
	if ( key == 40 ) return 9;
}

void Keyboard::key( unsigned char key, bool keyDown, int mod, int x, int y)
{
	cout << "Keypress: " << (int) key << endl; // DEBUG

	// Select cut mode
	// Load next keypress buffer to cutting board
	if ( m_selectCut && keyDown )
	{
		unsigned int n_buffer = getBufferId( key );

		if ( n_buffer != 99 && g_kitchen.m_cuts[ n_buffer ].m_cutSize )
			g_kitchen.m_board.select( &g_kitchen.m_cuts[ n_buffer ] );
	}
	else if ( m_putCut && keyDown )
	{
		unsigned int n_buffer = getBufferId( key );
	
		if ( n_buffer != 99 )
			g_kitchen.m_board.put( &g_kitchen.m_cuts[ n_buffer ] );

		m_putCut = false;
	}
	else if ( m_deleteCut && keyDown )
	{
		unsigned int n_buffer = getBufferId( key );

		if ( n_buffer != 99 )
		{
			g_kitchen.m_cuts[ n_buffer ].clear();
		}
	}
	else if ( m_exit && keyDown && key == 27 ) exit(0); // Second consecutive ESC press

	// Exiting requires two consecutive ESC presses
	m_exit = false;

	// Lowercase Alpha Keys
	// Playback stored cuts
	if ( key >= 97 && key <= 122 ) 
	{
		unsigned int n_buffer = getBufferId( key );
	
		if ( keyDown )
			g_kitchen.m_blade.startServe( n_buffer, mod == GLUT_ACTIVE_ALT, m_once, m_loop, m_playFromMark );
			
		else // Key up
		{
			g_kitchen.m_blade.stopServe( n_buffer, mod == GLUT_ACTIVE_ALT );

			// In case shift was put down after recording
			if ( g_kitchen.m_cuts[ n_buffer ].m_writeOn )			
				g_kitchen.m_blade.stopCut( n_buffer ); 
		}
	}	

	// Uppercase Alpha Keys
	// Record cuts
	else if ( key >= 65 && key <= 90 )
	{
		unsigned int buffer_n = getBufferId( key );

		if ( keyDown )
			g_kitchen.m_blade.startCut( buffer_n );

		else // Key up
		{
			g_kitchen.m_blade.stopCut( buffer_n );

			// In case shift was hit after playing
			if ( g_kitchen.m_cuts[ buffer_n ].m_readOn 
				 && !g_kitchen.m_cuts[ buffer_n ].m_loop 			
				 && !g_kitchen.m_cuts[ buffer_n ].m_once )			
				g_kitchen.m_blade.stopServe( buffer_n, mod == GLUT_ACTIVE_ALT ); 
		}
	}

	// 0 - 10
	// Jump to read mark if it exists, for playing buffers that are not looped
	else if ( (key >= 48 && key <= 57) && keyDown )
	{
		bool jumpMark = false;

		for ( int i = 0; i < 26; i++ )
		{
			Cut * cut = &g_kitchen.m_cuts[i];

			if ( cut->m_readOn && !cut->m_loop )
			{
				jumpMark = true;
				cut->gotoMark( key - 48 );
			}
		}

		// If no samples are being played back, jump to the mark on the cutting board
		if ( !jumpMark && g_kitchen.m_board.m_cut )
			g_kitchen.m_board.m_cut->gotoMark( key - 48 );

		if ( keyDown ) m_playFromMark = key - 48;
	}
	else if ( key >= 48 && key <= 57 ) // Key up
	{
		if ( m_playFromMark == key - 48 ) m_playFromMark = 0;
	}

	// ! @ # $ % ^ & * (
	// Set mark 1, 2, 3, 4, 5, 6, 7, 8, or 9 
	else if ( ( key == 33 || key == 64 || key == 35 || key == 36 || key == 37 || key == 94 ||
			    key == 38 || key == 42 || key == 40 ) && keyDown ) 
	{
		unsigned int mark_n = getMarkN( key );
		bool setMark = false;

		for ( int i = 0; i < 26; i++ )
		{
			Cut * cut = &g_kitchen.m_cuts[i];

			if ( cut->m_readOn && !cut->m_loop )
			{
				cut->setMark( mark_n );
				setMark = true;
			}
		}
	
		// If no samples are being played back to be marked, try marking the cutting board sample
		if ( !setMark )
			g_kitchen.m_board.setMark( mark_n );
	}

	// Shift-Minus
	// Loads ingredients from default folder 
	else if ( key == 95 && keyDown )
		g_kitchen.thawIngredients( DEFAULT_INGREDIENTS_DIR );

	// Shift-Plus
	// Freeze all current ingredients
	else if ( key == 43 && keyDown )
		g_kitchen.freezeIngredients( DEFAULT_INGREDIENTS_DIR );

	
	// - Decrease gain 
	else if ( key == 45 && keyDown ) //g_kitchen.m_board.adjustGain( DECREASE );
	{
		for ( int i = 0; i < 26; i++ ) // TODO: make this iterate over ALL playing samples
		{
			Cut * cut = &g_kitchen.m_cuts[i];

			// Modify currently playing samples that are not being looped
			if ( cut->m_readOn && !cut->m_loop ) cut->m_volumeCoef -= GAIN_INC;
		}
	}

	// + Increase gain on cutting board
	else if ( key == 61 && keyDown ) //g_kitchen.m_board.adjustGain( INCREASE );
	{
		for ( int i = 0; i < 26; i++ ) // TODO: make this iterate over ALL playing samples
		{
			Cut * cut = &g_kitchen.m_cuts[i];

			// Modify currently playing samples that are not being looped
			if ( cut->m_readOn && !cut->m_loop ) cut->m_volumeCoef += GAIN_INC;
		}
	}
	
	// [ Shorten the start of the cut on the cutting board
	else if ( key == 91 && keyDown ) 
		if ( mod == GLUT_ACTIVE_ALT ) g_kitchen.m_board.chop( LEFT );
		else g_kitchen.m_board.resize( SHORTEN, START );

	// { Lengthen the start of the cut on the cutting board
	else if ( key == 123 && keyDown ) g_kitchen.m_board.resize( EXTEND, START );

	// ] Shorten the length of the cut on the cutting board
	else if ( key == 93 && keyDown ) 
		if ( mod == GLUT_ACTIVE_ALT ) g_kitchen.m_board.chop( RIGHT );
		else g_kitchen.m_board.resize( SHORTEN, END );

	// } Lengthen the end of the cut on the cutting board
	else if ( key == 125 && keyDown ) g_kitchen.m_board.resize( EXTEND, END );

	// , Pitch shift down
	else if ( key == 44 && keyDown ) g_kitchen.m_board.shiftPitch( false );

	// . Pitch shift up
	else if ( key == 46 && keyDown ) g_kitchen.m_board.shiftPitch( true );



	// [space] play the cut on the board
//	else if ( key == 32 && keyDown ) g_kitchen.m_board.playLoopToggle();

	// \ to toggle the keyboard display
	else if ( key == 92 && keyDown ) m_showKeyboard = !m_showKeyboard;

	// LEFT to move the cutting board cursor left
	else if ( key == 200 )
	{
		if ( keyDown ) g_kitchen.m_board.adjustCursor( LEFT );
		else g_kitchen.m_board.adjustCursor( STOP );
	}

	// RIGHT to move the cutting board cursor right
	else if ( key == 202 )
	{
		if ( keyDown ) g_kitchen.m_board.adjustCursor( RIGHT );
		else g_kitchen.m_board.adjustCursor( STOP );
	}


	// ` Select a cut for the cutting board
	else if ( key == 96 ) m_selectCut = keyDown;

	// ~ Put the cut from the board
	else if ( key == 126 && keyDown ) m_putCut = true;

	// DEL Delete a cut
	else if ( key == 127 ) m_deleteCut = keyDown;

	// Enter to play a sample once
	else if ( key == 13 ) 
	{
		m_once = keyDown;

		// Play once any samples that are already going (but that are not looping)
		if ( keyDown )
		{
			for ( int i = 0; i < 26; i++ )
			{
				if ( g_kitchen.m_cuts[ i ].m_readOn && !g_kitchen.m_cuts[ i ].m_loop )
					g_kitchen.m_cuts[ i ].m_once = true;
			}
		}
	}

	// Space to loop it
	else if ( key == 32 ) 
	{
		m_loop = keyDown;

		// Loop any samples that are already going (but that are not just playing out once)
		if ( keyDown )
		{
			for ( int i = 0; i < 26; i++ )
			{
				if ( g_kitchen.m_cuts[ i ].m_readOn && !g_kitchen.m_cuts[ i ].m_once )
					g_kitchen.m_cuts[ i ].m_loop = true;
			}
		}
	} 


	// Escape Key 
	else if ( key == 27 ) 
	{
		// Save ingredients
		if ( keyDown ) g_kitchen.freezeIngredients( WORKING_DIR );
		m_exit = true; // If pressed twice, quit
	} 
}


unsigned char getBufferId( unsigned char key )
{
	if ( key >= 97 && key <= 122 ) // Lowercase keys
		return key - 97;

	else if ( key >= 65 && key <= 90 ) // Uppercase keys
		return key - 65;

	return 99; // TODO: make this return something more meaningful
}





//-----------------------------------------------------------------------------
// Keyboard::drawKeyboard( )
// 
//-----------------------------------------------------------------------------

void Keyboard::drawKeyboard( float x, float y )
{
	x = BOARD_X + .5; y = BOARD_Y - .8;

	if ( !m_showKeyboard ) return; 

	unsigned int n_square = 0; // number off each square to refer against the table
	
	drawRow( x, y / 2, 10, &n_square );
	drawRow( x + .3f, y / 2 - (KEY_HEIGHT + SPACE_HEIGHT), 9, &n_square );
	drawRow( x + .6f, y / 2 - 2 * (KEY_HEIGHT + SPACE_HEIGHT), 7, &n_square );
}


void drawKey( float x, float y, unsigned int *n )
{
	unsigned int bufferIndex = g_keyboard_table[ (*n)++ ];
	
	// Empty buffer color (grey)
	float *color = (float *) &g_grey_rgba;
	
	// Writing (red)
	if ( g_kitchen.m_cuts[ bufferIndex ].m_writeOn )
		color = (float *) &g_grey_rgba;

	// Playing back (blue)
	else if ( g_kitchen.m_cuts[ bufferIndex ].m_readOn &&
			  g_kitchen.m_cuts[ bufferIndex ].m_cutSize )
		color = (float *) &g_blue_rgba;

	// Trying to playback without buffer (black)
	else if ( g_kitchen.m_cuts[ bufferIndex ].m_readOn )
		color = (float *) &g_black_rgba;

	// Buffer loaded (white)
	else if ( g_kitchen.m_cuts[ bufferIndex ].m_cutSize )
		color = (float *) &g_white_rgba;

	g_glut->drawRect( x, y, KEY_WIDTH, KEY_HEIGHT, color );
	
	char letter[2] = { (char) bufferIndex + 65, NULL };

	g_glut->print( x + .1f, y + .1f, (char *) &letter, g_black_rgba );
}


void drawRow( float x, float y, unsigned int n, unsigned int *n_square )
{
	for ( int i = 0; i < n; i++ )
	{
		drawKey( x + i * (KEY_WIDTH + SPACE_WIDTH), y, n_square );
	}
}


//------------------------------------------------------------------------------
//
// class Mouse
// 
//------------------------------------------------------------------------------

bool Mouse::m_showMouse;
float Mouse::m_x, Mouse::m_y;
float Mouse::m_initMouseX;
float Mouse::m_initMouseY;
Cut Mouse::m_noise;
float Mouse::m_playbackSpeed;


Mouse::Mouse()
{
	m_initMouseX = m_initMouseY = 1000; // TODO: Make this values more meaningful

	// Fill noise buffer with white noise
	srand( time ( 0 ) ); // seed random function
	for ( int i = 0; i < CUT_BUFFER_SIZE; i++ )
		m_noise.writeTick( 10 * rand() / float(RAND_MAX + 1.0) );
	m_noise.m_cutSize = CUT_BUFFER_SIZE;
}


//-----------------------------------------------------------------------------
// Mouse::motion( )
// 
//-----------------------------------------------------------------------------

void Mouse::motion( int x, int y )
{ 
	// Set x and y values within bounds
	if ( y < 0 ) y = 0;
	else if ( y > g_height ) y = g_height;

	if ( x < 0 ) x = 0;
	else if ( x > g_width ) x = g_width;

	// Set inital position and display the mouse if it was hidden and unused
	if ( !m_showMouse )
	{
		m_initMouseX = x; 
		m_initMouseY = y;
	}

	// Update class position
	m_x = x; m_y = y;

	// TODO: check that the mouse has moved beyond an intitial threshold
			
	// TODO: finish implement scratching here
	if ( x < m_initMouseX ) 
	{
		float xRatio = (float) x / (float) m_initMouseX;
		float targetPosition = m_noise.m_cutSize - SCRATCH_DISTANCE * xRatio;
		float position = m_noise.m_readHead;


		if ( m_noise.m_readHead + m_noise.m_cutSize < targetPosition )
		{
			//m_noise.m_readOn = true;
			//m_noise.m_playbackSpeedTarget = 1;
		}
		else if ( m_noise.m_readHead > targetPosition )
		{
			//m_noise.m_readOn = true;
			//m_noise.m_playbackSpeedTarget = -1;
		}
		else m_noise.m_readOn = false;
	}
	else if ( x > m_initMouseX  ) 
	{
		float xRatio = (float) (x - m_initMouseX) / (float) (g_width - m_initMouseX);
		float targetPosition = SCRATCH_DISTANCE * xRatio;
	}
	else
	{
		float targetPosition = 0;
	}
			

	// Change playback speed by mousing up and down

	for ( int i = 0; i < 26; i++ )
	{
		Cut * cut = &g_kitchen.m_cuts[i];

		// Apply to every sample that is not being looped

		if ( cut->m_readOn && !cut->m_loop )
		{
			// Only show values if there is a buffer being modified
			m_showMouse = true;

			if ( y < m_initMouseY ) // Slow down playback
			{
				float yRatio = (float) y / (float) m_initMouseY;
				float target = PLAYBACK_SPEED_DEC * yRatio + 1 - PLAYBACK_SPEED_DEC;
				cut->m_playbackSpeed = target;
			}
	
			else if ( y > m_initMouseY  ) // Speed up playback
			{
				float yRatio = (float) (y - m_initMouseY) / (float) (g_height - m_initMouseY);
				float target = 1 + PLAYBACK_SPEED_INC * yRatio;
				cut->m_playbackSpeed = target;
			}

			else cut->m_playbackSpeed = 1.0f; 

			m_playbackSpeed = cut->m_playbackSpeed;
		}
	}
}


//-----------------------------------------------------------------------------
// Mouse::button( )
// 
//-----------------------------------------------------------------------------

void Mouse::button( int button, int state, int x, int y )
{
	m_initMouseX = m_initMouseY = 1000; // TODO: Make this values more meaningful
	m_showMouse = false;

	// Reset playback speeds for all other samples	
    if ( button == GLUT_RIGHT_BUTTON )
	{
		for ( int i = 0; i < 26; i++ )
		{
			Cut * cut = &g_kitchen.m_cuts[i];
			if ( cut->m_readOn ) cut->m_playbackSpeed = 1.0f;
		}
	}

	m_noise.m_readOn = false;
	m_noise.m_readHead = 0;
}


//-----------------------------------------------------------------------------
// Mouse::drawMouse( )
// 
//-----------------------------------------------------------------------------

void Mouse::draw( )
{
	if ( !m_showMouse ) return;

    GLdouble modelMatrix[16];
    glGetDoublev( GL_MODELVIEW_MATRIX, modelMatrix );
    GLdouble projMatrix[16];
    glGetDoublev( GL_PROJECTION_MATRIX, projMatrix );
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);

	double pos[3];

    gluUnProject( m_x, m_y, 0, modelMatrix, projMatrix, viewport, &pos[0], &pos[1], &pos[2] );

	pos[0] *= 10;
	pos[1] *= -10;

	g_glut->drawRect( pos[0], pos[1], 1, .5, (float *) &g_black_rgba );

	char speedStr[5];
	snprintf( speedStr, 5, "%f", m_playbackSpeed );

	g_glut->print( pos[0] + .4, pos[1] + .2, speedStr, (float *) &g_white_rgba );
}

