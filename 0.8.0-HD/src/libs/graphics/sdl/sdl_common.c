//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "sdl_common.h"
#include "opengl.h"
#include "pure.h"
#include "primitives.h"
#include "options.h"
#include "uqmversion.h"
#include "libs/graphics/drawcmd.h"
#include "libs/graphics/dcqueue.h"
#include "libs/graphics/cmap.h"
#include "libs/input/sdl/input.h"
		// for ProcessInputEvent()
#include "libs/graphics/bbox.h"
#include "port.h"
#include "libs/uio.h"
#include "libs/log.h"
#include "libs/memlib.h"
#include "libs/vidlib.h"
#include SDL_INCLUDE(SDL_thread.h)

SDL_Surface *SDL_Video;
SDL_Surface *SDL_Screen;
SDL_Surface *TransitionScreen;

SDL_Surface *SDL_Screens[TFB_GFX_NUMSCREENS];

SDL_Surface *format_conv_surf = NULL;

static volatile BOOLEAN abortFlag = FALSE;

int GfxFlags = 0;

TFB_GRAPHICS_BACKEND *graphics_backend = NULL;

volatile int QuitPosted = 0;
volatile int GameActive = 1; // Track the SDL_ACTIVEEVENT state SDL_APPACTIVE

static void TFB_PreQuit (void);

void
TFB_PreInit (void)
{
	log_add (log_Info, "Initializing base SDL functionality.");
	log_add (log_Info, "Using SDL version %d.%d.%d (compiled with "
			"%d.%d.%d)", SDL_Linked_Version ()->major,
			SDL_Linked_Version ()->minor, SDL_Linked_Version ()->patch,
			SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
#if 0
	if (SDL_Linked_Version ()->major != SDL_MAJOR_VERSION ||
			SDL_Linked_Version ()->minor != SDL_MINOR_VERSION ||
			SDL_Linked_Version ()->patch != SDL_PATCHLEVEL) {
		log_add (log_Warning, "The used SDL library is not the same version "
				"as the one used to compile The Ur-Quan Masters with! "
				"If you experience any crashes, this would be an excellent "
				"suspect.");
	}
#endif

	if ((SDL_Init (SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1))
	{
		log_add (log_Fatal, "Could not initialize SDL: %s.", SDL_GetError ());
		exit (EXIT_FAILURE);
	}

	atexit (TFB_PreQuit);
}

static void
TFB_PreQuit (void)
{
	SDL_Quit ();
}

int
TFB_ReInitGraphics (int driver, int flags, int width, int height)
{
	int result;
	int togglefullscreen = 0;
	char caption[200];

	if (GfxFlags == (flags ^ TFB_GFXFLAGS_FULLSCREEN) &&
			driver == GraphicsDriver &&
			width == ScreenWidthActual && height == ScreenHeightActual)
	{
		togglefullscreen = 1;
	}

	GfxFlags = flags;

	if (driver == TFB_GFXDRIVER_SDL_OPENGL)
	{
#ifdef HAVE_OPENGL
		result = TFB_GL_ConfigureVideo (driver, flags, width, height,
				togglefullscreen);
#else
		driver = TFB_GFXDRIVER_SDL_PURE;
		log_add (log_Warning, "OpenGL support not compiled in,"
				" so using pure SDL driver");
		result = TFB_Pure_ConfigureVideo (driver, flags, width, height,
				togglefullscreen);
#endif
	}
	else
	{
		result = TFB_Pure_ConfigureVideo (driver, flags, width, height,
				togglefullscreen);
	}

	sprintf (caption, "The Ur-Quan Masters v%d.%d.%d%s",
			UQM_MAJOR_VERSION, UQM_MINOR_VERSION,
			UQM_PATCH_VERSION, UQM_EXTRA_VERSION);
	SDL_WM_SetCaption (caption, NULL);

	if (flags & TFB_GFXFLAGS_FULLSCREEN)
		SDL_ShowCursor (SDL_DISABLE);
	else
		SDL_ShowCursor (SDL_ENABLE);

	return result;
}

int
TFB_InitGraphics (int driver, int flags, int width, int height)
{
	int result;
	char caption[200];

	GfxFlags = flags;

	if (driver == TFB_GFXDRIVER_SDL_OPENGL)
	{
#ifdef HAVE_OPENGL
		result = TFB_GL_InitGraphics (driver, flags, width, height);
#else
		driver = TFB_GFXDRIVER_SDL_PURE;
		log_add (log_Warning, "OpenGL support not compiled in,"
				" so using pure SDL driver");
		result = TFB_Pure_InitGraphics (driver, flags, width, height);
#endif
	}
	else
	{
		result = TFB_Pure_InitGraphics (driver, flags, width, height);
	}

	sprintf (caption, "The Ur-Quan Masters v%d.%d.%d%s", 
			UQM_MAJOR_VERSION, UQM_MINOR_VERSION, 
			UQM_PATCH_VERSION, UQM_EXTRA_VERSION);
	SDL_WM_SetCaption (caption, NULL);

	if (flags & TFB_GFXFLAGS_FULLSCREEN)
		SDL_ShowCursor (SDL_DISABLE);

	Init_DrawCommandQueue ();

	TFB_DrawCanvas_Initialize ();

	return 0;
}

void
TFB_UninitGraphics (void)
{
	int i;

	Uninit_DrawCommandQueue ();

	for (i = 0; i < TFB_GFX_NUMSCREENS; i++)
		UnInit_Screen (&SDL_Screens[i]);

	TFB_Pure_UninitGraphics ();
#ifdef HAVE_OPENGL
	TFB_GL_UninitGraphics ();
#endif

	UnInit_Screen (&format_conv_surf);
}

void
TFB_ProcessEvents ()
{
	SDL_Event Event;

	while (SDL_PollEvent (&Event) > 0)
	{
		/* Run through the InputEvent filter. */
		ProcessInputEvent (&Event);
		/* Handle graphics and exposure events. */
		switch (Event.type) {
			case SDL_ACTIVEEVENT:    /* Lose/gain visibility or focus */
				/* Up to three different state changes can occur in one event. */
				/* Here, disregard least significant change (mouse focus). */
#if 0 /* Currently disabled in mainline */
				// This controls the automatic sleep/pause when minimized.
				// On small displays (e.g. mobile devices), APPINPUTFOCUS would 
				//  be an appropriate substitution for APPACTIVE:
				// if (Event.active.state & SDL_APPINPUTFOCUS)
				if (Event.active.state & SDL_APPACTIVE)
					GameActive = Event.active.gain;
#endif
				break;
			case SDL_QUIT:
				QuitPosted = 1;
				break;
			case SDL_VIDEORESIZE:    /* User resized video mode */
				// TODO
				break;
			case SDL_VIDEOEXPOSE:    /* Screen needs to be redrawn */
				TFB_SwapBuffers (TFB_REDRAW_EXPOSE);
				break;
			default:
				break;
		}
	}
}

static BOOLEAN system_box_active = 0;
static SDL_Rect system_box;

void
SetSystemRect (const RECT *r)
{
	system_box_active = TRUE;
	system_box.x = r->corner.x;
	system_box.y = r->corner.y;
	system_box.w = r->extent.width;
	system_box.h = r->extent.height;
}

void
ClearSystemRect (void)
{
	system_box_active = FALSE;
}

void
TFB_SwapBuffers (int force_full_redraw)
{
	static int last_fade_amount = 255, last_transition_amount = 255;
	static int fade_amount = 255, transition_amount = 255;

	fade_amount = GetFadeAmount ();
	transition_amount = TransitionAmount;

	if (force_full_redraw == TFB_REDRAW_NO && !TFB_BBox.valid &&
			fade_amount == 255 && transition_amount == 255 &&
			last_fade_amount == 255 && last_transition_amount == 255)
		return;

	if (force_full_redraw == TFB_REDRAW_NO &&
			(fade_amount != 255 || transition_amount != 255 ||
			last_fade_amount != 255 || last_transition_amount != 255))
		force_full_redraw = TFB_REDRAW_FADING;

	last_fade_amount = fade_amount;
	last_transition_amount = transition_amount;	

	graphics_backend->preprocess (force_full_redraw, transition_amount,
			fade_amount);
	graphics_backend->screen (TFB_SCREEN_MAIN, 255, NULL);

	if (transition_amount != 255)
	{
		SDL_Rect r;
		r.x = TransitionClipRect.corner.x;
		r.y = TransitionClipRect.corner.y;
		r.w = TransitionClipRect.extent.width;
		r.h = TransitionClipRect.extent.height;
		graphics_backend->screen (TFB_SCREEN_TRANSITION,
				255 - transition_amount, &r);
	}

	if (fade_amount != 255)
	{
		if (fade_amount < 255)
		{
			graphics_backend->color (0, 0, 0, 255 - fade_amount, NULL);
		}
		else
		{
			graphics_backend->color (255, 255, 255, 
					fade_amount - 255, NULL);
		}
	}

	if (system_box_active)
	{
		graphics_backend->screen (TFB_SCREEN_MAIN, 255, &system_box);
	}

	graphics_backend->postprocess ();
}

/* Probably ought to clean this away at some point. */
SDL_Surface *
TFB_DisplayFormatAlpha (SDL_Surface *surface)
{
	SDL_Surface* newsurf;
	SDL_PixelFormat* dstfmt;
	const SDL_PixelFormat* srcfmt = surface->format;
	
	// figure out what format to use (alpha/no alpha)
	if (surface->format->Amask)
		dstfmt = format_conv_surf->format;
	else
		dstfmt = SDL_Screen->format;

	if (srcfmt->BytesPerPixel == dstfmt->BytesPerPixel &&
			srcfmt->Rmask == dstfmt->Rmask &&
			srcfmt->Gmask == dstfmt->Gmask &&
			srcfmt->Bmask == dstfmt->Bmask &&
			srcfmt->Amask == dstfmt->Amask)
		return surface; // no conversion needed

	newsurf = SDL_ConvertSurface (surface, dstfmt, surface->flags);
	// SDL_SRCCOLORKEY and SDL_SRCALPHA cannot work at the same time,
	// so we need to disable one of them
	if ((surface->flags & SDL_SRCCOLORKEY) && newsurf
			&& (newsurf->flags & SDL_SRCCOLORKEY)
			&& (newsurf->flags & SDL_SRCALPHA))
		SDL_SetAlpha (newsurf, 0, 255);

	return newsurf;
}

// This function should only be called from the graphics thread,
// like from a TFB_DrawCommand_Callback command.
TFB_Canvas
TFB_GetScreenCanvas (SCREEN screen)
{
	return SDL_Screens[screen];
}

void
TFB_UploadTransitionScreen (void)
{
#ifdef HAVE_OPENGL
	if (GraphicsDriver == TFB_GFXDRIVER_SDL_OPENGL)
	{
		TFB_GL_UploadTransitionScreen ();
	}
#endif
}

bool
TFB_SetGamma (float gamma)
{
	return (SDL_SetGamma (gamma, gamma, gamma) == 0);
}

SDL_Surface *
Create_Screen (SDL_Surface *templat, int w, int h)
{
	SDL_Surface *newsurf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
			templat->format->BitsPerPixel,
			templat->format->Rmask, templat->format->Gmask,
			templat->format->Bmask, 0);
	if (newsurf == 0) {
		log_add (log_Error, "Couldn't create screen buffers: %s",
				SDL_GetError());
	}
	return newsurf;
}

int
ReInit_Screen (SDL_Surface **screen, SDL_Surface *templat, int w, int h)
{
	UnInit_Screen (screen);
	*screen = Create_Screen (templat, w, h);
	
	return *screen == 0 ? -1 : 0;
}

void
UnInit_Screen (SDL_Surface **screen)
{
	if (*screen == NULL)
		return;

	SDL_FreeSurface (*screen);
	*screen = NULL;
}
