//------------------------------------------------------------------------------
//
//	board.cpp
//	Copyleft (C) 2010 Ilias Karim
// 
//------------------------------------------------------------------------------

#include "stk/Stk.h"
#include "stk/PitShift.h"
#include "stk/LentPitShift.h"
#include "defs.h"
#include "kitchen.h"
#include "board.h"
#include "graphics.h"

#include <iostream>
#include <assert.h>
#include <math.h>

using namespace std;
using namespace stk;


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

extern Kitchen g_kitchen;
extern GLUT * g_glut;

const int g_rainbow_resolution = 0x10000;
bool g_rainbow_init = false;
Spectre g_rainbow[ g_rainbow_resolution ]; 


//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

void HSVtoRGB( float h, float s, float v, float * rgb );
void apply_window( float * data, float * window, unsigned long length );
void rfft( float * x, long N );
void scale_fft( float *buffer, int len, int fft_size, int wnd_size );
void hanning( float * window, unsigned long length );
void stereo2mono( float * buffy, int num_frames );


//------------------------------------------------------------------------------
//
// class CuttingBoard
// 
//------------------------------------------------------------------------------

CuttingBoard::CuttingBoard( )
{
	m_cut = NULL;
	m_tex_id = 0;
	m_spectrogram = new Spectrogram();
}


//-----------------------------------------------------------------------------
// CuttingBoard::draw()
// 
//-----------------------------------------------------------------------------


void CuttingBoard::draw()
{
	// Draw the spectrogram if it is rendered
	if ( m_cut && m_cut->m_tex_id )
	{
		m_spectrogram->draw( BOARD_X, BOARD_Y, m_cut->m_tex_id );
	

		for ( int i = 0; i < 10; i++ )
		{
			float pos = m_cut->m_marks[ i ];
		
			// Only show the marker if it is within bounds (not at start)
			// Unless it is the origin marker
			if ( ( pos > m_cut->m_start && pos < m_cut->m_cutSize ) || i == 0 )
			{
				float x = (((float) pos - m_cut->m_start) / 
							(float) (m_cut->m_cutSize - m_cut->m_start) )
							* BOARD_WIDTH + BOARD_X;

		/*		cout << "printing mark " << i << " with x " << x << endl; */
				
//j				char number[1];
//				number = itoa( i, number, 10 );

				ostringstream number;
				number << i;

				g_glut->print( x, 3.93, number.str().c_str(), g_white_rgba, true );

				glBegin( GL_LINES );
				glColor3f( 0, 0, 0 );
				glVertex2f( x, 3.9 );
				glVertex2f( x, -4.1 );
				glEnd();
			}
		}

		// Draw current playback line
		if ( m_cut->m_readHead )
		{
			float x = (((float) m_cut->m_readHead - m_cut->m_start) / 
						(float) (m_cut->m_cutSize - m_cut->m_start) )
					  * BOARD_WIDTH + BOARD_X;

			glBegin( GL_LINES );
			glColor3f( 1, 1, 1 );
			glVertex2f( x, 3.9 );
			glVertex2f( x, -4.1 );
		}

		glEnd();
	}

	// Draw a blank black square that may be bordered by the red, loop-rolling bg
	else 
	{
		float color[] = { 0, 0, 0, 1 };
		g_glut->drawRect( BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, color ); 
	}
}


//-----------------------------------------------------------------------------
// CuttingBoard::select( )
// 
//-----------------------------------------------------------------------------

void CuttingBoard::select( Cut * cut )
{
	cout << endl << "selected " << cut << endl;

	if ( m_cut && m_cut->m_readOn ) m_cut->m_readOn = false;

	m_cut = cut;

	if ( !m_cut->m_tex_id )
		m_cut->m_tex_id = m_spectrogram->generateTexture( m_cut );
}


//-----------------------------------------------------------------------------
// CuttingBoard::put( )
// 
//-----------------------------------------------------------------------------

void CuttingBoard::put( Cut * cut )
{
	if ( !m_cut ) return;

	m_cut->tick( 0, (float *) &cut->m_buffy, 
				 m_cut->m_cutSize - m_cut->m_start );

	cut->m_cutSize = m_cut->m_cutSize - m_cut->m_start;
	cut->m_readHead = cut->m_start = 0;

	cut->m_tex_id = m_spectrogram->generateTexture( cut );
	select( cut );
}


//-----------------------------------------------------------------------------
// CuttingBoard::rollLoop( )
// 
//-----------------------------------------------------------------------------

void CuttingBoard::rollLoop( bool record, bool recut )
{
	if ( !record && m_loop.m_writeOn ) // Stop rolling the loop
	{
		glClearColor( 0, 0, 0, 0 );

		m_loop.m_writeOn = false;
		if ( recut ) m_loop.m_cutSize = m_loop.m_writeHead;

	 	m_loop.m_readHead = m_loop.m_start; // Reset the previous buffer's

		// should replace this with the next commented line
		m_loop.m_tex_id = 0;
		g_kitchen.m_board.select( &m_loop );
	
//		m_loop.m_tex_id = g_kitchen.m_board.m_spectrogram->generateTexture( &m_loop );
	}

	else if ( record && !m_loop.m_writeOn ) // Start rolling the loop
	{
		glClearColor( 1.0f, 0, 0, 0 );

		m_loop.clear();
		m_loop.m_writeOn = true;
	}

	else // if ( record ) //Continue rolling the loop
	{
		m_loop.m_cutSize = m_loop.m_writeHead; 

		// should replace this 
		//m_loop.m_tex_id = 0;
		//g_kitchen.m_board.select( &m_loop );
	}
}


//-----------------------------------------------------------------------------
// CuttingBoard::resize( )
// 
//-----------------------------------------------------------------------------

void CuttingBoard::resize( bool extend, bool end )
{
	if ( !m_cut ) return;

	// Modify the end of the sample
	if ( end )
	{
		if ( extend && m_cut->m_cutSize < CUT_BUFFER_SIZE - RESIZE_INCREMENT )
		{
			m_cut->m_cutSize += RESIZE_INCREMENT;
			cout << "GOOOOOD" << endl;
		}

		else if ( !extend && m_cut->m_cutSize - RESIZE_INCREMENT > 0 )
			m_cut->m_cutSize -= RESIZE_INCREMENT;
	}

	// Modify the beginning of the sample
	else 
	{
		if ( extend && m_cut->m_start > RESIZE_INCREMENT )
			m_cut->m_start -= RESIZE_INCREMENT;

		else if ( !extend ) m_cut->m_start += RESIZE_INCREMENT;
	}

	// Reposition the read head if need be
	if ( m_cut->m_readHead > m_cut->m_cutSize || m_cut->m_readHead < m_cut->m_start )
		m_cut->m_readHead = m_cut->m_start;

	// Regenerate a spectrogram texture
	m_cut->m_tex_id = m_spectrogram->generateTexture( m_cut );
}


//-----------------------------------------------------------------------------
// CuttingBoard::adjustGain( )
// 
//-----------------------------------------------------------------------------

void CuttingBoard::adjustGain( bool increase )
{
	if ( !m_cut ) return;

	if ( increase ) 
		m_cut->m_volumeCoef += GAIN_INC;
//		if ( m_cut->m_readOn ) m_cut->m_volumeCoefTarget += GAIN_INC;
//		else m_cut->m_volumeCoef += GAIN_INC;

	else 
		m_cut->m_volumeCoef -= GAIN_INC;
//		if ( m_cut->m_readOn ) m_cut->m_volumeCoefTarget -= GAIN_INC;
//		else m_cut->m_volumeCoef -= GAIN_INC;

//	m_cut->m_tex_id = m_spectrogram->generateTexture( m_cut );
}


//-----------------------------------------------------------------------------
// CuttingBoard::shiftPitch( )
// 
//-----------------------------------------------------------------------------

void CuttingBoard::shiftPitch( bool increase ) // TODO: implement increase/decrease
{
	if ( !m_cut ) return;

/*	StkFrames frames( m_cut->m_cutSize - m_cut->m_start, 2 );

	m_cut->tick( 0, (float *) &frames[0], 
				 m_cut->m_cutSize - m_cut->m_start );

	LentPitShift pitShift;
	pitShift.setShift( 1.2f );
	pitShift.tick( frames, 0 );
	pitShift.tick( frames, 1 ); 

	for ( int i = 0; i < m_cut->m_cutSize - m_cut->m_start; i++ )
	{
		m_cut->m_buffy[ m_cut->m_start + i ] = frames[i];
	} */

	LentPitShift pitShift;
	pitShift.setShift( 1.2f );

	for ( int i = m_cut->m_start; i < m_cut->m_cutSize; i++ )
	{
		float sample;
		sample = pitShift.tick( m_cut->m_buffy[i] );
		m_cut->m_buffy[i] = sample;	
	}

	m_cut->m_tex_id = m_spectrogram->generateTexture( m_cut );
}


//-----------------------------------------------------------------------------
// CuttingBoard::adjustCursor( )
// 
//-----------------------------------------------------------------------------

void CuttingBoard::adjustCursor( int direction )
{
	if ( !m_cut ) return;

	if ( direction == LEFT ) // Reverse halfspeed playback
	{
		//g_kitchen.m_board.m_cut->m_playbackSpeedTarget = -.5f;
		g_kitchen.m_board.m_cut->m_playbackSpeed = -.5f;
		g_kitchen.m_board.m_cut->m_readOn = true;
		cout << "LEFT";
	}	
	else if ( direction == RIGHT ) // Forward slowed playback
	{
		//g_kitchen.m_board.m_cut->m_playbackSpeedTarget = .5f;
		g_kitchen.m_board.m_cut->m_playbackSpeed = .5f;
		g_kitchen.m_board.m_cut->m_readOn = true;
	}
	else
	{
		//g_kitchen.m_board.m_cut->m_playbackSpeedTarget = 1;
		g_kitchen.m_board.m_cut->m_playbackSpeed = 1;
		g_kitchen.m_board.m_cut->m_readOn = false;
	}
}


//-----------------------------------------------------------------------------
// CuttingBoard::chop( )
// 
//-----------------------------------------------------------------------------

void CuttingBoard::chop( int direction )
{
	if ( !m_cut ) return;

	if ( direction == LEFT && m_cut->m_readHead < m_cut->m_cutSize )
		m_cut->m_start = m_cut->m_readHead;

	else if ( m_cut->m_readHead > m_cut->m_start )
		m_cut->m_cutSize = m_cut->m_readHead;

	m_cut->m_tex_id = m_spectrogram->generateTexture( m_cut );
	cout << "eh?";
}


//-----------------------------------------------------------------------------
// CuttingBoard::setMark( )
//
//-----------------------------------------------------------------------------

void CuttingBoard::setMark( unsigned int i )
{
	if ( !m_cut || i > 9 ) return;

	m_cut->setMark( i );
}


//-----------------------------------------------------------------------------
// CuttingBoard::play( )
// 
//-----------------------------------------------------------------------------

// TODO: Have this function call a method native to Cut rather than implementing here

void CuttingBoard::play( bool once, bool loop )
{
	// Set playback settings: play once, or loop
	m_cut->m_once = once;
	m_cut->m_loop = loop;

	// If it is already playing doing nothing
	if ( m_cut->m_readOn ) return;

	// Begin reading from buffer
	m_cut->m_readOn = true; 
	
	// Start openning playback envelope
	m_cut->m_openEnv = true;
	m_cut->m_envFactor = 0.0f;
}

//------------------------------------------------------------------------------
//
// class Spectrogram
// Adapted from Tapastrea source code. See http://taps.cs.princeton.edu/
// 
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Spectrogram::Spectrogram()
// 
//-----------------------------------------------------------------------------

Spectrogram::Spectrogram()
{
    if( !g_rainbow_init )
    {
		// Init rainbow colors
        for( int i = 0; i < g_rainbow_resolution; i++ )
            HSVtoRGB( i / (float) g_rainbow_resolution * 300.0f, 1.0f, 1.0f,
					 (float *)&g_rainbow[i] );
		
		g_rainbow_init = true;
	}

	m_display_width = 13;
	m_display_height = 8; 

	m_nextTexId = 1;

	// set the buffer_size and window size
	// not sure why these can't just be #define'd
	m_fft_size = FFT_SIZE;
	m_wnd_size = m_fft_size / 8;

	hanning( m_window, m_wnd_size ); // window size could be changed
}


//-----------------------------------------------------------------------------
// Spectrogram::Spectrogram()
// 
//-----------------------------------------------------------------------------

unsigned int Spectrogram::generateTexture( Cut * cut )
{
	if ( cut->m_cutSize - cut->m_start < MIN_AUDIO_FRAMES ) return 0;

	unsigned int tex_id;

	if ( cut->m_tex_id ) tex_id = cut->m_tex_id;
	else tex_id = m_nextTexId++;

	float time_res = ( cut->m_cutSize - cut->m_start ) / SG_WIDTH;

	float max = -10, min = 10; // Default comparison values used for calibration

	for( int x = 0; x < SG_WIDTH; x++ )
	{	
		cut->tick( (unsigned int) (x * time_res + .5f), 
				   (float *) &m_buffer, m_wnd_size * 2 );		

/*
		for ( int j = 0; j < m_wnd_size * 2; j++ )
		{
			unsigned int index = (unsigned int) ( j + x * time_res + .5f );
			
			assert( index < cut->m_cutSize );

			m_buffer[j] = cut->m_buffy[ index ];
		} */

		stereo2mono( m_buffer, m_wnd_size * 2 );
		
		// window and zero pad
		apply_window( m_buffer, m_window, m_wnd_size );

		for( int j = m_wnd_size; j < m_fft_size; j++ )
			m_buffer[j] = 0.0f;

		rfft( m_buffer, m_fft_size / 2 );

		// scale from zero padding
		scale_fft( m_buffer, m_fft_size, m_fft_size, m_wnd_size );

		complex * cbuf = (complex *) m_buffer;

		// float freq_res = 2.0f;
		float freq_res = (float) m_fft_size  / ( (float) SG_HEIGHT * SG_FREQ_FACTOR );

		// fill
		for( int j = 0; j < SG_HEIGHT; j++ )
		{
			unsigned int index = (unsigned int) ( j * freq_res );

			assert( index < FFT_SIZE );

			// Magic function:
			m_spectre[x][j] = .45f + (20.0f * log10( cmp_abs( cbuf[ index ] ) ) + 60.0f ) / 60.0f; 
			// Save max's and min's for standardization
			if( m_spectre[x][j] > max ) max = m_spectre[x][j];
			if( m_spectre[x][j] < min ) min = m_spectre[x][j];
		}
	}

	if( min < -10 ) min = -1.25; // check bounds ??

	// calibrate values
	for( int i = 0; i < SG_WIDTH; i++ )
		for( int j = 0; j < SG_HEIGHT; j++ )
			m_spectre[i][j] = (m_spectre[i][j] - min)/(max - min);

    // try to make a texture
    unsigned char * spec_tex = new unsigned char[SG_WIDTH * SG_HEIGHT * 3]; 

    for( int j = 0; j < SG_HEIGHT; j++ )
        for( int i = 0; i < SG_WIDTH; i++ )
        {
            int index = (int) ((m_spectre[i][j]) * g_rainbow_resolution - .5f );
            if( index >= g_rainbow_resolution ) index = g_rainbow_resolution - 1;
            if( index < 0 ) index = 0;
            Spectre * color = &g_rainbow[index];
            spec_tex[3*(SG_WIDTH*j+i)] = (unsigned char)(255 * color->r);
            spec_tex[3*(SG_WIDTH*j+i)+1] = (unsigned char)(255 * color->g);
            spec_tex[3*(SG_WIDTH*j+i)+2] = (unsigned char)(255 * color->b);
        }

    glGenTextures( 1, &tex_id ); 
    glBindTexture( GL_TEXTURE_2D, tex_id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 

    if( tex_id )
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, SG_WIDTH, SG_HEIGHT, 0, 
					  GL_RGB, GL_UNSIGNED_BYTE, spec_tex );

    delete [] spec_tex;

	return tex_id;
}


//-----------------------------------------------------------------------------
// Spectrogram::draw()
// 
//-----------------------------------------------------------------------------

void Spectrogram::draw( float x, float y, unsigned int tex_id )
{
    float hinc = m_display_height / SG_HEIGHT;
    float winc = m_display_width / SG_WIDTH;
    float yy = y;
    float xx = x;

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable( GL_LIGHTING );
    float brightness = .35f;
    float contrast = .7f;

    glPushName( SPECTROGRAM_ID );
    
	glPushMatrix();
	glTranslatef( 0.0f, 0.0f, -0.01f );
	glColor4f( brightness, brightness, brightness, contrast ); 
	glBegin( GL_QUADS );
	glVertex2d( x, y );
	glVertex2d( x + m_display_width, y );
	glVertex2d( x + m_display_width, y + m_display_height );
	glVertex2d( x, y + m_display_height );
	glEnd();
	glPopMatrix();

	if( tex_id ) {
		glColor4f( 1.0f, 1.0f, 1.0f, contrast==0 ? 0 : 1/(2*contrast) ); // alpha seems useless but makes me feel better
		// texture
		glBindTexture( GL_TEXTURE_2D, tex_id );
		glEnable( GL_TEXTURE_2D );
		// blend
		glEnable( GL_BLEND );
		glBlendFunc( GL_DST_ALPHA, GL_ONE );
	}
	else
		glColor3f( 0.4f, 0.4f, 0.4f );

	glBegin( GL_QUADS );
	if( tex_id ) glTexCoord2f( 0.0f, 0.0f );
	glVertex2d( x, y );
	if( tex_id ) glTexCoord2f( 1.0f, 0.0f );
	glVertex2d( x + m_display_width, y );
	if( tex_id ) glTexCoord2f( 1.0f, 1.0f );
	glVertex2d( x + m_display_width, y + m_display_height );
	if( tex_id ) glTexCoord2f( 0.0f, 1.0f );
	glVertex2d( x, y + m_display_height );
	glEnd(); 

	if( tex_id ) {
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );
	}

    glPopName();
        
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
//    glEnable( GL_LIGHTING );

}



void HSVtoRGB( float h, float s, float v, float * rgb ) 
{ 
    int i; 
    float f, p, q, t,hTemp;
    float r, g, b;
  
    if( s == 0.0 || h == -1.0) // s==0? Totally unsaturated = grey so R,G and B all equal value 
    { 
      rgb[0] = v;
      rgb[1] = v;
      rgb[2] = v;
      return; 
    }

    hTemp = h/60.0f; 
    i = (int)floor( hTemp );                 // which sector 
    f = hTemp - i;                      // how far through sector 
    p = v * ( 1 - s ); 
    q = v * ( 1 - s * f ); 
    t = v * ( 1 - s * ( 1 - f ) ); 
  
    switch( i )  
    { 
    case 0:{r = v;g = t;b = p;break;} 
    case 1:{r = q;g = v;b = p;break;} 
    case 2:{r = p;g = v;b = t;break;} 
    case 3:{r = p;g = q;b = v;break;}  
    case 4:{r = t;g = p;b = v;break;} 
    case 5: default: {r = v;g = p;b = q;break;} 
    }

    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
} 


void apply_window( float * data, float * window, unsigned long length )
{
    unsigned long i;

    for ( i = 0; i < length; i++ )
        data[i] *= window[i];
}

//-----------------------------------------------------------------------------
// name: bit_reverse()
// desc: bitreverse places float array x containing N/2 complex values
//       into bit-reversed order
//-----------------------------------------------------------------------------

void bit_reverse( float * x, long N )
{
    float rtemp, itemp;
    long i, j, m;

    for ( i = j = 0 ; i < N ; i += 2, j += m )
    {
        if ( j > i )
        {
            rtemp = x[j]; itemp = x[j+1]; /* complex exchange */
            x[j] = x[i]; x[j+1] = x[i+1];
            x[i] = rtemp; x[i+1] = itemp;
        }

        for ( m = N>>1; m >= 2 && j >= m; m >>= 1 )
            j -= m;
    }
}

//-----------------------------------------------------------------------------
// cfft()
// complex value fft
//
//  these routines from CARL software, spect.c
//  check out the CARL CMusic distribution for more software
//
//  cfft replaces float array x containing NC complex values (2*NC float 
//  values alternating real, imagininary, etc.) by its Fourier transform 
//  if forward is true, or by its inverse Fourier transform ifforward is 
//  false, using a recursive Fast Fourier transform method due to 
//  Danielson and Lanczos.
//
//  NC MUST be a power of 2.
//
//-----------------------------------------------------------------------------

void cfft( float * x, long NC, unsigned int forward )
{
    float wr, wi, wpr, wpi, theta, scale ;
    long mmax, ND, m, i, j, delta ;
    ND = NC<<1 ;
    bit_reverse( x, ND ) ;
    
    for( mmax = 2 ; mmax < ND ; mmax = delta )
    {
        delta = mmax<<1 ;
        theta = TWOPI/( forward? mmax : -mmax ) ;
        wpr = (float) (-2.*pow( sin( 0.5*theta ), 2. )) ;
        wpi = (float) sin( theta ) ;
        wr = 1. ;
        wi = 0. ;

        for( m = 0 ; m < mmax ; m += 2 )
        {
            register float rtemp, itemp ;
            for( i = m ; i < ND ; i += delta )
            {
                j = i + mmax ;
                rtemp = wr*x[j] - wi*x[j+1] ;
                itemp = wr*x[j+1] + wi*x[j] ;
                x[j] = x[i] - rtemp ;
                x[j+1] = x[i+1] - itemp ;
                x[i] += rtemp ;
                x[i+1] += itemp ;
            }

            wr = (rtemp = wr)*wpr - wi*wpi + wr ;
            wi = wi*wpr + rtemp*wpi + wi ;
        }
    }

    // scale output
    scale = (float)(forward ? 1./ND : 2.) ;
    {
        register float *xi=x, *xe=x+ND ;
        while( xi < xe )
            *xi++ *= scale ;
    }
}


//-----------------------------------------------------------------------------
// rfft()
// real value fft
//
//  these routines from the CARL software, spect.c
//  check out the CARL CMusic distribution for more source code
//
//  if forward is true, rfft replaces 2*N real data points in x with N complex 
//  values representing the positive frequency half of their Fourier spectrum,
//  with x[1] replaced with the real part of the Nyquist frequency value.
//
//  N MUST be a power of 2.
//
//-----------------------------------------------------------------------------

void rfft( float * x, long N )
{
    float c1, c2, h1r, h1i, h2r, h2i, wr, wi, wpr, wpi, temp, theta ;
    float xr, xi ;
    long i, i1, i2, i3, i4, N2p1 ;

    theta = PIE/N ;
    wr = 1. ;
    wi = 0. ;
    c1 = 0.5 ;

	c2 = -0.5 ;
	cfft( x, N, 1 ) ;
	xr = x[0] ;
	xi = x[1] ;


    wpr = (float) (-2.*pow( sin( 0.5*theta ), 2. )) ;
    wpi = (float) sin( theta ) ;
    N2p1 = (N<<1) + 1 ;
    
    for( i = 0 ; i <= N>>1 ; i++ )
    {
        i1 = i<<1 ;
        i2 = i1 + 1 ;
        i3 = N2p1 - i2 ;
        i4 = i3 + 1 ;
        if( i == 0 )
        {
            h1r =  c1*(x[i1] + xr ) ;
            h1i =  c1*(x[i2] - xi ) ;
            h2r = -c2*(x[i2] + xi ) ;
            h2i =  c2*(x[i1] - xr ) ;
            x[i1] =  h1r + wr*h2r - wi*h2i ;
            x[i2] =  h1i + wr*h2i + wi*h2r ;
            xr =  h1r - wr*h2r + wi*h2i ;
            xi = -h1i + wr*h2i + wi*h2r ;
        }
        else
        {
            h1r =  c1*(x[i1] + x[i3] ) ;
            h1i =  c1*(x[i2] - x[i4] ) ;
            h2r = -c2*(x[i2] + x[i4] ) ;
            h2i =  c2*(x[i1] - x[i3] ) ;
            x[i1] =  h1r + wr*h2r - wi*h2i ;
            x[i2] =  h1i + wr*h2i + wi*h2r ;
            x[i3] =  h1r - wr*h2r + wi*h2i ;
            x[i4] = -h1i + wr*h2i + wi*h2r ;
        }

        wr = (temp = wr)*wpr - wi*wpi + wr ;
        wi = wi*wpr + temp*wpi + wi ;
    }

	x[1] = xr ;
}


void scale_fft( float *buffer, int len, int fft_size, int wnd_size )
{
    assert( fft_size != 0 && wnd_size != 0 );

	float factor = fft_size / (float) wnd_size / 2.0f;

    for( int i = 0; i < len; i++ )
        buffer[i] *= factor;
}


void hanning( float * window, unsigned long length )
{
    unsigned long i;
    double phase = 0, delta;

    delta = 2 * PIE / (double) length;

    for( i = 0; i < length; i++ )
    {
        window[i] = (float)(0.5 * (1.0 - cos(phase)));
        phase += delta;
    }
}


void stereo2mono( float * buffy, int num_frames )
{
    // start from end of mono
    float* src = buffy;
	float* end = buffy + ( num_frames * 2 );
    float* dest = buffy;

    // copy
    while( src < end )
    {
        *dest = ( *src + *(src + 1) ) / 2.0f;
		dest++;
        src += 2; 
    }

    // zero
    while( dest < end )
        *dest++ = 0.0f;
} 

