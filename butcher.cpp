/*------------------------------------------------------------------------------

	AUDIO BUTCHER
	Copyleft (C) 2010 Ilias Karim
	CS 476a Music, Computing, and Design
	Stanford University

	https://ccrma.stanford.edu/~ilias/476a/AudioButcher

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    Any person wishing to distribute modifications to the Software is
    asked to send the modifications to the original developer so that
    they can be incorporated into the canonical version.  This is,
    however, not a binding provision of this license.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
//
//	butcher.cpp	
//	Audio Butcher Copyleft (C) 2010 Ilias Karim
//
//	Entry point
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
// Globals
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
const CmdLineArgs g_defaultParams = { 
	DEFAULT_IN_DEVICE, DEFAULT_OUT_DEVICE, 
	DEFAULT_INGREDIENTS_DIR
};


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
	cout << endl << "Welcome, Butcher!" << endl << endl;
	
	// Parse command line arguments
	CmdLineArgs params = parseCmdLineArgs( argc, argv );

	RtAudio *audio = startAudio( params );
	
	g_glut = new GLUT( argc, argv );

	if ( audio )
	{
		g_kitchen.thawIngredients( params.directory );

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
	for ( int i = 1; i < argc; i++ )
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
	

