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

#include "../commall.h"
#include "resinst.h"
#include "strings.h"

#include "uqm/build.h"
#include "uqm/gameev.h"
#include "libs/mathlib.h"


static LOCDATA yehat_desc =
{
	NULL, /* init_encounter_func */
	NULL, /* post_encounter_func */
	NULL, /* uninit_encounter_func */
	YEHAT_PMAP_ANIM, /* AlienFrame */
	YEHAT_FONT, /* AlienFont */
	WHITE_COLOR_INIT, /* AlienTextFColor */
	BLACK_COLOR_INIT, /* AlienTextBColor */
	{0, 0}, /* AlienTextBaseline */
	0, /* (SIS_TEXT_WIDTH - 16) * 2 / 3, */ /* AlienTextWidth */
	ALIGN_CENTER, /* AlienTextAlign */
	VALIGN_MIDDLE, /* AlienTextValign */
	YEHAT_COLOR_MAP, /* AlienColorMap */
	YEHAT_MUSIC, /* AlienSong */
	NULL_RESOURCE, /* AlienAltSong */
	0, /* AlienSongFlags */
	YEHAT_CONVERSATION_PHRASES, /* PlayerPhrases */
	15, /* NumAnimations */
	{ /* AlienAmbientArray (ambient animations) */
		{ /* right hand-wing tapping keyboard; front guy */
			4, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM
					| WAIT_TALKING, /* AnimFlags */
			ONE_SECOND / 10, ONE_SECOND / 10, /* FrameRate */
			ONE_SECOND / 4, ONE_SECOND / 2,/* RestartRate */
			(1 << 6) | (1 << 7),
		},
		{ /* left hand-wing tapping keyboard; front guy */
			7, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM
					| WAIT_TALKING, /* AnimFlags */
			ONE_SECOND / 10, ONE_SECOND / 10, /* FrameRate */
			ONE_SECOND / 4, ONE_SECOND / 2,/* RestartRate */
			(1 << 6) | (1 << 7),
		},
		{
			10, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 20, 0, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
			(1 << 4) | (1 << 14),
		},
		{
			13, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 20, 0, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
			(1 << 5),
		},
		{
			16, /* StartIndex */
			5, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 15, ONE_SECOND / 15, /* FrameRate */
			ONE_SECOND * 6, ONE_SECOND * 3,/* RestartRate */
			(1 << 2) | (1 << 14),
		},
		{
			21, /* StartIndex */
			5, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 15, ONE_SECOND / 15, /* FrameRate */
			ONE_SECOND * 6, ONE_SECOND * 3,/* RestartRate */
			(1 << 3),
		},
		{ /* right arm-wing rising; front guy */
			26, /* StartIndex */
			2, /* NumFrames */
			YOYO_ANIM | WAIT_TALKING, /* AnimFlags */
			ONE_SECOND / 15, ONE_SECOND / 15, /* FrameRate */
			ONE_SECOND * 6, ONE_SECOND * 3,/* RestartRate */
			(1 << 0) | (1 << 1),
		},
		{ /* left arm-wing rising; front guy */
			28, /* StartIndex */
			2, /* NumFrames */
			YOYO_ANIM | WAIT_TALKING, /* AnimFlags */
			ONE_SECOND / 15, ONE_SECOND / 15, /* FrameRate */
			ONE_SECOND * 6, ONE_SECOND * 3,/* RestartRate */
			(1 << 0) | (1 << 1),
		},
		{
			30, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 30, ONE_SECOND / 30, /* FrameRate */
			ONE_SECOND / 30, ONE_SECOND / 30, /* RestartRate */
			0, /* BlockMask */
		},
		{
			33, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 30, ONE_SECOND / 30, /* FrameRate */
			ONE_SECOND / 30, ONE_SECOND / 30, /* RestartRate */
			0, /* BlockMask */
		},
		{
			36, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 30, ONE_SECOND / 30, /* FrameRate */
			ONE_SECOND / 30, ONE_SECOND / 30, /* RestartRate */
			0, /* BlockMask */
		},
		{
			39, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 30, ONE_SECOND / 30, /* FrameRate */
			ONE_SECOND / 30, ONE_SECOND / 30, /* RestartRate */
			0, /* BlockMask */
		},
		{
			42, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 30, ONE_SECOND / 30, /* FrameRate */
			ONE_SECOND / 30, ONE_SECOND / 30, /* RestartRate */
			0, /* BlockMask */
		},
		{
			45, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 30, ONE_SECOND / 30, /* FrameRate */
			ONE_SECOND / 30, ONE_SECOND / 30, /* RestartRate */
			0, /* BlockMask */
		},
		{
			48, /* StartIndex */
			4, /* NumFrames */
			YOYO_ANIM | WAIT_TALKING, /* AnimFlags */
			ONE_SECOND / 30, 0, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
			(1 << 2) | (1 << 4),
		},
	},
	{ /* AlienTransitionDesc - empty */
		0, /* StartIndex */
		0, /* NumFrames */
		0, /* AnimFlags */
		0, 0, /* FrameRate */
		0, 0, /* RestartRate */
		0, /* BlockMask */
	},
	{ /* AlienTalkDesc */
		1, /* StartIndex */
		3, /* NumFrames */
		0, /* AnimFlags */
		ONE_SECOND / 15, 0, /* FrameRate */
		ONE_SECOND / 12, 0, /* RestartRate */
		0, /* BlockMask */
	},
	NULL, /* AlienNumberSpeech - none */
	/* Filler for loaded resources */
	NULL, NULL, NULL,
	NULL,
	NULL,
};

static void
ExitConversation (RESPONSE_REF R)
{
	SET_GAME_STATE (BATTLE_SEGUE, 1);

	if (PLAYER_SAID (R, bye_homeworld))
		NPCPhrase (GOODBYE_AND_DIE_HOMEWORLD);
	else if (PLAYER_SAID (R, bye_royalist))
		NPCPhrase (GOODBYE_AND_DIE_ROYALIST);
	else if (PLAYER_SAID (R, i_demand_you_ally_homeworld0))
	{
		NPCPhrase (ENEMY_MUST_DIE);

		SET_GAME_STATE (NO_YEHAT_ALLY_HOME, 1);
	}
	else if (PLAYER_SAID (R, bye_space))
	{
		if ((BYTE)TFB_Random () & 1)
			NPCPhrase (GOODBYE_AND_DIE_SPACE);
		else
		{
			NPCPhrase (GO_IN_PEACE);

			SET_GAME_STATE (BATTLE_SEGUE, 0);
		}
	}
	else if (PLAYER_SAID (R, not_here)
			|| PLAYER_SAID (R, not_send))
	{
		switch (GET_GAME_STATE (YEHAT_REBEL_VISITS))
		{
			case 0:
				NPCPhrase (JUST_A_TRICK_1);
				break;
			case 1:
				NPCPhrase (JUST_A_TRICK_2);
				break;
		}
		SET_GAME_STATE (YEHAT_REBEL_VISITS, 1);
	}
	else if (PLAYER_SAID (R, ok_send))
	{
		NPCPhrase (WE_REVOLT);

		SET_GAME_STATE (BATTLE_SEGUE, 0);
		SET_GAME_STATE (YEHAT_CIVIL_WAR, 1);
		SET_GAME_STATE (YEHAT_VISITS, 0);
		SET_GAME_STATE (YEHAT_HOME_VISITS, 0);
		SET_GAME_STATE (YEHAT_REBEL_VISITS, 0);
		SET_GAME_STATE (YEHAT_REBEL_INFO, 0);
		SET_GAME_STATE (YEHAT_REBEL_TOLD_PKUNK, 0);
		SET_GAME_STATE (NO_YEHAT_INFO, 0);

		AddEvent (RELATIVE_EVENT, 0, 0, 0, YEHAT_REBEL_EVENT);
	}
}

static void
Royalists (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, how_is_rebellion))
	{
		BYTE NumVisits;

		NumVisits = GET_GAME_STATE (YEHAT_ROYALIST_INFO);
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (ROYALIST_REBELLION_1);
				break;
			case 1:
				NPCPhrase (ROYALIST_REBELLION_2);
				--NumVisits;
				break;
		}
		SET_GAME_STATE (YEHAT_ROYALIST_INFO, NumVisits);

		DISABLE_PHRASE (how_is_rebellion);
	}
	else if (PLAYER_SAID (R, what_about_pkunk_royalist))
	{
		if (GET_GAME_STATE (YEHAT_ABSORBED_PKUNK))
			NPCPhrase (PKUNK_ABSORBED_ROYALIST);
		else
			NPCPhrase (HATE_PKUNK_ROYALIST);

		SET_GAME_STATE (YEHAT_ROYALIST_TOLD_PKUNK, 1);
	}
	else if (PLAYER_SAID (R, sorry_about_revolution))
	{
		NPCPhrase (ALL_YOUR_FAULT);

		SET_GAME_STATE (NO_YEHAT_INFO, 1);
	}

	if (PHRASE_ENABLED (how_is_rebellion))
		Response (how_is_rebellion, Royalists);
	if (!GET_GAME_STATE (YEHAT_ROYALIST_TOLD_PKUNK)
			&& GET_GAME_STATE (PKUNK_VISITS)
			&& GET_GAME_STATE (PKUNK_HOME_VISITS))
		Response (what_about_pkunk_royalist, Royalists);
	if (!GET_GAME_STATE (NO_YEHAT_INFO))
		Response (sorry_about_revolution, Royalists);
	Response (bye_royalist, ExitConversation);
}

static void
StartRevolt (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, shofixti_alive_1))
	{
		NPCPhrase (SEND_HIM_OVER_1);

		SET_GAME_STATE (YEHAT_REBEL_TOLD_PKUNK, 1);
	}
	else if (PLAYER_SAID (R, shofixti_alive_2))
		NPCPhrase (SEND_HIM_OVER_2);

	if (ActivateStarShip (SHOFIXTI_SHIP, ESCORTING_FLAGSHIP))
		Response (ok_send, ExitConversation);
	else
		Response (not_here, ExitConversation);
	Response (not_send, ExitConversation);
}

static void
YehatHome (RESPONSE_REF R)
{

	if (PLAYER_SAID (R, whats_up_homeworld))
	{
		BYTE NumVisits;

		NumVisits = GET_GAME_STATE (YEHAT_ROYALIST_INFO);
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (GENERAL_INFO_HOMEWORLD_1);
				break;
			case 1:
				NPCPhrase (GENERAL_INFO_HOMEWORLD_2);
				--NumVisits;
				break;
		}
		SET_GAME_STATE (YEHAT_ROYALIST_INFO, NumVisits);

		DISABLE_PHRASE (whats_up_homeworld);
	}
	else if (PLAYER_SAID (R, at_least_help_us_homeworld))
	{
		NPCPhrase (NO_HELP_ENEMY);

		SET_GAME_STATE (NO_YEHAT_HELP_HOME, 1);
	}
	else if (PLAYER_SAID (R, give_info))
	{
		NPCPhrase (NO_INFO_FOR_ENEMY);

		SET_GAME_STATE (NO_YEHAT_INFO, 1);
	}
	else if (PLAYER_SAID (R, what_about_pkunk_royalist))
	{
		if (GET_GAME_STATE (YEHAT_ABSORBED_PKUNK))
			NPCPhrase (PKUNK_ABSORBED_ROYALIST);
		else
			NPCPhrase (HATE_PKUNK_ROYALIST);

		SET_GAME_STATE (YEHAT_ROYALIST_TOLD_PKUNK, 1);
	}

	if (PHRASE_ENABLED (whats_up_homeworld))
		Response (whats_up_homeworld, YehatHome);
	if (!GET_GAME_STATE (YEHAT_ROYALIST_TOLD_PKUNK)
			&& GET_GAME_STATE (PKUNK_VISITS)
			&& GET_GAME_STATE (PKUNK_HOME_VISITS))
		Response (what_about_pkunk_royalist, YehatHome);
	if (!GET_GAME_STATE (NO_YEHAT_HELP_HOME))
		Response (at_least_help_us_homeworld, YehatHome);
	if (!GET_GAME_STATE (NO_YEHAT_INFO))
		Response (give_info, YehatHome);
	if (!GET_GAME_STATE (NO_YEHAT_ALLY_HOME))
	{
		UNICODE buf[ALLIANCE_NAME_BUFSIZE];

		GetAllianceName (buf, name_1);
		construct_response (
				shared_phrase_buf,
				i_demand_you_ally_homeworld0,
				GLOBAL_SIS (CommanderName),
				i_demand_you_ally_homeworld1,
				buf,
				i_demand_you_ally_homeworld2,
				GLOBAL_SIS (ShipName),
				i_demand_you_ally_homeworld3,
				(UNICODE*)NULL);
		DoResponsePhrase (i_demand_you_ally_homeworld0,
				ExitConversation, shared_phrase_buf);
	}
	Response (bye_homeworld, ExitConversation);
}

static void
YehatSpace (RESPONSE_REF R)
{
	BYTE i, LastStack;
	RESPONSE_REF pStr[3];

	LastStack = 0;
	pStr[0] = pStr[1] = pStr[2] = 0;
	if (PLAYER_SAID (R, whats_up_space_1)
			|| PLAYER_SAID (R, whats_up_space_2)
			|| PLAYER_SAID (R, whats_up_space_3)
			|| PLAYER_SAID (R, whats_up_space_4))
	{
		BYTE NumVisits;

		NumVisits = GET_GAME_STATE (YEHAT_REBEL_INFO);
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (GENERAL_INFO_SPACE_1);
				break;
			case 1:
				NPCPhrase (GENERAL_INFO_SPACE_2);
				break;
			case 2:
				NPCPhrase (GENERAL_INFO_SPACE_3);
				break;
			case 3:
				NPCPhrase (GENERAL_INFO_SPACE_4);
				--NumVisits;
				break;
		}
		SET_GAME_STATE (YEHAT_REBEL_INFO, NumVisits);

		DISABLE_PHRASE (whats_up_space_1);
	}
	else if (PLAYER_SAID (R, i_demand_you_ally_space0))
	{
		NPCPhrase (WE_CANNOT_1);

		LastStack = 2;
		SET_GAME_STATE (NO_YEHAT_ALLY_SPACE, 1);
	}
	else if (PLAYER_SAID (R, obligation))
	{
		NPCPhrase (WE_CANNOT_2);

		SET_GAME_STATE (BATTLE_SEGUE, 0);
		SET_GAME_STATE (NO_YEHAT_ALLY_SPACE, 2);

		return;
	}
	else if (PLAYER_SAID (R, at_least_help_us_space))
	{
		NPCPhrase (SORRY_CANNOT);

		LastStack = 1;
		SET_GAME_STATE (NO_YEHAT_HELP_SPACE, 1);
	}
	else if (PLAYER_SAID (R, dishonor))
	{
		NPCPhrase (HERES_A_HINT);

		SET_GAME_STATE (NO_YEHAT_HELP_SPACE, 2);
	}
	else if (PLAYER_SAID (R, what_about_pkunk_royalist))
	{
		if (GET_GAME_STATE (YEHAT_ABSORBED_PKUNK))
			NPCPhrase (PKUNK_ABSORBED_ROYALIST);
		else
			NPCPhrase (HATE_PKUNK_ROYALIST);

		SET_GAME_STATE (YEHAT_ROYALIST_TOLD_PKUNK, 1);
	}

// SET_FUNCPTR (&PtrDesc, YehatSpace);
	if (PHRASE_ENABLED (whats_up_space_1))
	{
		switch (GET_GAME_STATE (YEHAT_REBEL_INFO))
		{
			case 0:
				pStr[0] = whats_up_space_1;
				break;
			case 1:
				pStr[0] = whats_up_space_2;
				break;
			case 2:
				pStr[0] = whats_up_space_3;
				break;
			case 3:
				pStr[0] = whats_up_space_4;
				break;
		}
	}
	switch (GET_GAME_STATE (NO_YEHAT_HELP_SPACE))
	{
		case 0:
			pStr[1] = at_least_help_us_space;
			break;
		case 1:
			pStr[1] = dishonor;
			break;
	}
	switch (GET_GAME_STATE (NO_YEHAT_ALLY_SPACE))
	{
		case 0:
		{
			UNICODE buf[ALLIANCE_NAME_BUFSIZE];

			GetAllianceName (buf, name_1);
			construct_response (
					shared_phrase_buf,
					i_demand_you_ally_space0,
					GLOBAL_SIS (CommanderName),
					i_demand_you_ally_space1,
					GLOBAL_SIS (ShipName),
					i_demand_you_ally_space2,
					buf,
					i_demand_you_ally_space3,
					(UNICODE*)NULL);
			pStr[2] = i_demand_you_ally_space0;
			break;
		}
		case 1:
			pStr[2] = obligation;
			break;
	}

	if (pStr[LastStack])
	{
		if (pStr[LastStack] != i_demand_you_ally_space0)
			Response (pStr[LastStack], YehatSpace);
		else
			DoResponsePhrase (pStr[LastStack], YehatSpace, shared_phrase_buf);
	}
	for (i = 0; i < 3; ++i)
	{
		if (i != LastStack && pStr[i])
		{
			if (pStr[i] != i_demand_you_ally_space0)
				Response (pStr[i], YehatSpace);
			else
				DoResponsePhrase (pStr[i], YehatSpace, shared_phrase_buf);
		}
	}
	if (!GET_GAME_STATE (YEHAT_ROYALIST_TOLD_PKUNK)
			&& GET_GAME_STATE (PKUNK_VISITS)
			&& GET_GAME_STATE (PKUNK_HOME_VISITS))
		Response (what_about_pkunk_royalist, YehatSpace);
	if (GET_GAME_STATE (SHOFIXTI_VISITS))
	{
		switch (GET_GAME_STATE (YEHAT_REBEL_TOLD_PKUNK))
		{
			case 0:
				Response (shofixti_alive_1, StartRevolt);
				break;
			case 1:
				Response (shofixti_alive_2, StartRevolt);
				break;
		}
	}
	Response (bye_space, ExitConversation);
}

static void
Intro (void)
{
	BYTE NumVisits;

	if (LOBYTE (GLOBAL (CurrentActivity)) == WON_LAST_BATTLE)
	{
		NPCPhrase (OUT_TAKES);

		SET_GAME_STATE (BATTLE_SEGUE, 0);
		return;
	}

	if (GET_GAME_STATE (YEHAT_CIVIL_WAR))
	{
		if (GET_GAME_STATE (GLOBAL_FLAGS_AND_DATA) & (1 << 7))
		{
			NumVisits = GET_GAME_STATE (YEHAT_HOME_VISITS);
			switch (NumVisits++)
			{
				case 0:
					NPCPhrase (ROYALIST_HOMEWORLD_HELLO_1);
					break;
				case 1:
					NPCPhrase (ROYALIST_HOMEWORLD_HELLO_2);
					--NumVisits;
					break;
			}
			SET_GAME_STATE (YEHAT_HOME_VISITS, NumVisits);
		}
		else
		{
			NumVisits = GET_GAME_STATE (YEHAT_VISITS);
			switch (NumVisits++)
			{
				case 0:
					NPCPhrase (ROYALIST_SPACE_HELLO_1);
					break;
				case 1:
					NPCPhrase (ROYALIST_SPACE_HELLO_2);
					--NumVisits;
					break;
			}
			SET_GAME_STATE (YEHAT_VISITS, NumVisits);
		}

		Royalists ((RESPONSE_REF)0);
	}
	else if (GET_GAME_STATE (GLOBAL_FLAGS_AND_DATA) & (1 << 7))
	{
		NumVisits = GET_GAME_STATE (YEHAT_HOME_VISITS);
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (HOMEWORLD_HELLO_1);
				break;
			case 1:
				NPCPhrase (HOMEWORLD_HELLO_2);
				--NumVisits;
				break;
		}
		SET_GAME_STATE (YEHAT_HOME_VISITS, NumVisits);

		YehatHome ((RESPONSE_REF)0);
	}
	else
	{
		NumVisits = GET_GAME_STATE (YEHAT_VISITS);
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (SPACE_HELLO_1);
				break;
			case 1:
				NPCPhrase (SPACE_HELLO_2);
				break;
			case 2:
				NPCPhrase (SPACE_HELLO_3);
				break;
			case 3:
				NPCPhrase (SPACE_HELLO_4);
				--NumVisits;
				break;
		}
		SET_GAME_STATE (YEHAT_VISITS, NumVisits);

		YehatSpace ((RESPONSE_REF)0);
	}
}

static COUNT
uninit_yehat (void)
{
	return (0);
}

static void
post_yehat_enc (void)
{
	// nothing defined so far
}

LOCDATA*
init_yehat_comm (void)
{
	LOCDATA *retval;

	yehat_desc.init_encounter_func = Intro;
	yehat_desc.post_encounter_func = post_yehat_enc;
	yehat_desc.uninit_encounter_func = uninit_yehat;

	yehat_desc.AlienTextBaseline.x = SIS_SCREEN_WIDTH * 2 / 3;
	yehat_desc.AlienTextBaseline.y = 60;
	yehat_desc.AlienTextWidth = (SIS_TEXT_WIDTH - 16) * 2 / 3;

	if (LOBYTE (GLOBAL (CurrentActivity)) != WON_LAST_BATTLE)
	{
		SET_GAME_STATE (BATTLE_SEGUE, 1);
	}
	else
	{
		SET_GAME_STATE (BATTLE_SEGUE, 0);
	}
	retval = &yehat_desc;

	return (retval);
}
