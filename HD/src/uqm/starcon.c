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

#include <stdlib.h>

#include "comm.h"
#include "battle.h"
#include "fmv.h"
#include "gameev.h"
#include "types.h"
#include "globdata.h"
#include "load.h"
#include "resinst.h"
#include "restart.h"
#include "starbase.h"
#include "setup.h"
#include "master.h"
#include "controls.h"
#include "starcon.h"
#include "clock.h"
		// for GameClockTick()
#include "hyper.h"
		// for SeedUniverse()
#include "planets/planets.h"
		// for ExploreSolarSys()
#include "uqmdebug.h"
#include "libs/tasklib.h"
#include "libs/log.h"
#include "libs/gfxlib.h"
#include "libs/graphics/gfx_common.h"
#include "libs/graphics/tfb_draw.h"
#include "libs/misc.h"

#include "uqmversion.h"
#include "options.h"
#include "build.h"
#include "gameopt.h" // JMS: For naming captain and ship at game start.

volatile int MainExited = FALSE;

// Open or close the periodically occuring QuasiSpace portal.
// It changes the appearant portal size when necessary.
static void
checkArilouGate (void)
{
	BYTE counter;

	counter = GET_GAME_STATE (ARILOU_SPACE_COUNTER);
	if (GET_GAME_STATE (ARILOU_SPACE) == OPENING)
	{	// The portal is opening or fully open
		if (counter < 9)
			++counter;
	}
	else
	{	// The portal is closing or fully closed
		if (counter > 0)
			--counter;
	}
	SET_GAME_STATE (ARILOU_SPACE_COUNTER, counter);
}

// Battle frame callback function.
// Called with GraphicsLock held
static void
on_battle_frame (void)
{
	GameClockTick ();
	checkArilouGate ();

	if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
		SeedUniverse ();

	DrawAutoPilotMessage (FALSE);
}

static void
BackgroundInitKernel (DWORD TimeOut)
{
	LoadMasterShipList (TaskSwitch);
	TaskSwitch ();
	InitGameKernel ();

	while ((GetTimeCounter () <= TimeOut) &&
	       !(GLOBAL (CurrentActivity) & CHECK_ABORT))
	{
		UpdateInputState ();
		TaskSwitch ();
	}
}

void
SignalStopMainThread (void)
{
	GamePaused = FALSE;
	GLOBAL (CurrentActivity) |= CHECK_ABORT;
	TaskSwitch ();
}

void
ProcessUtilityKeys (void)
{
	if (ImmediateInputState.menu[KEY_ABORT])
	{
		log_showBox (false, false);
		exit (EXIT_SUCCESS);
	}
	
	if (ImmediateInputState.menu[KEY_FULLSCREEN])
	{
		int flags = GfxFlags ^ TFB_GFXFLAGS_FULLSCREEN;
		
		// JMS: Force the usage of bilinear scaler in 1280x960 fullscreen.
		if (resolutionFactor > 0)
		{
			flags |= TFB_GFXFLAGS_SCALE_BILINEAR;
		}
		
		// clear ImmediateInputState so we don't repeat this next frame
		FlushInput ();
		TFB_DrawScreen_ReinitVideo (GraphicsDriver, flags, ScreenWidthActual,
				ScreenHeightActual);
	}

#if defined(DEBUG) || defined(USE_DEBUG_KEY)
	if (ImmediateInputState.menu[KEY_DEBUG])
	{
		// clear ImmediateInputState so we don't repeat this next frame
		FlushInput ();
		debugKeyPressed ();
	}
	
	// JMS: The secondary debug key.
	if (ImmediateInputState.menu[KEY_DEBUG_2])
	{
		// clear ImmediateInputState so we don't repeat this next frame
		FlushInput ();
		debugKey2Pressed ();
	}
	
	// JMS: The tertiary debug key.
	if (ImmediateInputState.menu[KEY_DEBUG_3])
	{
		// clear ImmediateInputState so we don't repeat this next frame
		FlushInput ();
		debugKey3Pressed ();
	}
	
	// JMS: The quaternary debug key.
	if (ImmediateInputState.menu[KEY_DEBUG_4])
	{
		// clear ImmediateInputState so we don't repeat this next frame
		FlushInput ();
		debugKey4Pressed ();
	}
#endif  /* DEBUG */
}

/* TODO: Remove these declarations once threading is gone. */
extern int snddriver, soundflags;

int
Starcon2Main (void *threadArg)
{
#if CREATE_JOURNAL
{
int ac = argc;
char **av = argv;

while (--ac > 0)
{
	++av;
	if ((*av)[0] == '-')
	{
		switch ((*av)[1])
		{
#if CREATE_JOURNAL
			case 'j':
				++create_journal;
				break;
#endif //CREATE_JOURNAL
		}
	}
}
}
#endif // CREATE_JOURNAL

	{
		/* TODO: Put initAudio back in main where it belongs once threading
		 *       is gone.
		 */
		extern sint32 initAudio (sint32 driver, sint32 flags);
		initAudio (snddriver, soundflags);
	}

	if (!LoadKernel (0,0))
	{
		log_add (log_Fatal, "\n  *** FATAL ERROR: Could not load basic content ***\n\nUQM requires at least the base content pack to run properly.");
		log_add (log_Fatal, "This file is typically called uqm-%d.%d.0-content.uqm.  UQM was expecting", UQM_MAJOR_VERSION, UQM_MINOR_VERSION);
		log_add (log_Fatal, "it in the %s/packages directory.", baseContentPath);
		log_add (log_Fatal, "Either your installation did not install the content pack at all, or it\ninstalled it in a different directory.\n\nFix your installation and rerun UQM.\n\n  *******************\n");
		log_showBox (true, true);

		MainExited = TRUE;
		return EXIT_FAILURE;
	}
	log_add (log_Info, "We've loaded the Kernel");
	
	GLOBAL (CurrentActivity) = 0;
	// show logo then splash and init the kernel in the meantime
	if(!optSkipIntro && optFMV){
		Logo ();
	}
	SplashScreen (BackgroundInitKernel);

//	OpenJournal ();
	while (StartGame ())
	{
		// Initialise a new game
		if (!SetPlayerInputAll ()) {
			log_add (log_Fatal, "Could not set player input.");
			explode ();  // Does not return;
		}
		InitGameStructures ();
		InitGameClock ();
		AddInitialGameEvents();
		
		// JMS: Name Captain & Ship at start (not at loading old game).
		if (LastActivity == (CHECK_LOAD | CHECK_RESTART))
			AskNameForCaptainAndShip();

		do
		{
			// Un-#if'ed to be used with bubblewarp.
			if (debugHook != NULL){
				void (*saveDebugHook) (void);
				saveDebugHook = debugHook;
				debugHook = NULL; // No further debugHook calls unless the called function resets debugHook.
				(*saveDebugHook) ();
				continue;
			} // Serosis

			SetStatusMessageMode (SMM_DEFAULT);

			if (!((GLOBAL (CurrentActivity) | NextActivity) & CHECK_LOAD))
				ZeroVelocityComponents (&GLOBAL (velocity));
					// not going into talking pet conversation
			else if (GLOBAL (CurrentActivity) & CHECK_LOAD)
				GLOBAL (CurrentActivity) = NextActivity;

			if ((GLOBAL (CurrentActivity) & START_ENCOUNTER)
					|| GET_GAME_STATE (CHMMR_BOMB_STATE) == 2)
			{
				if (GET_GAME_STATE (CHMMR_BOMB_STATE) == 2
						&& !GET_GAME_STATE (STARBASE_AVAILABLE))
				{	/* BGD mode */
					InstallBombAtEarth ();
				}
				else if (GET_GAME_STATE (GLOBAL_FLAGS_AND_DATA) == (BYTE)~0
						|| GET_GAME_STATE (CHMMR_BOMB_STATE) == 2)
				{
					GLOBAL (CurrentActivity) |= START_ENCOUNTER;
					VisitStarBase ();
				}
				else
				{
					GLOBAL (CurrentActivity) |= START_ENCOUNTER;
					RaceCommunication ();
				}

				if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
				{
					GLOBAL (CurrentActivity) &= ~START_ENCOUNTER;
					if (LOBYTE (GLOBAL (CurrentActivity)) == IN_INTERPLANETARY)
						GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
				}
			}
			else if (GLOBAL (CurrentActivity) & START_INTERPLANETARY)
			{
				GLOBAL (CurrentActivity) = MAKE_WORD (IN_INTERPLANETARY, 0);

				DrawAutoPilotMessage (TRUE);
				if (optTimeDilation){
					SetGameClockRate (INTERPLANETARY_CLOCK_RATE * 6);
				} else if(optFastForward){
					SetGameClockRate (INTERPLANETARY_CLOCK_RATE / 5);
				} else {
					SetGameClockRate (INTERPLANETARY_CLOCK_RATE);
				}
				ExploreSolarSys ();
			}
			else
			{
				// Entering HyperSpace or QuasiSpace.
				GLOBAL (CurrentActivity) = MAKE_WORD (IN_HYPERSPACE, 0);

				DrawAutoPilotMessage (TRUE);
				if (optTimeDilation){
					SetGameClockRate (HYPERSPACE_CLOCK_RATE * 6);
				} else if(optFastForward) {
					SetGameClockRate (HYPERSPACE_CLOCK_RATE / 5);
				} else {
					SetGameClockRate (HYPERSPACE_CLOCK_RATE);
				}
				Battle (&on_battle_frame);
			}

			LockMutex (GraphicsLock);
			SetFlashRect (NULL);
			UnlockMutex (GraphicsLock);

			LastActivity = GLOBAL (CurrentActivity);

			if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
					&& (LOBYTE (GLOBAL (CurrentActivity)) == WON_LAST_BATTLE
							// if died for some reason
					|| GLOBAL_SIS (CrewEnlisted) == (COUNT)~0))
			{
				if (GET_GAME_STATE (KOHR_AH_KILLED_ALL))
					InitCommunication (BLACKURQ_CONVERSATION);
						// surrendered to Ur-Quan
				else if (GLOBAL (CurrentActivity) & CHECK_RESTART)
					GLOBAL (CurrentActivity) &= ~CHECK_RESTART;
				break;
			}
			
			if (optUnlockShips){
				ActivateStarShip (VUX_SHIP, SET_ALLIED);
				ActivateStarShip (MELNORME_SHIP, SET_ALLIED);
				ActivateStarShip (ILWRATH_SHIP, SET_ALLIED);
				ActivateStarShip (MYCON_SHIP, SET_ALLIED);
				ActivateStarShip (SLYLANDRO_SHIP, SET_ALLIED);
				ActivateStarShip (URQUAN_SHIP, SET_ALLIED);
				ActivateStarShip (BLACK_URQUAN_SHIP, SET_ALLIED);
			}
			if (optUnlockUpgrades){
				SET_GAME_STATE (IMPROVED_LANDER_SPEED, 1);
				SET_GAME_STATE (IMPROVED_LANDER_CARGO, 1);
				SET_GAME_STATE (IMPROVED_LANDER_SHOT, 1);
				SET_GAME_STATE (LANDER_SHIELDS, (1 << EARTHQUAKE_DISASTER) | (1 << BIOLOGICAL_DISASTER) |
					(1 << LIGHTNING_DISASTER) | (1 << LAVASPOT_DISASTER));				
				GLOBAL (ModuleCost[ANTIMISSILE_DEFENSE]) = 4000 / MODULE_COST_SCALE;				
				GLOBAL (ModuleCost[BLASTER_WEAPON]) = 4000 / MODULE_COST_SCALE;
				GLOBAL (ModuleCost[HIGHEFF_FUELSYS]) = 1000 / MODULE_COST_SCALE;
				GLOBAL (ModuleCost[TRACKING_SYSTEM]) = 5000 / MODULE_COST_SCALE;
				GLOBAL (ModuleCost[CANNON_WEAPON]) = 6000 / MODULE_COST_SCALE;
				GLOBAL (ModuleCost[SHIVA_FURNACE]) = 4000 / MODULE_COST_SCALE;
				SET_GAME_STATE (MELNORME_TECH_STACK, 13);
			}
		} while (!(GLOBAL (CurrentActivity) & CHECK_ABORT));

		StopSound ();
		UninitGameClock ();
		UninitGameStructures ();
		ClearPlayerInputAll ();
	}
//	CloseJournal ();

	UninitGameKernel ();
	FreeMasterShipList ();
	FreeKernel ();

	log_showBox (false, false);
	MainExited = TRUE;

	(void) threadArg;  /* Satisfying compiler (unused parameter) */
	return 0;
}

