//------------------------------------------------------------------------------
//
// 	kitchen.cpp
// 	Audio Butcher Copyleft (C) 2010 Ilias Karim
// 
//------------------------------------------------------------------------------

#include "defs.h"
#include "kitchen.h"
#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

using namespace std;


//------------------------------------------------------------------------------
// Globals
//------------------------------------------------------------------------------

Kitchen g_kitchen;


//------------------------------------------------------------------------------
//
// class Kitchen
// 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Kitchen( )
// Constructor
//------------------------------------------------------------------------------

Kitchen::Kitchen( )
{
	m_directory = NULL;
	// check if DEFAULT_INGREDIENTS_DIR exists, make it if it doesn't
//		thawIngredients( DEFAULT_INGREDIENTS_DIR ); // If it does, load recipe
}


//------------------------------------------------------------------------------
// thawIngredients( )
// Load all audio data in a given directory
//------------------------------------------------------------------------------

void Kitchen::thawIngredients()
{
	if ( !m_directory ) thawIngredients( DEFAULT_INGREDIENTS_DIR );
	else thawIngredients( m_directory );
}

void Kitchen::thawIngredients( const char* path )
{
	if ( !m_directory ) m_directory = path;

	// Try to make the directory (if it doesn't exist) then return if succesful
	if ( mkdir( path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) != -1 )
		return;

	// Otherwise, there are samples in the given directory to be loaded

	// Iterate over each cut
	for ( int i = 0; i < 26; i++ )
	{
		string filename = string( path ) + (char) (i + 65) + ".wav";

		// If a file exists, load it 
		struct stat stFileInfo;

		if ( !stat( filename.c_str(), &(stFileInfo) ) )
		{
			m_cuts[i].thaw( filename.c_str() );
			
		}
	}

	ifstream paramFile;
	string paramFilename = string( path ) + string( "/recipe.txt" );
	paramFile.open( paramFilename.c_str() );
		
	for ( int i = 0; i < 26; i++ )
	{
		if ( paramFile.is_open() && paramFile.good() ) 
		{
			string line;
			getline( paramFile, line ); 

			// A:
			// [volume coefficient]
			// [playback rate]
			if ( line[1] == ':' )
			{
				string volumeLine, speedLine;
				float volume = 1.0f, speed = 1.0f;

				// Read volume data from file 
				getline( paramFile, volumeLine ); 
				sscanf( volumeLine.c_str(), "%f", &volume );

				// Read playback speed data from file 
				getline( paramFile, speedLine ); 
				sscanf( speedLine.c_str(), "%f", &speed );

				// Store values in correct objects
				unsigned int index = (unsigned int) (line[0] - 65);
				m_cuts[ index ].m_volumeCoef = volume;
				m_cuts[ index ].m_playbackSpeed = speed;
			}
		}
	}
}


//------------------------------------------------------------------------------
// freezeIngredients( )
// Save all audio data to a given directory
//------------------------------------------------------------------------------

void Kitchen::freezeIngredients()
{
	if ( !m_directory ) freezeIngredients( DEFAULT_INGREDIENTS_DIR );
	else freezeIngredients( m_directory );
}

void Kitchen::freezeIngredients( const char* path )
{
	// Make the directory in case it doesn't exist
	mkdir( path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	
	// Delete files in the directory, in case they exist
	cleanFridge( path );

	// Open a file to save the sample parameters
	ofstream paramFile;
	string paramFilename = string( path ) + string( "/recipe.txt" );
	paramFile.open( paramFilename.c_str() );

	// Iterate over each cut
	for ( int i = 0; i < 26; i++ )
	{
		// If it has been cut save it
		if ( m_cuts[i].m_cutSize )
		{
			string filename = string( path ) + (char) (i + 65) + ".wav";

			m_cuts[i].freeze( filename.c_str() );

			if ( paramFile.is_open() )
			{
				paramFile << (char) (i + 65) << ":" << endl;
				paramFile << m_cuts[i].m_volumeCoef << endl;
				paramFile << m_cuts[i].m_playbackSpeed << endl;
			}
		}

	}
	
	paramFile.close();
}


//-----------------------------------------------------------------------------
// Kitchen::cleanFridge( )
// 
//-----------------------------------------------------------------------------

void Kitchen::cleanFridge( const char* path )
{
	for ( int i = 0; i < 26; i++ )
	{
		string filename = string( path ) + (char) (i + 65) + ".wav";
		
		remove( filename.c_str() );
	}	

	remove( "recipe.txt" );
}

