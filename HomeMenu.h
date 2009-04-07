#ifndef __HomeMenu__
#define __HomeMenu__

#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SHORT_RUMBLE			0.047f
#define RUMBLE_COOLDOWN 		0.30f
#define MENU_VOLUME				196

#define HM_GFX_FAILSAFE			0
#define HM_GFX_GRRLIB			1
#define HM_GFX_LIBWIISPRITE		2

#define HM_SND_NOSOUND			0
#define HM_SND_ASND 			1
#define HM_SND_SDL				2

#define HM_HOTSPOT_INACTIVE		0	// no cursor interaction
#define HM_HOTSPOT_HOVER		1	// cursor over hotspot
#define HM_HOTSPOT_ACTIVATED	2	// cursor just activated hotspot (clicked)
#define HM_HOTSPOT_ACTIVE		4	// hotspot action is free to begin

typedef struct HomeMenu_image {
	const void *texture;	// pointer to texture
	f32 x;			// x-coord	(of texture center)
	f32 y;			// y-coord	(of texture center)
	u32 w;			// width
	u32 h;			// height
	u8 r;			// red channel
	u8 g;			// green
	u8 b;			// blue
	u8 a;			// alpha
	f32 t;			// rotation (theta)
	f32 s;			// scale
	bool visible;	// whether or not to draw
} HomeMenu_image;

typedef struct HomeMenu_cursor {
	HomeMenu_image *pointer;
	f32 rumbleTimer;
	f32 cooldownTimer;
} HomeMenu_cursor;

bool HomeMenu_Init(int screenWidth, int screenHeight, void* framebuffer0, void* framebuffer1, u8 framebufferIndex);
void HomeMenu_SetGFX(u8 lib);
void HomeMenu_SetSND(u8 lib);
void HomeMenu_Destroy();
bool HomeMenu_Show();
void HomeMenu_Hide();

// Callback Setters:
void HomeMenu_SetBeforeShowMenu(void (*func)());
void HomeMenu_SetAfterShowMenu(void (*func)());
void HomeMenu_SetBeforeDraw(void (*func)());
void HomeMenu_SetAfterDraw(void (*func)());
void HomeMenu_SetBeforeHideMenu(void (*func)());
void HomeMenu_SetAfterHideMenu(void (*func)());

// Callbacks:
void (*HomeMenu_BeforeShowMenu)();		// called just before starting menu-display animation
void (*HomeMenu_AfterShowMenu)();		// called just after finishing menu-display animation
void (*HomeMenu_BeforeDraw)();			// called at the very beginning of the menu loop
void (*HomeMenu_AfterDraw)();			// called at the very end of the menu loop
void (*HomeMenu_BeforeHideMenu)();		// called just before starting menu-hide animation
void (*HomeMenu_AfterHideMenu)();		// called just after finishing menu-hide animaiton

// "private" stuff:
// timer variables
u32 __HomeMenu_last;
f32 __HomeMenu_elapsed;

void __HomeMenu_resetCursors();
void __HomeMenu_updateWiimotes();
void __HomeMenu_slide(bool reverse);
void __HomeMenu_animate();
void __HomeMenu_draw();
void __HomeMenu_drawFader();
void __HomeMenu_updateTimer();
void __HomeMenu_drawImage(HomeMenu_image *img);
void __HomeMenu_playPCM(const void* pcm, s32 pcm_size, s32 left, s32 right, bool stereo);
void __HomeMenu_setVisible(bool value);
void __HomeMenu_moveAll(f32 offset);

// frame buffer stuff:
void* __HomeMenu_fb[2];		// framebuffers
u8 __HomeMenu_fbi;			// frame buffer index (current)
u8 __HomeMenu_fbi0;			// frame buffer index (before HomeMenu starts drawing)


u8  __HomeMenu_gfx;			// underlying graphics library.  Values are 1: GRRLIB;  2: LibWiiSprite;  3:  LibWiiGUI;  0: Failsafe
u8  __HomeMenu_snd;			// underlying sound library.  Values are 1: ASND;  2: SDL;  0: No Sound
u8  __HomeMenu_rumbleIntensity;	// values ranges from 0-2, only rumble when on 1 or 2.  This lowers the perceived rumble intensity.
f32 __HomeMenu_xRatio;		// 1 for normal screen; 0.75 for widescreen.  Used to horizontally scale images in widescreen mode.

// tex buffers ( don't forget __attribute__((aligned(32))) when giving them values)
extern const unsigned char HomeMenu_tex_top[];
extern const unsigned char HomeMenu_tex_top_hover[];
extern const unsigned char HomeMenu_tex_top_active[];
extern const unsigned char HomeMenu_tex_bottom[];
extern const unsigned char HomeMenu_tex_bottom_hover[];
extern const unsigned char HomeMenu_tex_bottom_active[];
extern const unsigned char HomeMenu_tex_text_top[];
extern const unsigned char HomeMenu_tex_text_bottom[];
extern const unsigned char HomeMenu_tex_wiimote[];
extern const unsigned char HomeMenu_tex_battery_info[];
extern const unsigned char HomeMenu_tex_battery0[];	// empty
extern const unsigned char HomeMenu_tex_battery1[];
extern const unsigned char HomeMenu_tex_battery2[];	// half
extern const unsigned char HomeMenu_tex_battery3[];
extern const unsigned char HomeMenu_tex_battery4[];	// full
extern const unsigned char HomeMenu_tex_button_wiiMenu[];
extern const unsigned char HomeMenu_tex_button_wiiMenu_active[];
extern const unsigned char HomeMenu_tex_button_loader[];
extern const unsigned char HomeMenu_tex_button_loader_active[];
extern const unsigned char HomeMenu_tex_button_close[];
extern const unsigned char HomeMenu_tex_p1_point[];
extern const unsigned char HomeMenu_tex_p2_point[];
extern const unsigned char HomeMenu_tex_p3_point[];
extern const unsigned char HomeMenu_tex_p4_point[];
extern const unsigned char HomeMenu_tex_p1[];
extern const unsigned char HomeMenu_tex_p2[];
extern const unsigned char HomeMenu_tex_p3[];
extern const unsigned char HomeMenu_tex_p4[];
void *HomeMenu_tex_bg;		// buffer for texture made from background

HomeMenu_image HomeMenu_top, HomeMenu_top_hover, HomeMenu_top_active, HomeMenu_bottom, HomeMenu_bottom_hover, HomeMenu_bottom_active;
HomeMenu_image HomeMenu_text_top, HomeMenu_text_bottom, HomeMenu_wiimote, HomeMenu_battery_info, HomeMenu_battery[4];
HomeMenu_image HomeMenu_p[4];		// the "P" label beside the batter gauge
HomeMenu_image HomeMenu_button_wiiMenu, HomeMenu_button_wiiMenu_active, HomeMenu_button_loader, HomeMenu_button_loader_active, HomeMenu_button_close;
HomeMenu_image HomeMenu_pointer[4];
HomeMenu_image HomeMenu_background;

#define HM_IMG_COUNT 28
HomeMenu_image* HomeMenu_images[HM_IMG_COUNT];		// conventient array of pointers to all images
HomeMenu_cursor p1, p2, p3, p4;
HomeMenu_cursor HomeMenu_cursors[4];

u16 HomeMenu_fader;								// Foreground fader value (0 = transparent, 255 = opaque black)

const u8  HomeMenu_dimAmount		= 96;		// amount background is dimmed when menu is open;
const f32 HomeMenu_zoomRate			= 1.01f;	// Rate at which buttons zoom (orders/sec)
const f32 HomeMenu_fadeRate			= 2048;		// Rate at which top and bottom bars fade (256ths/sec)
const f32 HomeMenu_hoverZoomLevel	= 1.07f;	// Level to which a hovered-upon button zooms
const f32 HomeMenu_activeZoomLevel	= 1.04f;	// Level to which an activated button zooms

u32 HomeMenu_wm_status[4], HomeMenu_wm_type[4];

// state values:	0: inactive;	1: hover;	2: activated
u8 HomeMenu_topState[4], HomeMenu_bottomState[4], HomeMenu_wiiMenuState[4], HomeMenu_loaderState[4];

int HomeMenu_screenWidth, HomeMenu_screenHeight;

// Whether or not HomeMenu_Init() has been called
extern bool HomeMenu_initialized;
extern bool HomeMenu_active;		// if false, menu closes.	(other threads may set false through HideMenu())

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
