/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      MacOS X quartz windowed gfx driver
 *
 *      By Angelo Mottola.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/aintosx.h"

#ifndef ALLEGRO_MACOSX
   #error something is wrong with the makefile
#endif


static BITMAP *osx_qz_window_init(int, int, int, int, int);
static void osx_qz_window_exit(BITMAP *);
static void osx_qz_window_vsync(void);
static void osx_qz_window_set_palette(AL_CONST struct RGB *, int, int, int);
static void osx_signal_vsync(void);
static BITMAP* osx_create_video_bitmap(int w, int h);
static int osx_show_video_bitmap(BITMAP*);
static void osx_destroy_video_bitmap(BITMAP*);
static BITMAP* create_video_page(unsigned char*);

static pthread_mutex_t vsync_mutex;
static pthread_cond_t vsync_cond;
static int lock_nesting = 0;
static AllegroWindowDelegate *window_delegate = NULL;
static char driver_desc[256];
static int requested_color_depth;
static COLORCONV_BLITTER_FUNC *colorconv_blitter = NULL;
static RgnHandle update_region = NULL;
static RgnHandle temp_region = NULL;
static AllegroView *qd_view = NULL;
static int desktop_depth;
static BITMAP* pseudo_screen = NULL;
static BITMAP* first_page = NULL;
static unsigned char *pseudo_screen_addr = NULL;
static int pseudo_screen_pitch;
static int pseudo_screen_depth;
static char *dirty_lines = NULL;
static GFX_VTABLE _special_vtable; /* special vtable for active video page */
static GFX_VTABLE _unspecial_vtable; /* special vtable for inactive video page */
static BITMAP* current_video_page = NULL;

void* osx_window_mutex;


GFX_DRIVER gfx_quartz_window =
{
   GFX_QUARTZ_WINDOW,
   empty_string, 
   empty_string,
   "Quartz window", 
   osx_qz_window_init,
   osx_qz_window_exit,
   NULL,                         /* AL_METHOD(int, scroll, (int x, int y)); */
   osx_qz_window_vsync,
   osx_qz_window_set_palette,
   NULL,                         /* AL_METHOD(int, request_scroll, (int x, int y)); */
   NULL,                         /* AL_METHOD(int, poll_scroll, (void)); */
   NULL,                         /* AL_METHOD(void, enable_triple_buffer, (void)); */
   osx_create_video_bitmap,      /* AL_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height)); */
   osx_destroy_video_bitmap,     /* AL_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap)); */
   osx_show_video_bitmap,        /* AL_METHOD(int, show_video_bitmap, (BITMAP *bitmap)); */
   NULL,                         /* AL_METHOD(int, request_video_bitmap, (BITMAP *bitmap)); */
   NULL,                         /* AL_METHOD(BITMAP *, create_system_bitmap, (int width, int height)); */
   NULL,                         /* AL_METHOD(void, destroy_system_bitmap, (BITMAP *bitmap)); */
   osx_mouse_set_sprite,         /* AL_METHOD(int, set_mouse_sprite, (BITMAP *sprite, int xfocus, int yfocus)); */
   osx_mouse_show,               /* AL_METHOD(int, show_mouse, (BITMAP *bmp, int x, int y)); */
   osx_mouse_hide,               /* AL_METHOD(void, hide_mouse, (void)); */
   osx_mouse_move,               /* AL_METHOD(void, move_mouse, (int x, int y)); */
   NULL,                         /* AL_METHOD(void, drawing_mode, (void)); */
   NULL,                         /* AL_METHOD(void, save_video_state, (void)); */
   NULL,                         /* AL_METHOD(void, restore_video_state, (void)); */
   NULL,                         /* AL_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a)); */
   NULL,                         /* AL_METHOD(int, fetch_mode_list, (void)); */
   0, 0,                         /* physical (not virtual!) screen size */
   TRUE,                         /* true if video memory is linear */
   0,                            /* bank size, in bytes */
   0,                            /* bank granularity, in bytes */
   0,                            /* video memory size, in bytes */
   0,                            /* physical address of video memory */
   TRUE
};



/* prepare_window_for_animation:
 *  Prepares the window for a (de)miniaturization animation.
 *  Called by the window display method when the window is about to be
 *  deminiaturized, this updates the QuickDraw view contents and sets the
 *  alpha component to 255 (opaque).
 *  When called from the miniaturize window method, only the alpha value
 *  is updated.
 */
static void prepare_window_for_animation(int refresh_view)
{
   struct GRAPHICS_RECT src_gfx_rect, dest_gfx_rect;
   unsigned int *addr;
   int pitch, y, x;
   
   _unix_lock_mutex(osx_window_mutex);
   while (![qd_view lockFocusIfCanDraw]);
   while (!QDDone([qd_view qdPort]));
   LockPortBits([qd_view qdPort]);
   pitch = GetPixRowBytes(GetPortPixMap([qd_view qdPort])) / 4;
   addr = (unsigned int *)GetPixBaseAddr(GetPortPixMap([qd_view qdPort])) +
      ((int)([osx_window frame].size.height) - gfx_quartz_window.h) * pitch;
   if (refresh_view && colorconv_blitter) {
      src_gfx_rect.width  = gfx_quartz_window.w;
      src_gfx_rect.height = gfx_quartz_window.h;
      src_gfx_rect.pitch  = pseudo_screen_pitch;
      src_gfx_rect.data   = pseudo_screen_addr;
      dest_gfx_rect.pitch = pitch * 4;
      dest_gfx_rect.data  = addr;
      colorconv_blitter(&src_gfx_rect, &dest_gfx_rect);
   }
   for (y = gfx_quartz_window.h; y; y--) {
      for (x = 0; x < gfx_quartz_window.w; x++)
         *(addr + x) |= 0xff000000;
      addr += pitch;
   }
   UnlockPortBits([qd_view qdPort]);
   [qd_view unlockFocus];
   _unix_unlock_mutex(osx_window_mutex);
}



@implementation AllegroWindow

/* display:
 *  Called when the window is about to be deminiaturized.
 */
- (void)display
{
   [super display];
   if (desktop_depth == 32)
      prepare_window_for_animation(TRUE);
}



/* miniaturize:
 *  Called when the window is miniaturized.
 */
- (void)miniaturize: (id)sender
{
   if (desktop_depth == 32)
      prepare_window_for_animation(FALSE);
   [super miniaturize: sender];
}

@end



@implementation AllegroWindowDelegate

/* windowShouldClose:
 *  Called when the user attempts to close the window.
 *  Default behaviour is to call the user callback (if any) and deny closing.
 */
- (BOOL)windowShouldClose: (id)sender
{
   if (osx_window_close_hook)
      osx_window_close_hook();
   return NO;
}



/* windowDidDeminiaturize:
 *  Called when the window deminiaturization animation ends; marks the whole
 *  window contents as dirty, so it is updated on next refresh.
 */
- (void)windowDidDeminiaturize: (NSNotification *)aNotification
{
   _unix_lock_mutex(osx_window_mutex);
   memset(dirty_lines, 1, gfx_quartz_window.h);
   _unix_unlock_mutex(osx_window_mutex);
}



/* windowDidBecomeKey:
 * Sent by the default notification center immediately after an NSWindow
 * object has become key.
 */
- (void)windowDidBecomeKey:(NSNotification *)notification
{
   _unix_lock_mutex(osx_skip_events_processing_mutex);
   osx_skip_events_processing = FALSE;
   _unix_unlock_mutex(osx_skip_events_processing_mutex);
}



/* windowDidResignKey:
 * Sent by the default notification center immediately after an NSWindow
 * object has resigned its status as key window.
 */
- (void)windowDidResignKey:(NSNotification *)notification
{
   _unix_lock_mutex(osx_skip_events_processing_mutex);
   osx_skip_events_processing = TRUE;
   _unix_unlock_mutex(osx_skip_events_processing_mutex);
}

@end



@implementation AllegroView

/* resetCursorRects:
 *  Called when the view needs to reset its cursor rects; this restores the
 *  Allegro cursor rect and enables it.
 */
- (void)resetCursorRects
{
   [super resetCursorRects];
   [self addCursorRect: NSMakeRect(0, 0, gfx_quartz_window.w, gfx_quartz_window.h)
      cursor: osx_cursor];
   [osx_cursor setOnMouseEntered: YES];
}

@end



/* osx_qz_acquire_win:
*  Bitmap locking for Quartz windowed mode.
*/
static void osx_qz_acquire_win(BITMAP *bmp)
{
	/* to prevent the drawing threads and the rendering proc
	from concurrently accessing the dirty lines array */
	_unix_lock_mutex(osx_window_mutex);
	if (lock_nesting++ == 0) {
		bmp->id |= BMP_ID_LOCKED;
	}
} 

/* osx_qz_release_win:
 *  Bitmap unlocking for Quartz windowed mode.
 */
static void osx_qz_release_win(BITMAP *bmp)
{
	ASSERT(lock_nesting > 0);
	if (--lock_nesting == 0) {
		bmp->id &= ~BMP_ID_LOCKED;
	}
	
	_unix_unlock_mutex(osx_window_mutex);
}



/* osx_qz_write_line_win:
 *  Line switcher for Quartz windowed mode.
 */
static unsigned long osx_qz_write_line_win(BITMAP *bmp, int line)
{
   if (!(bmp->id & BMP_ID_LOCKED)) {
      osx_qz_acquire_win(bmp);
      if (bmp->extra) {
         while (!QDDone(BMP_EXTRA(bmp)->port));
         while (LockPortBits(BMP_EXTRA(bmp)->port));
      }
      bmp->id |= BMP_ID_AUTOLOCK;
   }
   dirty_lines[line + bmp->y_ofs] = 1;
   
   return (unsigned long)(bmp->line[line]);
}



/* osx_qz_unwrite_line_win:
 *  Line updater for Quartz windowed mode.
 */
static void osx_qz_unwrite_line_win(BITMAP *bmp)
{
   if (bmp->id & BMP_ID_AUTOLOCK) {
      osx_qz_release_win(bmp);
      if (bmp->extra)
         UnlockPortBits(BMP_EXTRA(bmp)->port);
      bmp->id &= ~BMP_ID_AUTOLOCK;
   }
}



/* update_dirty_lines:
 *  Dirty lines updater routine. This is always called from the main app
 *  thread only.
 */
void osx_update_dirty_lines(void)
{
   struct GRAPHICS_RECT src_gfx_rect, dest_gfx_rect;
   Rect rect;
   CGrafPtr qd_view_port;
   int qd_view_pitch;
   char *qd_view_addr;

   if (![osx_window isVisible])
      return;
   
   /* Skip everything if there are no dirty lines */
   _unix_lock_mutex(osx_window_mutex);
   for (rect.top = 0; (rect.top < gfx_quartz_window.h) && (!dirty_lines[rect.top]); rect.top++)
      ;
   if (rect.top >= gfx_quartz_window.h) {
      _unix_unlock_mutex(osx_window_mutex);
      osx_signal_vsync();
      return;
   }
   
   /* Dirty lines need to be updated */
   if ([qd_view lockFocusIfCanDraw] == YES) {
      while (!QDDone([qd_view qdPort]));
      LockPortBits([qd_view qdPort]);
   
      qd_view_port = [qd_view qdPort];
      if (qd_view_port) {
         qd_view_pitch = GetPixRowBytes(GetPortPixMap(qd_view_port));
         qd_view_addr = GetPixBaseAddr(GetPortPixMap(qd_view_port)) +
            ((int)([osx_window frame].size.height) - gfx_quartz_window.h) * qd_view_pitch;
         
         if (colorconv_blitter || (osx_setup_colorconv_blitter() == 0)) {
            SetEmptyRgn(update_region);
            
            rect.left = 0;
            rect.right = gfx_quartz_window.w;
            
            while (rect.top < gfx_quartz_window.h) {
               while ((!dirty_lines[rect.top]) && (rect.top < gfx_quartz_window.h))
                  rect.top++;
               if (rect.top >= gfx_quartz_window.h)
                  break;
               rect.bottom = rect.top;
               while ((dirty_lines[rect.bottom]) && (rect.bottom < gfx_quartz_window.h)) {
                  dirty_lines[rect.bottom] = 0;
                  rect.bottom++;
               }
               /* fill in source graphics rectangle description */
               src_gfx_rect.width  = rect.right - rect.left;
               src_gfx_rect.height = rect.bottom - rect.top;
               src_gfx_rect.pitch  = pseudo_screen_pitch;
               src_gfx_rect.data   = pseudo_screen_addr +
                  (rect.top * pseudo_screen_pitch) +
                  (rect.left * BYTES_PER_PIXEL(pseudo_screen_depth));
               
               /* fill in destination graphics rectangle description */
               dest_gfx_rect.pitch = qd_view_pitch;
               dest_gfx_rect.data  = qd_view_addr +
                  (rect.top * qd_view_pitch) + 
                  (rect.left * BYTES_PER_PIXEL(desktop_depth));
      
               /* function doing the hard work */
               colorconv_blitter(&src_gfx_rect, &dest_gfx_rect);
      
               RectRgn(temp_region, &rect);
               UnionRgn(temp_region, update_region, update_region);
               rect.top = rect.bottom;
            }
         }   
         QDFlushPortBuffer(qd_view_port, update_region);
      }
      UnlockPortBits([qd_view qdPort]);
      [qd_view unlockFocus];
   }
   _unix_unlock_mutex(osx_window_mutex);
   
   osx_signal_vsync();
}



/* osx_setup_colorconv_blitter:
 *  Sets up the window color conversion blitter function depending on the
 *  Allegro requested color depth and the current desktop color depth.
 */
int osx_setup_colorconv_blitter()
{
   CFDictionaryRef mode;
   void *vp;
   int dd;
   if (qd_view && (vp = [qd_view qdPort]))
   {
      dd = GetPixDepth(GetPortPixMap(vp));
   }   
   else
   {
       mode = CGDisplayCurrentMode(kCGDirectMainDisplay);
       CFNumberGetValue(CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel), kCFNumberSInt32Type, &desktop_depth);
       dd = desktop_depth;
   }
   if (dd == 16) dd = 15;
   _unix_lock_mutex(osx_window_mutex);
   if (colorconv_blitter)
      _release_colorconv_blitter(colorconv_blitter);
   colorconv_blitter = _get_colorconv_blitter(requested_color_depth, dd);
   /* We also need to update the color conversion palette to reflect the change */
   if (colorconv_blitter)
      _set_colorconv_palette(_current_palette, 0, 255);
   /* Mark all the window as dirty */
   memset(dirty_lines, 1, gfx_quartz_window.h);
   _unix_unlock_mutex(osx_window_mutex);
   
   return (colorconv_blitter ? 0 : -1);
}



/* osx_qz_window_init:
*  Initializes windowed gfx mode.
*/
static BITMAP *private_osx_qz_window_init(int w, int h, int v_w, int v_h, int color_depth)
{
	CFDictionaryRef mode;
	NSRect rect = NSMakeRect(0, 0, w, h);
	int refresh_rate;
	char tmp1[128], tmp2[128];
	
	pthread_cond_init(&vsync_cond, NULL);
	pthread_mutex_init(&vsync_mutex, NULL);
	osx_window_mutex=_unix_create_mutex();
	lock_nesting = 0;
	
	if (1
#ifdef ALLEGRO_COLOR8
		&& (color_depth != 8)
#endif
#ifdef ALLEGRO_COLOR16
		&& (color_depth != 15)
		&& (color_depth != 16)
#endif
#ifdef ALLEGRO_COLOR24
		&& (color_depth != 24)
#endif
#ifdef ALLEGRO_COLOR32
		&& (color_depth != 32)
#endif
		) {
		ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Unsupported color depth"));
		return NULL;
	}
	
	if ((w == 0) && (h == 0)) {
		w = 320;
		h = 200;
	}
	
	if (v_w < w) v_w = w;
	if (v_h < h) v_h = h;
	
	if ((v_w != w) || (v_h != h)) {
		ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Resolution not supported"));
		return NULL;
	}
	
	osx_window = [[AllegroWindow alloc] initWithContentRect: rect
												  styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask
													backing: NSBackingStoreBuffered
													  defer: NO];
	
	window_delegate = [[[AllegroWindowDelegate alloc] init] autorelease];
	[osx_window setDelegate: window_delegate];
	[osx_window setOneShot: YES];
	[osx_window setAcceptsMouseMovedEvents: YES];
	[osx_window setViewsNeedDisplay: NO];
	[osx_window setReleasedWhenClosed: YES];
	[osx_window useOptimizedDrawing: YES];
	[osx_window center];
	
	qd_view = [[AllegroView alloc] initWithFrame: rect];
	if (!qd_view) {
		ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Not enough memory"));
		return NULL;
	}
	[osx_window setContentView: qd_view];
	
	set_window_title(osx_window_title);
	[osx_window makeKeyAndOrderFront: nil];
	
	/* the last flag serves as an end of loop delimiter */
	dirty_lines = calloc(h + 1, sizeof(char));
	/* Mark all the window as dirty */
	memset(dirty_lines, 1, h + 1);
	
	setup_direct_shifts();
	
	gfx_quartz_window.w = w;
	gfx_quartz_window.h = h;
	gfx_quartz_window.vid_mem = w * h * BYTES_PER_PIXEL(color_depth);
	
	requested_color_depth = color_depth;
	colorconv_blitter=NULL;   
	mode = CGDisplayCurrentMode(kCGDirectMainDisplay);
	CFNumberGetValue(CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel), kCFNumberSInt32Type, &desktop_depth);
	CFNumberGetValue(CFDictionaryGetValue(mode, kCGDisplayRefreshRate), kCFNumberSInt32Type, &refresh_rate);
	_set_current_refresh_rate(refresh_rate);
	
	pseudo_screen_pitch = w * BYTES_PER_PIXEL(color_depth);
	pseudo_screen_addr = _AL_MALLOC(h * pseudo_screen_pitch);
	pseudo_screen = _make_bitmap(w, h, (unsigned long) pseudo_screen_addr, &gfx_quartz_window, color_depth, pseudo_screen_pitch);
	if (!pseudo_screen) {
		return NULL;
	}
	current_video_page = pseudo_screen;
	first_page = NULL;
	/* create a new special vtable for the pseudo screen */
	memcpy(&_special_vtable, &_screen_vtable, sizeof(GFX_VTABLE));
	_special_vtable.acquire = osx_qz_acquire_win;
	_special_vtable.release = osx_qz_release_win;
	_special_vtable.unwrite_bank = osx_qz_unwrite_line_win;
	memcpy(&_unspecial_vtable, _get_vtable(color_depth), sizeof(GFX_VTABLE));
	pseudo_screen->read_bank = osx_qz_write_line_win;
	pseudo_screen->write_bank = osx_qz_write_line_win;
	pseudo_screen->vtable = &_special_vtable;
	uszprintf(driver_desc, sizeof(driver_desc), uconvert_ascii("Cocoa window using QuickDraw view, %d bpp %s", tmp1),
			  color_depth, uconvert_ascii(color_depth == desktop_depth ? "in matching" : "in fast emulation", tmp2));
	gfx_quartz_window.desc = driver_desc;
	
	update_region = NewRgn();
	temp_region = NewRgn();
	
	osx_mouse_tracking_rect = [qd_view addTrackingRect: rect
												 owner: NSApp
											  userData: nil
										  assumeInside: YES];
	
	osx_keyboard_focused(FALSE, 0);
	clear_keybuf();
	osx_gfx_mode = OSX_GFX_WINDOW;
	osx_skip_mouse_move = TRUE;
	osx_window_first_expose = TRUE;
	
	return pseudo_screen;
}

static BITMAP *osx_qz_window_init(int w, int h, int v_w, int v_h, int color_depth)
{
   BITMAP *bmp;
   _unix_lock_mutex(osx_event_mutex);
   bmp = private_osx_qz_window_init(w, h, v_w, v_h, color_depth);
   _unix_unlock_mutex(osx_event_mutex);
   if (!bmp)
      osx_qz_window_exit(bmp);
   return bmp;
}



/* osx_qz_window_exit:
 *  Shuts down windowed gfx mode.
 */
static void osx_qz_window_exit(BITMAP *bmp)
{
   _unix_lock_mutex(osx_event_mutex);

   if (update_region) {
      DisposeRgn(update_region);
      update_region = NULL;
   }
   if (temp_region) {
      DisposeRgn(temp_region);
      temp_region = NULL;
   }
   
   if (osx_window) {
      [osx_window close];
      osx_window = NULL;
   }
   
   if (pseudo_screen_addr) {
      free(pseudo_screen_addr);
      pseudo_screen_addr = NULL;
   }
   
 
   if (dirty_lines) {
      free(dirty_lines);
      dirty_lines = NULL;
   }
   
   if (colorconv_blitter) {
      _release_colorconv_blitter(colorconv_blitter);
      colorconv_blitter = NULL;
   }
   
   _unix_destroy_mutex(osx_window_mutex);
   pthread_cond_destroy(&vsync_cond);
   pthread_mutex_destroy(&vsync_mutex);
   
   osx_mouse_tracking_rect = -1;
   
   osx_gfx_mode = OSX_GFX_NONE;
   
   _unix_unlock_mutex(osx_event_mutex);
}



/* osx_qz_window_vsync:
 *  Quartz video vertical synchronization routine for windowed mode.
 */
static void osx_qz_window_vsync(void)
{
  if (lock_nesting==0) {
    pthread_mutex_trylock(&vsync_mutex);
    pthread_cond_wait(&vsync_cond, &vsync_mutex);
    pthread_mutex_unlock(&vsync_mutex);
  }
  else {
    ASSERT(0); /* Screen already acquired, don't call vsync() */
  }
}



/* osx_qz_window_set_palette:
 *  Sets palette for quartz window.
 */
static void osx_qz_window_set_palette(AL_CONST struct RGB *p, int from, int to, int vsync)
{
   if (vsync)
      osx_qz_window_vsync();
   
   _unix_lock_mutex(osx_window_mutex);
   _set_colorconv_palette(p, from, to);
         
   /* invalidate the whole screen */
   memset(dirty_lines, 1, gfx_quartz_window.h);
   
   _unix_unlock_mutex(osx_window_mutex);
}

void osx_signal_vsync(void) 
{
   pthread_mutex_lock(&vsync_mutex);
   pthread_cond_broadcast(&vsync_cond);
   pthread_mutex_unlock(&vsync_mutex);
}

// create_video_bitmap:
// Create a video bitmap - actually a memory bitmap or
// a pseudo-screen if the dimensions match the screen
static BITMAP* osx_create_video_bitmap(int w, int h)
{
	if ((w ==  gfx_quartz_window.w) && (h ==  gfx_quartz_window.h))
	{
		if (first_page == NULL)
		{
			// First ever call, return a bitmap mapping the screen memory 
			first_page = create_video_page(pseudo_screen_addr);
			return first_page;
		}
		else 
		{
			// Must create another
			return create_video_page(NULL);
		}
	}
	else
	{
		return create_bitmap(w, h);
	}
}

static BITMAP* create_video_page(unsigned char* addr)
{
	BITMAP* page;
	int i;
	int w = gfx_quartz_window.w, h = gfx_quartz_window.h;
	
	page = (BITMAP*) _AL_MALLOC(sizeof(BITMAP) + sizeof(char*) * h);
	if (!page) {
		ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Not enough memory"));
		return NULL;
	}
	if (addr == NULL)
	{
		addr = _AL_MALLOC(gfx_quartz_window.vid_mem);
		// Use page->dat to indicate that we own the memory
		page->dat = addr;
		page->vtable = &_unspecial_vtable;
		page->write_bank = page->read_bank = _stub_bank_switch;
		
	}
	else
	{
		page->dat = NULL;		
		page->vtable = &_special_vtable;
		page->write_bank = page->read_bank = osx_qz_write_line_win;
	}
	if (!addr) {
		ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Not enough memory"));
		return NULL;
	}
	page->w = page->cr = w;
	page->h = page->cb = h;
	page->clip = TRUE;
	page->cl = page->ct = 0;
	page->id = BMP_ID_VIDEO;
	page->extra = NULL;
	page->x_ofs = 0;
	page->y_ofs = 0;
	page->seg = _video_ds();
	for (i = 0; i < h; ++i)
	{
		page->line[i] = addr;
		addr += pseudo_screen_pitch;
	}
	
	return page;
}

static int osx_show_video_bitmap(BITMAP* vb)
{
	if (vb->vtable == &_special_vtable)
	{
		// This bitmap already switched in, therefore no-op
		return 0;
	}
	if (vb->vtable == &_unspecial_vtable)
	{
		_unix_lock_mutex(osx_window_mutex);
		
		// switch out the old one
		if ((current_video_page == pseudo_screen) && (first_page != NULL))
		{
			// switching out screen, also do page 1
			first_page->vtable = &_unspecial_vtable;
			first_page->write_bank = first_page->read_bank =_stub_bank_switch;
		}
		else if ((current_video_page == first_page) && (first_page != NULL))
		{
			// If switching out page 1, also do screen
			pseudo_screen->vtable = &_unspecial_vtable;
			pseudo_screen->write_bank = pseudo_screen->read_bank = _stub_bank_switch;
		}
		if (current_video_page != NULL)
		{
			current_video_page->vtable = &_unspecial_vtable;
			current_video_page->write_bank = current_video_page->read_bank = _stub_bank_switch;
		}
		// Switch in this one
		if ((vb == pseudo_screen) && (first_page != NULL))
		{
			// If asking for show_video_bitmap(screen), also do page 1
			first_page->vtable = &_special_vtable;
			first_page->write_bank = first_page->read_bank = osx_qz_write_line_win;
		}
		else if (vb == first_page)
		{
			// If asking for show_video_bitmap( page 1), also do screen
			pseudo_screen->vtable = &_special_vtable;
			pseudo_screen->write_bank = pseudo_screen->read_bank = osx_qz_write_line_win;
		}
		pseudo_screen_addr = vb->line[0];
		vb->vtable = &_special_vtable;
		vb->write_bank = vb->read_bank = osx_qz_write_line_win;
		current_video_page = vb;
		// mark all lines dirty - they will be flushed to the screen.
		memset(dirty_lines, 1, gfx_quartz_window.h);
		_unix_unlock_mutex(osx_window_mutex);
		
		return 0;
	}
	// Otherwise it's not eligible to be switched in 
	return -1;
}

static void osx_destroy_video_bitmap(BITMAP* bm)
{
	if (bm == pseudo_screen)
	{
		// Don't do this!
		return;
	}
	if (bm == first_page)
		first_page = NULL;
	if (bm->vtable == &_special_vtable) 
	{
		osx_show_video_bitmap(pseudo_screen);
		free(bm->dat);
		free(bm);
	}
	else if (bm->vtable == &_unspecial_vtable)
	{
		free(bm->dat);
		free(bm);
	}
	// Otherwise it wasn't a video page
}
/* Local variables:       */
/* c-basic-offset: 3      */
/* indent-tabs-mode: nil  */
/* c-file-style: "linux" */
/* End:                   */
