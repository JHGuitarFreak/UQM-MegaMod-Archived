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

#include "gameopt.h"

#include "build.h"
#include "colors.h"
#include "controls.h"
#include "starmap.h"
#include "menustat.h"
#include "sis.h"
#include "units.h"
#include "gamestr.h"
#include "options.h"
#include "save.h"
#include "settings.h"
#include "setup.h"
#include "sounds.h"
#include "util.h"
#include "libs/graphics/gfx_common.h"

#include <ctype.h>

extern FRAME PlayFrame;

#define MAX_SAVED_GAMES 50
#define SUMMARY_X_OFFS 14
#define SUMMARY_SIDE_OFFS 7
#define SAVES_PER_PAGE 5

#define MAX_NAME_SIZE  SIS_NAME_SIZE

static COUNT lastUsedSlot;

static NamingCallback *namingCB;

void
ConfirmSaveLoad (STAMP *MsgStamp)
{
	RECT r, clip_r;
	TEXT t;

	SetContextFont (StarConFont);
	GetContextClipRect (&clip_r);

	t.baseline.x = clip_r.extent.width >> 1;
	t.baseline.y = (clip_r.extent.height >> 1) + 3;
	t.align = ALIGN_CENTER;
	t.CharCount = (COUNT)~0;
	if (MsgStamp)
		t.pStr = GAME_STRING (SAVEGAME_STRING_BASE + 0);
				// "Saving . . ."
	else
		t.pStr = GAME_STRING (SAVEGAME_STRING_BASE + 1);
				// "Loading . . ."
	TextRect (&t, &r, NULL);
	r.corner.x -= 4;
	r.corner.y -= 4;
	r.extent.width += 8;
	r.extent.height += 8;
	if (MsgStamp)
	{
		*MsgStamp = SaveContextFrame (&r);
	}
	DrawStarConBox (&r, 2,
			BUILD_COLOR (MAKE_RGB15 (0x10, 0x10, 0x10), 0x19),
			BUILD_COLOR (MAKE_RGB15 (0x08, 0x08, 0x08), 0x1F),
			TRUE, BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x0A), 0x08));
	SetContextForeGroundColor (
			BUILD_COLOR (MAKE_RGB15 (0x14, 0x14, 0x14), 0x0F));
	font_DrawText (&t);
}

enum
{
	SAVE_GAME = 0,
	LOAD_GAME,
	QUIT_GAME,
	SETTINGS,
	EXIT_GAME_MENU,
};

enum
{
	SOUND_ON_SETTING,
	SOUND_OFF_SETTING,
	MUSIC_ON_SETTING,
	MUSIC_OFF_SETTING,
	CYBORG_OFF_SETTING,
	CYBORG_NORMAL_SETTING,
	CYBORG_DOUBLE_SETTING,
	CYBORG_SUPER_SETTING,
	CHANGE_CAPTAIN_SETTING,
	CHANGE_SHIP_SETTING,
	EXIT_SETTINGS_MENU,
};

static void
FeedbackSetting (BYTE which_setting)
{
	UNICODE buf[128];
	const char *tmpstr;

	buf[0] = '\0';
	// pre-terminate buffer in case snprintf() overflows
	buf[sizeof (buf) - 1] = '\0';

	switch (which_setting)
	{
		case SOUND_ON_SETTING:
		case SOUND_OFF_SETTING:
			snprintf (buf, sizeof (buf) - 1, "%s %s",
					GAME_STRING (OPTION_STRING_BASE + 0),
					GLOBAL (glob_flags) & SOUND_DISABLED
					? GAME_STRING (OPTION_STRING_BASE + 3) :
					GAME_STRING (OPTION_STRING_BASE + 4));
			break;
		case MUSIC_ON_SETTING:
		case MUSIC_OFF_SETTING:
			snprintf (buf, sizeof (buf) - 1, "%s %s",
					GAME_STRING (OPTION_STRING_BASE + 1),
					GLOBAL (glob_flags) & MUSIC_DISABLED
					? GAME_STRING (OPTION_STRING_BASE + 3) :
					GAME_STRING (OPTION_STRING_BASE + 4));
			break;
		case CYBORG_OFF_SETTING:
		case CYBORG_NORMAL_SETTING:
		case CYBORG_DOUBLE_SETTING:
		case CYBORG_SUPER_SETTING:
			if (optWhichMenu == OPT_PC &&
					which_setting > CYBORG_NORMAL_SETTING)
			{
				if (which_setting == CYBORG_DOUBLE_SETTING)
					tmpstr = "+";
				else
					tmpstr = "++";
			}
			else
				tmpstr = "";
			snprintf (buf, sizeof (buf) - 1, "%s %s%s",
					GAME_STRING (OPTION_STRING_BASE + 2),
					!(GLOBAL (glob_flags) & CYBORG_ENABLED)
					? GAME_STRING (OPTION_STRING_BASE + 3) :
					GAME_STRING (OPTION_STRING_BASE + 4),
					tmpstr);
			break;
		case CHANGE_CAPTAIN_SETTING:
		case CHANGE_SHIP_SETTING:
			utf8StringCopy (buf, sizeof (buf),
					GAME_STRING (NAMING_STRING_BASE + 0));
			break;
	}

	DrawStatusMessage (buf);
}

#define DDSHS_NORMAL   0
#define DDSHS_EDIT     1
#define DDSHS_BLOCKCUR 2

static const RECT captainNameRect = {
	/* .corner = */ {
		/* .x = */ 3,
		/* .y = */ 10
	}, /* .extent = */ {
		/* .width = */ SHIP_NAME_WIDTH - 2,
		/* .height = */ SHIP_NAME_HEIGHT
	}
};
static const RECT shipNameRect = {
	/* .corner = */ {
		/* .x = */ 2,
		/* .y = */ 20
	}, /* .extent = */ {
		/* .width = */ SHIP_NAME_WIDTH,
		/* .height = */ SHIP_NAME_HEIGHT
	}
};


static BOOLEAN
DrawNameString (bool nameCaptain, UNICODE *Str, COUNT CursorPos,
		COUNT state)
{
	RECT r;
	TEXT lf;
	Color BackGround, ForeGround;
	FONT Font;

	{
		if (nameCaptain)
		{	// Naming the captain
			Font = TinyFont;
			r = captainNameRect;
			lf.baseline.x = r.corner.x + (r.extent.width >> 1) - 1;

			BackGround = BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x1F), 0x09);
			ForeGround = BUILD_COLOR (MAKE_RGB15 (0x0A, 0x1F, 0x1F), 0x0B);
		}
		else
		{	// Naming the flagship
			Font = StarConFont;
			r = shipNameRect;
			lf.baseline.x = r.corner.x + (r.extent.width >> 1);

			BackGround = BUILD_COLOR (MAKE_RGB15 (0x0F, 0x00, 0x00), 0x2D);
			ForeGround = BUILD_COLOR (MAKE_RGB15 (0x1F, 0x0A, 0x00), 0x7D);
		}

		lf.baseline.y = r.corner.y + r.extent.height - 1;
		lf.align = ALIGN_CENTER;
	}

	SetContext (StatusContext);
	SetContextFont (Font);
	lf.pStr = Str;
	lf.CharCount = (COUNT)~0;

	if (!(state & DDSHS_EDIT))
	{	// normal state
		if (nameCaptain)
			DrawCaptainsName ();
		else
			DrawFlagshipName (TRUE);
	}
	else
	{	// editing state
		COUNT i;
		RECT text_r;
		BYTE char_deltas[MAX_NAME_SIZE];
		BYTE *pchar_deltas;

		TextRect (&lf, &text_r, char_deltas);
		if ((text_r.extent.width + 2) >= r.extent.width)
		{	// the text does not fit the input box size and so
			// will not fit when displayed later
			// disallow the change
			return (FALSE);
		}

		PreUpdateFlashRect ();

		SetContextForeGroundColor (BackGround);
		DrawFilledRectangle (&r);

		pchar_deltas = char_deltas;
		for (i = CursorPos; i > 0; --i)
			text_r.corner.x += *pchar_deltas++;
		if (CursorPos < lf.CharCount) /* end of line */
			--text_r.corner.x;

		if (state & DDSHS_BLOCKCUR)
		{	// Use block cursor for keyboardless systems
			if (CursorPos == lf.CharCount)
			{	// cursor at end-line -- use insertion point
				text_r.extent.width = 1;
			}
			else if (CursorPos + 1 == lf.CharCount)
			{	// extra pixel for last char margin
				text_r.extent.width = (SIZE)*pchar_deltas + 2;
			}
			else
			{	// normal mid-line char
				text_r.extent.width = (SIZE)*pchar_deltas + 1;
			}
		}
		else
		{	// Insertion point cursor
			text_r.extent.width = 1;
		}

		text_r.corner.y = r.corner.y;
		text_r.extent.height = r.extent.height;
		SetContextForeGroundColor (BLACK_COLOR);
		DrawFilledRectangle (&text_r);

		SetContextForeGroundColor (ForeGround);
		font_DrawText (&lf);

		PostUpdateFlashRect ();
	}

	return (TRUE);
}

static BOOLEAN
OnNameChange (TEXTENTRY_STATE *pTES)
{
	bool nameCaptain = (bool) pTES->CbParam;
	COUNT hl = DDSHS_EDIT;

	if (pTES->JoystickMode)
		hl |= DDSHS_BLOCKCUR;

	return DrawNameString (nameCaptain, pTES->BaseStr, pTES->CursorPos, hl);
}

static void
NameCaptainOrShip (bool nameCaptain)
{
	UNICODE buf[MAX_NAME_SIZE] = "";
	TEXTENTRY_STATE tes;
	UNICODE *Setting;

	SetFlashRect (nameCaptain ? &captainNameRect : &shipNameRect);

	DrawNameString (nameCaptain, buf, 0, DDSHS_EDIT);

	DrawStatusMessage (GAME_STRING (NAMING_STRING_BASE + 0));

	if (nameCaptain)
	{
		Setting = GLOBAL_SIS (CommanderName);
		tes.MaxSize = sizeof (GLOBAL_SIS (CommanderName));
	}
	else
	{
		Setting = GLOBAL_SIS (ShipName);
		tes.MaxSize = sizeof (GLOBAL_SIS (ShipName));
	}

	// text entry setup
	tes.Initialized = FALSE;
	tes.BaseStr = buf;
	tes.CursorPos = 0;
	tes.CbParam = (void*) nameCaptain;
	tes.ChangeCallback = OnNameChange;
	tes.FrameCallback = 0;

	if (DoTextEntry (&tes))
		utf8StringCopy (Setting, tes.MaxSize, buf);
	else
		utf8StringCopy (buf, sizeof (buf), Setting);

	SetFlashRect (SFR_MENU_3DO);

	DrawNameString (nameCaptain, buf, 0, DDSHS_NORMAL);

	if (namingCB)
		namingCB ();
}

static BOOLEAN
DrawSaveNameString (UNICODE *Str, COUNT CursorPos, COUNT state, COUNT gameIndex)
{
	RECT r;
	TEXT lf;
	Color BackGround, ForeGround;
	FONT Font;
	UNICODE fullStr[256], dateStr[80];

	DateToString (dateStr, sizeof dateStr, GLOBAL(GameClock.month_index),
			GLOBAL(GameClock.day_index), GLOBAL(GameClock.year_index));
	strncat (dateStr, ": ", sizeof(dateStr) - strlen(dateStr) -1);
	snprintf (fullStr, sizeof fullStr, "%s%s", dateStr, Str);

	SetContextForeGroundColor (BUILD_COLOR (MAKE_RGB15 (0x1B, 0x00, 0x1B), 0x33));
	r.extent.width = 15;
	if (MAX_SAVED_GAMES > 99)
		r.extent.width += 5;
	r.extent.height = 11;
	r.corner.x = 8;
	r.corner.y = (160 + ((gameIndex % SAVES_PER_PAGE) * 13));
	DrawRectangle (&r);

	r.extent.width = (204 - SAFE_X);
	r.corner.x = (30 + SAFE_X);
	DrawRectangle (&r);

	Font = TinyFont;
	lf.baseline.x = r.corner.x + 3;
	lf.baseline.y = r.corner.y + 8;

	BackGround = BUILD_COLOR (MAKE_RGB15 (0x1B, 0x00, 0x1B), 0x33);
	ForeGround = BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x14), 0x01);

	lf.align = ALIGN_LEFT;

	SetContextFont (Font);
	lf.pStr = fullStr;
	lf.CharCount = (COUNT)~0;

	if (!(state & DDSHS_EDIT))
	{
		TEXT t;

		SetContextForeGroundColor (BLACK_COLOR);
		DrawFilledRectangle (&r);

		t.baseline.x = r.corner.x + 3;
		t.baseline.y = r.corner.y + 8;
		t.align = ALIGN_LEFT;
		t.pStr = Str;
		t.CharCount = (COUNT)~0;
		SetContextForeGroundColor (CAPTAIN_NAME_TEXT_COLOR);
		font_DrawText (&lf);
	}
	else
	{	// editing state
		COUNT i, FullCursorPos;
		RECT text_r;
		BYTE char_deltas[256];
		BYTE *pchar_deltas;

		TextRect (&lf, &text_r, char_deltas);
		if ((text_r.extent.width + 2) >= r.extent.width)
		{	// the text does not fit the input box size and so
			// will not fit when displayed later
			// disallow the change
			return (FALSE);
		}

		PreUpdateFlashRect ();

		SetContextForeGroundColor (BackGround);
		DrawFilledRectangle (&r);

		pchar_deltas = char_deltas;

		FullCursorPos = CursorPos + strlen(dateStr) - 1;
		for (i = FullCursorPos; i > 0; --i)
			text_r.corner.x += *pchar_deltas++;

		if (FullCursorPos < lf.CharCount) /* end of line */
			--text_r.corner.x;

		if (state & DDSHS_BLOCKCUR)
		{	// Use block cursor for keyboardless systems
			if (FullCursorPos == lf.CharCount)
			{	// cursor at end-line -- use insertion point
				text_r.extent.width = 1;
			}
			else if (FullCursorPos + 1 == lf.CharCount)
			{	// extra pixel for last char margin
				text_r.extent.width = (SIZE)*pchar_deltas + 2;
			}
			else
			{	// normal mid-line char
				text_r.extent.width = (SIZE)*pchar_deltas + 1;
			}
		}
		else
		{	// Insertion point cursor
			text_r.extent.width = 1;
		}

		text_r.corner.y = r.corner.y;
		text_r.extent.height = r.extent.height;
		SetContextForeGroundColor (BLACK_COLOR);
		DrawFilledRectangle (&text_r);

		SetContextForeGroundColor (ForeGround);
		font_DrawText (&lf);
		PostUpdateFlashRect ();
	}

	return (TRUE);
}

static BOOLEAN
OnSaveNameChange (TEXTENTRY_STATE *pTES)
{
	COUNT hl = DDSHS_EDIT;
	COUNT *gameIndex = pTES->CbParam;

	if (pTES->JoystickMode)
		hl |= DDSHS_BLOCKCUR;

	return DrawSaveNameString (pTES->BaseStr, pTES->CursorPos, hl, *gameIndex);
}

static BOOLEAN
NameSaveGame (COUNT gameIndex, UNICODE *buf)
{
	TEXTENTRY_STATE tes;
	COUNT CursPos = strlen(buf);
	COUNT *gIndex = HMalloc (sizeof (COUNT));
	RECT r;
	*gIndex = gameIndex;

	DrawSaveNameString (buf, CursPos, DDSHS_EDIT, gameIndex);

	tes.MaxSize = SAVE_NAME_SIZE;

	// text entry setup
	tes.Initialized = FALSE;
	tes.BaseStr = buf;
	tes.CursorPos = CursPos;
	tes.CbParam = gIndex;
	tes.ChangeCallback = OnSaveNameChange;
	tes.FrameCallback = 0;
	r.extent.width = (204 - SAFE_X);
	r.extent.height = 11;
	r.corner.x = (30 + SAFE_X);
	r.corner.y = (160 + ((gameIndex % SAVES_PER_PAGE) * 13));
	SetFlashRect (&r);

	if (!DoTextEntry (&tes))
		buf[0] = 0;

	SetFlashRect(NULL);

	DrawSaveNameString (buf, CursPos, DDSHS_NORMAL, gameIndex);

	if (namingCB)
		namingCB ();

	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);

	HFree (gIndex);

	SetFlashRect (NULL);

	if (tes.Success)
		return (TRUE);
	else
		return (FALSE);
}

void
SetNamingCallback (NamingCallback *callback)
{
	namingCB = callback;
}

static BOOLEAN
DoSettings (MENU_STATE *pMS)
{
	BYTE cur_speed;

	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		return FALSE;

	cur_speed = (GLOBAL (glob_flags) & COMBAT_SPEED_MASK) >> COMBAT_SPEED_SHIFT;

	if (PulsedInputState.menu[KEY_MENU_CANCEL]
			|| (PulsedInputState.menu[KEY_MENU_SELECT]
			&& pMS->CurState == EXIT_SETTINGS_MENU))
	{
		return FALSE;
	}
	else if (PulsedInputState.menu[KEY_MENU_SELECT])
	{
		switch (pMS->CurState)
		{
			case SOUND_ON_SETTING:
			case SOUND_OFF_SETTING:
				ToggleSoundEffect ();
				pMS->CurState ^= 1;
				DrawMenuStateStrings (PM_SOUND_ON, pMS->CurState);
				break;
			case MUSIC_ON_SETTING:
			case MUSIC_OFF_SETTING:
				ToggleMusic ();
				pMS->CurState ^= 1;
				DrawMenuStateStrings (PM_SOUND_ON, pMS->CurState);
				break;
			case CHANGE_CAPTAIN_SETTING:
			case CHANGE_SHIP_SETTING:
				NameCaptainOrShip (pMS->CurState == CHANGE_CAPTAIN_SETTING);
				break;
			default:
				if (cur_speed++ < NUM_COMBAT_SPEEDS - 1)
					GLOBAL (glob_flags) |= CYBORG_ENABLED;
				else
				{
					cur_speed = 0;
					GLOBAL (glob_flags) &= ~CYBORG_ENABLED;
				}
				GLOBAL (glob_flags) =
						((GLOBAL (glob_flags) & ~COMBAT_SPEED_MASK)
						| (cur_speed << COMBAT_SPEED_SHIFT));
				pMS->CurState = CYBORG_OFF_SETTING + cur_speed;
				DrawMenuStateStrings (PM_SOUND_ON, pMS->CurState);
		}

		FeedbackSetting (pMS->CurState);
	}
	else if (DoMenuChooser (pMS, PM_SOUND_ON))
		FeedbackSetting (pMS->CurState);

	return TRUE;
}

static void
SettingsMenu (void)
{
	MENU_STATE MenuState;

	memset (&MenuState, 0, sizeof MenuState);
	MenuState.CurState = SOUND_ON_SETTING;

	DrawMenuStateStrings (PM_SOUND_ON, MenuState.CurState);
	FeedbackSetting (MenuState.CurState);

	MenuState.InputFunc = DoSettings;
	DoInput (&MenuState, FALSE);

	DrawStatusMessage (NULL);
}

typedef struct
{
	SUMMARY_DESC summary[MAX_SAVED_GAMES];
	BOOLEAN saving;
			// TRUE when saving, FALSE when loading
	BOOLEAN success;
			// TRUE when load/save succeeded
	FRAME SummaryFrame;

} PICK_GAME_STATE;

static void
DrawBlankSavegameDisplay (PICK_GAME_STATE *pickState)
{
	STAMP s;

	s.origin.x = 0;
	s.origin.y = 0;
	s.frame = SetAbsFrameIndex (pickState->SummaryFrame,
			GetFrameCount (pickState->SummaryFrame) - 1);
	DrawStamp (&s);
}

static void
DrawSaveLoad (PICK_GAME_STATE *pickState)
{
	STAMP s;

	s.origin.x = SUMMARY_X_OFFS + 1;
	s.origin.y = 0;
	s.frame = SetAbsFrameIndex (pickState->SummaryFrame,
			GetFrameCount (pickState->SummaryFrame) - 2);
	if (pickState->saving)
		s.frame = DecFrameIndex (s.frame);
	DrawStamp (&s);
}

static void
DrawSavegameCargo (SIS_STATE *sisState)
{
	COUNT i;
	STAMP s;
	TEXT t;
	UNICODE buf[40];
	static const Color cargo_color[] =
	{
		BUILD_COLOR (MAKE_RGB15_INIT (0x02, 0x0E, 0x13), 0x00),
		BUILD_COLOR (MAKE_RGB15_INIT (0x19, 0x00, 0x00), 0x00),
		BUILD_COLOR (MAKE_RGB15_INIT (0x10, 0x10, 0x10), 0x00),
		BUILD_COLOR (MAKE_RGB15_INIT (0x03, 0x05, 0x1E), 0x00),
		BUILD_COLOR (MAKE_RGB15_INIT (0x00, 0x18, 0x00), 0x00),
		BUILD_COLOR (MAKE_RGB15_INIT (0x1B, 0x1B, 0x00), 0x00),
		BUILD_COLOR (MAKE_RGB15_INIT (0x1E, 0x0D, 0x00), 0x00),
		BUILD_COLOR (MAKE_RGB15_INIT (0x14, 0x00, 0x14), 0x05),
		BUILD_COLOR (MAKE_RGB15_INIT (0x0F, 0x00, 0x19), 0x00),
	};
#define ELEMENT_ORG_Y      17
#define ELEMENT_SPACING_Y  12
#define ELEMENT_SPACING_X  36

	SetContext (SpaceContext);
	BatchGraphics ();
	SetContextFont (StarConFont);

	// setup element icons
	s.frame = SetAbsFrameIndex (MiscDataFrame,
			(NUM_SCANDOT_TRANSITIONS << 1) + 3);
	s.origin.x = 7 + SUMMARY_X_OFFS - SUMMARY_SIDE_OFFS + 3;
	s.origin.y = ELEMENT_ORG_Y;
	// setup element amounts
	t.baseline.x = 33 + SUMMARY_X_OFFS - SUMMARY_SIDE_OFFS + 3;
	t.baseline.y = ELEMENT_ORG_Y + 3;
	t.align = ALIGN_RIGHT;
	t.pStr = buf;

	// draw element icons and amounts
	for (i = 0; i < NUM_ELEMENT_CATEGORIES; ++i)
	{
		if (i == NUM_ELEMENT_CATEGORIES / 2)
		{
			s.origin.x += ELEMENT_SPACING_X;
			s.origin.y = ELEMENT_ORG_Y;
			t.baseline.x += ELEMENT_SPACING_X;
			t.baseline.y = ELEMENT_ORG_Y + 3;
		}
		// draw element icon
		DrawStamp (&s);
		s.frame = SetRelFrameIndex (s.frame, 5);
		s.origin.y += ELEMENT_SPACING_Y;
		// print element amount
		SetContextForeGroundColor (cargo_color[i]);
		snprintf (buf, sizeof buf, "%u", sisState->ElementAmounts[i]);
		t.CharCount = (COUNT)~0;
		font_DrawText (&t);
		t.baseline.y += ELEMENT_SPACING_Y;
	}

	// draw Bio icon
	s.origin.x = 24 + SUMMARY_X_OFFS - SUMMARY_SIDE_OFFS;
	s.origin.y = 68;
	s.frame = SetAbsFrameIndex (s.frame, 68);
	DrawStamp (&s);
	// print Bio amount
	t.baseline.x = 50 + SUMMARY_X_OFFS;
	t.baseline.y = s.origin.y + 3;
	SetContextForeGroundColor (cargo_color[i]);
	snprintf (buf, sizeof buf, "%u", sisState->TotalBioMass);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);

	UnbatchGraphics ();
}

static void
DrawSavegameSummary (PICK_GAME_STATE *pickState, COUNT gameIndex)
{
	SUMMARY_DESC *pSD = pickState->summary + gameIndex;
	RECT r;
	STAMP s;

	BatchGraphics ();

	if (pSD->year_index == 0)
	{
		// Unused save slot, draw 'Empty Game' message.
		s.origin.x = 0;
		s.origin.y = 0;
		s.frame = SetAbsFrameIndex (pickState->SummaryFrame,
				GetFrameCount (pickState->SummaryFrame) - 4);
		DrawStamp (&s);
	}
	else
	{
		// Game slot used, draw information about save game.
		COUNT i;
		RECT OldRect;
		TEXT t;
		QUEUE player_q;
		CONTEXT OldContext;
		SIS_STATE SaveSS;
		UNICODE buf[256];
		POINT starPt;

		// Save the states because we will hack them
		SaveSS = GlobData.SIS_state;
		player_q = GLOBAL (built_ship_q);

		OldContext = SetContext (StatusContext);
		// Hack StatusContext so we can use standard SIS display funcs
		GetContextClipRect (&OldRect);
		r.corner.x = SIS_ORG_X + ((SIS_SCREEN_WIDTH - STATUS_WIDTH) >> 1) +
				SAFE_X - 16 + SUMMARY_X_OFFS;
//		r.corner.x = SIS_ORG_X + ((SIS_SCREEN_WIDTH - STATUS_WIDTH) >> 1);
		r.corner.y = SIS_ORG_Y;
		r.extent.width = STATUS_WIDTH;
		r.extent.height = STATUS_HEIGHT;
		SetContextClipRect (&r);

		// Hack the states so that we can use standard SIS display funcs
		GlobData.SIS_state = pSD->SS;
		InitQueue (&GLOBAL (built_ship_q),
				MAX_BUILT_SHIPS, sizeof (SHIP_FRAGMENT));
		for (i = 0; i < pSD->NumShips; ++i)
			CloneShipFragment (pSD->ShipList[i], &GLOBAL (built_ship_q), 0);
		DateToString (buf, sizeof buf,
				pSD->month_index, pSD->day_index, pSD->year_index),
		ClearSISRect (DRAW_SIS_DISPLAY);
		DrawStatusMessage (buf);
		UninitQueue (&GLOBAL (built_ship_q));

		SetContextClipRect (&OldRect);

		SetContext (SpaceContext);
		// draw devices
		s.origin.y = 13;
		for (i = 0; i < 4; ++i)
		{
			COUNT j;

			s.origin.x = 140 + SUMMARY_X_OFFS + SUMMARY_SIDE_OFFS;
			for (j = 0; j < 4; ++j)
			{
				COUNT devIndex = (i * 4) + j;
				if (devIndex < pSD->NumDevices)
				{
					s.frame = SetAbsFrameIndex (MiscDataFrame, 77
							+ pSD->DeviceList[devIndex]);
					DrawStamp (&s);
				}
				s.origin.x += 18;
			}
			s.origin.y += 18;
		}

		SetContextFont (StarConFont);
		t.baseline.x = 173 + SUMMARY_X_OFFS + SUMMARY_SIDE_OFFS;
		t.align = ALIGN_CENTER;
		t.CharCount = (COUNT)~0;
		t.pStr = buf;
		if (pSD->Flags & AFTER_BOMB_INSTALLED)
		{
			// draw the bomb and the escape pod
			s.origin.x = SUMMARY_X_OFFS - SUMMARY_SIDE_OFFS + 6;
			s.origin.y = 0;
			s.frame = SetRelFrameIndex (pickState->SummaryFrame, 0);
			DrawStamp (&s);
			// draw RU "NO LIMIT" 
			s.origin.x = SUMMARY_X_OFFS + SUMMARY_SIDE_OFFS;
			s.frame = IncFrameIndex (s.frame);
			DrawStamp (&s);
		}
		else
		{
			DrawSavegameCargo (&pSD->SS);

			SetContext (RadarContext);
			// Hack RadarContext so we can use standard Lander display funcs
			GetContextClipRect (&OldRect);
			r.corner.x = SIS_ORG_X + 10 + SUMMARY_X_OFFS - SUMMARY_SIDE_OFFS;
			r.corner.y = SIS_ORG_Y + 84;
			r.extent = OldRect.extent;
			SetContextClipRect (&r);
			// draw the lander with upgrades
			InitLander (pSD->Flags | OVERRIDE_LANDER_FLAGS);
			SetContextClipRect (&OldRect);
			SetContext (SpaceContext);

			snprintf (buf, sizeof buf, "%u", pSD->SS.ResUnits);
			t.baseline.y = 102;
			SetContextForeGroundColor (
					BUILD_COLOR (MAKE_RGB15 (0x10, 0x00, 0x10), 0x01));
			font_DrawText (&t);
			t.CharCount = (COUNT)~0;
		}
		t.baseline.y = 126;
		snprintf (buf, sizeof buf, "%u",
				MAKE_WORD (pSD->MCreditLo, pSD->MCreditHi));
		SetContextForeGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x10, 0x00, 0x10), 0x01));
		font_DrawText (&t);

		// print the location
		t.baseline.x = 6;
		t.baseline.y = 139 + 6;
		t.align = ALIGN_LEFT;
		t.pStr = buf;
		starPt.x = LOGX_TO_UNIVERSE (pSD->SS.log_x);
		starPt.y = LOGY_TO_UNIVERSE (pSD->SS.log_y);
		switch (pSD->Activity)
		{
			case IN_LAST_BATTLE:
			case IN_INTERPLANETARY:
			case IN_PLANET_ORBIT:
			case IN_STARBASE:
			{
				BYTE QuasiState;
				STAR_DESC *SDPtr;

				QuasiState = GET_GAME_STATE (ARILOU_SPACE_SIDE);
				SET_GAME_STATE (ARILOU_SPACE_SIDE, 0);
				SDPtr = FindStar (NULL, &starPt, 1, 1);
				SET_GAME_STATE (ARILOU_SPACE_SIDE, QuasiState);
				if (SDPtr)
				{
					GetClusterName (SDPtr, buf);
					starPt = SDPtr->star_pt;
					break;
				}
			}
			default:
				buf[0] = '\0';
				break;
			case IN_HYPERSPACE:
				utf8StringCopy (buf, sizeof (buf),
						GAME_STRING (NAVIGATION_STRING_BASE + 0));
				break;
			case IN_QUASISPACE:
				utf8StringCopy (buf, sizeof (buf),
						GAME_STRING (NAVIGATION_STRING_BASE + 1));
				break;
		}

		SetContextFont (TinyFont);
		SetContextForeGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x1B, 0x00, 0x1B), 0x33));
		t.CharCount = (COUNT)~0;
		font_DrawText (&t);
		t.align = ALIGN_CENTER;
		t.baseline.x = SIS_SCREEN_WIDTH - SIS_TITLE_BOX_WIDTH - 4
				+ (SIS_TITLE_WIDTH >> 1);
		switch (pSD->Activity)
		{
			case IN_STARBASE:
				utf8StringCopy (buf, sizeof (buf), // Starbase
						GAME_STRING (STARBASE_STRING_BASE));
				break;
			case IN_LAST_BATTLE:
				utf8StringCopy (buf, sizeof (buf), // Sa-Matra
						GAME_STRING (PLANET_NUMBER_BASE + 32));
				break;
			case IN_PLANET_ORBIT:
				utf8StringCopy (buf, sizeof (buf), pSD->SS.PlanetName);
				break;
			default:
				snprintf (buf, sizeof buf, "%03u.%01u : %03u.%01u",
						starPt.x / 10, starPt.x % 10,
						starPt.y / 10, starPt.y % 10);
		}
		t.CharCount = (COUNT)~0;
		font_DrawText (&t);

		SetContext (OldContext);

		// Restore the states because we hacked them
		GLOBAL (built_ship_q) = player_q;
		GlobData.SIS_state = SaveSS;
	}

	UnbatchGraphics ();
}

static void
DrawGameSelection (PICK_GAME_STATE *pickState, COUNT selSlot)
{
	RECT r;
	TEXT t;
	COUNT i;
	COUNT curSlot;
	UNICODE buf[256];
	UNICODE buf2[80];

	BatchGraphics ();

	SetContextFont (TinyFont);

	// Erase the selection menu
	r.extent.width = 240;
	r.extent.height = 65;
	r.corner.x = 1;
	r.corner.y = 160;
	SetContextForeGroundColor (BLACK_COLOR);
	DrawFilledRectangle (&r);

	t.CharCount = (COUNT)~0;
	t.pStr = buf;
	t.align = ALIGN_LEFT;

	// Draw savegame slots info
	curSlot = selSlot - (selSlot % SAVES_PER_PAGE);
	for (i = 0; i < SAVES_PER_PAGE && curSlot < MAX_SAVED_GAMES;
			++i, ++curSlot)
	{
		SUMMARY_DESC *desc = &pickState->summary[curSlot];

		SetContextForeGroundColor ((curSlot == selSlot) ?
				(BUILD_COLOR (MAKE_RGB15 (0x1B, 0x00, 0x1B), 0x33)):
				(BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x14), 0x01)));
		r.extent.width = 15;
		if (MAX_SAVED_GAMES > 99)
			r.extent.width += 5;
		r.extent.height = 11;
		r.corner.x = 8;
		r.corner.y = 160 + (i * 13);
		DrawRectangle (&r);

		t.baseline.x = r.corner.x + 3;
		t.baseline.y = r.corner.y + 8;
		snprintf (buf, sizeof buf, (MAX_SAVED_GAMES > 99) ? "%03u" : "%02u",
				curSlot);
		font_DrawText (&t);

		r.extent.width = 204 - SAFE_X;
		r.corner.x = 30 + SAFE_X;
		DrawRectangle (&r);

		t.baseline.x = r.corner.x + 3;
		if (desc->year_index == 0)
		{
			utf8StringCopy (buf, sizeof buf,
					GAME_STRING (SAVEGAME_STRING_BASE + 3)); // "Empty Slot"
		}
		else
		{
			DateToString (buf2, sizeof buf2, desc->month_index,
					desc->day_index, desc->year_index);
			snprintf (buf, sizeof buf, "%s: %s", buf2, desc->SaveName[0] ? desc->SaveName : GAME_STRING (SAVEGAME_STRING_BASE + 4));
		}
		font_DrawText (&t);
	}

	UnbatchGraphics ();
}

static void
RedrawPickDisplay (PICK_GAME_STATE *pickState, COUNT selSlot)
{
	BatchGraphics ();
	DrawBlankSavegameDisplay (pickState);
	DrawSavegameSummary (pickState, selSlot);
	DrawGameSelection (pickState, selSlot);
	UnbatchGraphics ();
}

static void
LoadGameDescriptions (SUMMARY_DESC *pSD)
{
	COUNT i;

	for (i = 0; i < MAX_SAVED_GAMES; ++i, ++pSD)
	{
		if (!LoadGame (i, pSD))
			pSD->year_index = 0;
	}
}

static BOOLEAN
DoPickGame (MENU_STATE *pMS)
{
	PICK_GAME_STATE *pickState = pMS->privData;
	BYTE NewState;
	SUMMARY_DESC *pSD;
	DWORD TimeIn = GetTimeCounter ();

	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		return FALSE;

	if (PulsedInputState.menu[KEY_MENU_CANCEL])
	{
		pickState->success = FALSE;
		return FALSE;
	}
	else if (PulsedInputState.menu[KEY_MENU_SELECT])
	{
		pSD = &pickState->summary[pMS->CurState];
		if (pickState->saving || pSD->year_index)
		{	// valid slot
			PlayMenuSound (MENU_SOUND_SUCCESS);
			pickState->success = TRUE;
			return FALSE;
		}
		PlayMenuSound (MENU_SOUND_FAILURE);
	}
	else
	{
		NewState = pMS->CurState;
		if (PulsedInputState.menu[KEY_MENU_LEFT]
				|| PulsedInputState.menu[KEY_MENU_PAGE_UP])
		{
			if (NewState == 0)
				NewState = MAX_SAVED_GAMES - 1;
			else if ((NewState - SAVES_PER_PAGE) > 0)
				NewState -= SAVES_PER_PAGE;
			else 
				NewState = 0;
		}
		else if (PulsedInputState.menu[KEY_MENU_RIGHT]
				|| PulsedInputState.menu[KEY_MENU_PAGE_DOWN])
		{
			if (NewState == MAX_SAVED_GAMES - 1)
				NewState = 0;
			else if ((NewState + SAVES_PER_PAGE) < MAX_SAVED_GAMES - 1)
				NewState += SAVES_PER_PAGE;
			else 
				NewState = MAX_SAVED_GAMES - 1;
		}
		else if (PulsedInputState.menu[KEY_MENU_UP])
		{
			if (NewState == 0)
				NewState = MAX_SAVED_GAMES - 1;
			else
				NewState--;
		}
		else if (PulsedInputState.menu[KEY_MENU_DOWN])
		{
			if (NewState == MAX_SAVED_GAMES - 1)
				NewState = 0;
			else
				NewState++;
		}

		if (NewState != pMS->CurState)
		{
			pMS->CurState = NewState;
			SetContext (SpaceContext);
			RedrawPickDisplay (pickState, pMS->CurState);
		}

		SleepThreadUntil (TimeIn + ONE_SECOND / 30);
	}

	return TRUE;
}

static BOOLEAN
SaveLoadGame (PICK_GAME_STATE *pickState, COUNT gameIndex, BOOLEAN *canceled_by_user)
{
	SUMMARY_DESC *desc = pickState->summary + gameIndex;
	UNICODE nameBuf[256];
	STAMP saveStamp;
	BOOLEAN success;

	saveStamp.frame = NULL;

	if (pickState->saving)
	{
		// Initialize the save name with whatever name is there already
		// SAVE_NAME_SIZE is less than 256, so this is safe.
		strncpy(nameBuf, desc->SaveName, SAVE_NAME_SIZE);
		nameBuf[SAVE_NAME_SIZE] = 0;
		if (NameSaveGame (gameIndex, nameBuf))
		{
			PlayMenuSound (MENU_SOUND_SUCCESS);
			ConfirmSaveLoad (pickState->saving ? &saveStamp : NULL);
			success = SaveGame (gameIndex, desc, nameBuf);
		}
		else
		{
			success = FALSE;
			*canceled_by_user = TRUE;
		}
	}
	else
	{
		ConfirmSaveLoad (pickState->saving ? &saveStamp : NULL);
		success = LoadGame (gameIndex, NULL);
	}

	// TODO: the same should be done for both save and load if we also
	//   display a load problem message
	if (pickState->saving)
	{	// restore the screen under "SAVING..." message
		DrawStamp (&saveStamp);
	}

	DestroyDrawable (ReleaseDrawable (saveStamp.frame));

	return success;
}

static BOOLEAN
PickGame (BOOLEAN saving, BOOLEAN fromMainMenu)
{
	CONTEXT OldContext;
	MENU_STATE MenuState;
	PICK_GAME_STATE pickState;
	RECT DlgRect;
	STAMP DlgStamp;
	TimeCount TimeOut;
	InputFrameCallback *oldCallback;

	memset (&pickState, 0, sizeof pickState);
	pickState.saving = saving;
	pickState.SummaryFrame = SetAbsFrameIndex (PlayFrame, 39);

	memset (&MenuState, 0, sizeof MenuState);
	MenuState.privData = &pickState;
	// select the last used slot
	MenuState.CurState = lastUsedSlot;

	TimeOut = FadeMusic (0, ONE_SECOND / 2);

	// Deactivate any background drawing, like planet rotation
	oldCallback = SetInputCallback (NULL);

	LoadGameDescriptions (pickState.summary);

	OldContext = SetContext (SpaceContext);
	// Save the current state of the screen for later restoration
	DlgStamp = SaveContextFrame (NULL);
	GetContextClipRect (&DlgRect);

	SleepThreadUntil (TimeOut);
	PauseMusic ();
	StopSound ();
	FadeMusic (NORMAL_VOLUME, 0);

	// draw the current savegame and fade in
	SetTransitionSource (NULL);
	BatchGraphics ();

	SetContextBackGroundColor (BLACK_COLOR);
	ClearDrawable ();
	RedrawPickDisplay (&pickState, MenuState.CurState);
	DrawSaveLoad (&pickState);

	if (fromMainMenu)
	{
		UnbatchGraphics ();
		FadeScreen (FadeAllToColor, ONE_SECOND / 2);
	}
	else
	{
		RECT ctxRect;

		GetContextClipRect (&ctxRect);
		ScreenTransition (3, &ctxRect);
		UnbatchGraphics ();
	}

	SetMenuSounds (MENU_SOUND_ARROWS | MENU_SOUND_PAGEUP | MENU_SOUND_PAGEDOWN,
			0);
	MenuState.InputFunc = DoPickGame;

	// Save/load retry loop
	while (1)
	{
		BOOLEAN canceled_by_user = FALSE;

		pickState.success = FALSE;
		DoInput (&MenuState, TRUE);
		if (!pickState.success)
			break; // canceled

		lastUsedSlot = MenuState.CurState;

		if (SaveLoadGame (&pickState, MenuState.CurState, &canceled_by_user))
			break; // all good

		// something broke
		if (saving && !canceled_by_user)
			SaveProblem ();
		// TODO: Shouldn't we have a Problem() equivalent for Load too?

		// reload and redraw everything
		LoadGameDescriptions (pickState.summary);
		RedrawPickDisplay (&pickState, MenuState.CurState);
	}

	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);

	if (pickState.success && !saving)
	{	// Load succeeded, signal up the chain
		GLOBAL (CurrentActivity) |= CHECK_LOAD;
	}

	if (!(GLOBAL (CurrentActivity) & CHECK_ABORT) &&
			(saving || (!pickState.success && !fromMainMenu)))
	{	// Restore previous screen
		SetTransitionSource (&DlgRect);
		BatchGraphics ();
		DrawStamp (&DlgStamp);
		ScreenTransition (3, &DlgRect);
		UnbatchGraphics ();
	}

	DestroyDrawable (ReleaseDrawable (DlgStamp.frame));

	SetContext (OldContext);

	ResumeMusic ();

	// Reactivate any background drawing, like planet rotation
	SetInputCallback (oldCallback);

	return pickState.success;
}

static BOOLEAN
DoGameOptions (MENU_STATE *pMS)
{
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		return FALSE;

	if (PulsedInputState.menu[KEY_MENU_CANCEL]
			|| (PulsedInputState.menu[KEY_MENU_SELECT]
			&& pMS->CurState == EXIT_GAME_MENU))
	{
		return FALSE;
	}
	else if (PulsedInputState.menu[KEY_MENU_SELECT])
	{
		switch (pMS->CurState)
		{
			case SAVE_GAME:
			case LOAD_GAME:
				SetFlashRect (NULL);
				if (PickGame (pMS->CurState == SAVE_GAME, FALSE))
					return FALSE;
				SetFlashRect (SFR_MENU_3DO);
				break;
			case QUIT_GAME:
				if (ConfirmExit ())
					return FALSE;
				break;
			case SETTINGS:
				SettingsMenu ();
				DrawMenuStateStrings (PM_SAVE_GAME, pMS->CurState);
				break;
		}
	}
	else
		DoMenuChooser (pMS, PM_SAVE_GAME);

	return TRUE;
}

// Returns TRUE when the owner menu should continue
BOOLEAN
GameOptions (void)
{
	MENU_STATE MenuState;

	memset (&MenuState, 0, sizeof MenuState);

	if (LastActivity == CHECK_LOAD)
	{	// Selected LOAD from main menu
		BOOLEAN success;

		DrawMenuStateStrings (PM_SAVE_GAME, LOAD_GAME);
		success	= PickGame (FALSE, TRUE);
		if (!success)
		{	// Selected LOAD from main menu, and canceled
			GLOBAL (CurrentActivity) |= CHECK_ABORT;
		}
		return FALSE;
	}

	MenuState.CurState = SAVE_GAME;
	DrawMenuStateStrings (PM_SAVE_GAME, MenuState.CurState);

	SetFlashRect (SFR_MENU_3DO);

	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
	MenuState.InputFunc = DoGameOptions;
	DoInput (&MenuState, TRUE);

	SetFlashRect (NULL);

	return !(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD));
}
