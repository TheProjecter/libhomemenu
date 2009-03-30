#ifndef __HomeMenu__
#define __HomeMenu__

#include <gccore.h>

#define SHORT_RUMBLE 0.047f
#define RUMBLE_COOLDOWN 0.30f
#define MENU_VOLUME 196

struct image {
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
};

struct cursor {
	image *pointer;
	f32 rumbleTimer;
	f32 cooldownTimer;
};

class HomeMenu
{
  public:
	// Factory function used to create a HomeMenu (this prevents the creation of multiple menus)
	static HomeMenu *CreateMenu(int screenWidth, int screenHeight, void* framebuffer0, void* framebuffer1, u8 framebufferIndex);

	~HomeMenu();
	bool ShowMenu();
	static void HideMenu();
	
	// Callback Setters:
	void SetBeforeShowMenu(void (*func)());
	void SetAfterShowMenu(void (*func)());
	void SetBeforeDraw(void (*func)());
	void SetAfterDraw(void (*func)());
	void SetBeforeHideMenu(void (*func)());
	void SetAfterHideMenu(void (*func)());
	
	// Callbacks:
	void (*BeforeShowMenu)();		// called just before starting menu-display animation
	void (*AfterShowMenu)();		// called just after finishing menu-display animation
	void (*BeforeDraw)();			// called at the very beginning of the menu loop
	void (*AfterDraw)();			// called at the very end of the menu loop
	void (*BeforeHideMenu)();		// called just before starting menu-hide animation
	void (*AfterHideMenu)();		// called just after finishing menu-hide animaiton
	
	// timer variables
	u32 last;
	f32 elapsed;
	

  private:
	// private construction (use public factory function CreateMenu())
  	HomeMenu(int width, int height, void* framebuffer0, void* framebuffer1, u8 framebufferIndex);
	void resetCursors();
	void updateWiimotes();
	void slide(bool reverse);
	void animate();
	void draw();
	void updateTimer();
	void __drawImage(image *img);
	void __setVisible(bool value);
	void __slider(f32 offset);
	
	// frame buffer stuff:
	void* fb[2];	// framebuffers
	u8 fbi;			// frame buffer index
	u8 rumbleIntensity;			// values ranges from 0-2, only rumble when on 1 or 2.  This lowers the perceived rumble intensity.
	
	// tex buffers ( don't forget __attribute__((aligned(32))) when giving them values)
	static const unsigned char tex_top[];
	static const unsigned char tex_top_active[];
	static const unsigned char tex_bottom[];
	static const unsigned char tex_bottom_active[];
	static const unsigned char tex_text_top[];
	static const unsigned char tex_text_bottom[];
	static const unsigned char tex_wiimote[];
	static const unsigned char tex_battery_info[];
	static const unsigned char tex_battery0[];	// empty
	static const unsigned char tex_battery1[];
	static const unsigned char tex_battery2[];	// half
	static const unsigned char tex_battery3[];
	static const unsigned char tex_battery4[];	// full
	static const unsigned char tex_button_wiiMenu[];
	static const unsigned char tex_button_loader[];
	static const unsigned char tex_button_close[];
	static const unsigned char tex_p1_point[];
	static const unsigned char tex_p2_point[];
	static const unsigned char tex_p3_point[];
	static const unsigned char tex_p4_point[];
	static const unsigned char tex_p1[];
	static const unsigned char tex_p2[];
	static const unsigned char tex_p3[];
	static const unsigned char tex_p4[];
	void *tex_bg;		// buffer for texture made from background
	
	// A pointer to the first (and only allowed) HomeMenu instance
	static HomeMenu* theOmegaMenu;
	
	image top, top_active, bottom, bottom_active;
	image text_top, text_bottom, wiimote, battery_info, battery[4];
	image p[4];		// the "P" label beside the batter gauge
	image button_wiiMenu, button_loader, button_close;
	image pointer[4];
	image background;
	static const int imageCount = 24;
	image* images[imageCount];		// conventient array of pointers to all images
	cursor p1, p2, p3, p4;
	cursor cursors[4];

	f32 fader;	// Background and Foreground fader values.
	const static int dimAmount = 96;	// amount background is dimmed when menu is open;
	
	u32 wm_status[4], wm_type[4];
	
	bool topHover[4], bottomHover[4], wiiMenuHover[4], loaderHover[4];

	f32 zoomRate, fadeRate;	// rate at which top&bottom move, buttons zoom, active layers fade in/out.
	
	int screenWidth, screenHeight;
	
	static bool active;		// if false, menu closes.	(other threads may set false through HideMenu())
};

#endif
