//------------------------------------------------------------------------------
//
//	sauce.h
// 
//------------------------------------------------------------------------------

#include "stk/Stk.h"
#include "stk/Echo.h"
//#include "stk/Delay.h"

using namespace stk;


//------------------------------------------------------------------------------
//
// class Sauce
// 
//------------------------------------------------------------------------------

class Sauce 
{

public:

	Sauce(); // constructor

public:

	Sauce * m_next; // Is a linked list of effects

	virtual float tick( float sample ) = 0;

};


//------------------------------------------------------------------------------
//
// class EchoS : Sauce
// 
//------------------------------------------------------------------------------

class EchoS : public Sauce
{

public:

	EchoS(); // constructor

	float tick ( float sample );
//	void tick( float *samples, unsigned int n );

	Echo * m_echo;	

};
