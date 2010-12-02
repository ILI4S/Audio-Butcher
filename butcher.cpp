//------------------------------------------------------------------------------
//
//	AUDIO BUTCHER
//	by Ilias Karim
//
//	CS 476a Music, Computing, and Design
//	Stanford University 
//
//	butcher.cpp	
//
//------------------------------------------------------------------------------

#include "defs.h"
#include "kitchen.h"
#include "graphics.h"
#include "RtAudio.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <string.h>
#include <sndfile.h>

using namespace std;


//------------------------------------------------------------------------------
// External Global
//------------------------------------------------------------------------------

// The butcher's workplace
extern Kitchen g_kitchen;

GLUT * g_glut;

//------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------

// Struct returned by readArgs 
typedef struct {
	unsigned int inDevice, outDevice;
	char directory[256];
} CmdLineArgs; 

// Default values for command line parameters 
const CmdLineArgs g_defaultParams = { DEFAULT_IN_DEVICE, DEFAULT_OUT_DEVICE };


void printUsage();
CmdLineArgs parseCmdLineArgs ( int argc, char ** argv );
RtAudio *startAudio( CmdLineArgs &params );
void stopAudio( RtAudio *audio );


//------------------------------------------------------------------------------
// audioCallback( )
// Audio callback function that generates a sine wave at the specified frequency
//------------------------------------------------------------------------------

int audioCallback ( void* outBuffer, void* inBuffer, unsigned int nBufferFrames,
					double streamTime, RtAudioStreamStatus status, void *userData )
{
	float* out 	= (float*) outBuffer;
	float* in 	= (float*) inBuffer; 

	for ( int i = 0; i < nBufferFrames * N_CHANNELS; i++ )
	{
		in[i] += g_kitchen.m_blade.readTick();

		g_kitchen.m_blade.writeTick( in[i] );

		out[i] = in[i]; 
		//out[i] = 0; // Airplane control
	}

	return 0;
}


//------------------------------------------------------------------------------
// main( )
// Entry point
//------------------------------------------------------------------------------

int main( int argc, char ** argv )
{
	cout << endl << "Welcome to your kitchen!" << endl << endl;
	
	// Parse command line arguments
	CmdLineArgs params = parseCmdLineArgs( argc, argv );

	RtAudio *audio = startAudio( params );
	
	g_glut = new GLUT( argc, argv );

	if ( audio )
	{
		//g_kitchen.m_cuts[0].thaw( "terminator.wav" );	

		// Start the graphics engine main loop
	    glutMainLoop();
		
		stopAudio( audio );
	}
    
	return 0;
}


//------------------------------------------------------------------------------
// parseCmdLineArgs( )
// Parses command-line arguments and return a struct of options
//------------------------------------------------------------------------------

CmdLineArgs parseCmdLineArgs ( int argc, char ** argv )
{
	CmdLineArgs params = g_defaultParams;

	// Iterate over command-line arguments to check for flags
	for ( int i = 0; i < argc; i++ )
	{
		// --in [input device number]
		if ( !strcmp( argv[ i ], "--in" ) && argc > i + 1 )
			params.inDevice = atoi( argv[ i + 1 ] );
		
		// --out [output device number]
		else if ( !strcmp( argv[ i ], "--out" ) && argc > i + 1 )
			params.outDevice = atoi( argv[ i + 1 ] );

		else 
			strcpy( params.directory, argv[i] );
	}

	return params;
} 


//------------------------------------------------------------------------------
// printUsage()
// Prints correct command line usage and options
//------------------------------------------------------------------------------

void printUsage() 
{
	cout << endl << "usage: Catch \n";
}


//------------------------------------------------------------------------------
// printDeviceInfo( )
// Prints audio input and output device info
//------------------------------------------------------------------------------

void printDeviceInfo( RtAudio *audio )
{
	int defaultOut = audio->getDefaultOutputDevice();
	int defaultIn = audio->getDefaultInputDevice();
	
	cout << "Your audio input and output devices:" << endl;

	for ( int i = 0; i < audio->getDeviceCount(); i++ )
	{
		RtAudio::DeviceInfo deviceInfo = audio->getDeviceInfo( i );

		// Print device info
		cout << "[" << i << "] " << deviceInfo.name;

		if ( i == defaultOut ) cout << " (default system output)";
		else if ( i == defaultIn ) cout << " (default system input)";

		cout << endl;
	}

	cout << endl;
}


//------------------------------------------------------------------------------
// startAudio( )
// Opens and starts the RtAudio stream according to the signal parameters
//------------------------------------------------------------------------------

RtAudio *startAudio( CmdLineArgs &params )
{
	RtAudio *audio = new RtAudio();
	
	// Print input and output audio devices
	printDeviceInfo( audio );

	// RtAudio settings
	RtAudio::StreamParameters iParams, oParams;

	unsigned int bufferFrames = BUFFER_FRAMES;
	
	oParams.nChannels = iParams.nChannels = N_CHANNELS;
	oParams.deviceId = params.outDevice;
	iParams.deviceId = params.inDevice;

	// Open and start the stream
	try {
		audio->openStream( &oParams, &iParams, RTAUDIO_FLOAT32,
						   SAMPLE_RATE, &bufferFrames, &audioCallback, NULL );
		audio->startStream();
	}
	catch ( RtError &e ) 
	{
		e.printMessage();
		return false;
	}

	cout << "Input device:  " << audio->getDeviceInfo( iParams.deviceId ).name << endl;
	cout << "Output device: " << audio->getDeviceInfo( oParams.deviceId ).name << endl;

	if ( audio->getDefaultOutputDevice() != iParams.deviceId )
		cout << endl << "WARNING: " 
			 << "Please set your default system output device to the specified input device!" 
			 << endl;

	return audio;
}


//------------------------------------------------------------------------------
// stopAudio( )
//------------------------------------------------------------------------------

void stopAudio( RtAudio *audio )
{
	// Stop and close the stream
	try { 
		audio->stopStream();
	} catch ( RtError &e ) {
		e.printMessage();
	}

	if ( audio->isStreamOpen() ) audio->closeStream();
	delete audio;
}
	

