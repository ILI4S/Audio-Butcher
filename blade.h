//------------------------------------------------------------------------------
//
// blade.h
// 
//------------------------------------------------------------------------------

#ifndef __SAMPLER__
#define __SAMPLER__

#include "sauce.h"

// Buffer size: seconds of audio at 44100 Hz 
#define CUT_BUFFER_SIZE 44100 * 2 * 20

#define VOLUME_SLEW_RATE .001f
#define GAIN_INC .5f 

#define PLAYBACK_SLEW_RATE .01f

#define MIN_PLAYBACK_SPEED .5f
#define MAX_PLAYBACK_SPEED 2.0f

#define PLAYBACK_SPEED_DEC .5f
#define PLAYBACK_SPEED_INC 1.0f

#define PLAYBACK_SPEED_CHANGE 

#define SCRATCH_DISTANCE 44100


//------------------------------------------------------------------------------
//
// class Cut
// 
//------------------------------------------------------------------------------

class Cut 
{

public: 

	Cut(); // Constructor

	void clear();
	
	// Load file to buffer
	void thaw( const char* path );
	void freeze( const char* path );

	// Writing interface (cutting)
	void writeTick( float sample );

	// Reading interface (serving)
	float readTick();

	float tick( unsigned int start, float * buffer, int n_frames );

	void addFx( Sauce * fx );

public:
	
	unsigned int m_cutSize;
	unsigned int m_start; 
	
	float m_playbackSpeed, m_playbackSpeedTarget;
	float m_volumeCoef, m_volumeCoefTarget;

	// Flag active read or writes
	bool m_writeOn, m_readOn;
	
	// Head positions in the buffer
	float m_writeHead, m_readHead;

	float m_buffy[ CUT_BUFFER_SIZE ];

	Sauce *m_fx;

	unsigned int m_tex_id;		

};


//------------------------------------------------------------------------------
//
// class Blade
// 
//------------------------------------------------------------------------------

class Blade
{

public:
	
	Blade(); // Constructor

	// RtAudio interface
	float readTick();
	void writeTick( float sample );

	// Cut cuts
	void startCut( unsigned int n_buffer );
	void stopCut( unsigned int n_buffer );

	// Playback cuts
	void startServe( unsigned int n_buffer, bool record, int x, int y );
	void stopServe( unsigned int n_buffer, bool record );

	void resetCuts();

public:
	
	bool m_record;
};


#endif
