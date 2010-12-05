//------------------------------------------------------------------------------
//
//	sauce.cpp 
// 
//------------------------------------------------------------------------------

#include "defs.h"
#include "sauce.h"


//------------------------------------------------------------------------------
//
// class Sauce
// 
//------------------------------------------------------------------------------

Sauce::Sauce() 
{
	m_next = NULL;
}


//------------------------------------------------------------------------------
//
// class EchoS : Sauce
// 
//------------------------------------------------------------------------------

EchoS::EchoS()
{
	m_echo = new Echo();
	m_echo->setDelay( SAMPLE_RATE );
}


//-----------------------------------------------------------------------------
// EchoS::tick( )
// 
//-----------------------------------------------------------------------------

/*
void EchoS::tick( float *samples, unsigned int n )
{
	// Make an StkFrames object from the input
	StkFrames frames( n, N_CHANNELS );
	
	for ( int i = 0; i < n; i++ )
		frames[i] = samples[i]

	for ( int i = 0; i < _cut->m_start; i++ )
	{
		m_cut->m_buffy[ m_cut->m_start + i ] = frames[i];
	}
} */

float EchoS::tick( float sample )
{
	//StkFrames frames( 1, 1 );
	//frames[0] = sample;
	
	return m_echo->tick( sample );

	//return frames[0];
}

