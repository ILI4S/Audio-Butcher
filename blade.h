//------------------------------------------------------------------------------
//
// 	blade.h
//	Audio Butcher Copyleft (C) 2010 Ilias Karim
// 
//------------------------------------------------------------------------------

#ifndef __SAMPLER__
#define __SAMPLER__

// TODO: Implement scratching
#define SCRATCH_DISTANCE 44100

using namespace stk;

//------------------------------------------------------------------------------
//
// class Cut
// 
//------------------------------------------------------------------------------

class Cut 
{

public: 

	Cut(); // Constructor
	~Cut(); // Destructor

	void clear();
	
	// Load file to buffer
	void thaw( const char* path );
	void freeze( const char* path );

	// Writing interface (cutting)
	void writeTick( float sample );

	// Reading interface (serving)
	float readTick();

	float tick( float * head, float n_frames );

	float tick( unsigned int start, float * buffer, int n_frames );

	void gotoMark( unsigned int x );
	void setMark( unsigned int x );

public:
	
	unsigned int m_marks[ 10 ];

	bool m_loop, m_once;
	
	unsigned int m_cutSize;
	unsigned int m_start; 
	
	double m_playbackSpeed;//, m_playbackSpeedTarget;
	float m_volumeCoef;//, m_volumeCoefTarget;

	// Flag active read or writes
	bool m_writeOn, m_readOn;
	
	// Head positions in the buffer
	float m_writeHead, m_readHead;

	// Used as buffer while cutting / writing
	float m_buffy[ CUT_BUFFER_SIZE ];

	StkFrames *m_frames;
	unsigned int m_n_frames;

	//Sauce *m_fx;

	unsigned int m_tex_id;		

	// Enveloping 
	bool m_openEnv, m_closeEnv;
	float m_envFactor;
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
//	void startServe( unsigned int n_buffer, bool record, int x, int y );
	void startServe( unsigned int n_buffer, bool record, bool once, bool loop, unsigned int mark );
	void stopServe( unsigned int n_buffer, bool record );

	void resetCuts();

public:
	
	bool m_record;
};


#endif
