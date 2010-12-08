//------------------------------------------------------------------------------
//
//	defs.h
//	Audio Butcher Copyleft (C) 2010 Ilias Karim
// 
//	Global settings
//
//------------------------------------------------------------------------------

#ifndef __DEFS__
#define __DEFS__

// File system settings
#define DEFAULT_INGREDIENTS_DIR "recipe"
#define WORKING_DIR "temp"
#define SNDFILE_EXT .wav

// Hardware settings
#define DEFAULT_IN_DEVICE 3
#define DEFAULT_OUT_DEVICE 2

#define SAMPLE double
#define N_CHANNELS 1
#define SAMPLE_RATE 44100
#define FILE_SAMPLE_RATE 44100 // Lower this
#define BUFFER_FRAMES 512 * N_CHANNELS

// Keyboard graphics settings
#define KEY_HEIGHT .5f
#define KEY_WIDTH .8f
#define SPACE_HEIGHT .1f
#define SPACE_WIDTH .1f

#define PLAYBACK_SPEED_DEC .5f
#define PLAYBACK_SPEED_INC 1.0f
#define GAIN_INC .3f 

// Playback settings
#define ENV_FACTOR_STEP_SIZE .0003f

// Reference variables
#define PIE 3.14159265358979
#define TWOPI 2 * PIE

#define BOARD_X -6.5
#define BOARD_Y -4
#define BOARD_WIDTH BOARD_X * -2
#define BOARD_HEIGHT BOARD_Y * -2

#define RESIZE_INCREMENT SAMPLE_RATE / 32 
#define MIN_AUDIO_FRAMES SAMPLE_RATE / 4

// Buffer size: seconds of audio at 44100 Hz 
#define CUT_BUFFER_SIZE 44100 * 2 * 30

#define DEFAULT_WINDOW_WIDTH 960
#define DEFAULT_WINDOW_HEIGHT 600

#define TEXT_BUFFER_SIZE 512
#define GLUT_FONT_LARGE GLUT_BITMAP_HELVETICA_18
#define GLUT_FONT_SMALL GLUT_BITMAP_HELVETICA_12

#endif
