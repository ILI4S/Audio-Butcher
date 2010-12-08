//------------------------------------------------------------------------------
//
// 	blade.cpp
//	Audio Butcher Copyleft (C) 2010 Ilias Karim
// 
//------------------------------------------------------------------------------

#include "stk/Stk.h"
#include "stk/FileRead.h"
#include "stk/FileWrite.h"
#include "defs.h"
#include "blade.h"
#include "interface.h"
#include "kitchen.h"
#include "sndfile.h"

#include <assert.h>
#include <samplerate.h>
#include <iostream>
#include <string>
#include <math.h>

using namespace std;
using namespace stk;

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

extern Kitchen g_kitchen;
extern Mouse g_mouse;
extern GLsizei g_width;
extern GLsizei g_height;


//------------------------------------------------------------------------------
//
// class Blade
// 
//------------------------------------------------------------------------------

Blade::Blade()
{
	m_record = false;
}


//------------------------------------------------------------------------------
// Blade::startCut( )
// 
//------------------------------------------------------------------------------

void Blade::startCut( unsigned int n_buffer  )
{
	assert( n_buffer <= 25 );
	
	// Retrieve the correct audio buffer
	Cut *cut = &g_kitchen.m_cuts[ n_buffer ];

	if ( cut->m_writeOn ) return;

	cut->clear();
	
	cut->m_writeHead = 0; // Reset write head
	cut->m_writeOn = true; // Begin writing to buffer
	
	cout << "Writing to buffer " << n_buffer << endl;
}


//------------------------------------------------------------------------------
// Blade::stopCut( )
// 
//------------------------------------------------------------------------------

void Blade::stopCut( unsigned int n_buffer )
{
	assert( n_buffer <= 25 );

	// Retrieve the correct audio buffer
	Cut *cut = &g_kitchen.m_cuts[ n_buffer ];

	if ( !cut->m_writeOn ) return;

	cut->m_writeOn = false; // Stop writing to the buffer

	// Set the length of buffer used
	cut->m_cutSize = cut->m_writeHead; 
	cut->m_n_frames = cut->m_writeHead;
	cut->m_start = 0; // Fresh cut

	// Transfer data into StkFrames
	cut->m_frames = new StkFrames( cut->m_n_frames, N_CHANNELS );
	for ( int i = 0; i < cut->m_cutSize; i++ )
		(*cut->m_frames)[i] = cut->m_buffy[i];

	// Refresh spectrogram texture if on the cutting board
	if ( g_kitchen.m_board.m_cut == cut ) 
		cut->m_tex_id = g_kitchen.m_board.m_spectrogram->generateTexture( cut );
	else 
		cut->m_tex_id = 0; // Otherwise just reset the texture
}


//------------------------------------------------------------------------------
// Blade::startServe( )
// 
//------------------------------------------------------------------------------

void Blade::startServe( unsigned int n_buffer, bool record, bool once, bool loop,
						unsigned int mark )
{
	// Start or stop rolling a loop
	g_kitchen.m_board.rollLoop( record, false );
	
	// Retrieve the correct audio buffer
	Cut *cut = &g_kitchen.m_cuts[ n_buffer ];

	if ( !cut->m_frames ) return;

	// Set playback settings: play once, or loop
	cut->m_once = once;
	cut->m_loop = loop;

	// If it is already playing do nothing
	//if ( cut->m_readOn ) return;

	// Reset read head to indicated mark
	// Make sure mark is in bounds, otherwise start from 0
	/*if ( cut->m_marks[ mark ] < cut->m_cutSize && cut->m_marks[ mark ] > cut->m_start )
		cut->m_readHead = cut->m_marks[ mark ];
	else */

	cut->m_readHead = cut->m_start; 

	// Begin reading from buffer
	cut->m_readOn = true; 
	
	// Start openning playback envelope
	cut->m_openEnv = true;
	cut->m_envFactor = 0.0f;
}


//------------------------------------------------------------------------------
// Blade::stopServe( )
// 
//------------------------------------------------------------------------------

void Blade::stopServe( unsigned int n_buffer, bool record )
{
	// Stop or continue rolling a loop
	if ( g_kitchen.m_board.m_loop.m_writeOn )
		g_kitchen.m_board.rollLoop( record, true );

	assert( n_buffer <= 25 );

	// Retrieve the correct audio buffer
	Cut *cut = &g_kitchen.m_cuts[ n_buffer ];
	
	if ( !cut->m_readOn || cut->m_once || cut->m_loop ) return;

	//cut->m_readOn = false; // Stop reading from buffer

	// Stop reading from buffer by starting the closing envelope
	// (which decrements to 0 by ENV_FACTOR_STEP_SIZE
	cut->m_closeEnv = true; 
	cut->m_envFactor = 1.0f; 

	cout << "Finished reading" << endl;
}


//------------------------------------------------------------------------------
// Blade::writeTick( )
// 
//------------------------------------------------------------------------------

void Blade::writeTick( float sample )
{
	// (could optimize by only iterating over buffers that are currently writing)
	for ( int i = 0; i <= 25; i++ )
		if ( g_kitchen.m_cuts[i].m_writeOn ) 
			g_kitchen.m_cuts[i].writeTick( sample );
}


//------------------------------------------------------------------------------
// Blade::readTick( )
// 
//------------------------------------------------------------------------------

float Blade::readTick()
{
	float sample = 0.0f;

	// (could optimize by only iterating over buffers that are currently reading)
	// Add all playing samples together
	for ( int i = 0; i <= 25; i++ )
		if ( g_kitchen.m_cuts[i].m_readOn ) 
			sample += g_kitchen.m_cuts[i].readTick();
	
	// Scratch effect
	if ( g_mouse.m_noise.m_readOn ) 
	{
		float t = g_mouse.m_noise.readTick();
		sample += t;
		//cout << g_mouse.m_noise.m_readHead << " " << t << endl;
	}
	
	// Board loop (rolled with ALT combo)
//	if ( g_kitchen.m_board.m_loop.m_readOn ) 
//		sample += g_kitchen.m_board.m_loop.readTick() * .5; // To prevent audio clipping

	// Write to the loop if there is one currently being rolled
	// Idea: why not update the spectrogram every N samples?
	if ( g_kitchen.m_board.m_loop.m_writeOn ) 
		g_kitchen.m_board.m_loop.writeTick( sample );

	return sample;
}


//-----------------------------------------------------------------------------
// Blade::resetCuts( )
// 
//-----------------------------------------------------------------------------

void Blade::resetCuts()
{
	// (could optimize by only iterating over buffers that are currently reading)
	for ( int i = 0; i <= 25; i++ )
		if ( g_kitchen.m_cuts[i].m_readOn ) 
			g_kitchen.m_cuts[i].m_readHead = g_kitchen.m_cuts[i].m_start;
}



//------------------------------------------------------------------------------
//
// class Cut
// 
//------------------------------------------------------------------------------

Cut::Cut() // Constructor
{
	clear();

	for ( int i = 0; i < 10; i++ ) // Reset mark positions
		m_marks[i] = 0;
}


Cut::~Cut() // Destructor
{
	if ( m_frames ) delete m_frames;
}


//-----------------------------------------------------------------------------
// Cut::clear( )
// 
//-----------------------------------------------------------------------------

void Cut::clear()
{
	//m_playbackSpeed = m_playbackSpeedTarget = 1.0f;
	m_playbackSpeed = 1.0f;
	//m_volumeCoef = m_volumeCoefTarget = 0;
	m_volumeCoef = 0;

	m_writeHead = m_readHead = m_cutSize = m_start = 0;
	m_writeOn = m_readOn = false;

	for ( int i = 0; i < CUT_BUFFER_SIZE; i++ )
		m_buffy[ i ] = 0;

	if ( m_frames ) delete m_frames;
	m_frames = NULL;
	m_n_frames = 0;

	//m_fx = NULL;

	m_loop = m_once = false;

	m_closeEnv = false;
	m_openEnv = false;
	m_envFactor = 1.0f;
}


//------------------------------------------------------------------------------
// thaw( )
// Loads a 44100Hz file from the given path
//------------------------------------------------------------------------------

void Cut::thaw( const char* path )
{
	clear();

	FileRead thaw;
	
	thaw.open( path );

	m_cutSize = thaw.fileSize();
	m_frames = new StkFrames( thaw.fileSize(), N_CHANNELS );


	thaw.read( *m_frames, 0, true );
}


//------------------------------------------------------------------------------
// Cut::freeze( )
//
//------------------------------------------------------------------------------

void Cut::freeze( const char* path )
{
	if ( !m_frames ) return;

	FileWrite freeze;

	freeze.open( path, N_CHANNELS, FileWrite::FILE_WAV, FileWrite::STK_SINT16 );
	freeze.write( *m_frames );
	freeze.close();
}


//------------------------------------------------------------------------------
// writeTick( )
// 
//------------------------------------------------------------------------------

void Cut::writeTick( float sample ) 
{ 
	//cout << "writing: " << sample << endl;

	if ( !m_writeOn ) return;

	m_buffy[ (unsigned int) m_writeHead++ ] = sample; 
	
	if ( m_writeHead >= CUT_BUFFER_SIZE )
	{
		cout << "Buffer full!" << endl;

		m_writeOn = false; // Stop writing to the buffer

		// Set the length of buffer used
		m_cutSize = m_writeHead; 
		m_n_frames = m_writeHead;
		m_start = 0; // Fresh cut

		// Transfer data into StkFrames
		m_frames = new StkFrames( m_n_frames, N_CHANNELS );
		for ( int i = 0; i < m_cutSize; i++ )
			(*m_frames)[i] = m_buffy[i];

		// Refresh spectrogram texture if on the cutting board
		if ( g_kitchen.m_board.m_cut == this ) 
			m_tex_id = g_kitchen.m_board.m_spectrogram->generateTexture( this );
		else 
			m_tex_id = 0; // Otherwise just reset the texture
	} 
}


//------------------------------------------------------------------------------
// readTick( )
// 
//------------------------------------------------------------------------------

float Cut::readTick()
{
	if ( !m_frames || !m_readOn ) return 0;

//  =(
//	StkFloat sample = tick( &m_readHead, m_playbackSpeed );

	StkFloat sample = 0; // return value

	if ( m_readHead > m_cutSize - 1 ) 
	{
		m_readHead = m_start;
		if ( m_once ) m_readOn = false;
	}
	else if ( m_readHead < m_start ) m_readHead = m_cutSize - 3; // for reverse playback

	sample = m_frames->interpolate( m_readHead );
	m_readHead += m_playbackSpeed;



	// Adjust volume
	sample *= pow( 20, m_volumeCoef );

	if ( m_openEnv )
	{
		sample *= m_envFactor;
		
		m_envFactor += ENV_FACTOR_STEP_SIZE;

		if ( m_envFactor >= 1 )
		{
			m_openEnv = false;
		}
	}


	if ( m_closeEnv ) 
	{
		sample *= m_envFactor;
		
		m_envFactor -= ENV_FACTOR_STEP_SIZE;
		if ( m_envFactor <= 0 )
		{
			m_closeEnv = false;
			m_readOn = false;
		}
	}

	if ( m_readHead == m_start  && m_once ) m_readOn = false;


	return sample; 
}




float Cut::tick( float *head, float n_frames )
{
	StkFloat frame = 0; // return value

	if ( *head > m_cutSize - 1 && m_playbackSpeed > 0 )
		*head = m_start;
	
	else if ( *head < m_start ) *head = m_cutSize - 10000; // for reverse playback

	frame = m_frames->interpolate( *head );
	*head += n_frames;

	return frame;
}
	


//-----------------------------------------------------------------------------
// Cut::tick( )
// 
//-----------------------------------------------------------------------------

float Cut::tick( unsigned int start, float * buffer, int n_frames )
{
	assert( start < m_cutSize );

	float head = (float) start;

	for ( int i = 0; i < n_frames; i++ )
		buffer[i] = tick( &head, 1 );
}


//-----------------------------------------------------------------------------
// Cut::gotoMark( )
// 
//-----------------------------------------------------------------------------

void Cut::gotoMark( unsigned int x )
{
	if ( x > 9 ) return;

	if ( m_marks[ x ] < m_start )
		m_readHead = m_start;

	else if ( m_marks[ x ] < m_cutSize )
		m_readHead = m_marks[ x ];
	
	else m_readHead = 0;
}


//-----------------------------------------------------------------------------
// Cut::setMark( )
// 
//-----------------------------------------------------------------------------

void Cut::setMark( unsigned int x )
{
	if ( x > 9 ) return;

	m_marks[ x ] = m_readHead;
}
