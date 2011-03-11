/**
 * Wii64 - main_gc-menu2.cpp (aka MenuV2)
 * Copyright (C) 2007, 2008, 2009, 2010 Mike Slegeir
 * Copyright (C) 2007, 2008, 2009, 2010, 2011 sepp256
 * Copyright (C) 2007, 2008, 2009, 2010, 2011 emu_kidid
 * 
 * New main that uses menu's instead of prompts
 *
 * Wii64 homepage: http://www.emulatemii.com
 * email address: tehpola@gmail.com
 *                sepp256@gmail.com
 *                emukidid@gmail.com
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


/* INCLUDES */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include "../menu/MenuContext.h"
#include "../libgui/MessageBox.h"
#include "timers.h"

#include "winlnxdefs.h"
extern "C" {
#include "main.h"
#include "rom.h"
#include "plugin.h"
#include "../gc_input/controller.h"

#include "../r4300/r4300.h"
#include "../gc_memory/memory.h"
#include "../gc_memory/tlb.h"
#include "../gc_memory/pif.h"
#include "../gc_memory/flashram.h"
#include "../gc_memory/Saves.h"
#include "../main/savestates.h"
#include "ROM-Cache.h"
#include "wii64config.h"
#include "../fileBrowser/fileBrowser.h"
#include "../fileBrowser/fileBrowser-ps3.h"
}

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ppu-types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/socket.h>
#include <sysutil/sysutil.h>

#include <assert.h>
#include <io/pad.h>
#include <sysutil/video.h>
#include <rsx/rsx.h>
#include "rsxutil.h"

int s, running;
struct sockaddr_in server;
#define TESTIP				"192.168.2.3"
#define TESTPORT			18194


void program_exit_callback()
{
	gcmSetWaitFlip(context);
	rsxFinish(context,1);
}

extern char shutdownMenu;
void sysutil_exit_callback(u64 status,u64 param,void *usrdata)
{
	switch(status) {
		case SYSUTIL_EXIT_GAME:
			r4300.stop = 1;		//Halt emulation
			shutdownMenu = 1;	//Exit menu
			break;
		case SYSUTIL_DRAW_BEGIN:
		case SYSUTIL_DRAW_END:
			break;
		default:
			break;
	}
}

void udp_setup()
{
	s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset(&server, 0, sizeof(server));
	server.sin_len = sizeof(server);
	server.sin_family = AF_INET;
	inet_pton(AF_INET, TESTIP, &server.sin_addr);
	server.sin_port = htons(TESTPORT);
}

extern "C" {
void dbg_printf(const char *fmt,...)
{
	int len;
	va_list args;
	char str[1024];

	va_start(args, fmt);
	len = vsprintf(str,fmt,args);
	va_end(args);
	
	sendto(s, str, len, 0, (struct sockaddr*)&server, sizeof(server));
}
}

static void Initialise (void){ }


/* NECESSARY FUNCTIONS AND VARIABLES */

// -- Initialization functions --
static void Initialise(void);
static void gfx_info_init(void);
static void audio_info_init(void);
static void rsp_info_init(void);
void control_info_init(void);
// -- End init functions --

// -- Plugin data --
//#define DEFAULT_FIFO_SIZE    (256*1024)//(64*1024) minimum

CONTROL Controls[4];

static GFX_INFO     gfx_info;
       AUDIO_INFO   audio_info;
static CONTROL_INFO control_info;
static RSP_INFO     rsp_info;

extern char audioEnabled;
char printToScreen;
char showFPSonScreen;
char printToSD;
char glN64_useFrameBufferTextures;
char glN64_use2xSaiTextures;
char renderCpuFramebuffer;
extern timers Timers;
char menuActive;
       char saveEnabled;
       char creditsScrolling;
       char padNeedScan;
       char wpadNeedScan;
       char shutdownMenu = 0;
	   char nativeSaveDevice;
	   char saveStateDevice;
       char autoSave;
       char screenMode = 0;
	   char padAutoAssign;
	   char padType[4];
	   char padAssign[4];
	   char pakMode[4];
	   char loadButtonSlot;

//TODO: remove these declarations
u32 dynacore;

static struct {
	char* key;
	char* value; // Not a string, but a char pointer
	char  min, max;
} OPTIONS[] =
{ { "Audio", &audioEnabled, AUDIO_DISABLE, AUDIO_ENABLE },
  { "FPS", &showFPSonScreen, FPS_HIDE, FPS_SHOW },
//  { "Debug", &printToScreen, DEBUG_HIDE, DEBUG_SHOW },
  { "FBTex", &glN64_useFrameBufferTextures, GLN64_FBTEX_DISABLE, GLN64_FBTEX_ENABLE },
  { "2xSaI", &glN64_use2xSaiTextures, GLN64_2XSAI_DISABLE, GLN64_2XSAI_ENABLE },
  { "ScreenMode", &screenMode, SCREENMODE_4x3, SCREENMODE_16x9_PILLARBOX },
  { "Core", ((char*)&dynacore)+3, DYNACORE_INTERPRETER, DYNACORE_PURE_INTERP },
  { "NativeDevice", &nativeSaveDevice, NATIVESAVEDEVICE_SD, NATIVESAVEDEVICE_CARDB },
  { "StatesDevice", &saveStateDevice, SAVESTATEDEVICE_SD, SAVESTATEDEVICE_USB },
  { "AutoSave", &autoSave, AUTOSAVE_DISABLE, AUTOSAVE_ENABLE },
  { "LimitVIs", &Timers.limitVIs, LIMITVIS_NONE, LIMITVIS_WAIT_FOR_FRAME },
/*  { "PadType1", &padType[0], PADTYPE_NONE, PADTYPE_WII },
  { "PadType2", &padType[1], PADTYPE_NONE, PADTYPE_WII },
  { "PadType3", &padType[2], PADTYPE_NONE, PADTYPE_WII },
  { "PadType4", &padType[3], PADTYPE_NONE, PADTYPE_WII },
  { "PadAssign1", &padAssign[0], PADASSIGN_INPUT0, PADASSIGN_INPUT3 },
  { "PadAssign2", &padAssign[1], PADASSIGN_INPUT0, PADASSIGN_INPUT3 },
  { "PadAssign3", &padAssign[2], PADASSIGN_INPUT0, PADASSIGN_INPUT3 },
  { "PadAssign4", &padAssign[3], PADASSIGN_INPUT0, PADASSIGN_INPUT3 },*/
  { "Pak1", &pakMode[0], PAKMODE_MEMPAK, PAKMODE_RUMBLEPAK },
  { "Pak2", &pakMode[1], PAKMODE_MEMPAK, PAKMODE_RUMBLEPAK },
  { "Pak3", &pakMode[2], PAKMODE_MEMPAK, PAKMODE_RUMBLEPAK },
  { "Pak4", &pakMode[3], PAKMODE_MEMPAK, PAKMODE_RUMBLEPAK },
  { "LoadButtonSlot", &loadButtonSlot, LOADBUTTON_SLOT0, LOADBUTTON_DEFAULT },
};
void handleConfigPair(char* kv);
void readConfig(FILE* f);
void writeConfig(FILE* f);

extern "C" void gfx_set_fb(unsigned int* fb1, unsigned int* fb2);
void gfx_set_window(int x, int y, int width, int height);
// -- End plugin data --

int GX_xfb_offset = 0;

// Dummy functions
static void dummy_func(){ }
void (*fBRead)(DWORD addr) = NULL;
void (*fBWrite)(DWORD addr, DWORD size) = NULL;
void (*fBGetFrameBufferInfo)(void *p) = NULL;
void new_frame(){ }
void new_vi(){ }
int loadROM();

extern u32 *color_buffer[2];
extern VideoResolution res;

int main(int argc, char* argv[]){

	/* INITIALIZE */
	Initialise();

	udp_setup();
	void *host_addr = memalign(1024*1024,HOST_SIZE);

	init_screen(host_addr,HOST_SIZE);
	ioPadInit(7);
	setRenderTarget(curr_fb);
	atexit(program_exit_callback);
	sysUtilRegisterCallback(0,sysutil_exit_callback,NULL);

	//Initialize controls once before menu runs <- needed?
	control_info_init();

	void* vmode=NULL;
	MenuContext *menu = new MenuContext(vmode);

#ifdef DEBUGON
//	DEBUG_Init(GDBSTUB_DEVICE_TCP,GDBSTUB_DEF_TCPPORT); //Default port is 2828
//	DEBUG_Init(GDBSTUB_DEVICE_USB, 1);
//	_break();
#endif

	// Default Settings
	audioEnabled     = 1; // Audio
#ifdef RELEASE
	showFPSonScreen  = 0; // Show FPS on Screen
#else
	showFPSonScreen  = 1; // Show FPS on Screen
#endif
	printToScreen    = 1; // Show DEBUG text on screen
	printToSD        = 0; // Disable SD logging
	Timers.limitVIs  = 0; // Sync to Audio
	saveEnabled      = 0; // Don't save game
	nativeSaveDevice = 0; // SD
	saveStateDevice	 = 0; // SD
	autoSave         = 1; // Auto Save Game
	creditsScrolling = 0; // Normal menu for now
	dynacore         = 1; // Dynarec
	screenMode		 = 0; // Stretch FB horizontally
	padAutoAssign	 = PADAUTOASSIGN_AUTOMATIC;
	padType[0]		 = PADTYPE_NONE;
	padType[1]		 = PADTYPE_NONE;
	padType[2]		 = PADTYPE_NONE;
	padType[3]		 = PADTYPE_NONE;
	padAssign[0]	 = PADASSIGN_INPUT0;
	padAssign[1]	 = PADASSIGN_INPUT1;
	padAssign[2]	 = PADASSIGN_INPUT2;
	padAssign[3]	 = PADASSIGN_INPUT3;
	pakMode[0]		 = PAKMODE_MEMPAK; // memPak plugged into controller 1
	pakMode[1]		 = PAKMODE_MEMPAK;
	pakMode[2]		 = PAKMODE_MEMPAK;
	pakMode[3]		 = PAKMODE_MEMPAK;
	loadButtonSlot	 = LOADBUTTON_DEFAULT;
#ifdef GLN64_GX
// glN64 specific  settings
 	glN64_useFrameBufferTextures = 0; // Disable FrameBuffer textures
	glN64_use2xSaiTextures = 0;	// Disable 2xSai textures
	renderCpuFramebuffer = 0; // Disable CPU Framebuffer Rendering
#endif //GLN64_GX
	menuActive = 1;

	//config stuff
	fileBrowser_file* configFile_file;
#ifdef PS3
	int (*configFile_init)(fileBrowser_file*) = fileBrowser_ps3_init;
	configFile_file = &saveDir_ps3_Default;
	if(configFile_init(configFile_file)) {                //only if device initialized ok
		FILE* f = fopen( "/dev_usb/wii64/settings.cfg", "r" );  //attempt to open file
		if(f) {        //open ok, read it
			readConfig(f);
			fclose(f);
		}
		f = fopen( "/dev_usb/wii64/controlP.cfg", "r" );  //attempt to open file
		if(f) {
			load_configurations(f, &controller_PS3);					//read in GC controller mappings
			fclose(f);
		}
	}
#else //PS3
	int (*configFile_init)(fileBrowser_file*) = fileBrowser_libfat_init;
#ifdef HW_RVL
	if(argv[0][0] == 'u') {  //assume USB
		configFile_file = &saveDir_libfat_USB;
		if(configFile_init(configFile_file)) {                //only if device initialized ok
			FILE* f = fopen( "usb:/wii64/settings.cfg", "r" );  //attempt to open file
			if(f) {        //open ok, read it
				readConfig(f);
				fclose(f);
			}
			f = fopen( "usb:/wii64/controlG.cfg", "r" );  //attempt to open file
			if(f) {
				load_configurations(f, &controller_GC);					//read in GC controller mappings
				fclose(f);
			}
#ifdef HW_RVL
			f = fopen( "usb:/wii64/controlC.cfg", "r" );  //attempt to open file
			if(f) {
				load_configurations(f, &controller_Classic);			//read in Classic controller mappings
				fclose(f);
			}
			f = fopen( "usb:/wii64/controlN.cfg", "r" );  //attempt to open file
			if(f) {
				load_configurations(f, &controller_WiimoteNunchuk);		//read in WM+NC controller mappings
				fclose(f);
			}
			f = fopen( "usb:/wii64/controlW.cfg", "r" );  //attempt to open file
			if(f) {
				load_configurations(f, &controller_Wiimote);			//read in Wiimote controller mappings
				fclose(f);
			}
#endif //HW_RVL
		}
	}
	else //if((argv[0][0]=='s') || (argv[0][0]=='/'))
#endif
	{ //assume SD
		configFile_file = &saveDir_libfat_Default;
		if(configFile_init(configFile_file)) {                //only if device initialized ok
			FILE* f = fopen( "sd:/wii64/settings.cfg", "r" );  //attempt to open file
			if(f) {        //open ok, read it
				readConfig(f);
				fclose(f);
			}
			f = fopen( "sd:/wii64/controlG.cfg", "r" );  //attempt to open file
			if(f) {
				load_configurations(f, &controller_GC);					//read in GC controller mappings
				fclose(f);
			}
#ifdef HW_RVL
			f = fopen( "sd:/wii64/controlC.cfg", "r" );  //attempt to open file
			if(f) {
				load_configurations(f, &controller_Classic);			//read in Classic controller mappings
				fclose(f);
			}
			f = fopen( "sd:/wii64/controlN.cfg", "r" );  //attempt to open file
			if(f) {
				load_configurations(f, &controller_WiimoteNunchuk);		//read in WM+NC controller mappings
				fclose(f);
			}
			f = fopen( "sd:/wii64/controlW.cfg", "r" );  //attempt to open file
			if(f) {
				load_configurations(f, &controller_Wiimote);			//read in Wiimote controller mappings
				fclose(f);
			}
#endif //HW_RVL
		}
	}
#endif //!PS3
#if defined(HW_RVL) || defined(PS3)
	// Handle options passed in through arguments
	int i;
	for(i=1; i<argc; ++i){
		handleConfigPair(argv[i]);
	}
#endif */
	running = 1;
	while (menu->isRunning() && running) {}

	delete menu;

	return 0;
}

#if defined(WII)
u16 readWPAD(void){
	if(wpadNeedScan){ WPAD_ScanPads(); wpadNeedScan = 0; }
	WPADData* wpad = WPAD_Data(0);

	u16 b = 0;
	if(wpad->err == WPAD_ERR_NONE &&
	   wpad->exp.type == WPAD_EXP_CLASSIC){
	   	u16 w = wpad->exp.classic.btns;
	   	b |= (w & CLASSIC_CTRL_BUTTON_UP)    ? PAD_BUTTON_UP    : 0;
	   	b |= (w & CLASSIC_CTRL_BUTTON_DOWN)  ? PAD_BUTTON_DOWN  : 0;
	   	b |= (w & CLASSIC_CTRL_BUTTON_LEFT)  ? PAD_BUTTON_LEFT  : 0;
	   	b |= (w & CLASSIC_CTRL_BUTTON_RIGHT) ? PAD_BUTTON_RIGHT : 0;
	   	b |= (w & CLASSIC_CTRL_BUTTON_A) ? PAD_BUTTON_A : 0;
	   	b |= (w & CLASSIC_CTRL_BUTTON_B) ? PAD_BUTTON_B : 0;
	}

	return b;
}
#else
u16 readWPAD(void){ return 0; }
#endif

extern BOOL eepromWritten;
extern BOOL mempakWritten;
extern BOOL sramWritten;
extern BOOL flashramWritten;
BOOL hasLoadedROM = FALSE;
int autoSaveLoaded = NATIVESAVEDEVICE_NONE;

int loadROM(fileBrowser_file* rom){
  int ret = 0;
  savestates_job = 0; //clear all pending save states
	// First, if there's already a loaded ROM
	if(hasLoadedROM){
		// Unload it, and deinit everything
		cpu_deinit();
		eepromWritten = FALSE;
		mempakWritten = FALSE;
		sramWritten = FALSE;
		flashramWritten = FALSE;
		romClosed_RSP();
		romClosed_input();
		romClosed_audio();
		romClosed_gfx();
		closeDLL_RSP();
		closeDLL_input();
		closeDLL_audio();
		closeDLL_gfx();
		ROMCache_deinit();
		free_memory();
#ifndef HW_RVL
//		ARAM_manager_deinit();
#endif
	}
	format_mempacks();
	reset_flashram();
	init_eeprom();
	hasLoadedROM = TRUE;
#ifndef HW_RVL
//	ARAM_manager_init();
//	TLBCache_init();
#else
	tlb_mem2_init();
#endif
	//romFile_init(rom);
	ret = rom_read(rom);
	if(ret){	// Something failed while trying to read the ROM.
		hasLoadedROM = FALSE;
		return ret;
	}

	// Init everything for this ROM
	init_memory();

//	gfx_set_fb(xfb[0], xfb[1]);
	if (screenMode == SCREENMODE_16x9_PILLARBOX)
		gfx_set_window( 78, 0, 483, 480);
	else
		gfx_set_window( 0, 0, 640, 480);

	gfx_info_init();
	audio_info_init();
	control_info_init();
	rsp_info_init();

	romOpen_gfx();
//	gfx_set_fb(xfb[0], xfb[1]);
	romOpen_audio();
	romOpen_input();

	cpu_init();

  if(autoSave==AUTOSAVE_ENABLE) {
#ifdef PS3
    // Adjust saveFile pointers
    saveFile_dir = &saveDir_ps3_Default;
    saveFile_readFile  = fileBrowser_ps3_readFile;
    saveFile_writeFile = fileBrowser_ps3_writeFile;
    saveFile_init      = fileBrowser_ps3_init;
    saveFile_deinit    = fileBrowser_ps3_deinit;
#else //PS3
    switch (nativeSaveDevice)
    {
    	case NATIVESAVEDEVICE_SD:
    	case NATIVESAVEDEVICE_USB:
    		// Adjust saveFile pointers
    		saveFile_dir = (nativeSaveDevice==NATIVESAVEDEVICE_SD) ? &saveDir_libfat_Default:&saveDir_libfat_USB;
    		saveFile_readFile  = fileBrowser_libfat_readFile;
    		saveFile_writeFile = fileBrowser_libfat_writeFile;
    		saveFile_init      = fileBrowser_libfat_init;
    		saveFile_deinit    = fileBrowser_libfat_deinit;
    		break;
    	case NATIVESAVEDEVICE_CARDA:
    	case NATIVESAVEDEVICE_CARDB:
    		// Adjust saveFile pointers
    		saveFile_dir       = (nativeSaveDevice==NATIVESAVEDEVICE_CARDA) ? &saveDir_CARD_SlotA:&saveDir_CARD_SlotB;
    		saveFile_readFile  = fileBrowser_CARD_readFile;
    		saveFile_writeFile = fileBrowser_CARD_writeFile;
    		saveFile_init      = fileBrowser_CARD_init;
    		saveFile_deinit    = fileBrowser_CARD_deinit;
    		break;
    }
#endif //!PS3
    // Try loading everything
  	int result = 0;
  	saveFile_init(saveFile_dir);
  	result += loadEeprom(saveFile_dir);
  	result += loadSram(saveFile_dir);
  	result += loadMempak(saveFile_dir);
  	result += loadFlashram(saveFile_dir);
  	saveFile_deinit(saveFile_dir);

#ifdef PS3
//	if (result) menu::MessageBox::getInstance().setMessage("Found & loaded save from USB device");
  	if (result) autoSaveLoaded = NATIVESAVEDEVICE_USB;
#else //PS3
  	switch (nativeSaveDevice)
  	{
  		case NATIVESAVEDEVICE_SD:
//			if (result) menu::MessageBox::getInstance().setMessage("Found & loaded save from SD card");
  			if (result) autoSaveLoaded = NATIVESAVEDEVICE_SD;
  			break;
  		case NATIVESAVEDEVICE_USB:
//			if (result) menu::MessageBox::getInstance().setMessage("Found & loaded save from USB device");
  			if (result) autoSaveLoaded = NATIVESAVEDEVICE_USB;
  			break;
  		case NATIVESAVEDEVICE_CARDA:
//			if (result) menu::MessageBox::getInstance().setMessage("Found & loaded save from memcard in slot A");
  			if (result) autoSaveLoaded = NATIVESAVEDEVICE_CARDA;
  			break;
  		case NATIVESAVEDEVICE_CARDB:
 //			if (result) menu::MessageBox::getInstance().setMessage("Found & loaded save from memcard in slot B");
  			if (result) autoSaveLoaded = NATIVESAVEDEVICE_CARDB;
  			break;
  	}
#endif //!PS3
  }
	return 0;
}

static void gfx_info_init(void){
	gfx_info.MemoryBswaped = TRUE;
	gfx_info.HEADER = (BYTE*)ROM_HEADER;
	gfx_info.RDRAM = (BYTE*)rdram;
	gfx_info.DMEM = (BYTE*)SP_DMEM;
	gfx_info.IMEM = (BYTE*)SP_IMEM;
	gfx_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
	gfx_info.DPC_START_REG = &(dpc_register.dpc_start);
	gfx_info.DPC_END_REG = &(dpc_register.dpc_end);
	gfx_info.DPC_CURRENT_REG = &(dpc_register.dpc_current);
	gfx_info.DPC_STATUS_REG = &(dpc_register.dpc_status);
	gfx_info.DPC_CLOCK_REG = &(dpc_register.dpc_clock);
	gfx_info.DPC_BUFBUSY_REG = &(dpc_register.dpc_bufbusy);
	gfx_info.DPC_PIPEBUSY_REG = &(dpc_register.dpc_pipebusy);
	gfx_info.DPC_TMEM_REG = &(dpc_register.dpc_tmem);
	gfx_info.VI_STATUS_REG = &(vi_register.vi_status);
	gfx_info.VI_ORIGIN_REG = &(vi_register.vi_origin);
	gfx_info.VI_WIDTH_REG = &(vi_register.vi_width);
	gfx_info.VI_INTR_REG = &(vi_register.vi_v_intr);
	gfx_info.VI_V_CURRENT_LINE_REG = &(vi_register.vi_current);
	gfx_info.VI_TIMING_REG = &(vi_register.vi_burst);
	gfx_info.VI_V_SYNC_REG = &(vi_register.vi_v_sync);
	gfx_info.VI_H_SYNC_REG = &(vi_register.vi_h_sync);
	gfx_info.VI_LEAP_REG = &(vi_register.vi_leap);
	gfx_info.VI_H_START_REG = &(vi_register.vi_h_start);
	gfx_info.VI_V_START_REG = &(vi_register.vi_v_start);
	gfx_info.VI_V_BURST_REG = &(vi_register.vi_v_burst);
	gfx_info.VI_X_SCALE_REG = &(vi_register.vi_x_scale);
	gfx_info.VI_Y_SCALE_REG = &(vi_register.vi_y_scale);
	gfx_info.CheckInterrupts = dummy_func;
	initiateGFX(gfx_info);
}

static void audio_info_init(void){
	audio_info.MemoryBswaped = TRUE;
	audio_info.HEADER = (BYTE*)ROM_HEADER;
	audio_info.RDRAM = (BYTE*)rdram;
	audio_info.DMEM = (BYTE*)SP_DMEM;
	audio_info.IMEM = (BYTE*)SP_IMEM;
	audio_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
	audio_info.AI_DRAM_ADDR_REG = &(ai_register.ai_dram_addr);
	audio_info.AI_LEN_REG = &(ai_register.ai_len);
	audio_info.AI_CONTROL_REG = &(ai_register.ai_control);
	audio_info.AI_STATUS_REG = &(ai_register.ai_status); // FIXME: This was set to dummy
	audio_info.AI_DACRATE_REG = &(ai_register.ai_dacrate);
	audio_info.AI_BITRATE_REG = &(ai_register.ai_bitrate);
	audio_info.CheckInterrupts = dummy_func;
	initiateAudio(audio_info);
}

void control_info_init(void){
	control_info.MemoryBswaped = TRUE;
	control_info.HEADER = (BYTE*)ROM_HEADER;
	control_info.Controls = Controls;
	int i;
	for (i=0; i<4; i++)
	  {
	     Controls[i].Present = FALSE;
	     Controls[i].RawData = FALSE;
	     Controls[i].Plugin = PLUGIN_NONE;
	  }
	initiateControllers(control_info);
}

static void rsp_info_init(void){
	static int cycle_count;
	rsp_info.MemoryBswaped = TRUE;
	rsp_info.RDRAM = (BYTE*)rdram;
	rsp_info.DMEM = (BYTE*)SP_DMEM;
	rsp_info.IMEM = (BYTE*)SP_IMEM;
	rsp_info.MI_INTR_REG = &MI_register.mi_intr_reg;
	rsp_info.SP_MEM_ADDR_REG = &sp_register.sp_mem_addr_reg;
	rsp_info.SP_DRAM_ADDR_REG = &sp_register.sp_dram_addr_reg;
	rsp_info.SP_RD_LEN_REG = &sp_register.sp_rd_len_reg;
	rsp_info.SP_WR_LEN_REG = &sp_register.sp_wr_len_reg;
	rsp_info.SP_STATUS_REG = &sp_register.sp_status_reg;
	rsp_info.SP_DMA_FULL_REG = &sp_register.sp_dma_full_reg;
	rsp_info.SP_DMA_BUSY_REG = &sp_register.sp_dma_busy_reg;
	rsp_info.SP_PC_REG = &rsp_register.rsp_pc;
	rsp_info.SP_SEMAPHORE_REG = &sp_register.sp_semaphore_reg;
	rsp_info.DPC_START_REG = &dpc_register.dpc_start;
	rsp_info.DPC_END_REG = &dpc_register.dpc_end;
	rsp_info.DPC_CURRENT_REG = &dpc_register.dpc_current;
	rsp_info.DPC_STATUS_REG = &dpc_register.dpc_status;
	rsp_info.DPC_CLOCK_REG = &dpc_register.dpc_clock;
	rsp_info.DPC_BUFBUSY_REG = &dpc_register.dpc_bufbusy;
	rsp_info.DPC_PIPEBUSY_REG = &dpc_register.dpc_pipebusy;
	rsp_info.DPC_TMEM_REG = &dpc_register.dpc_tmem;
	rsp_info.CheckInterrupts = dummy_func;
	rsp_info.ProcessDlistList = processDList;
	rsp_info.ProcessAlistList = processAList;
	rsp_info.ProcessRdpList = processRDPList;
	rsp_info.ShowCFB = showCFB;
	initiateRSP(rsp_info,(DWORD*)&cycle_count);
}

void stop_it() { r4300.stop = 1; }

#ifdef __GX__
void video_mode_init(GXRModeObj *videomode,unsigned int *fb1, unsigned int *fb2)
{
	vmode = videomode;
	rmode = videomode;
	xfb[0] = fb1;
	xfb[1] = fb2;
}
#endif //__GX__

void setOption(char* key, char value){
	for(unsigned int i=0; i<sizeof(OPTIONS)/sizeof(OPTIONS[0]); ++i){
		if(!strcmp(OPTIONS[i].key, key)){
			if(value >= OPTIONS[i].min && value <= OPTIONS[i].max)
				*OPTIONS[i].value = value;
			break;
		}
	}
}

void handleConfigPair(char* kv){
	char* vs = kv;
	while(*vs != ' ' && *vs != '\t' && *vs != ':' && *vs != '=')
			++vs;
	*(vs++) = 0;
	while(*vs == ' ' || *vs == '\t' || *vs == ':' || *vs == '=')
			++vs;

	setOption(kv, atoi(vs));
}

void readConfig(FILE* f){
	char line[256];
	while(fgets(line, 256, f)){
		if(line[0] == '#') continue;
		handleConfigPair(line);
	}
}

void writeConfig(FILE* f){
	for(unsigned int i=0; i<sizeof(OPTIONS)/sizeof(OPTIONS[0]); ++i){
		fprintf(f, "%s = %d\n", OPTIONS[i].key, *OPTIONS[i].value);
	}
}
