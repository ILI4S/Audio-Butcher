//------------------------------------------------------------------------------
//
// 	blade.cpp
//	Audio Butcher Copyleft (C) 2010 Ilias Karim
// 
//------------------------------------------------------------------------------

#include "stk/Stk.h"
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
	//Cut *cut = &g_cuts[ n_buffer ];
	Cut *cut = &g_kitchen.m_cuts[ n_buffer ];

	if ( !cut->m_writeOn ) return;

	cut->m_writeOn = false; // Stop writing to the buffer
	cut->m_cutSize = cut->m_writeHead; // Set the length of buffer used
	cut->m_start = 0; // Fresh cut

	// Refresh spectrogram texture if on the cutting board
	if ( g_kitchen.m_board.m_cut == cut ) 
		cut->m_tex_id = g_kitchen.m_board.m_spectrogram->generateTexture( cut );
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

	// Set playback settings: play once, or loop
	cut->m_once = once;
	cut->m_loop = loop;

	// If it is already playing doing nothing
	if ( cut->m_readOn ) return;

	// Reset read head to indicated mark
	// Make sure mark is in bounds, otherwise start from 0
	if ( cut->m_marks[ mark ] < cut->m_cutSize && cut->m_marks[ mark ] > cut->m_start )
		cut->m_readHead = cut->m_marks[ mark ];
	else 
		cut->m_readHead = 0; 

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
	if ( g_kitchen.m_board.m_loop.m_readOn ) 
		sample += g_kitchen.m_board.m_loop.readTick() * .5; // To prevent audio clipping

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

	m_fx = NULL;

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
	SF_INFO sf_info;
	SNDFILE* infile;

//	float buffy[ CUT_BUFFER_SIZE ];	// Temporary buffer to load the soundfile into

	// Open the soundfile
	if ( !(infile = sf_open( path, SFM_READ, &sf_info ) ) )
	{
		cout << "Error: could not open " << path << endl;
		return;
	}
	
	m_cutSize = sf_readf_float( infile, m_buffy, sf_info.frames );

	assert( sf_info.channels == N_CHANNELS );
	assert( sf_info.samplerate == SAMPLE_RATE );

/*
	cout << "channels: " << sf_info.channels << endl;
	cout << "samplerate: " << sf_info.samplerate << endl; */

/*
	// Next, change the sample rate of the soundfile to 44100Hz
	SRC_DATA params;
		
	// Set input and output buffers
	params.data_in = buffy;
	params.data_out = m_buffy;

	// Set rate ratio and number of frames
	params.src_ratio = SAMPLE_RATE / sf_info.samplerate;
	params.output_frames = params.input_frames = readcount; // ??? SAME NUMBER OF FRAMES??

	m_cutSize = readcount;

	// Resample and error check
	if ( int error = src_simple ( &params, SRC_SINC_MEDIUM_QUALITY, N_CHANNELS ) )
		cout << endl << "Error resampling " << path << ": " << error << endl; 

	else */

	cout << "Loaded " << path << " (" << sf_info.frames << " frames)" << endl;
}


//------------------------------------------------------------------------------
// Cut::freeze( )
// Saves the sample as a 44100Hz 24bit PCM Wav file in the given path
//------------------------------------------------------------------------------

void Cut::freeze( const char* path )
{
/*	// Temporary buffer to put the downsampled audio data in
	float buffy[ CUT_BUFFER_SIZE ];	
	
	SRC_DATA params;
		
	// Set input and output buffers
	params.data_in = m_buffy;
	params.data_out = buffy;

	// Set rate ratio and number of frames
	params.src_ratio = FILE_SAMPLE_RATE / SAMPLE_RATE;
	params.output_frames = params.input_frames = m_cutSize; // ?? SAME NUMBER OF FRAMES?
	
	if ( int error = src_simple ( &params, SRC_SINC_MEDIUM_QUALITY, N_CHANNELS ) )
	{
		cout << endl << "Error resampling TEST: " << error << endl;
		return;
	} */

	SNDFILE* outfile;
	SF_INFO sf_info;

	sf_info.samplerate = SAMPLE_RATE;
	sf_info.channels = N_CHANNELS;
	sf_info.format = ( SF_FORMAT_WAV | SF_FORMAT_PCM_32 );

	if ( !(outfile = sf_open( path, SFM_WRITE, &sf_info ) ) )
	{
		cout << "Error: could not open " << path << endl;
		return;	
	}

	int frames = sf_write_float( outfile, (float *) &m_buffy[ m_start ], (m_cutSize - m_start) * N_CHANNELS );

	cout << "Saved " << path << endl;
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
		m_writeOn = false;
		m_cutSize = CUT_BUFFER_SIZE;

		cout << "Buffer full!" << endl;
	} 
}


//------------------------------------------------------------------------------
// readTick( )
// 
//------------------------------------------------------------------------------

// Maybe make this read more than just one tick at once... ala .tick method

float Cut::readTick()
{
	double sample = 0; // Return value

	if ( !m_readOn ) return 0;

/*
	if ( m_playbackSpeed == 1.0f )
		sample = m_buffy[ (unsigned int) m_readHead++ ] * pow( 20, m_volumeCoef );

	else
	{
		for ( int i = 0; i < m_playbackSpeed; i++ )
			sample += m_buffy[ (unsigned int) m_readHead + i ] * pow( 20, m_volumeCoef );

		sample = sample / (int) m_playbackSpeed;

		m_readHead += m_playbackSpeed;
	} */
	
	// TODO: have buffer frame values be interpolated by STK
	
	sample = m_buffy[ (unsigned int) m_readHead ] * pow( 20, m_volumeCoef );
	m_readHead += m_playbackSpeed;
	
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

	// When playback reaches the end of the used buffer ..
	if ( m_readHead >= m_cutSize ) 
	{
		m_readHead = m_start;
	
		if ( m_once ) 
		{
			m_readOn = false;
		}
	}

	// When reverse playback reaches the beginning ..
	if ( m_readHead < m_start ) m_readHead = m_cutSize;

	return sample; 
}


//-----------------------------------------------------------------------------
// Cut::tick( )
// 
//-----------------------------------------------------------------------------

float Cut::tick( unsigned int start, float * buffer, int n_frames )
{
	assert( start < m_cutSize );

	int index = start;

	for ( int i = 0; i < n_frames; i++ )
	{
		if ( index + i >= m_cutSize ) index -= (m_cutSize - m_start);

		buffer[i] = m_buffy[ m_start + index + i ] * pow( 20, m_volumeCoef );
	}
}


void Cut::addFx( Sauce * fx )
{
	// TODO
	m_fx = new EchoS();
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
