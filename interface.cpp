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
void drawKey( float x, float y, float length, float height, unsigned int *n );


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
}


//------------------------------------------------------------------------------
// Keyboard::key( )
// Handles keypresses. Is called from GLUT keyboard callbacks
//------------------------------------------------------------------------------

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
	else if ( m_exit && keyDown && key == 27 ) exit(0);

	// Exiting requires two consecutive ESC presses
	m_exit = false;

	// Lowercase Alpha Keys
	// Playback stored cuts
	if ( key >= 97 && key <= 122 ) 
	{
		unsigned int n_buffer = getBufferId( key );
	
		if ( keyDown )
		{
			if ( m_fx ) g_kitchen.m_cuts[ n_buffer ].addFx( NULL );
			g_kitchen.m_blade.startServe( n_buffer, mod == GLUT_ACTIVE_ALT, x, y );
		}
			
		else // Key up
		{
			g_kitchen.m_blade.stopServe( n_buffer, mod == GLUT_ACTIVE_ALT );
			
			g_kitchen.m_blade.stopCut( n_buffer ); // In case shift was put down after recording
		}
	}	

	// Uppercase Alpha Keys
	// Record cuts
	else if ( key >= 65 && key <= 90 )
	{
		unsigned int n_buffer = getBufferId( key );

		if ( keyDown )
			g_kitchen.m_blade.startCut( n_buffer );

		else // Key up
			g_kitchen.m_blade.stopCut( n_buffer );
	}

	// Shift-Minus
	// Loads ingredients from default folder 
	else if ( key == 95 && keyDown )
		g_kitchen.thawIngredients( DEFAULT_INGREDIENTS_DIR );

	// Shift-Plus
	// Freeze all current ingredients
	else if ( key == 43 && keyDown )
		g_kitchen.freezeIngredients( DEFAULT_INGREDIENTS_DIR );

	
	// - Decrease gain on cutting board
	else if ( key == 45 && keyDown ) g_kitchen.m_board.adjustGain( DECREASE );

	// + Increase gain on cutting board
	else if ( key == 61 && keyDown ) g_kitchen.m_board.adjustGain( INCREASE );


	
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
	else if ( key == 32 && keyDown ) g_kitchen.m_board.playLoopToggle();

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


	else if ( key == 39 ) m_fx = true;


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
	
	// Empty buffer color
	float rgb[] = { .5, .5, .5, 1 };

	// Writing (red)
	if ( g_kitchen.m_cuts[ bufferIndex ].m_writeOn )
	{
		rgb[0] = 1; rgb[1] = 0; rgb[2] = 0;
	}

	// Playing back (blue)
	else if ( g_kitchen.m_cuts[ bufferIndex ].m_readOn &&
			  g_kitchen.m_cuts[ bufferIndex ].m_cutSize )
	{
		rgb[0] = 0; rgb[1] = 0; rgb[2] = .8f;
	}

	// Trying to playback without buffer
	else if ( g_kitchen.m_cuts[ bufferIndex ].m_readOn )
	{
		rgb[0] = 0; rgb[1] = 0; rgb[2] = 0;
	}	

	// Buffer loaded
	else if ( g_kitchen.m_cuts[ bufferIndex ].m_cutSize )
	{
		rgb[0] = 1; rgb[1] = 1; rgb[2] = 1; rgb[3] = .2f;
	}

	g_glut->drawRect( x, y, KEY_WIDTH, KEY_HEIGHT, rgb );
	
	char letter[2] = { (char) bufferIndex + 65, NULL };

	g_glut->print( x + .1f, y + .1f, (char *) &letter, 0.8f, 0.8f, 0.8f, 1.0f );
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

float Mouse::m_initMouseX;
float Mouse::m_initMouseY;
Cut Mouse::m_noise;

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
	if ( y < 0 ) y = 0;
	else if ( y > g_height ) y = g_height;

	if ( x < 0 ) x = 0;
	else if ( x > g_width ) x = g_width;

	// TODO: make these values more meaningful
	if ( m_initMouseX == 1000 && m_initMouseY == 1000 )
	{
		m_initMouseX = x; m_initMouseY = y;
	}

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
			

	for ( int i = 0; i < 26; i++ )
	{
		Cut * cut = &g_kitchen.m_cuts[i];

		if ( cut->m_readOn )
		{
			// X motion currently unused
			// TODO: use x motion to scratch a white noise buffer
			// float xRatio = ( m_initMouseX - x ) / ;
	
		//	else yRatio = 
			if ( y < m_initMouseY ) // Slow down playback
			{
				float yRatio = (float) y / (float) m_initMouseY;
				float target = PLAYBACK_SPEED_DEC * yRatio + 1 - PLAYBACK_SPEED_DEC;
				cut->m_playbackSpeedTarget = target;
			}
	
			else if ( y > m_initMouseY  ) // Speed up playback
			{
				float yRatio = (float) (y - m_initMouseY) / (float) (g_height - m_initMouseY);
				float target = 1 + PLAYBACK_SPEED_INC * yRatio;
				cut->m_playbackSpeedTarget = target;
			}

			else cut->m_playbackSpeedTarget = 1.0f; 


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
	
	for ( int i = 0; i < 26; i++ )
	{
		Cut * cut = &g_kitchen.m_cuts[i];

		if ( cut->m_readOn ) cut->m_playbackSpeedTarget = 1.0f;
	}

	m_noise.m_readOn = false;
	m_noise.m_readHead = 0;
}


