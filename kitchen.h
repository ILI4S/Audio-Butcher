//------------------------------------------------------------------------------
//
// kitchen.h
// 
//------------------------------------------------------------------------------

#ifndef __KITCHEN__
#define __KITCHEN__

#include "blade.h"
#include "board.h"
	

//------------------------------------------------------------------------------
//
// class Kitchen
// 
//------------------------------------------------------------------------------

class Kitchen
{

public:

	// Constructor
	Kitchen( );

	void cleanFridge( const char* path );

	// Load all audio data in a given directory
	void thawIngredients( const char* path );

	// Save all audio data to a given directory
	void freezeIngredients( const char* path );

	Blade m_blade;

	Cut m_cuts[ 26 ]; // 26 alpha letter keys

	CuttingBoard m_board;
};

#endif
