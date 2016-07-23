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

#include "build.h"
#include "colors.h"
#include "controls.h"
// XXX: for CurStarDescPtr
#include "encount.h"
#include "comm.h"
#include "gamestr.h"
#include "load.h"
#include "starbase.h"
#include "sis.h"
#include "resinst.h"
#include "nameref.h"
#include "settings.h"
#include "setup.h"
#include "sounds.h"
#include "libs/graphics/gfx_common.h"
#include "libs/tasklib.h"


static void CleanupAfterStarBase (void);

static void
DrawBaseStateStrings (STARBASE_STATE OldState, STARBASE_STATE NewState)
{
	TEXT t;
	//STRING locString;

	SetContext (ScreenContext);
	SetContextFont (StarConFont);
	SetContextForeGroundColor (BLACK_COLOR);

	t.baseline.x = 73 - 4 + SAFE_X;
	t.align = ALIGN_CENTER;

	if (OldState == (STARBASE_STATE)~0)
	{
		t.baseline.y = 106 + 28 + (SAFE_Y + 4);
		for (OldState = TALK_COMMANDER; OldState < DEPART_BASE; ++OldState)
		{
			if (OldState != NewState)
			{
				t.pStr = GAME_STRING (STARBASE_STRING_BASE + 1 + OldState);
				t.CharCount = (COUNT)~0;
				font_DrawText (&t);
			}
			t.baseline.y += (23 - 4);
		}
	}

	t.baseline.y = 106 + 28 + (SAFE_Y + 4) + ((23 - 4) * OldState);
	t.pStr = GAME_STRING (STARBASE_STRING_BASE + 1 + OldState);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);

	SetContextForeGroundColor (
			BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x0A), 0x0E));
	t.baseline.y = 106 + 28 + (SAFE_Y + 4) + ((23 - 4) * NewState);
	t.pStr = GAME_STRING (STARBASE_STRING_BASE + 1 + NewState);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
}

void
DrawShipPiece (FRAME ModuleFrame, COUNT which_piece, COUNT which_slot,
		BOOLEAN DrawBluePrint)
{
	Color OldColor = UNDEFINED_COLOR;
			// Initialisation is just to keep the compiler silent.
	RECT r;
	STAMP Side, Top;
	SBYTE RepairSlot;

	RepairSlot = 0;
	switch (which_piece)
	{
		case FUSION_THRUSTER:
		case EMPTY_SLOT + 0:
			Side.origin.x = DRIVE_SIDE_X;
			Side.origin.y = DRIVE_SIDE_Y;
			Top.origin.x = DRIVE_TOP_X;
			Top.origin.y = DRIVE_TOP_Y;
			break;
		case TURNING_JETS:
		case EMPTY_SLOT + 1:
			Side.origin.x = JET_SIDE_X;
			Side.origin.y = JET_SIDE_Y;
			Top.origin.x = JET_TOP_X;
			Top.origin.y = JET_TOP_Y;
			break;
		default:
			if (which_piece < EMPTY_SLOT + 2)
			{
				RepairSlot = 1;
				if (which_piece < EMPTY_SLOT
						&& (which_slot == 0
						|| GLOBAL_SIS (ModuleSlots[
								which_slot - 1
								]) < EMPTY_SLOT))
					++RepairSlot;
			}
			else if (!DrawBluePrint)
			{
				if (which_slot == 0 || which_slot >= NUM_MODULE_SLOTS - 3)
					++which_piece;

				if (which_slot < NUM_MODULE_SLOTS - 1
						&& GLOBAL_SIS (ModuleSlots[
								which_slot + 1
								]) < EMPTY_SLOT)
				{
					RepairSlot = -1;
					if (which_piece == EMPTY_SLOT + 3
							|| which_slot + 1 == NUM_MODULE_SLOTS - 3)
						--RepairSlot;
				}
			}
			Side.origin.x = MODULE_SIDE_X;
			Side.origin.y = MODULE_SIDE_Y;
			Top.origin.x = MODULE_TOP_X;
			Top.origin.y = MODULE_TOP_Y;
			break;
	}

	Side.origin.x += which_slot * SHIP_PIECE_OFFSET;
	Side.frame = NULL;
	if (RepairSlot < 0)
	{
		Side.frame = SetAbsFrameIndex (ModuleFrame,
				((NUM_MODULES - 1) + (6 - 2)) + (NUM_MODULES + 6)
				- (RepairSlot + 1));
		DrawStamp (&Side);
	}
	else if (RepairSlot)
	{
		r.corner = Side.origin;
		r.extent.width = SHIP_PIECE_OFFSET;
		r.extent.height = 1;
		OldColor = SetContextForeGroundColor (BLACK_COLOR);
		DrawFilledRectangle (&r);
		r.corner.y += 23 - 1;
		DrawFilledRectangle (&r);

		r.extent.width = 1;
		r.extent.height = 8;
		if (RepairSlot == 2)
		{
			r.corner = Side.origin;
			DrawFilledRectangle (&r);
			r.corner.y += 15;
			DrawFilledRectangle (&r);
		}
		if (which_slot < (NUM_MODULE_SLOTS - 1))
		{
			r.corner = Side.origin;
			r.corner.x += SHIP_PIECE_OFFSET;
			DrawFilledRectangle (&r);
			r.corner.y += 15;
			DrawFilledRectangle (&r);
		}
	}

	if (DrawBluePrint)
	{
		if (RepairSlot)
			SetContextForeGroundColor (OldColor);
		Side.frame = SetAbsFrameIndex (ModuleFrame, which_piece - 1);
		DrawFilledStamp (&Side);
	}
	else
	{
		Top.origin.x += which_slot * SHIP_PIECE_OFFSET;
		if (RepairSlot < 0)
		{
			Top.frame = SetRelFrameIndex (Side.frame, -((NUM_MODULES - 1) + 6));
			DrawStamp (&Top);
		}
		else if (RepairSlot)
		{
			r.corner = Top.origin;
			r.extent.width = SHIP_PIECE_OFFSET;
			r.extent.height = 1;
			DrawFilledRectangle (&r);
			r.corner.y += 32 - 1;
			DrawFilledRectangle (&r);

			r.extent.width = 1;
			r.extent.height = 12;
			if (RepairSlot == 2)
			{
				r.corner = Top.origin;
				DrawFilledRectangle (&r);
				r.corner.y += 20;
				DrawFilledRectangle (&r);
			}
			RepairSlot = (which_slot < NUM_MODULE_SLOTS - 1);
			if (RepairSlot)
			{
				r.corner = Top.origin;
				r.corner.x += SHIP_PIECE_OFFSET;
				DrawFilledRectangle (&r);
				r.corner.y += 20;
				DrawFilledRectangle (&r);
			}
		}

		Top.frame = SetAbsFrameIndex (ModuleFrame, which_piece);
		DrawStamp (&Top);

		Side.frame = SetRelFrameIndex (Top.frame, (NUM_MODULES - 1) + 6);
		DrawStamp (&Side);

		if (which_slot == 1 && which_piece == EMPTY_SLOT + 2)
		{
			STAMP s;

			s.origin = Top.origin;
			s.origin.x -= SHIP_PIECE_OFFSET;
			s.frame = SetAbsFrameIndex (ModuleFrame, NUM_MODULES + 5);
			DrawStamp (&s);
			s.origin = Side.origin;
			s.origin.x -= SHIP_PIECE_OFFSET;
			s.frame = SetRelFrameIndex (s.frame, (NUM_MODULES - 1) + 6);
			DrawStamp (&s);
		}

		if (RepairSlot)
		{
			Top.origin.x += SHIP_PIECE_OFFSET;
			Side.origin.x += SHIP_PIECE_OFFSET;
			which_piece = GLOBAL_SIS (ModuleSlots[++which_slot]);
			if (which_piece == EMPTY_SLOT + 2
					&& which_slot >= NUM_MODULE_SLOTS - 3)
				++which_piece;

			Top.frame = SetAbsFrameIndex (ModuleFrame, which_piece);
			DrawStamp (&Top);

			Side.frame = SetRelFrameIndex (Top.frame, (NUM_MODULES - 1) + 6);
			DrawStamp (&Side);
		}
	}
}

static void
rotateStarbase (MENU_STATE *pMS, FRAME AniFrame)
{
	static TimeCount NextTime = 0;
	TimeCount Now = GetTimeCounter ();

	if (AniFrame)
	{	// Setup the animation
		pMS->flash_frame0 = AniFrame;
		pMS->flash_rect0.corner.x = SAFE_X;
		pMS->flash_rect0.corner.y = SAFE_Y + 4;
	}
	
	if (Now >= NextTime || AniFrame)
	{
		STAMP s;

		NextTime = Now + (ONE_SECOND / 20);

		s.origin = pMS->flash_rect0.corner;
		s.frame = pMS->flash_frame0;
		DrawStamp (&s);

		s.frame = IncFrameIndex (s.frame);
		if (GetFrameIndex (s.frame) == 0)
		{	// Do not redraw the base frame, animation loops to frame 1
			s.frame = IncFrameIndex (s.frame);
		}
		pMS->flash_frame0 = s.frame;
	}
}

BOOLEAN
DoStarBase (MENU_STATE *pMS)
{
	// XXX: This function is full of hacks and otherwise strange code

	if (GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
	{
		pMS->CurState = DEPART_BASE;
		goto ExitStarBase;
	}
	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);

	if (!pMS->Initialized)
	{
		LastActivity &= ~CHECK_LOAD;
		pMS->InputFunc = DoStarBase;

		LockMutex (GraphicsLock);
		SetFlashRect (NULL);

		if (pMS->hMusic)
		{
			StopMusic ();
			DestroyMusic (pMS->hMusic);
			pMS->hMusic = 0;
		}

		pMS->Initialized = TRUE;
		UnlockMutex (GraphicsLock);

		pMS->CurFrame = CaptureDrawable (LoadGraphic (STARBASE_ANIM));
		pMS->hMusic = LoadMusic (STARBASE_MUSIC);

		LockMutex (GraphicsLock);
		SetContext (ScreenContext);
		SetTransitionSource (NULL);
		BatchGraphics ();
		SetContextBackGroundColor (BLACK_COLOR);
		ClearDrawable ();
		rotateStarbase (pMS, pMS->CurFrame);
		DrawBaseStateStrings ((STARBASE_STATE)~0, pMS->CurState);
		ScreenTransition (3, NULL);
		PlayMusic (pMS->hMusic, TRUE, 1);
		UnbatchGraphics ();
		UnlockMutex (GraphicsLock);
	}
	else if (PulsedInputState.menu[KEY_MENU_SELECT])
	{
ExitStarBase:
		DestroyDrawable (ReleaseDrawable (pMS->CurFrame));
		pMS->CurFrame = 0;
		StopMusic ();
		if (pMS->hMusic)
		{
			DestroyMusic (pMS->hMusic);
			pMS->hMusic = 0;
		}

		if (pMS->CurState == DEPART_BASE)
		{
			if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
			{
				SET_GAME_STATE (STARBASE_VISITED, 0);
			}
			return (FALSE);
		}

		pMS->Initialized = FALSE;
		if (pMS->CurState == TALK_COMMANDER)
		{
			FlushInput ();
			InitCommunication (COMMANDER_CONVERSATION);
			// XXX: InitCommunication() clears these flags, and we need them
			//   This marks that we are in Starbase.
			SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, (BYTE)~0);
		}
		else
		{
			BYTE OldState;

			switch (OldState = pMS->CurState)
			{
				case OUTFIT_STARSHIP:
					pMS->InputFunc = DoOutfit;
					break;
				case SHIPYARD:
					pMS->InputFunc = DoShipyard;
					break;
			}

			SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
			DoInput (pMS, TRUE);

			pMS->Initialized = FALSE;
			pMS->CurState = OldState;
			pMS->InputFunc = DoStarBase;
		}
	}
	else
	{
		STARBASE_STATE NewState;

		NewState = pMS->CurState;
		if (PulsedInputState.menu[KEY_MENU_LEFT] || PulsedInputState.menu[KEY_MENU_UP])
		{
			if (NewState-- == TALK_COMMANDER)
				NewState = DEPART_BASE;
		}
		else if (PulsedInputState.menu[KEY_MENU_RIGHT] || PulsedInputState.menu[KEY_MENU_DOWN])
		{
			if (NewState++ == DEPART_BASE)
				NewState = TALK_COMMANDER;
		}

		LockMutex (GraphicsLock);
		BatchGraphics ();
		SetContext (ScreenContext);

		if (NewState != pMS->CurState)
		{
			DrawBaseStateStrings (pMS->CurState, NewState);
			pMS->CurState = NewState;
		}

		rotateStarbase (pMS, NULL);

		UnbatchGraphics ();
		UnlockMutex (GraphicsLock);

		SleepThread (ONE_SECOND / 30);
	}

	return (TRUE);
}

static void
DoTimePassage (void)
{
#define LOST_DAYS 14
	SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND * 2));
	LockMutex (GraphicsLock);
	MoveGameClockDays (LOST_DAYS);
	UnlockMutex (GraphicsLock);
}

void
VisitStarBase (void)
{
	MENU_STATE MenuState;
	CONTEXT OldContext;
	StatMsgMode prevMsgMode = SMM_UNDEFINED;

	// XXX: This should probably be moved out to Starcon2Main()
	if (GET_GAME_STATE (CHMMR_BOMB_STATE) == 2)
	{	// We were just transported by Chmmr to the Starbase
		// Force a reload of the SolarSys
		CurStarDescPtr = NULL;
		// This marks that we are in Starbase.
		SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, (BYTE)~0);
	}

	if (!GET_GAME_STATE (STARBASE_AVAILABLE))
	{
		HSHIPFRAG hStarShip;
		SHIP_FRAGMENT *FragPtr;

		// Unallied Starbase conversation
		SetCommIntroMode (CIM_CROSSFADE_SCREEN, 0);
		InitCommunication (COMMANDER_CONVERSATION);
		if (!GET_GAME_STATE (PROBE_ILWRATH_ENCOUNTER)
				|| (GLOBAL (CurrentActivity) & CHECK_ABORT))
		{
			CleanupAfterStarBase ();
			return;
		}

		/* Create an Ilwrath ship responding to the Ur-Quan
		 * probe's broadcast */
		hStarShip = CloneShipFragment (ILWRATH_SHIP,
				&GLOBAL (npc_built_ship_q), 7);
		FragPtr = LockShipFrag (&GLOBAL (npc_built_ship_q), hStarShip);
		/* Hack (sort of): Suppress the tally and salvage info
		 * after the battle */
		FragPtr->race_id = (BYTE)~0;
		UnlockShipFrag (&GLOBAL (npc_built_ship_q), hStarShip);

		InitCommunication (ILWRATH_CONVERSATION);
		if (GLOBAL_SIS (CrewEnlisted) == (COUNT)~0
				|| (GLOBAL (CurrentActivity) & CHECK_ABORT))
			return; // Killed by Ilwrath
		
		// After Ilwrath battle, about-to-ally Starbase conversation
		SetCommIntroMode (CIM_CROSSFADE_SCREEN, 0);
		InitCommunication (COMMANDER_CONVERSATION);
		if (GLOBAL (CurrentActivity) & CHECK_ABORT)
			return;
		// XXX: InitCommunication() clears these flags, and we need them
		//   This marks that we are in Starbase.
		SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, (BYTE)~0);
	}

	prevMsgMode = SetStatusMessageMode (SMM_RES_UNITS);

	if (GET_GAME_STATE (MOONBASE_ON_SHIP)
			|| GET_GAME_STATE (CHMMR_BOMB_STATE) == 2)
	{	// Go immediately into a conversation with the Commander when the
		// Starbase becomes available for the first time, or after Chmmr
		// install the bomb.
		DoTimePassage ();
		if (GLOBAL_SIS (CrewEnlisted) == (COUNT)~0)
			return; // You are now dead! Thank you! (killed by Kohr-Ah)

		SetCommIntroMode (CIM_FADE_IN_SCREEN, ONE_SECOND * 2);
		InitCommunication (COMMANDER_CONVERSATION);
		if (GLOBAL (CurrentActivity) & CHECK_ABORT)
			return;
		// XXX: InitCommunication() clears these flags, and we need them
		//   This marks that we are in Starbase.
		SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, (BYTE)~0);
	}

	memset (&MenuState, 0, sizeof (MenuState));
	MenuState.InputFunc = DoStarBase;
	
	OldContext = SetContext (ScreenContext);
	DoInput (&MenuState, TRUE);
	SetContext (OldContext);

	SetStatusMessageMode (prevMsgMode);
	CleanupAfterStarBase ();
}

static void
CleanupAfterStarBase (void)
{
	if (!(GLOBAL (CurrentActivity) & (CHECK_LOAD | CHECK_ABORT)))
	{
		// Mark as not in Starbase anymore
		SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 0);
		// Fake a load so Starcon2Main takes us to IP
		GLOBAL (CurrentActivity) = CHECK_LOAD;
		NextActivity = MAKE_WORD (IN_INTERPLANETARY, 0)
				| START_INTERPLANETARY;
	}
}

void
InstallBombAtEarth (void)
{
	DoTimePassage ();

	LockMutex (GraphicsLock);
	SetContext (ScreenContext);
	SetTransitionSource (NULL);
	SetContextBackGroundColor (BLACK_COLOR);
	ClearDrawable ();
	UnlockMutex (GraphicsLock);
	
	SleepThreadUntil (FadeScreen (FadeAllToColor, 0));
	
	SET_GAME_STATE (CHMMR_BOMB_STATE, 3); /* bomb processed */
	GLOBAL (CurrentActivity) = CHECK_LOAD; /* fake a load game */
	NextActivity = MAKE_WORD (IN_INTERPLANETARY, 0) | START_INTERPLANETARY;
	CurStarDescPtr = 0; /* force SolarSys reload */
}

// XXX: Doesn't really belong in this file.
COUNT
WrapText (const UNICODE *pStr, COUNT len, TEXT *tarray, SIZE field_width)
{
	COUNT num_lines;

	num_lines = 0;
	do
	{
		RECT r;
		COUNT OldCount;
		
		tarray->align = ALIGN_LEFT; /* set alignment to something */
		tarray->pStr = pStr;
		tarray->CharCount = 1;
		++num_lines;
		
		do
		{
			OldCount = tarray->CharCount;
			while (*++pStr != ' ' && (COUNT)(pStr - tarray->pStr) < len)
				;
			tarray->CharCount = pStr - tarray->pStr;
			TextRect (tarray, &r, NULL);
		} while (tarray->CharCount < len && r.extent.width < field_width);
	
		if (r.extent.width >= field_width)
		{
			if ((tarray->CharCount = OldCount) == 1)
			{
				do
				{
					++tarray->CharCount;
					TextRect (tarray, &r, NULL);
				} while (r.extent.width < field_width);
				--tarray->CharCount;
			}
		}
	
		pStr = tarray->pStr + tarray->CharCount;
		len -= tarray->CharCount;
		++tarray;
	
		if (len && (r.extent.width < field_width || OldCount > 1))
		{
			++pStr; /* skip white space */
			--len;
		}

	} while (len);

	return (num_lines);
}

