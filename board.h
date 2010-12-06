//------------------------------------------------------------------------------
//
//	board.h
//	Copyleft (C) 2010 Ilias Karim
//
// 	Implements cutting board class
//	 
//------------------------------------------------------------------------------

#ifndef __BOARD__
#define __BOARD__

#include "blade.h"
#include "rgbaColors.h"
#include <GLUT/glut.h>


//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

struct Spectre { float r; float g; float b; };

struct complex { float re; float im; };


//------------------------------------------------------------------------------
//
// class Spectrogram
// 
//------------------------------------------------------------------------------

#define SPECTROGRAM_ID 1 
#define SG_HEIGHT 512 // originally 1024
#define SG_WIDTH 512 
#define SG_FREQ_FACTOR 4
#define FFT_SIZE 4096

#define cmp_abs(x) ( sqrt( (x).re * (x).re + (x).im * (x).im ) )

#define FFT_FORWARD 1

class Spectrogram
{

public:

	Spectrogram(); // Constructor

	unsigned int generateTexture( Cut * cut );
	
	void draw( float x, float y, unsigned int tex_id );

private:

	unsigned int m_nextTexId;

	float m_display_width, m_display_height;

	float m_buffer[ FFT_SIZE ]; 
	float m_window[ FFT_SIZE ]; 
	int m_wnd_size, m_fft_size;

	float m_spectre[ SG_WIDTH ][ SG_HEIGHT ];
   // SpectroGram m_specgram;
};


//------------------------------------------------------------------------------
//
// class CuttingBoard
// 
//------------------------------------------------------------------------------

#define START false
#define END true
#define EXTEND true
#define SHORTEN false
#define INCREASE true
#define DECREASE false
#define STOP 0
#define LEFT 1
#define RIGHT 2
#define CURSOR_INCREMENT 1000

class CuttingBoard
{

public:
	
	CuttingBoard(); // Constructor

	// Redraws Board graphics every graphics cycle
	void draw();

	// Selects an alpha key buffer and places it on the coating board
	void select( Cut * cut );

	// Puts the cutting board audio buffer in an alpha key buffer
	void put( Cut * cut );

	// Rolls a combination loop and selects it onto the Board
	void rollLoop( bool record, bool recut );

	void setMark( unsigned int mark );

	void play( bool once, bool loop );

	//-------------------------------------------------------------------------
	// Processing / Modification
	//-------------------------------------------------------------------------

	void resize( bool extend, bool end );

	void adjustGain( bool increase );

	void shiftPitch( bool increase );

	//-------------------------------------------------------------------------
	// Cursor Options
	//-------------------------------------------------------------------------

	void adjustCursor( int direction );

	void chop( int direction ); 


public:

	Spectrogram * m_spectrogram;

	Cut m_loop;

	Cut * m_cut;

protected:
	
	GLuint m_tex_id;

};

#endif
