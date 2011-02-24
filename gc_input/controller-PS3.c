/**
 * Wii64 - controller-GC.c
 * Copyright (C) 2007, 2008, 2009 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009 sepp256
 * 
 * Gamecube controller input module
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
 *                sepp256@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
**/


#include <string.h>
#include <io/pad.h>
#include "controller.h"

enum {
	ANALOG_AS_ANALOG = 1, C_STICK_AS_ANALOG = 2,
};

enum {
	PS3_UP  = 0x01,
	PS3_LEFT  = 0x02,
	PS3_RIGHT  = 0x04,
	PS3_DOWN  = 0x08,
	PS3_CROSS = 0x10,
	PS3_SQUARE = 0x20,
	PS3_CIRCLE = 0x40,
	PS3_TRIANGLE = 0x80,
	PS3_R1  = 0x01 << 8,
	PS3_L2  = 0x02 << 8,
	PS3_R2  = 0x04 << 8,
	PS3_START  = 0x08 << 8,
	ANALOG_L  = 0x01 << 16,
	ANALOG_R  = 0x02 << 16,
	ANALOG_U  = 0x04 << 16,
	ANALOG_D  = 0x08 << 16,
	C_STICK_L = 0x10 << 16,
	C_STICK_R = 0x20 << 16,
	C_STICK_U = 0x40 << 16,
	C_STICK_D = 0x80 << 16,
};

static button_t buttons[] = {
	{  0, ~0,                "None" },
	{  1, PS3_UP,    "D-Up" },
	{  2, PS3_LEFT,  "D-Left" },
	{  3, PS3_RIGHT, "D-Right" },
	{  4, PS3_DOWN,  "D-Down" },
	{  5, PS3_R1,    "R1" },
	{  6, PS3_L2,    "L2" },
	{  7, PS3_R2,    "R2" },
	{  8, PS3_CROSS,     "Cross" },
	{  9, PS3_SQUARE,     "Square" },
	{ 10, PS3_CIRCLE,     "Circle" },
	{ 11, PS3_TRIANGLE,     "Triangle" },
	{ 12, PS3_START, "Start" },
	{ 13, C_STICK_U,        "C-Up" },
	{ 14, C_STICK_L,        "C-Left" },
	{ 15, C_STICK_R,        "C-Right" },
	{ 16, C_STICK_D,        "C-Down" },
	{ 17, ANALOG_U,         "A-Up" },
	{ 18, ANALOG_L,         "A-Left" },
	{ 19, ANALOG_R,         "A-Right" },
	{ 20, ANALOG_D,         "A-Down" },
};

static button_t analog_sources[] = {
	{ 0, ANALOG_AS_ANALOG,  "Analog Stick" },
	{ 1, C_STICK_AS_ANALOG, "C-Stick" },
};

static button_t menu_combos[] = {
	{ 0, PS3_CIRCLE|PS3_TRIANGLE, "Circle+Triangle" },
	{ 1, PS3_START|PS3_CROSS, "Start+Cross" },
};

padInfo padinfo;

static unsigned int getButtons(int Control)
{
	padData paddata;
	u32 b = 0;
	/*s8 stickX      = PAD_StickX(Control);
	s8 stickY      = PAD_StickY(Control);
	s8 substickX   = PAD_SubStickX(Control);
	s8 substickY   = PAD_SubStickY(Control);
	
	if(stickX    < -48) b |= ANALOG_L;
	if(stickX    >  48) b |= ANALOG_R;
	if(stickY    >  48) b |= ANALOG_U;
	if(stickY    < -48) b |= ANALOG_D;
	
	if(substickX < -48) b |= C_STICK_L;
	if(substickX >  48) b |= C_STICK_R;
	if(substickY >  48) b |= C_STICK_U;
	if(substickY < -48) b |= C_STICK_D;
	*/
	ioPadGetData(Control, &paddata);
	if(paddata.BTN_LEFT){
		b |= PS3_UP;
	}
	if(paddata.BTN_DOWN){
		b |= PS3_DOWN;
	}
	if(paddata.BTN_RIGHT){
		b |= PS3_RIGHT;
	}
	if(paddata.BTN_UP){
		b |= PS3_UP;
	}
	if(paddata.BTN_CROSS){
		b |= PS3_CROSS;
	}
	if(paddata.BTN_SQUARE){
		b |= PS3_SQUARE;
	}
	if(paddata.BTN_CIRCLE){
		b |= PS3_CIRCLE;
	}
	if(paddata.BTN_TRIANGLE){
		b |= PS3_TRIANGLE;
	}
	if(paddata.BTN_R1){
		b |= PS3_R1;
	}
	if(paddata.BTN_L2){
		b |= PS3_L2;
	}
	if(paddata.BTN_R2){
		b |= PS3_R2;
	}
	if(paddata.BTN_START){
		b |= PS3_START;
	}
	return b;
}

static int _GetKeys(int Control, BUTTONS * Keys, controller_config_t* config)
{
	BUTTONS* c = Keys;
	memset(c, 0, sizeof(BUTTONS));

	controller_PS3.available[Control] = padinfo.status[Control];
	if (!controller_PS3.available[Control]) return 0;

	unsigned int b = getButtons(Control);
	inline int isHeld(button_tp button){
		return (b & button->mask) == button->mask;
	}
	
	c->R_DPAD       = isHeld(config->DR);
	c->L_DPAD       = isHeld(config->DL);
	c->D_DPAD       = isHeld(config->DD);
	c->U_DPAD       = isHeld(config->DU);
	
	c->START_BUTTON = isHeld(config->START);
	c->B_BUTTON     = isHeld(config->B);
	c->A_BUTTON     = isHeld(config->A);

	c->Z_TRIG       = isHeld(config->Z);
	c->R_TRIG       = isHeld(config->R);
	c->L_TRIG       = isHeld(config->L);

	c->R_CBUTTON    = isHeld(config->CR);
	c->L_CBUTTON    = isHeld(config->CL);
	c->D_CBUTTON    = isHeld(config->CD);
	c->U_CBUTTON    = isHeld(config->CU);
/*
	if(config->analog->mask == ANALOG_AS_ANALOG){
		c->X_AXIS = 5*PAD_StickX(Control)/6;
		c->Y_AXIS = 5*PAD_StickY(Control)/6;
	} else if(config->analog->mask == C_STICK_AS_ANALOG){
		c->X_AXIS = 5*PAD_SubStickX(Control)/6;
		c->Y_AXIS = 5*PAD_SubStickY(Control)/6;
	}
	if(config->invertedY) c->Y_AXIS = -c->Y_AXIS;
*/
	// Return whether the exit button(s) are pressed
	return isHeld(config->exit);
}

static void pause(int Control){
	//PAD_ControlMotor(Control, PAD_MOTOR_STOP);
}

static void resume(int Control){ }

static void rumble(int Control, int rumble){
	//PAD_ControlMotor(Control, rumble ? PAD_MOTOR_RUMBLE : PAD_MOTOR_STOP);
}

static void configure(int Control, controller_config_t* config){
	// Don't know how this should be integrated
}

static void assign(int p, int v){
	// Nothing to do here
}


static void refreshAvailable(void);

controller_t controller_PS3 =
	{ 'G',
	  _GetKeys,
	  configure,
	  assign,
	  pause,
	  resume,
	  rumble,
	  refreshAvailable,
	  {0, 0, 0, 0},
	  sizeof(buttons)/sizeof(buttons[0]),
	  buttons,
	  sizeof(analog_sources)/sizeof(analog_sources[0]),
	  analog_sources,
	  sizeof(menu_combos)/sizeof(menu_combos[0]),
	  menu_combos,
	  { .DU        = &buttons[1],  // D-Pad Up
	    .DL        = &buttons[2],  // D-Pad Left
	    .DR        = &buttons[3],  // D-Pad Right
	    .DD        = &buttons[4],  // D-Pad Down
	    .Z         = &buttons[5],  // Z
	    .L         = &buttons[6],  // Left Trigger
	    .R         = &buttons[7],  // Right Trigger
	    .A         = &buttons[8],  // A
	    .B         = &buttons[9],  // B
	    .START     = &buttons[12], // Start
	    .CU        = &buttons[13], // C-Stick Up
	    .CL        = &buttons[14], // C-Stick Left
	    .CR        = &buttons[15], // C-Stick Right
	    .CD        = &buttons[16], // C-Stick Down
	    .analog    = &analog_sources[0],
	    .exit      = &menu_combos[0],
	    .invertedY = 0,
	  }
	 };

static void refreshAvailable(void){

	ioPadGetInfo(&padinfo);
	
	int i;
	for(i=0; i<4; ++i)
		controller_PS3.available[i] = padinfo.status[i];
}
