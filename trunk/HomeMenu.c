	#include "HomeMenu.h"

#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>
#include "asndlib.h"

#include "HomeMenu_snd_tick.h"
#include "HomeMenu_snd_exit.h"
#include "HomeMenu_snd_popup.h"
#include "HomeMenu_snd_popdown.h"
#include "HomeMenu_snd_ping.h"

// set HomeMenu to uninitialized
bool HomeMenu_initialized = false;
bool HomeMenu_active = false;

// Factory function
bool HomeMenu_Init(int width, int height, void* framebuffer0, void* framebuffer1, u8 framebufferIndex)
{
	// Do not reinitialize.
	if (HomeMenu_initialized)
		return false;
	
	HomeMenu_screenWidth = width;
	HomeMenu_screenHeight = height;
	__HomeMenu_fb[0] = framebuffer0;
	__HomeMenu_fb[1] = framebuffer1;
	__HomeMenu_fbi = framebufferIndex;
	__HomeMenu_fbi0 = framebufferIndex;
	__HomeMenu_rumbleIntensity = 0;
	__HomeMenu_gfx = HM_GFX_FAILSAFE;		// default to failsafe drawing code
	__HomeMenu_snd = HM_SND_NOSOUND;
	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) 
		__HomeMenu_xRatio = 0.75f;
	else
		__HomeMenu_xRatio = 1;

	// prepare buffer for screenshot to be used as our background
	HomeMenu_tex_bg = memalign(32, GX_GetTexBufferSize(HomeMenu_screenWidth, HomeMenu_screenHeight, GX_TF_RGBA8, GX_FALSE, 1));

	// set callbacks to NULL
	HomeMenu_BeforeShowMenu = NULL;
	HomeMenu_AfterShowMenu = NULL;
	HomeMenu_BeforeDraw = NULL;
	HomeMenu_AfterDraw = NULL;
	HomeMenu_BeforeHideMenu = NULL;
	HomeMenu_AfterHideMenu = NULL;
	
	// prepare images
	HomeMenu_top.texture = &HomeMenu_tex_top;
	HomeMenu_top.w = 4;	HomeMenu_top.h = 112;
	HomeMenu_top.x = HomeMenu_top.y = HomeMenu_top.r = HomeMenu_top.g = HomeMenu_top.b = HomeMenu_top.a = 0xFF;
	HomeMenu_top.t = 0;	HomeMenu_top.s = 1;	HomeMenu_top.visible = true;

	HomeMenu_top_hover.texture = &HomeMenu_tex_top_hover;
	HomeMenu_top_hover.w = 4;	HomeMenu_top_hover.h = 112;
	HomeMenu_top_hover.x = HomeMenu_top_hover.y = HomeMenu_top_hover.r = HomeMenu_top_hover.g = HomeMenu_top_hover.b = 0xFF;
	HomeMenu_top_hover.a = HomeMenu_top_hover.t = 0;	HomeMenu_top_hover.s = 1;	HomeMenu_top_hover.visible = true;

	HomeMenu_top_active.texture = &HomeMenu_tex_top_active;
	HomeMenu_top_active.w = 4;	HomeMenu_top_active.h = 112;
	HomeMenu_top_active.x = HomeMenu_top_active.y = HomeMenu_top_active.r = HomeMenu_top_active.g = HomeMenu_top_active.b = 0xFF;
	HomeMenu_top_active.a = HomeMenu_top_active.t = 0;	HomeMenu_top_active.s = 1;	HomeMenu_top_active.visible = true;

	HomeMenu_bottom.texture = &HomeMenu_tex_bottom;
	HomeMenu_bottom.w = 4;	HomeMenu_bottom.h = 112;
	HomeMenu_bottom.x = HomeMenu_bottom.y = HomeMenu_bottom.r = HomeMenu_bottom.g = HomeMenu_bottom.b = HomeMenu_bottom.a = 0xFF;
	HomeMenu_bottom.t = 0;	HomeMenu_bottom.s = 1;	HomeMenu_bottom.visible = true;

	HomeMenu_bottom_hover.texture = &HomeMenu_tex_bottom_hover;
	HomeMenu_bottom_hover.w = 4;	HomeMenu_bottom_hover.h = 112;
	HomeMenu_bottom_hover.x = HomeMenu_bottom_hover.y = HomeMenu_bottom_hover.r = HomeMenu_bottom_hover.g = HomeMenu_bottom_hover.b = 0xFF;
	HomeMenu_bottom_hover.a = HomeMenu_bottom_hover.t = 0;	HomeMenu_bottom_hover.s = 1;	HomeMenu_bottom_hover.visible = true;

	HomeMenu_bottom_active.texture = &HomeMenu_tex_bottom_active;
	HomeMenu_bottom_active.w = 4;	HomeMenu_bottom_active.h = 112;
	HomeMenu_bottom_active.x = HomeMenu_bottom_active.y = HomeMenu_bottom_active.r = HomeMenu_bottom_active.g = HomeMenu_bottom_active.b = 0xFF;
	HomeMenu_bottom_active.a = HomeMenu_bottom_active.t = 0;	HomeMenu_bottom_active.s = 1;	HomeMenu_bottom_active.visible = true;

	HomeMenu_text_top.texture = &HomeMenu_tex_text_top;
	HomeMenu_text_top.w = 328;	HomeMenu_text_top.h = 36;
	HomeMenu_text_top.x = HomeMenu_text_top.y = HomeMenu_text_top.r = HomeMenu_text_top.g = HomeMenu_text_top.b = HomeMenu_text_top.a = 0xFF;
	HomeMenu_text_top.t = 0;	HomeMenu_text_top.s = 1;	HomeMenu_text_top.visible = true;

	HomeMenu_text_bottom.texture = &HomeMenu_tex_text_bottom;
	HomeMenu_text_bottom.w = 276;	HomeMenu_text_bottom.h = 24;
	HomeMenu_text_bottom.x = HomeMenu_text_bottom.y = HomeMenu_text_bottom.r = HomeMenu_text_bottom.g = HomeMenu_text_bottom.b = HomeMenu_text_bottom.a = 0xFF;
	HomeMenu_text_bottom.t = 0;	HomeMenu_text_bottom.s = 1;	HomeMenu_text_bottom.visible = true;

	HomeMenu_wiimote.texture = &HomeMenu_tex_wiimote;
	HomeMenu_wiimote.w = 88;	HomeMenu_wiimote.h = 164;
	HomeMenu_wiimote.x = HomeMenu_wiimote.y = HomeMenu_wiimote.r = HomeMenu_wiimote.g = HomeMenu_wiimote.b = HomeMenu_wiimote.a = 0xFF;
	HomeMenu_wiimote.t = 0;	HomeMenu_wiimote.s = 1;	HomeMenu_wiimote.visible = true;
	
	HomeMenu_battery_info.texture = &HomeMenu_tex_battery_info;
	HomeMenu_battery_info.w = 448;	HomeMenu_battery_info.h = 44;
	HomeMenu_battery_info.x = HomeMenu_battery_info.y = HomeMenu_battery_info.r = HomeMenu_battery_info.g = HomeMenu_battery_info.b = HomeMenu_battery_info.a = 0xFF;
	HomeMenu_battery_info.t = 0;	HomeMenu_battery_info.s = 1;	HomeMenu_battery_info.visible = true;

	HomeMenu_battery[0].texture = &HomeMenu_tex_battery0;
	HomeMenu_battery[0].w = 44;	HomeMenu_battery[0].h = 24;
	HomeMenu_battery[0].x = HomeMenu_battery[0].y = HomeMenu_battery[0].r = HomeMenu_battery[0].g = HomeMenu_battery[0].b = HomeMenu_battery[0].a = 0xFF;
	HomeMenu_battery[0].t = 0;	HomeMenu_battery[0].s = 1;	HomeMenu_battery[0].visible = true;

	HomeMenu_battery[1].texture = &HomeMenu_tex_battery0;
	HomeMenu_battery[1].w = 44;	HomeMenu_battery[1].h = 24;
	HomeMenu_battery[1].x = HomeMenu_battery[1].y = HomeMenu_battery[1].r = HomeMenu_battery[1].g = HomeMenu_battery[1].b = HomeMenu_battery[1].a = 0xFF;
	HomeMenu_battery[1].t = 0;	HomeMenu_battery[1].s = 1;	HomeMenu_battery[1].visible = true;

	HomeMenu_battery[2].texture = &HomeMenu_tex_battery0;
	HomeMenu_battery[2].w = 44;	HomeMenu_battery[2].h = 24;
	HomeMenu_battery[2].x = HomeMenu_battery[2].y = HomeMenu_battery[2].r = HomeMenu_battery[2].g = HomeMenu_battery[2].b = HomeMenu_battery[2].a = 0xFF;
	HomeMenu_battery[2].t = 0;	HomeMenu_battery[2].s = 1;	HomeMenu_battery[2].visible = true;

	HomeMenu_battery[3].texture = &HomeMenu_tex_battery0;
	HomeMenu_battery[3].w = 44;	HomeMenu_battery[3].h = 24;
	HomeMenu_battery[3].x = HomeMenu_battery[3].y = HomeMenu_battery[3].r = HomeMenu_battery[3].g = HomeMenu_battery[3].b = HomeMenu_battery[3].a = 0xFF;
	HomeMenu_battery[3].t = 0;	HomeMenu_battery[3].s = 1;	HomeMenu_battery[3].visible = true;

	HomeMenu_p[0].texture = &HomeMenu_tex_p1;
	HomeMenu_p[0].w = 28;	HomeMenu_p[0].h = 20;
	HomeMenu_p[0].x = HomeMenu_p[0].y = HomeMenu_p[0].r = HomeMenu_p[0].g = HomeMenu_p[0].b = HomeMenu_p[0].a = 0xFF;
	HomeMenu_p[0].t = 0;	HomeMenu_p[0].s = 1;	HomeMenu_p[0].visible = true;

	HomeMenu_p[1].texture = &HomeMenu_tex_p2;
	HomeMenu_p[1].w = 28;	HomeMenu_p[1].h = 20;
	HomeMenu_p[1].x = HomeMenu_p[1].y = HomeMenu_p[1].r = HomeMenu_p[1].g = HomeMenu_p[1].b = HomeMenu_p[1].a = 0xFF;
	HomeMenu_p[1].t = 0;	HomeMenu_p[1].s = 1;	HomeMenu_p[1].visible = true;

	HomeMenu_p[2].texture = &HomeMenu_tex_p3;
	HomeMenu_p[2].w = 28;	HomeMenu_p[2].h = 20;
	HomeMenu_p[2].x = HomeMenu_p[2].y = HomeMenu_p[2].r = HomeMenu_p[2].g = HomeMenu_p[2].b = HomeMenu_p[2].a = 0xFF;
	HomeMenu_p[2].t = 0;	HomeMenu_p[2].s = 1;	HomeMenu_p[2].visible = true;

	HomeMenu_p[3].texture = &HomeMenu_tex_p4;
	HomeMenu_p[3].w = 28;	HomeMenu_p[3].h = 20;
	HomeMenu_p[3].x = HomeMenu_p[3].y = HomeMenu_p[3].r = HomeMenu_p[3].g = HomeMenu_p[3].b = HomeMenu_p[3].a = 0xFF;
	HomeMenu_p[3].t = 0;	HomeMenu_p[3].s = 1;	HomeMenu_p[3].visible = true;

	HomeMenu_button_wiiMenu.texture = &HomeMenu_tex_button_wiiMenu;
	HomeMenu_button_wiiMenu.w = 240;	HomeMenu_button_wiiMenu.h = 104;
	HomeMenu_button_wiiMenu.x = HomeMenu_button_wiiMenu.y = HomeMenu_button_wiiMenu.r = HomeMenu_button_wiiMenu.g = HomeMenu_button_wiiMenu.b = 0xFF;
	HomeMenu_button_wiiMenu.a = HomeMenu_button_wiiMenu.t = 0;	HomeMenu_button_wiiMenu.s = 1;	HomeMenu_button_wiiMenu.visible = true;

	HomeMenu_button_wiiMenu_active.texture = &HomeMenu_tex_button_wiiMenu_active;
	HomeMenu_button_wiiMenu_active.w = 240;	HomeMenu_button_wiiMenu_active.h = 104;
	HomeMenu_button_wiiMenu_active.x = HomeMenu_button_wiiMenu_active.y = HomeMenu_button_wiiMenu_active.r = HomeMenu_button_wiiMenu_active.g = HomeMenu_button_wiiMenu_active.b = 0xFF;
	HomeMenu_button_wiiMenu_active.a = HomeMenu_button_wiiMenu_active.t = 0;	HomeMenu_button_wiiMenu_active.s = 1;	HomeMenu_button_wiiMenu_active.visible = true;

	HomeMenu_button_loader.texture = &HomeMenu_tex_button_loader;
	HomeMenu_button_loader.w = 240;	HomeMenu_button_loader.h = 104;
	HomeMenu_button_loader.x = HomeMenu_button_loader.y = HomeMenu_button_loader.r = HomeMenu_button_loader.g = HomeMenu_button_loader.b = 0xFF;
	HomeMenu_button_loader.a = HomeMenu_button_loader.t = 0;	HomeMenu_button_loader.s = 1;	HomeMenu_button_loader.visible = true;

	HomeMenu_button_loader_active.texture = &HomeMenu_tex_button_loader_active;
	HomeMenu_button_loader_active.w = 240;	HomeMenu_button_loader_active.h = 104;
	HomeMenu_button_loader_active.x = HomeMenu_button_loader_active.y = HomeMenu_button_loader_active.r = HomeMenu_button_loader_active.g = HomeMenu_button_loader_active.b = 0xFF;
	HomeMenu_button_loader_active.a = HomeMenu_button_loader_active.t = 0;	HomeMenu_button_loader_active.s = 1;	HomeMenu_button_loader_active.visible = true;

	HomeMenu_button_close.texture = &HomeMenu_tex_button_close;
	HomeMenu_button_close.w = 184;	HomeMenu_button_close.h = 56;
	HomeMenu_button_close.x = HomeMenu_button_close.y = HomeMenu_button_close.r = HomeMenu_button_close.g = HomeMenu_button_close.b = HomeMenu_button_close.a = 0xFF;
	HomeMenu_button_close.t = 0;	HomeMenu_button_close.s = 1;	HomeMenu_button_close.visible = true;

	HomeMenu_pointer[0].texture = &HomeMenu_tex_p1_point;
	HomeMenu_pointer[0].w = 96;	HomeMenu_pointer[0].h = 96;
	HomeMenu_pointer[0].x = HomeMenu_pointer[0].y = HomeMenu_pointer[0].r = HomeMenu_pointer[0].g = HomeMenu_pointer[0].b = HomeMenu_pointer[0].a = 0xFF;
	HomeMenu_pointer[0].t = 0;	HomeMenu_pointer[0].s = 1;	HomeMenu_pointer[0].visible = false;

	HomeMenu_pointer[1].texture = &HomeMenu_tex_p2_point;
	HomeMenu_pointer[1].w = 96;	HomeMenu_pointer[1].h = 96;
	HomeMenu_pointer[1].x = HomeMenu_pointer[1].y = HomeMenu_pointer[1].r = HomeMenu_pointer[1].g = HomeMenu_pointer[1].b = HomeMenu_pointer[1].a = 0xFF;
	HomeMenu_pointer[1].t = 0;	HomeMenu_pointer[1].s = 1;	HomeMenu_pointer[1].visible = false;

	HomeMenu_pointer[2].texture = &HomeMenu_tex_p3_point;
	HomeMenu_pointer[2].w = 96;	HomeMenu_pointer[2].h = 96;
	HomeMenu_pointer[2].x = HomeMenu_pointer[2].y = HomeMenu_pointer[2].r = HomeMenu_pointer[2].g = HomeMenu_pointer[2].b = HomeMenu_pointer[2].a = 0xFF;
	HomeMenu_pointer[2].t = 0;	HomeMenu_pointer[2].s = 1;	HomeMenu_pointer[2].visible = false;

	HomeMenu_pointer[3].texture = &HomeMenu_tex_p4_point;
	HomeMenu_pointer[3].w = 96;	HomeMenu_pointer[3].h = 96;
	HomeMenu_pointer[3].x = HomeMenu_pointer[3].y = HomeMenu_pointer[3].r = HomeMenu_pointer[3].g = HomeMenu_pointer[3].b = HomeMenu_pointer[3].a = 0xFF;
	HomeMenu_pointer[3].t = 0;	HomeMenu_pointer[3].s = 1;	HomeMenu_pointer[3].visible = false;
	
	HomeMenu_background.texture = HomeMenu_tex_bg;
	HomeMenu_background.w = HomeMenu_screenWidth;	HomeMenu_background.h = HomeMenu_screenHeight;
	HomeMenu_background.x = HomeMenu_background.w/2; HomeMenu_background.y = HomeMenu_background.h/2;
	HomeMenu_background.r = HomeMenu_background.g = HomeMenu_background.b = HomeMenu_background.a = 0xFF;
	HomeMenu_background.t = 0;	HomeMenu_background.s = 1;	HomeMenu_background.visible = true;

	// put pointers to images in a nice convenient array
	HomeMenu_images[0]  = &HomeMenu_background;
	HomeMenu_images[1]  = &HomeMenu_top;
	HomeMenu_images[2]  = &HomeMenu_top_hover;
	HomeMenu_images[3]  = &HomeMenu_top_active;
	HomeMenu_images[4]  = &HomeMenu_bottom;
	HomeMenu_images[5]  = &HomeMenu_bottom_hover;
	HomeMenu_images[6]  = &HomeMenu_bottom_active;
	HomeMenu_images[7]  = &HomeMenu_text_top;
	HomeMenu_images[8]  = &HomeMenu_text_bottom;
	HomeMenu_images[9]  = &HomeMenu_wiimote;
	HomeMenu_images[10] = &HomeMenu_battery_info;
	HomeMenu_images[11] = &HomeMenu_battery[0];
	HomeMenu_images[12] = &HomeMenu_battery[1];
	HomeMenu_images[13] = &HomeMenu_battery[2];
	HomeMenu_images[14] = &HomeMenu_battery[3];
	HomeMenu_images[15] = &HomeMenu_p[0];
	HomeMenu_images[16] = &HomeMenu_p[1];
	HomeMenu_images[17] = &HomeMenu_p[2];
	HomeMenu_images[18] = &HomeMenu_p[3];
	HomeMenu_images[19] = &HomeMenu_button_wiiMenu;
	HomeMenu_images[20] = &HomeMenu_button_wiiMenu_active;
	HomeMenu_images[21] = &HomeMenu_button_loader;
	HomeMenu_images[22] = &HomeMenu_button_loader_active;
	HomeMenu_images[23] = &HomeMenu_button_close;
	HomeMenu_images[24] = &HomeMenu_pointer[0];
	HomeMenu_images[25] = &HomeMenu_pointer[1];
	HomeMenu_images[26] = &HomeMenu_pointer[2];
	HomeMenu_images[27] = &HomeMenu_pointer[3];
	
	// prepare our HomeMenu_cursors
	p1.pointer = &HomeMenu_pointer[0];
	p2.pointer = &HomeMenu_pointer[1];
	p3.pointer = &HomeMenu_pointer[2];
	p4.pointer = &HomeMenu_pointer[3];
	HomeMenu_cursors[0] = p1;
	HomeMenu_cursors[1] = p2;
	HomeMenu_cursors[2] = p3;
	HomeMenu_cursors[3] = p4;

	// prepare our faders
	HomeMenu_fader = 0;

	// Hide all sprites:
	__HomeMenu_setVisible(false);	// set visible to false
	__HomeMenu_moveAll(-57);		// draw offscreen

	__HomeMenu_resetCursors();
	
	// if we've gotten this far, all is well!
	return true;
}


void HomeMenu_SetGFX(u8 lib) {
	__HomeMenu_gfx = lib;
}

void HomeMenu_SetSND(u8 lib) {
	switch (lib) {
	  case HM_SND_ASND:
		__HomeMenu_snd = lib;
		// Initialize sound in case it hasn't been done already
		ASND_Init();	// I don't think this causes problems if called twice (I did some quick checks).
		ASND_Pause(0);	// resume playback (if it had been paused)
		break;
	  case HM_SND_NOSOUND:
		__HomeMenu_snd = lib;
		break;
	  default:	// no support for SDL yet
		__HomeMenu_snd = HM_SND_NOSOUND;
		break;
	}
}

void HomeMenu_Destroy()
{
	if (HomeMenu_tex_bg != NULL) {
		free(MEM_K1_TO_K0(HomeMenu_tex_bg));
		HomeMenu_tex_bg = NULL;
	}
}


bool HomeMenu_Show()
{
	if (HomeMenu_BeforeShowMenu != NULL) HomeMenu_BeforeShowMenu();
	
	// Set GX to our liking
	GX_SetScissorBoxOffset(0, 0);
	GX_SetScissor(0, 0, HomeMenu_screenWidth, HomeMenu_screenHeight);

	// Set WPAD Data Format
	// (Users should restore their Data Format when menu is closed)
	// (If it's possible to automatically restore previous data format on exit, I'll implement that later.)
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_2, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_3, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(0, HomeMenu_screenWidth, HomeMenu_screenHeight);

	// take a screenshot to be used as our background
	GX_SetTexCopySrc(0, 0, HomeMenu_screenWidth, HomeMenu_screenHeight);
	GX_SetTexCopyDst(HomeMenu_screenWidth, HomeMenu_screenHeight, GX_TF_RGBA8, GX_FALSE);
	GX_CopyTex(HomeMenu_tex_bg, GX_FALSE);
	GX_PixModeSync();
	DCFlushRange(HomeMenu_tex_bg, GX_GetTexBufferSize(HomeMenu_screenWidth, HomeMenu_screenHeight, GX_TF_RGBA8, GX_FALSE, 1));

	// Startup animation:
	__HomeMenu_setVisible(true);
	HomeMenu_cursors[0].pointer->visible = false;
	HomeMenu_cursors[1].pointer->visible = false;
	HomeMenu_cursors[2].pointer->visible = false;
	HomeMenu_cursors[3].pointer->visible = false;
	HomeMenu_top_active.a = 0;
	HomeMenu_bottom_active.a = 0;
	HomeMenu_button_wiiMenu.s = 1;
	HomeMenu_button_loader.s = 1;

	__HomeMenu_playPCM(HomeMenu_snd_popup, HomeMenu_snd_popup_size, 128, 128, false);
	__HomeMenu_slide(false);
	__HomeMenu_updateTimer();
	
	if (HomeMenu_AfterShowMenu != NULL) HomeMenu_AfterShowMenu();

	//----------- Start of Main Menu Loop -----------//
	HomeMenu_active = true;
	while (HomeMenu_active)
	{
		if (HomeMenu_BeforeDraw != NULL) HomeMenu_BeforeDraw();
		
		// update value of '__HomeMenu_elapsed'
		__HomeMenu_updateTimer();
		
		// Do wiimote logic
		__HomeMenu_updateWiimotes();

		// render animations triggered by the cursor
		__HomeMenu_animate();
		
		int i;
		for (i = 0; i < 4; i++) {
			// Catch escape via "Home" Button
			if (WPAD_BUTTON_HOME & WPAD_ButtonsDown(i))
				HomeMenu_active = false;
			
			// Catch other clicks:
			if (WPAD_BUTTON_A & WPAD_ButtonsDown(i) && HomeMenu_topState[i] == HM_HOTSPOT_HOVER) {
				__HomeMenu_playPCM(HomeMenu_snd_ping, HomeMenu_snd_ping_size, 128, 128, false);
				HomeMenu_topState[i] = HM_HOTSPOT_ACTIVATED;
			}
				
			if (WPAD_BUTTON_A & WPAD_ButtonsDown(i) && HomeMenu_bottomState[i] == HM_HOTSPOT_HOVER) {
				__HomeMenu_playPCM(HomeMenu_snd_ping, HomeMenu_snd_ping_size, 128, 128, false);
				HomeMenu_bottomState[i] = HM_HOTSPOT_ACTIVATED;
			}

			if (WPAD_BUTTON_A & WPAD_ButtonsDown(i) && HomeMenu_wiiMenuState[i] == HM_HOTSPOT_HOVER) {
				__HomeMenu_playPCM(HomeMenu_snd_ping, HomeMenu_snd_ping_size, 96, 32, false);
				HomeMenu_wiiMenuState[i] = HM_HOTSPOT_ACTIVATED;
			}
					
			if (WPAD_BUTTON_A & WPAD_ButtonsDown(i) && HomeMenu_loaderState[i] == HM_HOTSPOT_HOVER) {
				__HomeMenu_playPCM(HomeMenu_snd_ping, HomeMenu_snd_ping_size, 32, 96, false);
				HomeMenu_loaderState[i] = HM_HOTSPOT_ACTIVATED;
			}
			
			// Carry out hotspot actions
			if ((HomeMenu_topState[i]     & HM_HOTSPOT_ACTIVE) == HM_HOTSPOT_ACTIVE) {
				HomeMenu_topState[i]    = HM_HOTSPOT_INACTIVE;
				HomeMenu_active = false;
			}
			
			if ((HomeMenu_bottomState[i]  & HM_HOTSPOT_ACTIVE) == HM_HOTSPOT_ACTIVE) {
				HomeMenu_bottomState[i] = HM_HOTSPOT_INACTIVE;
				HomeMenu_active = false;
			}
			
			if ((HomeMenu_wiiMenuState[i] & HM_HOTSPOT_ACTIVE) == HM_HOTSPOT_ACTIVE) {
				HomeMenu_wiiMenuState[i] = HM_HOTSPOT_INACTIVE;
				// implement and call returnToMenu callback
				// manditory callback... since game has to clean up
				
				// fade to black
				__HomeMenu_playPCM(HomeMenu_snd_exit, HomeMenu_snd_exit_size, 128, 128, true);
				for (HomeMenu_fader = 0; HomeMenu_fader < 256; HomeMenu_fader++) {
					HomeMenu_fader = MIN(255, HomeMenu_fader + 6);
					__HomeMenu_draw();
				}
				
				HomeMenu_active = false;
				SYS_ResetSystem(SYS_RETURNTOMENU,0,0);
			}
					
			if ((HomeMenu_loaderState[i]  & HM_HOTSPOT_ACTIVE) == HM_HOTSPOT_ACTIVE) {
				HomeMenu_loaderState[i]  = HM_HOTSPOT_INACTIVE;
				// implement and call returnToLoader callback
				// manditory callback... since game has to clean up
				
				// fade to black
				__HomeMenu_playPCM(HomeMenu_snd_exit, HomeMenu_snd_exit_size, 128, 128, true);
				for (HomeMenu_fader = 0; HomeMenu_fader < 256; HomeMenu_fader++) {
					HomeMenu_fader = MIN(255, HomeMenu_fader + 6);
					__HomeMenu_draw();
				}
				
				HomeMenu_active = false;
				exit(1);	// eventually, return a value.
			}
		}
		
		// draw
		HomeMenu_fader = 0;	// not necessary, but good just in case.
		__HomeMenu_draw();
		
		if (HomeMenu_AfterDraw != NULL) HomeMenu_AfterDraw();
	}
	__HomeMenu_playPCM(HomeMenu_snd_popdown, HomeMenu_snd_popdown_size, 128, 128, false);
	//------------ End of Main Menu Loop ------------//


	if (HomeMenu_BeforeHideMenu != NULL) HomeMenu_BeforeHideMenu();
	
	__HomeMenu_resetCursors();
	
	// close animation
	__HomeMenu_slide(true);
	__HomeMenu_setVisible(false);

	if (__HomeMenu_fbi != __HomeMenu_fbi0)
		__HomeMenu_draw();	// draw a second time so we end on the right buffer.
	// leave a copy of background in framebuffer to avoid flicker on exit
	__HomeMenu_drawImage(&HomeMenu_background);	// is there a better way of doing this?

	
	if (HomeMenu_AfterHideMenu != NULL) HomeMenu_AfterHideMenu();

	return true;
}


// the sole purpose of this function is so other threads can tell the HomeMenu to close
void HomeMenu_Hide()
{
	HomeMenu_active = false;
}


void HomeMenu_SetBeforeShowMenu(void (*func)())
{
	HomeMenu_BeforeShowMenu = func;
}

void HomeMenu_SetAfterShowMenu(void (*func)())
{
	HomeMenu_AfterShowMenu = func;
}

void HomeMenu_SetBeforeDraw(void (*func)())
{
	HomeMenu_BeforeDraw = func;
}

void HomeMenu_SetAfterDraw(void (*func)())
{
	HomeMenu_AfterDraw = func;
}

void HomeMenu_SetBeforeHideMenu(void (*func)())
{
	HomeMenu_BeforeHideMenu = func;
}

void HomeMenu_SetAfterHideMenu(void (*func)())
{
	HomeMenu_AfterHideMenu = func;
}


void __HomeMenu_resetCursors()
{
	// set all hotspots to inactive, and prepare our HomeMenu_cursors (continued)
	int i;
	for (i = 0; i < 4; i++) {
		HomeMenu_topState[i]		= HomeMenu_bottomState[i] = HM_HOTSPOT_INACTIVE;	// Cursor is NOT hovering over buttons
		HomeMenu_wiiMenuState[i]	= HomeMenu_loaderState[i] = HM_HOTSPOT_INACTIVE;	// ditto
		
		HomeMenu_cursors[i].pointer->visible = false;
		HomeMenu_cursors[i].pointer->x = -100;
		HomeMenu_cursors[i].pointer->y = HomeMenu_screenHeight/2;
		HomeMenu_cursors[i].rumbleTimer = 0;
		HomeMenu_cursors[i].cooldownTimer = 0;
	}
	WPAD_Rumble(WPAD_CHAN_ALL, 0);		// turn off rumbling
}


void __HomeMenu_updateWiimotes()
{
	int i;
	for (i = 0; i < 4; i++) {
		// Enable/Disable rumble
		if (HomeMenu_cursors[i].rumbleTimer > 0) {

			__HomeMenu_rumbleIntensity = (__HomeMenu_rumbleIntensity + 1) % 3;	// cycle value throught 0-2.

			if (__HomeMenu_rumbleIntensity % 3 == 0)					// rumble only 2 out of 3 times.
				WPAD_Rumble(i, 0);
			else
				WPAD_Rumble(i, 1);
			
			HomeMenu_cursors[i].rumbleTimer -= __HomeMenu_elapsed;				// note: this might result in a negative value, not necessarily 0.

			if (HomeMenu_cursors[i].rumbleTimer <= 0)				// begin cooldown timer if rumble has expired.
				HomeMenu_cursors[i].cooldownTimer = RUMBLE_COOLDOWN;

		} else {
			WPAD_Rumble(i, 0);
			__HomeMenu_rumbleIntensity = 0;		// setup so the next 2 values are "on" values.
		}
		if (HomeMenu_cursors[i].cooldownTimer > 0)
			HomeMenu_cursors[i].cooldownTimer -= __HomeMenu_elapsed;	// note: ditto
	
		HomeMenu_wm_status[i] = WPAD_Probe(i, &HomeMenu_wm_type[i]);
		ir_t irData;
		
		if(HomeMenu_wm_status[i] == WPAD_ERR_NONE) {
			HomeMenu_battery[i].texture = &HomeMenu_tex_battery2;		// set according to battery level (later)
			HomeMenu_battery[i].a = 255;
			HomeMenu_p[i].a = 255;
			WPAD_IR(i, &irData);
			if (irData.smooth_valid) {	// if the pointer is visible
				HomeMenu_cursors[i].pointer->visible = true;
				HomeMenu_cursors[i].pointer->x = HomeMenu_cursors[i].pointer->w/2 + irData.sx - HomeMenu_screenWidth*0.30f;
				HomeMenu_cursors[i].pointer->y = HomeMenu_cursors[i].pointer->h/2 + irData.sy - HomeMenu_screenHeight*0.75f;
				HomeMenu_cursors[i].pointer->t = irData.angle;
			} else {
				// hide,
				HomeMenu_cursors[i].pointer->visible = false;
				// and put safely offscreen (doesn't trigger any areas)
				HomeMenu_cursors[i].pointer->x = -100;
				HomeMenu_cursors[i].pointer->y = HomeMenu_screenHeight/2;
			}
		} else {
			HomeMenu_cursors[i].pointer->visible = false;
			HomeMenu_battery[i].texture = &HomeMenu_tex_battery0;
			HomeMenu_battery[i].a = 96;
			HomeMenu_p[i].a = 96;
		}
		
		// See if cursor is above hotspots (oooh, shiny)
		if (HomeMenu_cursors[i].pointer->y < HomeMenu_top.y + HomeMenu_top.h/2) {
			if (HomeMenu_topState[i] == HM_HOTSPOT_INACTIVE)
				__HomeMenu_playPCM(HomeMenu_snd_tick, HomeMenu_snd_tick_size, 64, 64, false);
			if (!HomeMenu_topState[i] == HM_HOTSPOT_HOVER && HomeMenu_cursors[i].cooldownTimer <= 0)
				HomeMenu_cursors[i].rumbleTimer = SHORT_RUMBLE;
			HomeMenu_topState[i] |= HM_HOTSPOT_HOVER;
		} else
			HomeMenu_topState[i] &= (0xFF - HM_HOTSPOT_HOVER);
		
		if (HomeMenu_cursors[i].pointer->y > HomeMenu_bottom.y - HomeMenu_bottom.h/2) {
			if (HomeMenu_bottomState[i] == HM_HOTSPOT_INACTIVE)
				__HomeMenu_playPCM(HomeMenu_snd_tick, HomeMenu_snd_tick_size, 64, 64, false);
			if (!HomeMenu_bottomState[i] == HM_HOTSPOT_HOVER && HomeMenu_cursors[i].cooldownTimer <= 0)
				HomeMenu_cursors[i].rumbleTimer = SHORT_RUMBLE;
			HomeMenu_bottomState[i] |= HM_HOTSPOT_HOVER;
		} else
			HomeMenu_bottomState[i] &= (0xFF - HM_HOTSPOT_HOVER);
		
		u8 before = HomeMenu_wiiMenuState[i];	// value of wiiMenuHover prior to collision test
		
		// if on HomeMenu_button_wiiMenu's bounding box
		HomeMenu_wiiMenuState[i] &= (0xFF - HM_HOTSPOT_HOVER);
		if ((HomeMenu_cursors[i].pointer->y > HomeMenu_button_wiiMenu.y - 46) &&
			(HomeMenu_cursors[i].pointer->y < HomeMenu_button_wiiMenu.y + 46) &&
			(HomeMenu_cursors[i].pointer->x > HomeMenu_button_wiiMenu.x - 114) &&
			(HomeMenu_cursors[i].pointer->x < HomeMenu_button_wiiMenu.x + 114)) {
			// if over HomeMenu_button_wiiMenu's inner rectangle
			if ((HomeMenu_cursors[i].pointer->x > HomeMenu_button_wiiMenu.x - 114 + 46) &&
				(HomeMenu_cursors[i].pointer->x < HomeMenu_button_wiiMenu.x + 114 - 46))
					HomeMenu_wiiMenuState[i] |= HM_HOTSPOT_HOVER;
			// if over HomeMenu_button_wiiMenu's outer circles
			f32 angleL = atan2(HomeMenu_cursors[i].pointer->x - (HomeMenu_button_wiiMenu.x - 68), HomeMenu_cursors[i].pointer->y - HomeMenu_button_wiiMenu.y);
			f32 angleR = atan2(HomeMenu_cursors[i].pointer->x - (HomeMenu_button_wiiMenu.x + 68), HomeMenu_cursors[i].pointer->y - HomeMenu_button_wiiMenu.y);
			
			// left
			if (HomeMenu_cursors[i].pointer->x > HomeMenu_button_wiiMenu.x - 68 + 46*sin(angleL) && HomeMenu_cursors[i].pointer->x < HomeMenu_button_wiiMenu.x)
				HomeMenu_wiiMenuState[i] |= HM_HOTSPOT_HOVER;
			
			// right
			if (HomeMenu_cursors[i].pointer->x < HomeMenu_button_wiiMenu.x + 68 + 46*sin(angleR) && HomeMenu_cursors[i].pointer->x > HomeMenu_button_wiiMenu.x)
				HomeMenu_wiiMenuState[i] |= HM_HOTSPOT_HOVER;
		}
		
		if (before == HM_HOTSPOT_INACTIVE && HomeMenu_wiiMenuState[i] != HM_HOTSPOT_INACTIVE) {	// if we just rolled onto HomeMenu_button_wiiMenu
			__HomeMenu_playPCM(HomeMenu_snd_tick, HomeMenu_snd_tick_size, 96, 32, false);
			if (HomeMenu_cursors[i].cooldownTimer <= 0)
				HomeMenu_cursors[i].rumbleTimer = SHORT_RUMBLE;
		}
		
		before = HomeMenu_loaderState[i];	// value of wiiMenuHover prior to collision test
		
		// if on HomeMenu_button_loader's bounding box
		HomeMenu_loaderState[i] &= (0xFF - HM_HOTSPOT_HOVER);
		if ((HomeMenu_cursors[i].pointer->y > HomeMenu_button_loader.y - 46) &&
			(HomeMenu_cursors[i].pointer->y < HomeMenu_button_loader.y + 46) &&
			(HomeMenu_cursors[i].pointer->x > HomeMenu_button_loader.x - 114) &&
			(HomeMenu_cursors[i].pointer->x < HomeMenu_button_loader.x + 114)) {
			// if over HomeMenu_button_wiiMenu's inner rectangle
			if ((HomeMenu_cursors[i].pointer->x > HomeMenu_button_loader.x - 114 + 46) &&
				(HomeMenu_cursors[i].pointer->x < HomeMenu_button_loader.x + 114 - 46))
					HomeMenu_loaderState[i] |= HM_HOTSPOT_HOVER;
			// if over HomeMenu_button_wiiMenu's outer circles
			f32 angleL = atan2(HomeMenu_cursors[i].pointer->x - (HomeMenu_button_loader.x - 68), HomeMenu_cursors[i].pointer->y - HomeMenu_button_loader.y);
			f32 angleR = atan2(HomeMenu_cursors[i].pointer->x - (HomeMenu_button_loader.x + 68), HomeMenu_cursors[i].pointer->y - HomeMenu_button_loader.y);
			
			// left
			if (HomeMenu_cursors[i].pointer->x > HomeMenu_button_loader.x - 68 + 46*sin(angleL) && HomeMenu_cursors[i].pointer->x < HomeMenu_button_loader.x)
				HomeMenu_loaderState[i] |= HM_HOTSPOT_HOVER;
			
			// right
			if (HomeMenu_cursors[i].pointer->x < HomeMenu_button_loader.x + 68 + 46*sin(angleR) && HomeMenu_cursors[i].pointer->x > HomeMenu_button_loader.x)
				HomeMenu_loaderState[i] |= HM_HOTSPOT_HOVER;
		}
		
		if (before == HM_HOTSPOT_INACTIVE && HomeMenu_loaderState[i] != HM_HOTSPOT_INACTIVE) {	// if we just rolled onto HomeMenu_button_loader
			__HomeMenu_playPCM(HomeMenu_snd_tick, HomeMenu_snd_tick_size, 32, 96, false);
			if (HomeMenu_cursors[i].cooldownTimer <= 0)
				HomeMenu_cursors[i].rumbleTimer = SHORT_RUMBLE;
		}
	}
	
	WPAD_ScanPads();
}


void __HomeMenu_slide(bool reverse)
{
	int direction = 1;
	if (reverse)
		direction = -1;

	HomeMenu_button_loader_active.a = 0;
	HomeMenu_button_wiiMenu_active.a = 0;
		
	__HomeMenu_draw();
	
	// we want a shift of 114
	// this is not implemented with a timer because: a) it's not as smooth.  b) at this point, outside load should be minimal
	float shift;
	for (shift = 0; shift < 114; shift += 7) {
		
		__HomeMenu_moveAll(direction*(shift - 57));
		if (reverse) {
			HomeMenu_button_wiiMenu.a = 255 - (shift / 114.f) * 255;
			HomeMenu_button_loader.a = 255 - (shift / 114.f) * 255;
			HomeMenu_background.r = HomeMenu_background.g = HomeMenu_background.b = 0xFF - (HomeMenu_dimAmount - (shift / 114.f) * HomeMenu_dimAmount);
		} else {
			HomeMenu_button_wiiMenu.a = (shift / 114.f) * 255;
			HomeMenu_button_loader.a = (shift / 114.f) * 255;
			HomeMenu_background.r = HomeMenu_background.g = HomeMenu_background.b = 0xFF - ((shift / 114.f) * HomeMenu_dimAmount);
		}

		__HomeMenu_draw();
	}
	
	__HomeMenu_moveAll(direction*57);
	if (reverse) {
			HomeMenu_button_wiiMenu.a = 0;
			HomeMenu_button_loader.a = 0;
			HomeMenu_background.r = HomeMenu_background.g = HomeMenu_background.b = 0xFF;
			HomeMenu_top_hover.a = 0;
			HomeMenu_bottom_hover.a = 0;
		} else {
			HomeMenu_button_wiiMenu.a = 255;
			HomeMenu_button_loader.a = 255;
			HomeMenu_background.r = HomeMenu_background.g = HomeMenu_background.b = 0xFF - HomeMenu_dimAmount;
		}
	
	__HomeMenu_draw();	
}


void __HomeMenu_animate()
{
	// aggregate all hotspot states
	u8 topState, bottomState, wiiMenuState, loaderState;
	topState = bottomState = wiiMenuState = loaderState = HM_HOTSPOT_INACTIVE;
	int i;
	for (i = 0; i < 4; i++) {
		topState     |= HomeMenu_topState[i];
		bottomState  |= HomeMenu_bottomState[i];
		wiiMenuState |= HomeMenu_wiiMenuState[i];
		loaderState  |= HomeMenu_loaderState[i];
	}
	
	// potential bug here.  If the user activates a hotspot before hover its animation begins then the activation animation is skipped.
	// Since this is very unlikely to happen, I'll ignore it.  Otherwise I'd have to use more variables to keep track of animations.

	// top
	if ((topState & HM_HOTSPOT_ACTIVATED) == HM_HOTSPOT_ACTIVATED) {
		if (HomeMenu_top_hover.a != 0) {	// use this to keep track if we're fading in or out.  This means fading in.
			if (HomeMenu_top_active.a != 255)
				HomeMenu_top_active.a = MIN(255, HomeMenu_top_active.a + HomeMenu_fadeRate*__HomeMenu_elapsed);
			else
				HomeMenu_top_hover.a = 0;	// turn off hover layer once active layer is at full opacity.  This triggers the fade out as well.
		} else
			if (HomeMenu_top_active.a != 0) {
				HomeMenu_top_active.a = MAX(0, HomeMenu_top_active.a - HomeMenu_fadeRate*__HomeMenu_elapsed);
				if (HomeMenu_top_active.a == 0)		// break out of animation cycle
					topState = HM_HOTSPOT_ACTIVE;
			}
	} else
		if (topState == HM_HOTSPOT_HOVER)
		{
			if (HomeMenu_top_hover.a != 255)
				HomeMenu_top_hover.a = MIN(255, HomeMenu_top_hover.a + HomeMenu_fadeRate*__HomeMenu_elapsed);
		} else {
			if (HomeMenu_top_hover.a != 0)
				HomeMenu_top_hover.a = MAX(0, HomeMenu_top_hover.a - HomeMenu_fadeRate*__HomeMenu_elapsed);
		}

	// bottom
	if ((bottomState & HM_HOTSPOT_ACTIVATED) == HM_HOTSPOT_ACTIVATED) {
		if (HomeMenu_bottom_hover.a != 0) {	// use this to keep track if we're fading in or out.  This means fading in.
			if (HomeMenu_bottom_active.a != 255)
				HomeMenu_bottom_active.a = MIN(255, HomeMenu_bottom_active.a + HomeMenu_fadeRate*__HomeMenu_elapsed);
			else
				HomeMenu_bottom_hover.a = 0;	// turn off hover layer once active layer is at full opacity.  This triggers the fade out as well.
		} else
			if (HomeMenu_bottom_active.a != 0) {
				HomeMenu_bottom_active.a = MAX(0, HomeMenu_bottom_active.a - HomeMenu_fadeRate*__HomeMenu_elapsed);
				HomeMenu_wiimote.y = HomeMenu_bottom.y + 5 - HomeMenu_bottom_active.a*0.1f;
				if (HomeMenu_bottom_active.a == 0)		// break out of animation cycle
					bottomState = HM_HOTSPOT_ACTIVE;
			}
	} else
		if (bottomState == HM_HOTSPOT_HOVER)
		{
			if (HomeMenu_bottom_hover.a != 255) {
				HomeMenu_bottom_hover.a = MIN(255, HomeMenu_bottom_hover.a + HomeMenu_fadeRate*__HomeMenu_elapsed);
				HomeMenu_wiimote.y = HomeMenu_bottom.y + 5 - HomeMenu_bottom_hover.a*0.1f;
			}
		} else {
			if (HomeMenu_bottom_hover.a != 0) {
				HomeMenu_bottom_hover.a = MAX(0, HomeMenu_bottom_hover.a - HomeMenu_fadeRate*__HomeMenu_elapsed);
				HomeMenu_wiimote.y = HomeMenu_bottom.y + 5 - HomeMenu_bottom_hover.a*0.1f;
			}
		}

	// wiiMenu button
	if ((wiiMenuState & HM_HOTSPOT_ACTIVATED) == HM_HOTSPOT_ACTIVATED) {
		// step 4: button returns to normal color
		if (HomeMenu_button_wiiMenu_active.a != 0 && HomeMenu_button_wiiMenu.s == HomeMenu_hoverZoomLevel) {
			HomeMenu_button_wiiMenu_active.a = MAX(0, HomeMenu_button_wiiMenu_active.a - 1.5f*HomeMenu_fadeRate*__HomeMenu_elapsed);
			if (HomeMenu_button_wiiMenu_active.a == 0) 	// break out of animation cycle
				wiiMenuState = HM_HOTSPOT_ACTIVE;
		}

		// step 3: button pulls out
		if (HomeMenu_button_wiiMenu.s < HomeMenu_hoverZoomLevel && HomeMenu_button_wiiMenu_active.a == 255) {
			HomeMenu_button_wiiMenu.s = MIN(HomeMenu_hoverZoomLevel, HomeMenu_button_wiiMenu.s + HomeMenu_zoomRate*__HomeMenu_elapsed);
			HomeMenu_button_wiiMenu_active.s = HomeMenu_button_wiiMenu.s;
		}
		
		// step 2: button turns white
		if (HomeMenu_button_wiiMenu_active.a != 255 && HomeMenu_button_wiiMenu.s == HomeMenu_activeZoomLevel)
			HomeMenu_button_wiiMenu_active.a = MIN(255, HomeMenu_button_wiiMenu_active.a + 1.5f*HomeMenu_fadeRate*__HomeMenu_elapsed);
		
		// step 1: button pushes in
		if (HomeMenu_button_wiiMenu.s > HomeMenu_activeZoomLevel && HomeMenu_button_wiiMenu_active.a == 0) {
			HomeMenu_button_wiiMenu.s = MAX(HomeMenu_activeZoomLevel, HomeMenu_button_wiiMenu.s - HomeMenu_zoomRate*__HomeMenu_elapsed);
			HomeMenu_button_wiiMenu_active.s = HomeMenu_button_wiiMenu.s;
		}
	} else
		if (wiiMenuState == HM_HOTSPOT_HOVER)
		{
			if (HomeMenu_button_wiiMenu.s < HomeMenu_hoverZoomLevel) {
				HomeMenu_button_wiiMenu.s = MIN(HomeMenu_hoverZoomLevel, HomeMenu_button_wiiMenu.s + HomeMenu_zoomRate*__HomeMenu_elapsed);
				HomeMenu_button_wiiMenu_active.s = HomeMenu_button_wiiMenu.s;
			}
		} else {
			if (HomeMenu_button_wiiMenu.s > 1) {
				HomeMenu_button_wiiMenu.s = MAX(1, HomeMenu_button_wiiMenu.s - HomeMenu_zoomRate*__HomeMenu_elapsed);
				HomeMenu_button_wiiMenu_active.s = HomeMenu_button_wiiMenu.s;
			}
		}

	// loader button
	if ((loaderState & HM_HOTSPOT_ACTIVATED) == HM_HOTSPOT_ACTIVATED) {
		// step 4: button returns to normal color
		if (HomeMenu_button_loader_active.a != 0 && HomeMenu_button_loader.s == HomeMenu_hoverZoomLevel) {
			HomeMenu_button_loader_active.a = MAX(0, HomeMenu_button_loader_active.a - 1.5f*HomeMenu_fadeRate*__HomeMenu_elapsed);
			if (HomeMenu_button_loader_active.a == 0) 	// break out of animation cycle
				loaderState = HM_HOTSPOT_ACTIVE;
		}

		// step 3: button pulls out
		if (HomeMenu_button_loader.s < HomeMenu_hoverZoomLevel && HomeMenu_button_loader_active.a == 255) {
			HomeMenu_button_loader.s = MIN(HomeMenu_hoverZoomLevel, HomeMenu_button_loader.s + HomeMenu_zoomRate*__HomeMenu_elapsed);
			HomeMenu_button_loader_active.s = HomeMenu_button_loader.s;
		}
		
		// step 2: button turns white
		if (HomeMenu_button_loader_active.a != 255 && HomeMenu_button_loader.s == HomeMenu_activeZoomLevel)
			HomeMenu_button_loader_active.a = MIN(255, HomeMenu_button_loader_active.a + 1.5f*HomeMenu_fadeRate*__HomeMenu_elapsed);
		
		// step 1: button pushes in
		if (HomeMenu_button_loader.s > HomeMenu_activeZoomLevel && HomeMenu_button_loader_active.a == 0) {
			HomeMenu_button_loader.s = MAX(HomeMenu_activeZoomLevel, HomeMenu_button_loader.s - HomeMenu_zoomRate*__HomeMenu_elapsed);
			HomeMenu_button_loader_active.s = HomeMenu_button_loader.s;
		}
	} else
		if (loaderState == HM_HOTSPOT_HOVER)
		{
			if (HomeMenu_button_loader.s < HomeMenu_hoverZoomLevel) {
				HomeMenu_button_loader.s = MIN(HomeMenu_hoverZoomLevel, HomeMenu_button_loader.s + HomeMenu_zoomRate*__HomeMenu_elapsed);
				HomeMenu_button_loader_active.s = HomeMenu_button_loader.s;
			}
		} else {
			if (HomeMenu_button_loader.s > 1) {
				HomeMenu_button_loader.s = MAX(1, HomeMenu_button_loader.s - HomeMenu_zoomRate*__HomeMenu_elapsed);
				HomeMenu_button_loader_active.s = HomeMenu_button_loader.s;
			}
		}
	
	// update hotspot sates if necessary (this is very dirty!  find a better way)
	for (i = 0; i < 4; i++) {
		if ((topState &= HM_HOTSPOT_ACTIVE)    == HM_HOTSPOT_ACTIVE)
			if ((HomeMenu_topState[i]     & HM_HOTSPOT_ACTIVATED) == HM_HOTSPOT_ACTIVATED)
				HomeMenu_topState[i]     |= HM_HOTSPOT_ACTIVE;

		if ((bottomState & HM_HOTSPOT_ACTIVE)  == HM_HOTSPOT_ACTIVE)
			if ((HomeMenu_bottomState[i]  & HM_HOTSPOT_ACTIVATED) == HM_HOTSPOT_ACTIVATED)
				HomeMenu_bottomState[i]  |= HM_HOTSPOT_ACTIVE;

		if ((wiiMenuState & HM_HOTSPOT_ACTIVE) == HM_HOTSPOT_ACTIVE)
			if ((HomeMenu_wiiMenuState[i] & HM_HOTSPOT_ACTIVATED) == HM_HOTSPOT_ACTIVATED)
				HomeMenu_wiiMenuState[i] |= HM_HOTSPOT_ACTIVE;

		if ((loaderState & HM_HOTSPOT_ACTIVE)  == HM_HOTSPOT_ACTIVE)
			if ((HomeMenu_loaderState[i]  & HM_HOTSPOT_ACTIVATED) == HM_HOTSPOT_ACTIVATED)
				HomeMenu_loaderState[i]  |= HM_HOTSPOT_ACTIVE;
	}
	
}


void __HomeMenu_drawImage(HomeMenu_image *img)
{
	if (!img->visible)
		return;
		
	float x, y, w, h;
	w = img->s * img->w;
	h = img->s * img->h;
	// width exceptions for top and bottom strips
	if (img == HomeMenu_images[1] ||
		img == HomeMenu_images[2] ||
		img == HomeMenu_images[3] ||
		img == HomeMenu_images[4] ||
		img == HomeMenu_images[5] ||
		img == HomeMenu_images[6])
		w = 728;
	else
		// scale everything according for widescreen (if needed), except for background
		if (img != HomeMenu_images[0])
			w *= __HomeMenu_xRatio;
	x = img->x - w/2;
	y = img->y - h/2;
	
	// Drawing code for GRRLIB
	if (__HomeMenu_gfx == HM_GFX_GRRLIB) {
		GXTexObj _texObj;
		GX_InitTexObj(&_texObj, (void*)img->texture, img->w, img->h, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
		//GX_InitTexObjLOD(&_texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
		GX_LoadTexObj(&_texObj,GX_TEXMAP0);
		
		GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
		GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

		Mtx model, tmp;
		guMtxIdentity(model);
		guMtxRotDeg(tmp, 'z', img->t);
		guMtxConcat(model, tmp, model);
		guMtxTransApply(model, model, img->x, img->y, 0.0f);
		GX_LoadPosMtxImm(model, GX_PNMTX0);

		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
			GX_Position3f32(-w/2, -h/2, 0);
			GX_Color4u8(img->r, img->g, img->b, img->a);
			GX_TexCoord2f32(0, 0);

			GX_Position3f32(w/2, -h/2, 0);
			GX_Color4u8(img->r, img->g, img->b, img->a);
			GX_TexCoord2f32(1, 0);

			GX_Position3f32(w/2, h/2, 0);
			GX_Color4u8(img->r, img->g, img->b, img->a);
			GX_TexCoord2f32(1, 1);

			GX_Position3f32(-w/2, h/2, 0);
			GX_Color4u8(img->r, img->g, img->b, img->a);
			GX_TexCoord2f32(0, 1);
		GX_End();
		
		GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
		GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
} else

// Drawing code for libwiisprite
	if (__HomeMenu_gfx == HM_GFX_LIBWIISPRITE) {
		GXTexObj _texObj;
		GX_InitTexObj(&_texObj, (void*)img->texture, img->w, img->h, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);

		GX_LoadTexObj(&_texObj,GX_TEXMAP0);
		Mtx model, tmp;
		guMtxIdentity(model);
		guMtxRotDeg(tmp, 'z', img->t);
		guMtxConcat(model, tmp, model);
		guMtxTransApply(model, model, img->x, img->y, 0.0f);
		GX_LoadPosMtxImm(model, GX_PNMTX0);

		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
			GX_Position2f32(-w/2, -h/2);
			GX_Color4u8(img->r, img->g, img->b, img->a);
			GX_TexCoord2f32(0, 0);

			GX_Position2f32(w/2, -h/2);
			GX_Color4u8(img->r, img->g, img->b, img->a);
			GX_TexCoord2f32(1, 0);

			GX_Position2f32(w/2, h/2);
			GX_Color4u8(img->r, img->g, img->b, img->a);
			GX_TexCoord2f32(1, 1);

			GX_Position2f32(-w/2, h/2);
			GX_Color4u8(img->r, img->g, img->b, img->a);
			GX_TexCoord2f32(0, 1);
		GX_End();
	} else {
	// Failsafe rendering code 
	//...
	}
}


void __HomeMenu_playPCM(const void* pcm, s32 pcm_size, s32 left, s32 right, bool stereo)
{
	if (__HomeMenu_snd == HM_SND_ASND) {
		if (stereo)
			ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 48000, 0, (void*)pcm, pcm_size, left, right, NULL);
		else
			ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_MONO_16BIT, 48000, 0, (void*)pcm, pcm_size, left, right, NULL);
	}
}


void __HomeMenu_draw()
{
	
	int i;
	for (i = 0; i < HM_IMG_COUNT; i++) {
		__HomeMenu_drawImage(HomeMenu_images[i]);
	}
	
	__HomeMenu_drawFader();

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(__HomeMenu_fb[__HomeMenu_fbi], GX_TRUE);
	GX_DrawDone();
	VIDEO_SetNextFramebuffer(__HomeMenu_fb[__HomeMenu_fbi]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	__HomeMenu_fbi ^= 1;
	GX_InvalidateTexAll();
}


void __HomeMenu_drawFader() {
	// code borrowed from LWS's quad draw
	
	f32 x, y, w, h;
	x = -40; y = -40;
	w = HomeMenu_screenWidth + 80; h = HomeMenu_screenHeight + 80;

	// Use all the position data one can get
	Mtx model;
	guMtxIdentity(model);
	guMtxTransApply(model, model, x+w/2, y+h/2, 0.0f);
	GX_LoadPosMtxImm(model, GX_PNMTX0);

	// Turn off texturing
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	if (__HomeMenu_gfx == HM_GFX_GRRLIB) {
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
			GX_Position3f32(-w, -h, 0);
			GX_Color4u8(0, 0, 0, HomeMenu_fader);
			GX_Position3f32(w, -h, 0);
			GX_Color4u8(0, 0, 0, HomeMenu_fader);
			GX_Position3f32(w, h, 0);
			GX_Color4u8(0, 0, 0, HomeMenu_fader);
			GX_Position3f32(-w, h, 0);
			GX_Color4u8(0, 0, 0, HomeMenu_fader);
		GX_End();
	} else if (__HomeMenu_gfx == HM_GFX_LIBWIISPRITE) {
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
			GX_Position2f32(-w, -h);
			GX_Color4u8(0, 0, 0, HomeMenu_fader);
			GX_Position2f32(w, -h);
			GX_Color4u8(0, 0, 0, HomeMenu_fader);
			GX_Position2f32(w, h);
			GX_Color4u8(0, 0, 0, HomeMenu_fader);
			GX_Position2f32(-w, h);
			GX_Color4u8(0, 0, 0, HomeMenu_fader);
		GX_End();
	} else {
		// failsafe... not done yet.
	}
	
	// Turn texturing back on
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
}


void __HomeMenu_updateTimer()
{
	u32 current = ticks_to_millisecs(gettime());
	__HomeMenu_elapsed = (current - __HomeMenu_last)/1000.f;
	__HomeMenu_last = current;
}


void __HomeMenu_setVisible(bool value)
{
	HomeMenu_fader = 0;
	HomeMenu_button_close.visible = value;
	HomeMenu_text_top.visible = value;
	HomeMenu_top_active.visible = value;
	HomeMenu_top.visible = value;
	HomeMenu_battery[0].visible = value;
	HomeMenu_battery[1].visible = value;
	HomeMenu_battery[2].visible = value;
	HomeMenu_battery[3].visible = value;
	HomeMenu_battery_info.visible = value;
	HomeMenu_text_bottom.visible = value;
	HomeMenu_wiimote.visible = value;
	HomeMenu_bottom_active.visible = value;
	HomeMenu_bottom.visible = value;
	HomeMenu_button_wiiMenu.visible = value;
	HomeMenu_button_loader.visible = value;
}


void __HomeMenu_moveAll(f32 offset)
{
	HomeMenu_top.x = HomeMenu_screenWidth/2;
	HomeMenu_top.y = offset - 7;
	HomeMenu_bottom.x = HomeMenu_screenWidth/2;
	HomeMenu_bottom.y = HomeMenu_screenHeight - offset + 7;

	// all other sprites get their vertical positions off top or HomeMenu_bottom.
	HomeMenu_top_hover.x = HomeMenu_top.x;
	HomeMenu_top_hover.y = HomeMenu_top.y;
	HomeMenu_top_active.x = HomeMenu_top.x;
	HomeMenu_top_active.y = HomeMenu_top.y;
	HomeMenu_bottom_hover.x = HomeMenu_bottom.x;
	HomeMenu_bottom_hover.y = HomeMenu_bottom.y;
	HomeMenu_bottom_active.x = HomeMenu_bottom.x;
	HomeMenu_bottom_active.y = HomeMenu_bottom.y;
	HomeMenu_text_top.x = HomeMenu_text_top.w/2 + HomeMenu_screenWidth*0.04f;
	HomeMenu_text_top.y = HomeMenu_top.y + 25;
	HomeMenu_text_bottom.x = HomeMenu_bottom.x + HomeMenu_screenWidth*0.07f;
	HomeMenu_text_bottom.y = HomeMenu_bottom.y;
	HomeMenu_wiimote.x = HomeMenu_screenWidth*0.12f;
	HomeMenu_wiimote.y = HomeMenu_bottom.y + 5;		// <<<------------ replace 5 with an offset which is also used in animations.
	HomeMenu_battery_info.x = HomeMenu_screenWidth*0.59f;
	HomeMenu_battery_info.y = HomeMenu_bottom.y - HomeMenu_bottom.h/2;
	HomeMenu_battery[0].x = HomeMenu_battery_info.x - 140 * __HomeMenu_xRatio;
	HomeMenu_battery[0].y = HomeMenu_battery_info.y - 1;
	HomeMenu_battery[1].x = HomeMenu_battery_info.x - 34 * __HomeMenu_xRatio;
	HomeMenu_battery[1].y = HomeMenu_battery_info.y - 1;
	HomeMenu_battery[2].x = HomeMenu_battery_info.x + 72 * __HomeMenu_xRatio;
	HomeMenu_battery[2].y = HomeMenu_battery_info.y - 1;
	HomeMenu_battery[3].x = HomeMenu_battery_info.x + 178 * __HomeMenu_xRatio;
	HomeMenu_battery[3].y = HomeMenu_battery_info.y - 1;
	HomeMenu_p[0].x = HomeMenu_battery[0].x - 46 * __HomeMenu_xRatio;
	HomeMenu_p[0].y = HomeMenu_battery[0].y;
	HomeMenu_p[1].x = HomeMenu_battery[1].x - 46 * __HomeMenu_xRatio;
	HomeMenu_p[1].y = HomeMenu_battery[1].y;
	HomeMenu_p[2].x = HomeMenu_battery[2].x - 46 * __HomeMenu_xRatio;
	HomeMenu_p[2].y = HomeMenu_battery[2].y;
	HomeMenu_p[3].x = HomeMenu_battery[3].x - 46 * __HomeMenu_xRatio;
	HomeMenu_p[3].y = HomeMenu_battery[3].y;
	HomeMenu_button_wiiMenu.x = HomeMenu_screenWidth*0.27f;
	HomeMenu_button_wiiMenu.y = HomeMenu_screenHeight*0.48f;
	HomeMenu_button_wiiMenu_active.x = HomeMenu_button_wiiMenu.x;
	HomeMenu_button_wiiMenu_active.y = HomeMenu_button_wiiMenu.y;
	HomeMenu_button_loader.x = HomeMenu_screenWidth*0.73f;
	HomeMenu_button_loader.y = HomeMenu_screenHeight*0.48f;
	HomeMenu_button_loader_active.x = HomeMenu_button_loader.x;
	HomeMenu_button_loader_active.y = HomeMenu_button_loader.y;
	HomeMenu_button_close.x = HomeMenu_screenWidth*0.96f - HomeMenu_button_close.w/2;
	HomeMenu_button_close.y = HomeMenu_top.y + 20;
}
