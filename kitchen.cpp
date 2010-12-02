//------------------------------------------------------------------------------
//
// kitchen.cpp
// 
//------------------------------------------------------------------------------

#include "defs.h"
#include "kitchen.h"
#include <string>
#include <stdio.h>
#include <sys/stat.h>

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
	// check if DEFAULT_INGREDIENTS_DIR exists, make it if it doesn't
	if ( mkdir( DEFAULT_INGREDIENTS_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) == -1 )
		thawIngredients( DEFAULT_INGREDIENTS_DIR ); // If it does, load recipe
}


//------------------------------------------------------------------------------
// thawIngredients( )
// Load all audio data in a given directory
//------------------------------------------------------------------------------

void Kitchen::thawIngredients( const char* path )
{
	// Iterate over each cut
	for ( int i = 0; i < 26; i++ )
	{
		string filename = string( path ) + (char) (i + 65) + ".wav";

		// If a file exists, load it 
		struct stat stFileInfo;

		if ( !stat( filename.c_str(), &(stFileInfo) ) )

			m_cuts[i].thaw( filename.c_str() );
	}

}


//------------------------------------------------------------------------------
// freezeIngredients( )
// Save all audio data to a given directory
//------------------------------------------------------------------------------

void Kitchen::freezeIngredients( const char* path )
{
	mkdir( path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	
	cleanFridge( path );

	// Iterate over each cut
	for ( int i = 0; i < 26; i++ )
	{
		// If it has been cut save it
		if ( m_cuts[i].m_cutSize )
		{
			string filename = string( path ) + (char) (i + 65) + ".wav";

			m_cuts[i].freeze( filename.c_str() );
		}
	}
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
}

