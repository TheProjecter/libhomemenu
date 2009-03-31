#include "HomeMenu.h"

#include <stdlib.h>
#include <malloc.h>
#include <cmath>
#include <wiiuse/wpad.h>
#include "asndlib.h"

#include "snd_tick.h"


/** public:	**/

// Factory function
HomeMenu *HomeMenu::CreateMenu(int width, int height, void* framebuffer0, void* framebuffer1, u8 framebufferIndex)
{
	if (theOmegaMenu != NULL) {
		return theOmegaMenu;
	}
	return new HomeMenu(width, height, framebuffer0, framebuffer1, framebufferIndex);
}


HomeMenu::~HomeMenu()
{
	if (tex_bg != NULL) {
		free(MEM_K1_TO_K0(tex_bg));
		tex_bg = NULL;
	}
}


bool HomeMenu::ShowMenu()
{
	if (BeforeShowMenu != NULL) BeforeShowMenu();
	
	// Set GX to our liking
	GX_SetScissorBoxOffset(0, 0);
	GX_SetScissor(0, 0, screenWidth, screenHeight);


	// Set WPAD Data Format
	// (Users should restore their Data Format when menu is closed)
	// (If it's possible to automatically restore previous data format on exit, I'll implement that later.)
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_2, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_3, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(0, screenWidth, screenHeight);

	// take a screenshot to be used as our background
	GX_SetTexCopySrc(0, 0, screenWidth, screenHeight);
	GX_SetTexCopyDst(screenWidth, screenHeight, GX_TF_RGBA8, GX_FALSE);
	GX_CopyTex(tex_bg, GX_FALSE);
	GX_PixModeSync();
	DCFlushRange(tex_bg, GX_GetTexBufferSize(screenWidth, screenHeight, GX_TF_RGBA8, GX_FALSE, 1));

	// Startup animation:
	__setVisible(true);
	updateWiimotes();
	cursors[0].pointer->visible = false;
	cursors[1].pointer->visible = false;
	cursors[2].pointer->visible = false;
	cursors[3].pointer->visible = false;
	top_active.a = 0;
	bottom_active.a = 0;
	button_wiiMenu.s = 1;
	button_loader.s = 1;
	
	slide(false);
	updateTimer();
	
	if (AfterShowMenu != NULL) AfterShowMenu();
	
	
	//----------- Start of Main Menu Loop -----------//
	active = true;
	while (active)
	{
		if (BeforeDraw != NULL) BeforeDraw();
		
		// update value of 'elapsed'
		updateTimer();
		
		// Do wiimote logic
		updateWiimotes();

		// render animations triggered by the cursor
		animate();
		
		for (int i = 0; i < 4; i++) {
			// Catch escape via "Home" Button
			if (WPAD_BUTTON_HOME & WPAD_ButtonsDown(i))
				active = false;
			
			// Catch other clicks:
			if (WPAD_BUTTON_A & WPAD_ButtonsDown(i) && (topHover[i] || bottomHover[i]))
				active = false;

			if (WPAD_BUTTON_A & WPAD_ButtonsDown(i) && wiiMenuHover[i]) {
				// implement and call returnToMenu callback
				// manditory callback... since game has to clean up
				SYS_ResetSystem(SYS_RETURNTOMENU,0,0);
			}
					
			if (WPAD_BUTTON_A & WPAD_ButtonsDown(i) && loaderHover[i]) {
				// implement and call returnToLoader callback
				// manditory callback... since game has to clean up
				resetCursors();
				exit(1);
			}
		}
		
		// draw
		draw();
		
		if (AfterDraw != NULL) AfterDraw();
	}
	//------------ End of Main Menu Loop ------------//

	if (BeforeHideMenu != NULL) BeforeHideMenu();
	
	resetCursors();
	
	// close animation
	slide(true);
	__setVisible(false);

	draw();	// draw a second time so that both framebuffers are the same
	// leave a copy of background in framebuffer to avoid flicker on exit
	__drawImage(&background);	// is there a better way of doing this?

	
	// restore GX here.

	
	if (AfterHideMenu != NULL) AfterHideMenu();

	return true;
}


// the sole purpose of this function is so other threads can tell the HomeMenu to close
void HomeMenu::HideMenu()
{
	active = false;
}


void HomeMenu::SetBeforeShowMenu(void (*func)())
{
	BeforeShowMenu = func;
}

void HomeMenu::SetAfterShowMenu(void (*func)())
{
	AfterShowMenu = func;
}

void HomeMenu::SetBeforeDraw(void (*func)())
{
	BeforeDraw = func;
}

void HomeMenu::SetAfterDraw(void (*func)())
{
	AfterDraw = func;
}

void HomeMenu::SetBeforeHideMenu(void (*func)())
{
	BeforeHideMenu = func;
}

void HomeMenu::SetAfterHideMenu(void (*func)())
{
	AfterHideMenu = func;
}


/** private: **/

// static variables
bool HomeMenu::active = false;
HomeMenu* HomeMenu::theOmegaMenu = NULL;

// Constructor (use Factory function CreateMenu instead)
HomeMenu::HomeMenu(int width, int height, void* framebuffer0, void* framebuffer1, u8 framebufferIndex)
{
	theOmegaMenu = this;
	
	screenWidth = width;
	screenHeight = height;
	fb[0] = framebuffer0;
	fb[1] = framebuffer1;
	fbi = framebufferIndex;
	rumbleIntensity = 0;
	
	// Initialize sound in case it hasn't been done already
	ASND_Init();	// I don't think this causes problems if called twice (I did some quick checks).
	
	// Set animation rates
	zoomRate  = 1.01f;	// orders/sec
	fadeRate  = 2048;	// 256ths/sec
	
	// prepare buffer for screenshot to be used as our background
	tex_bg = memalign(32, GX_GetTexBufferSize(screenWidth, screenHeight, GX_TF_RGBA8, GX_FALSE, 1));

	// set callbacks to NULL
	BeforeShowMenu = NULL;
	AfterShowMenu = NULL;
	BeforeDraw = NULL;
	AfterDraw = NULL;
	BeforeHideMenu = NULL;
	AfterHideMenu = NULL;
	
	// prepare images
	top.texture = &tex_top;
	top.w = 728;	top.h = 112;
	top.x = top.y = top.r = top.g = top.b = top.a = 0xFF;
	top.t = 0;	top.s = 1;	top.visible = true;

	top_hover.texture = &tex_top_hover;
	top_hover.w = 728;	top_hover.h = 112;
	top_hover.x = top_hover.y = top_hover.r = top_hover.g = top_hover.b = 0xFF;
	top_hover.a = top_hover.t = 0;	top_hover.s = 1;	top_hover.visible = true;

	top_active.texture = &tex_top_active;
	top_active.w = 728;	top_active.h = 112;
	top_active.x = top_active.y = top_active.r = top_active.g = top_active.b = 0xFF;
	top_active.a = top_active.t = 0;	top_active.s = 1;	top_active.visible = true;

	bottom.texture = &tex_bottom;
	bottom.w = 728;	bottom.h = 112;
	bottom.x = bottom.y = bottom.r = bottom.g = bottom.b = bottom.a = 0xFF;
	bottom.t = 0;	bottom.s = 1;	bottom.visible = true;

	bottom_hover.texture = &tex_bottom_hover;
	bottom_hover.w = 728;	bottom_hover.h = 112;
	bottom_hover.x = bottom_hover.y = bottom_hover.r = bottom_hover.g = bottom_hover.b = 0xFF;
	bottom_hover.a = bottom_hover.t = 0;	bottom_hover.s = 1;	bottom_hover.visible = true;

	bottom_active.texture = &tex_bottom_active;
	bottom_active.w = 728;	bottom_active.h = 112;
	bottom_active.x = bottom_active.y = bottom_active.r = bottom_active.g = bottom_active.b = 0xFF;
	bottom_active.a = bottom_active.t = 0;	bottom_active.s = 1;	bottom_active.visible = true;

	text_top.texture = &tex_text_top;
	text_top.w = 328;	text_top.h = 36;
	text_top.x = text_top.y = text_top.r = text_top.g = text_top.b = text_top.a = 0xFF;
	text_top.t = 0;	text_top.s = 1;	text_top.visible = true;

	text_bottom.texture = &tex_text_bottom;
	text_bottom.w = 276;	text_bottom.h = 24;
	text_bottom.x = text_bottom.y = text_bottom.r = text_bottom.g = text_bottom.b = text_bottom.a = 0xFF;
	text_bottom.t = 0;	text_bottom.s = 1;	text_bottom.visible = true;

	wiimote.texture = &tex_wiimote;
	wiimote.w = 88;	wiimote.h = 164;
	wiimote.x = wiimote.y = wiimote.r = wiimote.g = wiimote.b = wiimote.a = 0xFF;
	wiimote.t = 0;	wiimote.s = 1;	wiimote.visible = true;
	
	battery_info.texture = &tex_battery_info;
	battery_info.w = 448;	battery_info.h = 44;
	battery_info.x = battery_info.y = battery_info.r = battery_info.g = battery_info.b = battery_info.a = 0xFF;
	battery_info.t = 0;	battery_info.s = 1;	battery_info.visible = true;

	battery[0].texture = &tex_battery0;
	battery[0].w = 44;	battery[0].h = 24;
	battery[0].x = battery[0].y = battery[0].r = battery[0].g = battery[0].b = battery[0].a = 0xFF;
	battery[0].t = 0;	battery[0].s = 1;	battery[0].visible = true;

	battery[1].texture = &tex_battery0;
	battery[1].w = 44;	battery[1].h = 24;
	battery[1].x = battery[1].y = battery[1].r = battery[1].g = battery[1].b = battery[1].a = 0xFF;
	battery[1].t = 0;	battery[1].s = 1;	battery[1].visible = true;

	battery[2].texture = &tex_battery0;
	battery[2].w = 44;	battery[2].h = 24;
	battery[2].x = battery[2].y = battery[2].r = battery[2].g = battery[2].b = battery[2].a = 0xFF;
	battery[2].t = 0;	battery[2].s = 1;	battery[2].visible = true;

	battery[3].texture = &tex_battery0;
	battery[3].w = 44;	battery[3].h = 24;
	battery[3].x = battery[3].y = battery[3].r = battery[3].g = battery[3].b = battery[3].a = 0xFF;
	battery[3].t = 0;	battery[3].s = 1;	battery[3].visible = true;

	p[0].texture = &tex_p1;
	p[0].w = 28;	p[0].h = 20;
	p[0].x = p[0].y = p[0].r = p[0].g = p[0].b = p[0].a = 0xFF;
	p[0].t = 0;	p[0].s = 1;	p[0].visible = true;

	p[1].texture = &tex_p2;
	p[1].w = 28;	p[1].h = 20;
	p[1].x = p[1].y = p[1].r = p[1].g = p[1].b = p[1].a = 0xFF;
	p[1].t = 0;	p[1].s = 1;	p[1].visible = true;

	p[2].texture = &tex_p3;
	p[2].w = 28;	p[2].h = 20;
	p[2].x = p[2].y = p[2].r = p[2].g = p[2].b = p[2].a = 0xFF;
	p[2].t = 0;	p[2].s = 1;	p[2].visible = true;

	p[3].texture = &tex_p4;
	p[3].w = 28;	p[3].h = 20;
	p[3].x = p[3].y = p[3].r = p[3].g = p[3].b = p[3].a = 0xFF;
	p[3].t = 0;	p[3].s = 1;	p[3].visible = true;

	button_wiiMenu.texture = &tex_button_wiiMenu;
	button_wiiMenu.w = 240;	button_wiiMenu.h = 104;
	button_wiiMenu.x = button_wiiMenu.y = button_wiiMenu.r = button_wiiMenu.g = button_wiiMenu.b = 0xFF;
	button_wiiMenu.a = button_wiiMenu.t = 0;	button_wiiMenu.s = 1;	button_wiiMenu.visible = true;

	button_wiiMenu_active.texture = &tex_button_wiiMenu_active;
	button_wiiMenu_active.w = 240;	button_wiiMenu_active.h = 104;
	button_wiiMenu_active.x = button_wiiMenu_active.y = button_wiiMenu_active.r = button_wiiMenu_active.g = button_wiiMenu_active.b = 0xFF;
	button_wiiMenu_active.a = button_wiiMenu_active.t = 0;	button_wiiMenu_active.s = 1;	button_wiiMenu_active.visible = true;

	button_loader.texture = &tex_button_loader;
	button_loader.w = 240;	button_loader.h = 104;
	button_loader.x = button_loader.y = button_loader.r = button_loader.g = button_loader.b = 0xFF;
	button_loader.a = button_loader.t = 0;	button_loader.s = 1;	button_loader.visible = true;

	button_loader_active.texture = &tex_button_loader_active;
	button_loader_active.w = 240;	button_loader_active.h = 104;
	button_loader_active.x = button_loader_active.y = button_loader_active.r = button_loader_active.g = button_loader_active.b = 0xFF;
	button_loader_active.a = button_loader_active.t = 0;	button_loader_active.s = 1;	button_loader_active.visible = true;

	button_close.texture = &tex_button_close;
	button_close.w = 184;	button_close.h = 56;
	button_close.x = button_close.y = button_close.r = button_close.g = button_close.b = button_close.a = 0xFF;
	button_close.t = 0;	button_close.s = 1;	button_close.visible = true;

	pointer[0].texture = &tex_p1_point;
	pointer[0].w = 96;	pointer[0].h = 96;
	pointer[0].x = pointer[0].y = pointer[0].r = pointer[0].g = pointer[0].b = pointer[0].a = 0xFF;
	pointer[0].t = 0;	pointer[0].s = 1;	pointer[0].visible = false;

	pointer[1].texture = &tex_p2_point;
	pointer[1].w = 96;	pointer[1].h = 96;
	pointer[1].x = pointer[1].y = pointer[1].r = pointer[1].g = pointer[1].b = pointer[1].a = 0xFF;
	pointer[1].t = 0;	pointer[1].s = 1;	pointer[1].visible = false;

	pointer[2].texture = &tex_p3_point;
	pointer[2].w = 96;	pointer[2].h = 96;
	pointer[2].x = pointer[2].y = pointer[2].r = pointer[2].g = pointer[2].b = pointer[2].a = 0xFF;
	pointer[2].t = 0;	pointer[2].s = 1;	pointer[2].visible = false;

	pointer[3].texture = &tex_p4_point;
	pointer[3].w = 96;	pointer[3].h = 96;
	pointer[3].x = pointer[3].y = pointer[3].r = pointer[3].g = pointer[3].b = pointer[3].a = 0xFF;
	pointer[3].t = 0;	pointer[3].s = 1;	pointer[3].visible = false;
	
	background.texture = tex_bg;
	background.w = screenWidth;	background.h = screenHeight;
	background.x = background.w/2; background.y = background.h/2;
	background.r = background.g = background.b = background.a = 0xFF;
	background.t = 0;	background.s = 1;	background.visible = true;

	// put pointers to images in a nice convenient array
	images[0]  = &background;
	images[1]  = &top;
	images[2]  = &top_hover;
	images[3]  = &top_active;
	images[4]  = &bottom;
	images[5]  = &bottom_hover;
	images[6]  = &bottom_active;
	images[7]  = &text_top;
	images[8]  = &text_bottom;
	images[9]  = &wiimote;
	images[10] = &battery_info;
	images[11] = &battery[0];
	images[12] = &battery[1];
	images[13] = &battery[2];
	images[14] = &battery[3];
	images[15] = &p[0];
	images[16] = &p[1];
	images[17] = &p[2];
	images[18] = &p[3];
	images[19] = &button_wiiMenu;
	images[20] = &button_wiiMenu_active;
	images[21] = &button_loader;
	images[22] = &button_loader_active;
	images[23] = &button_close;
	images[24] = &pointer[0];
	images[25] = &pointer[1];
	images[26] = &pointer[2];
	images[27] = &pointer[3];
	

	// prepare our cursors
	p1.pointer = &pointer[0];
	p2.pointer = &pointer[1];
	p3.pointer = &pointer[2];
	p4.pointer = &pointer[3];
	cursors[0] = p1;
	cursors[1] = p2;
	cursors[2] = p3;
	cursors[3] = p4;

	// prepare our faders
	fader = 0;

	// Hide all sprites:
	__setVisible(false);	// set visible to false
	__slider(-57);			// draw offscreen

	resetCursors();
}


void HomeMenu::resetCursors()
{
	// set all hotspots to inactive, and prepare our cursors (continued)
	for (int i = 0; i < 4; i++) {
		topHover[i]		= bottomHover[i] = false;	// Cursor is NOT hovering over buttons
		wiiMenuHover[i]	= loaderHover[i] = false;	// ditto
		
		cursors[i].pointer->visible = false;
		cursors[i].pointer->x = -100;
		cursors[i].pointer->y = screenHeight/2;
		cursors[i].rumbleTimer = 0;
		cursors[i].cooldownTimer = 0;
	}
	WPAD_Rumble(WPAD_CHAN_ALL, 0);		// turn off rumbling
}


void HomeMenu::updateWiimotes()
{
	for (int i = 0; i < 4; i++) {
		// Enable/Disable rumble
		if (cursors[i].rumbleTimer > 0) {

			rumbleIntensity = (rumbleIntensity + 1) % 3;	// cycle value throught 0-2.

			if (rumbleIntensity % 3 == 0)					// rumble only 2 out of 3 times.
				WPAD_Rumble(i, 0);
			else
				WPAD_Rumble(i, 1);
			
			cursors[i].rumbleTimer -= elapsed;				// note: this might result in a negative value, not necessarily 0.

			if (cursors[i].rumbleTimer <= 0)				// begin cooldown timer if rumble has expired.
				cursors[i].cooldownTimer = RUMBLE_COOLDOWN;

		} else {
			WPAD_Rumble(i, 0);
			rumbleIntensity = 0;		// setup so the next 2 values are "on" values.
		}
		if (cursors[i].cooldownTimer > 0)
			cursors[i].cooldownTimer -= elapsed;	// note: ditto
	
		wm_status[i] = WPAD_Probe(i, &wm_type[i]);
		ir_t irData;
		
		if(wm_status[i] == WPAD_ERR_NONE) {
			battery[i].texture = &tex_battery2;		// set according to battery level (later)
			battery[i].a = 255;
			p[i].a = 255;
			WPAD_IR(i, &irData);
			if (irData.smooth_valid) {	// if the pointer is visible
				cursors[i].pointer->visible = true;
				cursors[i].pointer->x = cursors[i].pointer->w/2 + irData.sx - screenWidth*0.30f;
				cursors[i].pointer->y = cursors[i].pointer->h/2 + irData.sy - screenHeight*0.75f;
				cursors[i].pointer->t = irData.angle;
			} else {
				// hide,
				cursors[i].pointer->visible = false;
				// and put safely offscreen (doesn't trigger any areas)
				cursors[i].pointer->x = -100;
				cursors[i].pointer->y = screenHeight/2;
			}
		} else {
			cursors[i].pointer->visible = false;
			battery[i].texture = &tex_battery0;
			battery[i].a = 96;
			p[i].a = 96;
		}
		
		// See if cursor is above buttons (oooh, shiny)
		if (cursors[i].pointer->y < top.y + top.h/2) {
			if (!topHover[i])
				ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_MONO_16BIT, 48000, 0, (void*)snd_tick, snd_tick_size, 64, 64, NULL);
			if (!topHover[i] && cursors[i].cooldownTimer <= 0)
				cursors[i].rumbleTimer = SHORT_RUMBLE;
			topHover[i] = true;
		} else
			topHover[i] = false;
		
		if (cursors[i].pointer->y > bottom.y - bottom.h/2) {
			if (!bottomHover[i])
				ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_MONO_16BIT, 48000, 0, (void*)snd_tick, snd_tick_size, 64, 64, NULL);
			if (!bottomHover[i] && cursors[i].cooldownTimer <= 0)
				cursors[i].rumbleTimer = SHORT_RUMBLE;
			bottomHover[i] = true;
		} else
			bottomHover[i] = false;
		
		bool before = wiiMenuHover[i];	// value of wiiMenuHover prior to collision test
		
		// if on button_wiiMenu's bounding box
		wiiMenuHover[i] = false;
		if ((cursors[i].pointer->y > button_wiiMenu.y - 46) &&
			(cursors[i].pointer->y < button_wiiMenu.y + 46) &&
			(cursors[i].pointer->x > button_wiiMenu.x - 114) &&
			(cursors[i].pointer->x < button_wiiMenu.x + 114)) {
			// if over button_wiimenu's inner rectangle
			if ((cursors[i].pointer->x > button_wiiMenu.x - 114 + 46) &&
				(cursors[i].pointer->x < button_wiiMenu.x + 114 - 46))
					wiiMenuHover[i] = true;
			// if over button_wiimenu's outer circles
			f32 angleL = atan2(cursors[i].pointer->x - (button_wiiMenu.x - 68), cursors[i].pointer->y - button_wiiMenu.y);
			f32 angleR = atan2(cursors[i].pointer->x - (button_wiiMenu.x + 68), cursors[i].pointer->y - button_wiiMenu.y);
			
			// left
			if (cursors[i].pointer->x > button_wiiMenu.x - 68 + 46*sin(angleL) && cursors[i].pointer->x < button_wiiMenu.x)
				wiiMenuHover[i] = true;
			
			// right
			if (cursors[i].pointer->x < button_wiiMenu.x + 68 + 46*sin(angleR) && cursors[i].pointer->x > button_wiiMenu.x)
				wiiMenuHover[i] = true;
		}
		
		if (!before && wiiMenuHover[i]) {	// if we just rolled onto button_wiiMenu
			ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_MONO_16BIT, 48000, 0, (void*)snd_tick, snd_tick_size, 96, 32, NULL);
			if (cursors[i].cooldownTimer <= 0)
				cursors[i].rumbleTimer = SHORT_RUMBLE;
		}
		
		before = loaderHover[i];	// value of wiiMenuHover prior to collision test
		
		// if on button_loader's bounding box
		loaderHover[i] = false;
		if ((cursors[i].pointer->y > button_loader.y - 46) &&
			(cursors[i].pointer->y < button_loader.y + 46) &&
			(cursors[i].pointer->x > button_loader.x - 114) &&
			(cursors[i].pointer->x < button_loader.x + 114)) {
			// if over button_wiimenu's inner rectangle
			if ((cursors[i].pointer->x > button_loader.x - 114 + 46) &&
				(cursors[i].pointer->x < button_loader.x + 114 - 46))
					loaderHover[i] = true;
			// if over button_wiimenu's outer circles
			f32 angleL = atan2(cursors[i].pointer->x - (button_loader.x - 68), cursors[i].pointer->y - button_loader.y);
			f32 angleR = atan2(cursors[i].pointer->x - (button_loader.x + 68), cursors[i].pointer->y - button_loader.y);
			
			// left
			if (cursors[i].pointer->x > button_loader.x - 68 + 46*sin(angleL) && cursors[i].pointer->x < button_loader.x)
				loaderHover[i] = true;
			
			// right
			if (cursors[i].pointer->x < button_loader.x + 68 + 46*sin(angleR) && cursors[i].pointer->x > button_loader.x)
				loaderHover[i] = true;
		}
		
		if (!before && loaderHover[i]) {	// if we just rolled onto button_loader
			ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_MONO_16BIT, 48000, 0, (void*)snd_tick, snd_tick_size, 32, 96, NULL);
			if (cursors[i].cooldownTimer <= 0)
				cursors[i].rumbleTimer = SHORT_RUMBLE;
		}
	}
	
	WPAD_ScanPads();
}


void HomeMenu::slide(bool reverse)
{
	// most sprite movements are relative to 'top' and 'bottom,' so we'll move those and update the rest after.
	
	int direction = 1;
	if (reverse)
		direction = -1;
		
	draw();
	
	// we want a shift of 114
	// this is not implemented with a timer because: a) it's not as smooth.  b) at this point, outside load should be minimal
	for (float shift = 0; shift < 114; shift += 7) {
		
		__slider(direction*(shift - 57));
		if (reverse) {
			button_wiiMenu.a = 255 - (shift / 114.f) * 255;
			button_loader.a = 255 - (shift / 114.f) * 255;
			background.r = background.g = background.b = 0xFF - (dimAmount - (shift / 114.f) * dimAmount);
		} else {
			button_wiiMenu.a = (shift / 114.f) * 255;
			button_loader.a = (shift / 114.f) * 255;
			background.r = background.g = background.b = 0xFF - ((shift / 114.f) * dimAmount);
		}

		draw();
	}
	
	__slider(direction*57);
	if (reverse) {
			button_wiiMenu.a = 0;
			button_loader.a = 0;
			background.r = background.g = background.b = 0xFF;
		} else {
			button_wiiMenu.a = 255;
			button_loader.a = 255;
			background.r = background.g = background.b = 0xFF - dimAmount;
		}
	
	draw();	
}


void HomeMenu::animate()
{
	// aggregate all hover bools
	bool topH, bottomH, wiiMenuH, loaderH;
	topH = bottomH = wiiMenuH = loaderH = false;
	for (int i = 0; i < 4; i++) {
		topH     |= topHover[i];
		bottomH  |= bottomHover[i];
		wiiMenuH |= wiiMenuHover[i];
		loaderH  |= loaderHover[i];
	}
	if (topH)
	{
		if (top_hover.a != 255)
			top_hover.a = MIN(255, top_hover.a + fadeRate*elapsed);
	} else {
		if (top_hover.a != 0)
			top_hover.a = MAX(0, top_hover.a - fadeRate*elapsed);
	}
	
	if (bottomH)
	{
		if (bottom_hover.a != 255)
		{
			bottom_hover.a = MIN(255, bottom_hover.a + fadeRate*elapsed);
			wiimote.y = bottom.y + 12 - bottom_hover.a*0.1f;
		}
	} else {
		if (bottom_hover.a != 0)
		{
			bottom_hover.a = MAX(0, bottom_hover.a - fadeRate*elapsed);
			wiimote.y = bottom.y + 12 - bottom_hover.a*0.1f;
		}
	}
	
	if (wiiMenuH)
	{
		if (button_wiiMenu.s < 1.07f)
			button_wiiMenu.s = MIN(1.07f, button_wiiMenu.s + zoomRate*elapsed);
	} else {
		if (button_wiiMenu.s > 1)
			button_wiiMenu.s = MAX(1, button_wiiMenu.s - zoomRate*elapsed);
	}
	
	if (loaderH)
	{
		if (button_loader.s < 1.07f)
			button_loader.s = MIN(1.07f, button_loader.s + zoomRate*elapsed);
	} else {
		if (button_loader.s > 1)
			button_loader.s = MAX(1, button_loader.s - zoomRate*elapsed);
	}
}


void HomeMenu::__drawImage(image *img)
{
	if (!img->visible)
		return;
	
	float x, y, w, h;
	w = img->s * img->w;
	h = img->s * img->h;
	x = img->x - w/2;
	y = img->y - h/2;
	
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
}


void HomeMenu::draw()
{
	//__drawBackground();
	
	for (int i = 0; i < imageCount; i++) {
		__drawImage(images[i]);
	}

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(fb[fbi], GX_TRUE);
	GX_DrawDone();
	VIDEO_SetNextFramebuffer(fb[fbi]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	fbi ^= 1;
	GX_InvalidateTexAll();
}

void HomeMenu::updateTimer()
{
	u32 current = ASND_GetTime();
	elapsed = (current - last)/1000.f;
	last = current;
}


void HomeMenu::__setVisible(bool value)
{
	fader = 0;
	button_close.visible = value;
	text_top.visible = value;
	top_active.visible = value;
	top.visible = value;
	battery[0].visible = value;
	battery[1].visible = value;
	battery[2].visible = value;
	battery[3].visible = value;
	battery_info.visible = value;
	text_bottom.visible = value;
	wiimote.visible = value;
	bottom_active.visible = value;
	bottom.visible = value;
	button_wiiMenu.visible = value;
	button_loader.visible = value;
}


void HomeMenu::__slider(f32 offset)
{
	top.x = screenWidth/2;
	top.y = offset - 7;
	bottom.x = screenWidth/2;
	bottom.y = screenHeight - offset + 7;

	// all other sprites get their vertical positions off top or bottom.
	top_hover.x = top.x;
	top_hover.y = top.y;
	top_active.x = top.x;
	top_active.y = top.y;
	bottom_hover.x = bottom.x;
	bottom_hover.y = bottom.y;
	bottom_active.x = bottom.x;
	bottom_active.y = bottom.y;
	text_top.x = text_top.w/2 + screenWidth*0.04f;
	text_top.y = top.y + 25;
	text_bottom.x = bottom.x + screenWidth*0.07f;
	text_bottom.y = bottom.y;
	wiimote.x = screenWidth*0.12f;
	wiimote.y = bottom.y + 5;
	battery_info.x = screenWidth*0.59f;
	battery_info.y = bottom.y - bottom.h/2;
	battery[0].x = battery_info.x - 140;
	battery[0].y = battery_info.y - 1;
	battery[1].x = battery_info.x - 34;
	battery[1].y = battery_info.y - 1;
	battery[2].x = battery_info.x + 72;
	battery[2].y = battery_info.y - 1;
	battery[3].x = battery_info.x + 178;
	battery[3].y = battery_info.y - 1;
	p[0].x = battery[0].x - 46;
	p[0].y = battery[0].y;
	p[1].x = battery[1].x - 46;
	p[1].y = battery[1].y;
	p[2].x = battery[2].x - 46;
	p[2].y = battery[2].y;
	p[3].x = battery[3].x - 46;
	p[3].y = battery[3].y;
	button_wiiMenu.x = screenWidth*0.27f;
	button_wiiMenu.y = screenHeight*0.48f;
	button_wiiMenu_active.x = button_wiiMenu.x;
	button_wiiMenu_active.y = button_wiiMenu.y;
	button_loader.x = screenWidth*0.73f;
	button_loader.y = screenHeight*0.48f;
	button_loader_active.x = button_loader.x;
	button_loader_active.y = button_loader.y;
	button_close.x = screenWidth*0.96f - button_close.w/2;
	button_close.y = top.y + 20;
}
