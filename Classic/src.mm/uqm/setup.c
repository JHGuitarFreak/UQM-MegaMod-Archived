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

#include "setup.h"

#include "coderes.h"
#include "controls.h"
#include "options.h"
#include "nameref.h"
#ifdef NETPLAY
#	include "supermelee/netplay/netmelee.h"
#endif
#include "init.h"
#include "intel.h"
#include "status.h"
#include "resinst.h"
#include "sounds.h"
#include "libs/compiler.h"
#include "libs/uio.h"
#include "libs/file.h"
#include "libs/graphics/gfx_common.h"
#include "libs/sound/sound.h"
#include "libs/threadlib.h"
#include "libs/vidlib.h"
#include "libs/log.h"
#include "libs/misc.h"

#include <assert.h>
#include <errno.h>
#include <string.h>


ACTIVITY LastActivity;
BYTE PlayerControl[NUM_PLAYERS];

// XXX: These declarations should really go to the file they belong to.
RESOURCE_INDEX hResIndex;
CONTEXT ScreenContext;
CONTEXT SpaceContext;
CONTEXT StatusContext;
CONTEXT OffScreenContext;
SIZE screen_width, screen_height;
FRAME Screen;
FONT StarConFont;
FONT MicroFont;
FONT TinyFont;
QUEUE race_q[NUM_PLAYERS];
FRAME ActivityFrame;
FRAME StatusFrame;
FRAME FlagStatFrame;
FRAME MiscDataFrame;
FRAME FontGradFrame;
Mutex GraphicsLock;
STRING GameStrings;
QUEUE disp_q;

uio_Repository *repository;
uio_DirHandle *rootDir;


static void
InitPlayerInput (void)
{
}

void
UninitPlayerInput (void)
{
#if DEMO_MODE
	DestroyInputDevice (ReleaseInputDevice (DemoInput));
#endif /* DEMO_MODE */
}

BOOLEAN
LoadKernel (int argc, char *argv[])
{
	InitSound (argc, argv);
	InitVideoPlayer (TRUE);

	ScreenContext = CreateContext ("ScreenContext");
	if (ScreenContext == NULL)
		return FALSE;

	Screen = CaptureDrawable (CreateDisplay (WANT_MASK | WANT_PIXMAP,
				&screen_width, &screen_height));
	if (Screen == NULL)
		return FALSE;

	SetContext (ScreenContext);
	SetContextFGFrame (Screen);
	SetContextOrigin (MAKE_POINT (0, 0));

	hResIndex = (RESOURCE_INDEX) InitResourceSystem ();
	if (hResIndex == 0)
		return FALSE;
	
	/* Load base content. */
	if (loadIndices (contentDir) == 0)
		return FALSE; // Must have at least one index in content dir

	/* Load addons demanded by the current configuration. */
	if (opt3doMusic)
	{
		loadAddon ("3domusic");
	}

	/* Always try to use voice data */
	if (!loadAddon ("3dovoice"))
		speechVolumeScale = 0.0f; // XXX: need better no-speech indicator

	if (optRemixMusic)
	{
		loadAddon ("remix");
	}

	if (optWhichIntro == OPT_3DO)
	{
		loadAddon ("3dovideo");
	}

	/* Now load the rest of the addons, in order. */
	prepareAddons (optAddons);

	{
		COLORMAP ColorMapTab;

		ColorMapTab = CaptureColorMap (LoadColorMap (STARCON_COLOR_MAP));
		if (ColorMapTab == NULL)
			return FALSE; // The most basic resource is missing
		SetColorMap (GetColorMapAddress (ColorMapTab));
		DestroyColorMap (ReleaseColorMap (ColorMapTab));
	}

	InitPlayerInput ();

	GLOBAL (CurrentActivity) = (ACTIVITY)~0;
	return TRUE;
}

BOOLEAN
InitContexts (void)
{
	RECT r;
	
	StatusContext = CreateContext ("StatusContext");
	if (StatusContext == NULL)
		return FALSE;

	SetContext (StatusContext);
	SetContextFGFrame (Screen);
	r.corner.x = SPACE_WIDTH + SAFE_X;
	r.corner.y = SAFE_Y;
	r.extent.width = STATUS_WIDTH;
	r.extent.height = STATUS_HEIGHT;
	SetContextClipRect (&r);
	
	SpaceContext = CreateContext ("SpaceContext");
	if (SpaceContext == NULL)
		return FALSE;
		
	OffScreenContext = CreateContext ("OffScreenContext");
	if (OffScreenContext == NULL)
		return FALSE;

	if (!InitQueue (&disp_q, MAX_DISPLAY_ELEMENTS, sizeof (ELEMENT)))
		return FALSE;

	return TRUE;
}

static BOOLEAN
InitKernel (void)
{
	COUNT counter;

	for (counter = 0; counter < NUM_PLAYERS; ++counter)
		InitQueue (&race_q[counter], MAX_SHIPS_PER_SIDE, sizeof (STARSHIP));

	StarConFont = LoadFont (STARCON_FONT);
	if (StarConFont == NULL)
		return FALSE;

	TinyFont = LoadFont (TINY_FONT);
	if (TinyFont == NULL)
		return FALSE;

	ActivityFrame = CaptureDrawable (LoadGraphic (ACTIVITY_ANIM));
	if (ActivityFrame == NULL)
		return FALSE;

	StatusFrame = CaptureDrawable (LoadGraphic (STATUS_MASK_PMAP_ANIM));
	if (StatusFrame == NULL)
		return FALSE;

	GameStrings = CaptureStringTable (LoadStringTable (STARCON_GAME_STRINGS));
	if (GameStrings == 0)
		return FALSE;

	MicroFont = LoadFont (MICRO_FONT);
	if (MicroFont == NULL)
		return FALSE;

	MenuSounds = CaptureSound (LoadSound (MENU_SOUNDS));
	if (MenuSounds == 0)
		return FALSE;

	InitStatusOffsets ();
	InitSpace ();

	return TRUE;
}

BOOLEAN
InitGameKernel (void)
{
	if (ActivityFrame == 0)
	{
		InitKernel ();
		InitContexts ();
	}
	return TRUE;
}

bool
SetPlayerInput (COUNT playerI)
{
	assert (PlayerInput[playerI] == NULL);

	switch (PlayerControl[playerI] & CONTROL_MASK) {
		case HUMAN_CONTROL:
			PlayerInput[playerI] =
					(InputContext *) HumanInputContext_new (playerI);
			break;
		case COMPUTER_CONTROL:
		case CYBORG_CONTROL:
			// COMPUTER_CONTROL is used in SuperMelee; the computer chooses
			// the ships and fights the battles.
			// CYBORG_CONTROL is used in the full game; the computer only
			// fights the battles. XXX: This will need to be handled
			// separately in the future if we want to remove the special
			// cases for ship selection with CYBORG_CONTROL from the
			// computer handlers.
			PlayerInput[playerI] =
					(InputContext *) ComputerInputContext_new (playerI);
			break;
#ifdef NETPLAY
		case NETWORK_CONTROL:
			PlayerInput[playerI] =
					(InputContext *) NetworkInputContext_new (playerI);
			break;
#endif
		default:
			log_add (log_Fatal,
					"Invalid control method in SetPlayerInput().");
			explode ();  /* Does not return */
	}

	return PlayerInput[playerI] != NULL;
}

bool
SetPlayerInputAll (void)
{
	COUNT playerI;
	for (playerI = 0; playerI < NUM_PLAYERS; playerI++)
		if (!SetPlayerInput (playerI))
			return false;
	return true;
}

void
ClearPlayerInput (COUNT playerI)
{
	if (PlayerInput[playerI] == NULL) {
		log_add (log_Debug, "ClearPlayerInput(): PlayerInput[%d] was NULL.",
				playerI);
		return;
	}

	PlayerInput[playerI]->handlers->deleteContext (PlayerInput[playerI]);
	PlayerInput[playerI] = NULL;
}

void
ClearPlayerInputAll (void)
{
	COUNT playerI;
	for (playerI = 0; playerI < NUM_PLAYERS; playerI++)
		ClearPlayerInput (playerI);
}

int
initIO (void)
{
	uio_init ();
	repository = uio_openRepository (0);

	rootDir = uio_openDir (repository, "/", 0);
	if (rootDir == NULL)
	{
		log_add (log_Fatal, "Could not open '/' dir.");
		return -1;
	}
	return 0;
}

void
uninitIO (void)
{
	uio_closeDir (rootDir);
	uio_closeRepository (repository);
	uio_unInit ();
}

