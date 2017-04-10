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

// JMS 2011: - Added loading res_factor to summary_desc. It'll help making saves between different resolutions compatible.

// JMS_GFX 2012: Merged the resolution Factor stuff from P6014.

#include <assert.h>

#include "load.h"

#include "build.h"
#include "libs/declib.h"
#include "encount.h"
#include "libs/file.h"
#include "globdata.h"
#include "load.h"
#include "options.h"
#include "setup.h"
#include "state.h"
#include "grpinfo.h"

#include "libs/tasklib.h"
#include "libs/log.h"
#include "libs/misc.h"

#include "util.h"

#include "gamestr.h" // JMS: For GAME_STRING

//#define DEBUG_LOAD

ACTIVITY NextActivity;

// XXX: these should handle endian conversions later
static inline COUNT
cread_8 (DECODE_REF fh, BYTE *v)
{
	BYTE t;
	if (!v) /* read value ignored */
		v = &t;
	return cread (v, 1, 1, fh);
}

static inline COUNT
cread_16 (DECODE_REF fh, UWORD *v)
{
	UWORD t;
	if (!v) /* read value ignored */
		v = &t;
	return cread (v, 2, 1, fh);
}

static inline COUNT
cread_16s (DECODE_REF fh, SWORD *v)
{
	UWORD t;
	COUNT ret;
	// value was converted to unsigned when saved
	ret = cread_16 (fh, &t);
	// unsigned to signed conversion
	if (v)
		*v = t;
	return ret;
}

static inline COUNT
cread_32 (DECODE_REF fh, DWORD *v)
{
	DWORD t;
	if (!v) /* read value ignored */
		v = &t;
	return cread (v, 4, 1, fh);
}

static inline COUNT
cread_32s (DECODE_REF fh, SDWORD *v)
{
	DWORD t;
	COUNT ret;
	// value was converted to unsigned when saved
	ret = cread_32 (fh, &t);
	// unsigned to signed conversion
	if (v)
		*v = t;
	return ret;
}

static inline COUNT
cread_ptr (DECODE_REF fh)
{
	DWORD t;
	return cread_32 (fh, &t); /* ptrs are useless in saves */
}

static inline COUNT
cread_a8 (DECODE_REF fh, BYTE *ar, COUNT count)
{
	assert (ar != NULL);
	return cread (ar, 1, count, fh) == count;
}

static inline size_t
read_8 (void *fp, BYTE *v)
{
	BYTE t;
	if (!v) /* read value ignored */
		v = &t;
	return ReadResFile (v, 1, 1, fp);
}

static inline size_t
read_16 (void *fp, UWORD *v)
{
	UWORD t;
	if (!v) /* read value ignored */
		v = &t;
	return ReadResFile (v, 2, 1, fp);
}

static inline size_t
read_32 (void *fp, DWORD *v)
{
	DWORD t;
	if (!v) /* read value ignored */
		v = &t;
	return ReadResFile (v, 4, 1, fp);
}

static inline size_t
read_32s (void *fp, SDWORD *v)
{
	DWORD t;
	COUNT ret;
	// value was converted to unsigned when saved
	ret = read_32 (fp, &t);
	// unsigned to signed conversion
	if (v)
		*v = t;
	return ret;
}

static inline size_t
read_ptr (void *fp)
{
	DWORD t;
	return read_32 (fp, &t); /* ptrs are useless in saves */
}

static inline size_t
read_a8 (void *fp, BYTE *ar, COUNT count)
{
	assert (ar != NULL);
	return ReadResFile (ar, 1, count, fp) == count;
}

static inline size_t
read_str (void *fp, char *str, COUNT count)
{
	// no type conversion needed for strings
	return read_a8 (fp, (BYTE *)str, count);
}

static inline size_t
read_a16 (void *fp, UWORD *ar, COUNT count)
{
	assert (ar != NULL);

	for ( ; count > 0; --count, ++ar)
	{
		if (read_16 (fp, ar) != 1)
			return 0;
	}
	return 1;
}

static void
LoadEmptyQueue (DECODE_REF fh)
{
	COUNT num_links;

	cread_16 (fh, &num_links);
	if (num_links)
	{
		log_add (log_Error, "LoadEmptyQueue(): BUG: the queue is not empty!");
#ifdef DEBUG
		explode ();
#endif
	}
}

static void
LoadShipQueue (DECODE_REF fh, QUEUE *pQueue)
{
	COUNT num_links;

	cread_16 (fh, &num_links);

	while (num_links--)
	{
		HSHIPFRAG hStarShip;
		SHIP_FRAGMENT *FragPtr;
		COUNT Index;
		BYTE tmpb;

		cread_16 (fh, &Index);

		hStarShip = CloneShipFragment (Index, pQueue, 0);
		FragPtr = LockShipFrag (pQueue, hStarShip);

		// Read SHIP_FRAGMENT elements
		cread_16 (fh, NULL); /* unused: was which_side */
		cread_8  (fh, &FragPtr->captains_name_index);
		cread_8  (fh, NULL); /* padding */
		cread_16 (fh, NULL); /* unused: was ship_flags */
		cread_8  (fh, &FragPtr->race_id);
		cread_8  (fh, &FragPtr->index);
		// XXX: reading crew as BYTE to maintain savegame compatibility
		cread_8  (fh, &tmpb);
		FragPtr->crew_level = tmpb;
		cread_8  (fh, &tmpb);
		FragPtr->max_crew = tmpb;
		cread_8  (fh, &FragPtr->energy_level);
		cread_8  (fh, &FragPtr->max_energy);
		cread_16 (fh, NULL); /* unused; was loc.x */
		cread_16 (fh, NULL); /* unused; was loc.y */

		UnlockShipFrag (pQueue, hStarShip);
	}
}

static void
LoadRaceQueue (DECODE_REF fh, QUEUE *pQueue)
{
	COUNT num_links;
	cread_16 (fh, &num_links);
	
	while (num_links--)
	{
		HFLEETINFO hStarShip;
		FLEET_INFO *FleetPtr;
		COUNT Index;
		BYTE tmpb;

		cread_16 (fh, &Index);

		hStarShip = GetStarShipFromIndex (pQueue, Index);
		FleetPtr = LockFleetInfo (pQueue, hStarShip);

		// Read FLEET_INFO elements
		cread_16 (fh, &FleetPtr->allied_state);
		cread_8  (fh, &FleetPtr->days_left);
		cread_8  (fh, &FleetPtr->growth_fract);
		cread_8  (fh, &tmpb);
		FleetPtr->crew_level = tmpb;
		cread_8  (fh, &tmpb);
		FleetPtr->max_crew = tmpb;
		cread_8  (fh, &FleetPtr->growth);
		cread_8  (fh, &FleetPtr->max_energy);
		cread_16s(fh, &FleetPtr->loc.x);
		cread_16s(fh, &FleetPtr->loc.y);

		cread_16 (fh, &FleetPtr->actual_strength);
		cread_16 (fh, &FleetPtr->known_strength);
		cread_16s(fh, &FleetPtr->known_loc.x);
		cread_16s(fh, &FleetPtr->known_loc.y);
		cread_8  (fh, &FleetPtr->growth_err_term);
		cread_8  (fh, &FleetPtr->func_index);
		cread_16s(fh, &FleetPtr->dest_loc.x);
		cread_16s(fh, &FleetPtr->dest_loc.y);
		cread_16 (fh, NULL); /* alignment padding */

		UnlockFleetInfo (pQueue, hStarShip);
	}
}

static void
LoadGroupQueue (DECODE_REF fh, QUEUE *pQueue)
{
	COUNT num_links;

	cread_16 (fh, &num_links);

	while (num_links--)
	{
		HIPGROUP hGroup;
		IP_GROUP *GroupPtr;
		BYTE tmpb;

		cread_16 (fh, NULL); /* unused; was race_id */

		hGroup = BuildGroup (pQueue, 0);
		GroupPtr = LockIpGroup (pQueue, hGroup);

		cread_16 (fh, NULL); /* unused; was which_side */
		cread_8  (fh, NULL); /* unused; was captains_name_index */
		cread_8  (fh, NULL); /* padding; for savegame compat */
		cread_16 (fh, &GroupPtr->group_counter);
		cread_8  (fh, &GroupPtr->race_id);
		cread_8  (fh, &tmpb); /* was var2 */
		GroupPtr->sys_loc = LONIBBLE (tmpb);
		GroupPtr->task = HINIBBLE (tmpb);
		cread_8  (fh, &GroupPtr->in_system); /* was crew_level */
		cread_8  (fh, NULL); /* unused; was max_crew */
		cread_8  (fh, &tmpb); /* was energy_level */
		GroupPtr->dest_loc = LONIBBLE (tmpb);
		GroupPtr->orbit_pos = HINIBBLE (tmpb);
		cread_8  (fh, &GroupPtr->group_id); /* was max_energy */
		cread_16s(fh, &GroupPtr->loc.x);
		cread_16s(fh, &GroupPtr->loc.y);

		UnlockIpGroup (pQueue, hGroup);
	}
}

static void
LoadEncounter (ENCOUNTER *EncounterPtr, DECODE_REF fh)
{
	COUNT i;

	cread_ptr (fh); /* useless ptr; HENCOUNTER pred */
	EncounterPtr->pred = 0;
	cread_ptr (fh); /* useless ptr; HENCOUNTER succ */
	EncounterPtr->succ = 0;
	cread_ptr (fh); /* useless ptr; HELEMENT hElement */
	EncounterPtr->hElement = 0;
	cread_16s (fh, &EncounterPtr->transition_state);
	cread_16s (fh, &EncounterPtr->origin.x);
	cread_16s (fh, &EncounterPtr->origin.y);
	cread_16  (fh, &EncounterPtr->radius);
	// STAR_DESC fields
	cread_16s (fh, &EncounterPtr->SD.star_pt.x);
	cread_16s (fh, &EncounterPtr->SD.star_pt.y);
	cread_8   (fh, &EncounterPtr->SD.Type);
	cread_8   (fh, &EncounterPtr->SD.Index);
	cread_16  (fh, NULL); /* alignment padding */

	// Load each entry in the BRIEF_SHIP_INFO array
	for (i = 0; i < MAX_HYPER_SHIPS; i++)
	{
		BRIEF_SHIP_INFO *ShipInfo = &EncounterPtr->ShipList[i];
		BYTE tmpb;

		cread_16  (fh, NULL); /* useless; was SHIP_INFO.ship_flags */
		cread_8   (fh, &ShipInfo->race_id);
		cread_8   (fh, NULL); /* useless; was SHIP_INFO.var2 */
		// XXX: reading crew as BYTE to maintain savegame compatibility
		cread_8   (fh, &tmpb);
		ShipInfo->crew_level = tmpb;
		cread_8   (fh, &tmpb);
		ShipInfo->max_crew = tmpb;
		cread_8   (fh, NULL); /* useless; was SHIP_INFO.energy_level */
		cread_8   (fh, &ShipInfo->max_energy);
		cread_16  (fh, NULL); /* useless; was SHIP_INFO.loc.x */
		cread_16  (fh, NULL); /* useless; was SHIP_INFO.loc.y */
		cread_32  (fh, NULL); /* useless val; STRING race_strings */
		cread_ptr (fh); /* useless ptr; FRAME icons */
		cread_ptr (fh); /* useless ptr; FRAME melee_icon */
	}
	
	// Load the stuff after the BRIEF_SHIP_INFO array
	cread_32s (fh, &EncounterPtr->log_x);
	cread_32s (fh, &EncounterPtr->log_y);
	
	// JMS: Let's make savegames work even between different resolution modes.
	EncounterPtr->log_x <<= RESOLUTION_FACTOR;
	EncounterPtr->log_y <<= RESOLUTION_FACTOR;
}

static void
LoadEvent (EVENT *EventPtr, DECODE_REF fh)
{
	cread_ptr (fh); /* useless ptr; HEVENT pred */
	EventPtr->pred = 0;
	cread_ptr (fh); /* useless ptr; HEVENT succ */
	EventPtr->succ = 0;
	cread_8   (fh, &EventPtr->day_index);
	cread_8   (fh, &EventPtr->month_index);
	cread_16  (fh, &EventPtr->year_index);
	cread_8   (fh, &EventPtr->func_index);
	cread_8   (fh, NULL); /* padding */
	cread_16  (fh, NULL); /* padding */
}

static void
DummyLoadQueue (QUEUE *QueuePtr, DECODE_REF fh)
{
	/* QUEUE should never actually be loaded since it contains
	 * purely internal representation and the lists
	 * involved are actually loaded separately */
	(void)QueuePtr; /* silence compiler */

	/* QUEUE format with QUEUE_TABLE defined -- UQM default */
	cread_ptr (fh); /* HLINK head */
	cread_ptr (fh); /* HLINK tail */
	cread_ptr (fh); /* BYTE* pq_tab */
	cread_ptr (fh); /* HLINK free_list */
	cread_16  (fh, NULL); /* MEM_HANDLE hq_tab */
	cread_16  (fh, NULL); /* COUNT object_size */
	cread_8   (fh, NULL); /* BYTE num_objects */
	
	cread_8   (fh, NULL); /* padding */
	cread_16  (fh, NULL); /* padding */
}

static void
LoadClockState (CLOCK_STATE *ClockPtr, DECODE_REF fh)
{
	cread_8   (fh, &ClockPtr->day_index);
	cread_8   (fh, &ClockPtr->month_index);
	cread_16  (fh, &ClockPtr->year_index);
	cread_16s (fh, &ClockPtr->tick_count);
	cread_16s (fh, &ClockPtr->day_in_ticks);
	cread_ptr (fh); /* not loading ptr; Semaphore clock_sem */
	cread_ptr (fh); /* not loading ptr; Task clock_task */
	cread_32  (fh, NULL); /* not loading; DWORD TimeCounter */

	DummyLoadQueue (&ClockPtr->event_q, fh);
}

static BOOLEAN
LoadGameState (GAME_STATE *GSPtr, DECODE_REF fh, BOOLEAN vanilla)
{
	BYTE dummy8;
	BYTE res_scale; // JMS

	cread_8   (fh, &dummy8); /* obsolete */
	cread_8   (fh, &GSPtr->glob_flags);
	cread_8   (fh, &GSPtr->CrewCost);
	cread_8   (fh, &GSPtr->FuelCost);
	
	// JMS: Now that we have read the fuelcost, we can compare it
	// to the correct value. Fuel cost is always FUEL_COST_RU, and if
	// the savefile tells otherwise, we have read it with the wrong method
	// (The savegame is from vanilla UQM and we've been reading it as if it
	// were UQM-HD save.)
	//
	// At this point we must then cease reading the savefile, close it
	// and re-open it again, this time using the vanilla-reading method.
	if (GSPtr->FuelCost != FUEL_COST_RU)
		return FALSE;
	
	cread_a8  (fh, GSPtr->ModuleCost, NUM_MODULES);
	cread_a8  (fh, GSPtr->ElementWorth, NUM_ELEMENT_CATEGORIES);
	cread_ptr (fh); /* not loading ptr; PRIMITIVE *DisplayArray */
	cread_16  (fh, &GSPtr->CurrentActivity);
	
	// JMS
	if (LOBYTE (GSPtr->CurrentActivity) != IN_INTERPLANETARY)
		res_scale = RESOLUTION_FACTOR;
	else
		res_scale = 0;
	
	cread_16  (fh, NULL); /* CLOCK_STATE alignment padding */
	LoadClockState (&GSPtr->GameClock, fh);

	cread_16s (fh, &GSPtr->autopilot.x);
	cread_16s (fh, &GSPtr->autopilot.y);
	cread_16s (fh, &GSPtr->ip_location.x);
	cread_16s (fh, &GSPtr->ip_location.y);
	/* STAMP ShipStamp */
	cread_16s (fh, &GSPtr->ShipStamp.origin.x);
	cread_16s (fh, &GSPtr->ShipStamp.origin.y);
	cread_16  (fh, &GSPtr->ShipFacing);
	cread_8   (fh, &GSPtr->ip_planet);
	cread_8   (fh, &GSPtr->in_orbit);
	
	GSPtr->ShipStamp.origin.x <<= RESOLUTION_FACTOR; // JMS: Let's make savegames work even between different resolution modes.
	GSPtr->ShipStamp.origin.y <<= RESOLUTION_FACTOR; // JMS: Let's make savegames work even between different resolution modes.

	/* VELOCITY_DESC velocity */
	cread_16  (fh, &GSPtr->velocity.TravelAngle);
	cread_16s (fh, &GSPtr->velocity.vector.width);
	cread_16s (fh, &GSPtr->velocity.vector.height);
	cread_16s (fh, &GSPtr->velocity.fract.width);
	cread_16s (fh, &GSPtr->velocity.fract.height);
	cread_16s (fh, &GSPtr->velocity.error.width);
	cread_16s (fh, &GSPtr->velocity.error.height);
	cread_16s (fh, &GSPtr->velocity.incr.width);
	cread_16s (fh, &GSPtr->velocity.incr.height);
	cread_16  (fh, NULL); /* VELOCITY_DESC padding */
	
	GSPtr->velocity.vector.width  <<= res_scale; // JMS: Let's make savegames work even between different resolution modes.
	GSPtr->velocity.vector.height <<= res_scale; // JMS: Let's make savegames work even between different resolution modes.
	GSPtr->velocity.fract.width	  <<= res_scale; // JMS: Let's make savegames work even between different resolution modes.
	GSPtr->velocity.fract.height  <<= res_scale; // JMS: Let's make savegames work even between different resolution modes.
	GSPtr->velocity.error.width	  <<= res_scale; // JMS: Let's make savegames work even between different resolution modes.
	GSPtr->velocity.error.height  <<= res_scale; // JMS: Let's make savegames work even between different resolution modes.
	GSPtr->velocity.incr.width	  <<= res_scale; // JMS: Let's make savegames work even between different resolution modes.
	GSPtr->velocity.incr.height	  <<= res_scale; // JMS: Let's make savegames work even between different resolution modes.

	cread_32  (fh, &GSPtr->BattleGroupRef);
	
	DummyLoadQueue (&GSPtr->avail_race_q, fh);
	DummyLoadQueue (&GSPtr->npc_built_ship_q, fh);
	// Not loading ip_group_q, was not there originally
	DummyLoadQueue (&GSPtr->encounter_q, fh);
	DummyLoadQueue (&GSPtr->built_ship_q, fh);

	// JMS: Let's not read the 'autopilot ok' and QS portal
	// coord bits for vanilla UQM saves.
	if (vanilla)
		cread_a8  (fh, GSPtr->GameState, (sizeof (GSPtr->GameState) - 2));
	else
		cread_a8  (fh, GSPtr->GameState, sizeof (GSPtr->GameState));

	//assert (sizeof (GSPtr->GameState) % 4 == 1);
	cread_8  (fh, NULL); /* GAME_STATE alignment padding */
	
	return TRUE;
}

static BOOLEAN
LoadSisState (SIS_STATE *SSPtr, void *fp)
{
	if (
			read_32s (fp, &SSPtr->log_x) != 1 ||
			read_32s (fp, &SSPtr->log_y) != 1 ||
			read_32  (fp, &SSPtr->ResUnits) != 1 ||
			read_32  (fp, &SSPtr->FuelOnBoard) != 1 ||
			read_16  (fp, &SSPtr->CrewEnlisted) != 1 ||
			read_16  (fp, &SSPtr->TotalElementMass) != 1 ||
			read_16  (fp, &SSPtr->TotalBioMass) != 1 ||
			read_a8  (fp, SSPtr->ModuleSlots, NUM_MODULE_SLOTS) != 1 ||
			read_a8  (fp, SSPtr->DriveSlots, NUM_DRIVE_SLOTS) != 1 ||
			read_a8  (fp, SSPtr->JetSlots, NUM_JET_SLOTS) != 1 ||
			read_8   (fp, &SSPtr->NumLanders) != 1 ||
			read_a16 (fp, SSPtr->ElementAmounts, NUM_ELEMENT_CATEGORIES) != 1 ||

			read_str (fp, SSPtr->ShipName, SIS_NAME_SIZE) != 1 ||
			read_str (fp, SSPtr->CommanderName, SIS_NAME_SIZE) != 1 ||
			read_str (fp, SSPtr->PlanetName, SIS_NAME_SIZE) != 1 ||

			read_16  (fp, NULL) != 1 /* padding */
		)
		return FALSE;
	else
	{
		// JMS: Let's make savegames work even between different resolution modes.
		SSPtr->log_x <<= RESOLUTION_FACTOR;
		SSPtr->log_y <<= RESOLUTION_FACTOR;
		return TRUE;
	}
}

void
LoadProblemMessage ()
{
#define MAX_MSG_LINES 2
	COUNT i;
	RECT r;
	TEXT t;
	UNICODE *ppStr[MAX_MSG_LINES];
	ppStr[0] = GAME_STRING (SAVEGAME_STRING_BASE + 7);
	ppStr[1] = GAME_STRING (SAVEGAME_STRING_BASE + 8);
	
	SetContextFont (StarConFont);
	
	t.baseline.x = t.baseline.y = 0;
	t.align = ALIGN_CENTER;
	
	for (i = 0; i < MAX_MSG_LINES; ++i)
	{
		RECT tr;

		t.pStr = ppStr[i];
		if (*t.pStr == '\0')
			break;
		t.CharCount = (COUNT)~0;
		TextRect (&t, &tr, NULL);
		if (i == 0)
			r = tr;
		else
			BoxUnion (&tr, &r, &r);
		t.baseline.y += (11 << RESOLUTION_FACTOR);
	}
		
	t.baseline.x = ((SIS_SCREEN_WIDTH >> 1) - (r.extent.width >> 1))
		- r.corner.x;
	t.baseline.y = ((SIS_SCREEN_HEIGHT >> 1) - (r.extent.height >> 1))
		- r.corner.y;
	
	r.corner.x += t.baseline.x - (4 << RESOLUTION_FACTOR);
	r.corner.y += t.baseline.y - (4 << RESOLUTION_FACTOR);
	r.extent.width += (8 << RESOLUTION_FACTOR);
	r.extent.height += (8 << RESOLUTION_FACTOR);
	
	BatchGraphics ();
	DrawStarConBox (&r, 2,
					BUILD_COLOR (MAKE_RGB15 (0x10, 0x10, 0x10), 0x19),
					BUILD_COLOR (MAKE_RGB15 (0x08, 0x08, 0x08), 0x1F),
					TRUE, BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x0A), 0x08));
	SetContextForeGroundColor (
		BUILD_COLOR (MAKE_RGB15 (0x14, 0x14, 0x14), 0x0F));
	
	for (i = 0; i < MAX_MSG_LINES; ++i)
	{
		t.pStr = ppStr[i];
		if (*t.pStr == '\0')
			break;
		t.CharCount = (COUNT)~0;
		font_DrawText (&t);
		t.baseline.y += (11 << RESOLUTION_FACTOR);
	}
	
	UnbatchGraphics ();
}

static BOOLEAN
LoadSummary (SUMMARY_DESC *SummPtr, void *fp, BOOLEAN try_vanilla)
{
	// JMS: New variables required for compatibility between
	// old, unnamed saves and the new, named ones.
	SDWORD  temp_log_x = 0;
	SDWORD  temp_log_y = 0;
	DWORD   temp_ru    = 0;
	DWORD   temp_fuel  = 0;
	BOOLEAN no_savename = FALSE;
	
	// First we check if there is a savegamename identifier.
	// The identifier tells us whether the name exists at all.
	read_str (fp, SummPtr->SaveNameChecker, SAVE_CHECKER_SIZE);
		
	// If the name doesn't exist (because this most probably
	// is a savegame from an older version), we have to rewind the
	// savefile to be able to read the saved variables into their
	// correct places.
	if (strncmp(SummPtr->SaveNameChecker, SAVE_NAME_CHECKER, SAVE_CHECKER_SIZE))
	{
		COUNT i;
			
		// Apparently the bytes read to SummPtr->SaveNameChecker with
		// read_str are destroyed from fp, so we must copy these bytes
		// to temp variables at this point to preserve them.
		no_savename = TRUE;
		memcpy(&temp_log_x, SummPtr->SaveNameChecker, sizeof(SDWORD));
		memcpy(&temp_log_y, &(SummPtr->SaveNameChecker[sizeof(SDWORD)]), sizeof(SDWORD));
		memcpy(&temp_ru, &(SummPtr->SaveNameChecker[2 * sizeof(SDWORD)]), sizeof(DWORD));
		memcpy(&temp_fuel, &(SummPtr->SaveNameChecker[2 * sizeof(SDWORD)+ sizeof(DWORD)]), sizeof(DWORD));
			
		// Rewind the position in savefile.
		for (i = 0; i < SAVE_CHECKER_SIZE; i++)
			uio_backtrack (1, (uio_Stream *) fp);
			
		// Zero the bogus savenamechecker.
		for (i = 0; i < SAVE_CHECKER_SIZE; i++)
			SummPtr->SaveNameChecker[i] = 0;
			
		// Make sure the save's name is empty.
		for (i = 0; i < SAVE_NAME_SIZE; i++)
			SummPtr->SaveName[i] = 0;
		}
	else
	{
		// If the name identifier exists, let's also read
		// the savegame's actual name, which is situated right
		// after the identifier.
		read_str (fp, SummPtr->SaveName, SAVE_NAME_SIZE);
	}
		
	//log_add (log_Debug, "fp: %d Check:%s Name:%s", fp, SummPtr->SaveNameChecker, SummPtr->SaveName);
	
	if (!LoadSisState (&SummPtr->SS, fp))
		return FALSE;
		
	// JMS: Now we'll put those temp variables into action.
	if (no_savename)
	{
		SummPtr->SS.log_x = temp_log_x;
		SummPtr->SS.log_y = temp_log_y;
		SummPtr->SS.ResUnits = temp_ru;
		SummPtr->SS.FuelOnBoard = temp_fuel;
	}
	
	if (
			read_8  (fp, &SummPtr->Activity) != 1 ||
			read_8  (fp, &SummPtr->Flags) != 1 ||
			read_8  (fp, &SummPtr->day_index) != 1 ||
			read_8  (fp, &SummPtr->month_index) != 1 ||
			read_16 (fp, &SummPtr->year_index) != 1 ||
			read_8  (fp, &SummPtr->MCreditLo) != 1 ||
			read_8  (fp, &SummPtr->MCreditHi) != 1 ||
			read_8  (fp, &SummPtr->NumShips) != 1 ||
			read_8  (fp, &SummPtr->NumDevices) != 1 ||
			read_a8 (fp, SummPtr->ShipList, MAX_BUILT_SHIPS) != 1 ||
			read_a8 (fp, SummPtr->DeviceList, MAX_EXCLUSIVE_DEVICES) != 1 ||
			read_8  (fp, &SummPtr->res_factor) != 1 || // JMS: This'll help making saves between different resolutions compatible.
		
			read_8  (fp, NULL) != 1 /* padding */
		)
		return FALSE;
	else
	{
		// JMS: UQM-HD saves have an extra piece of padding to compensate for the
		// added res_factor in SummPtr.
		if (!try_vanilla)
			read_8 (fp, NULL); /* padding */
	
		return TRUE;
	}
}

static void
LoadStarDesc (STAR_DESC *SDPtr, DECODE_REF fh)
{
	cread_16s(fh, &SDPtr->star_pt.x);
	cread_16s(fh, &SDPtr->star_pt.y);
	cread_8  (fh, &SDPtr->Type);
	cread_8  (fh, &SDPtr->Index);
	cread_8  (fh, &SDPtr->Prefix);
	cread_8  (fh, &SDPtr->Postfix);
}

BOOLEAN
LoadGame (COUNT which_game, SUMMARY_DESC *SummPtr, BOOLEAN try_vanilla)
{
	uio_Stream *in_fp;
	char file[PATH_MAX];
	char buf[256];
	SUMMARY_DESC loc_sd;
	GAME_STATE_FILE *fp;
	DECODE_REF fh;
	COUNT num_links;
	STAR_DESC SD;
	ACTIVITY Activity;

	sprintf (file, "starcon2.%02u", which_game);
	in_fp = res_OpenResFile (saveDir, file, "rb");
	if (!in_fp)
		return FALSE;

	if (!LoadSummary (&loc_sd, in_fp, try_vanilla))
	{
		log_add (log_Error, "Warning: Savegame is corrupt");
		res_CloseResFile (in_fp);
		return FALSE;
	}

	if (!SummPtr)
	{
		SummPtr = &loc_sd;
	}
	else
	{	// only need summary for displaying to user
		memcpy (SummPtr, &loc_sd, sizeof (*SummPtr));
		res_CloseResFile (in_fp);
		return TRUE;
	}

	// Crude check for big-endian/little-endian incompatibilities.
	// year_index is suitable as it's a multi-byte value within
	// a specific recognisable range.
	if (SummPtr->year_index < START_YEAR ||
			SummPtr->year_index >= START_YEAR +
			YEARS_TO_KOHRAH_VICTORY + 1 /* Utwig intervention */ +
			1 /* time to destroy all races, plenty */ +
			25 /* for cheaters */)
	{
		log_add (log_Error, "Warning: Savegame corrupt or from "
				"an incompatible platform.");
		res_CloseResFile (in_fp);
		return FALSE;
	}

	GlobData.SIS_state = SummPtr->SS;

	if ((fh = copen (in_fp, FILE_STREAM, STREAM_READ)) == 0)
	{
		res_CloseResFile (in_fp);
		return FALSE;
	}

	ReinitQueue (&GLOBAL (GameClock.event_q));
	ReinitQueue (&GLOBAL (encounter_q));
	ReinitQueue (&GLOBAL (ip_group_q));
	ReinitQueue (&GLOBAL (npc_built_ship_q));
	ReinitQueue (&GLOBAL (built_ship_q));

	memset (&GLOBAL (GameState[0]), 0, sizeof (GLOBAL (GameState)));
	Activity = GLOBAL (CurrentActivity);
	
	// JMS: We can decide whether the current savefile is vanilla UQM or UQM-HD
	// only at this point, when reading the game states. If this turns out to be a 
	// vanilla UQM save, we must close the file and re-open it for reading
	// with the vanilla method.
	if (!(LoadGameState (&GlobData.Game_state, fh, try_vanilla)))
	{
		res_CloseResFile (in_fp);
		
		if (!try_vanilla)
		{
			LoadGame (which_game, NULL, TRUE);
			return TRUE;
		}
		else
			return FALSE;
	}
	
	NextActivity = GLOBAL (CurrentActivity);
	GLOBAL (CurrentActivity) = Activity;

	LoadRaceQueue (fh, &GLOBAL (avail_race_q));
	// START_INTERPLANETARY is only set when saving from Homeworld
	//   encounter screen. When the game is loaded, the
	//   GenerateOrbitalFunction for the current star system will
	//   create the encounter anew and populate the npc queue.
	if (!(NextActivity & START_INTERPLANETARY))
	{
		if (NextActivity & START_ENCOUNTER)
			LoadShipQueue (fh, &GLOBAL (npc_built_ship_q));
		else if (LOBYTE (NextActivity) == IN_INTERPLANETARY)
			// XXX: Technically, this queue does not need to be
			//   saved/loaded at all. IP groups will be reloaded
			//   from group state files. But the original code did,
			//   and so will we until we can prove we do not need to.
			LoadGroupQueue (fh, &GLOBAL (ip_group_q));
		else
			// XXX: The empty queue read is only needed to maintain
			//   the savegame compatibility
			LoadEmptyQueue (fh);
	}
	LoadShipQueue (fh, &GLOBAL (built_ship_q));

	// Load the game events (compressed)
	cread_16 (fh, &num_links);
	{
#ifdef DEBUG_LOAD
		log_add (log_Debug, "EVENTS:");
#endif /* DEBUG_LOAD */
		while (num_links--)
		{
			HEVENT hEvent;
			EVENT *EventPtr;

			hEvent = AllocEvent ();
			LockEvent (hEvent, &EventPtr);

			LoadEvent (EventPtr, fh);

#ifdef DEBUG_LOAD
		log_add (log_Debug, "\t%u/%u/%u -- %u",
				EventPtr->month_index,
				EventPtr->day_index,
				EventPtr->year_index,
				EventPtr->func_index);
#endif /* DEBUG_LOAD */
			UnlockEvent (hEvent);
			PutEvent (hEvent);
		}
	}

	// Load the encounters (black globes in HS/QS (compressed))
	cread_16 (fh, &num_links);
	{
		while (num_links--)
		{
			HENCOUNTER hEncounter;
			ENCOUNTER *EncounterPtr;

			hEncounter = AllocEncounter ();
			LockEncounter (hEncounter, &EncounterPtr);

			LoadEncounter (EncounterPtr, fh);

			UnlockEncounter (hEncounter);
			PutEncounter (hEncounter);
		}
	}

	// Copy the star info file from the compressed stream
	fp = OpenStateFile (STARINFO_FILE, "wb");
	if (fp)
	{
		DWORD flen;

		cread_32 (fh, &flen);
		while (flen)
		{
			COUNT num_bytes;

			num_bytes = flen >= sizeof (buf) ? sizeof (buf) : (COUNT)flen;
			cread (buf, num_bytes, 1, fh);
			WriteStateFile (buf, num_bytes, 1, fp);

			flen -= num_bytes;
		}
		CloseStateFile (fp);
	}

	// Copy the defined groupinfo file from the compressed stream
	fp = OpenStateFile (DEFGRPINFO_FILE, "wb");
	if (fp)
	{
		DWORD flen;

		cread_32 (fh, &flen);
		while (flen)
		{
			COUNT num_bytes;

			num_bytes = flen >= sizeof (buf) ? sizeof (buf) : (COUNT)flen;
			cread (buf, num_bytes, 1, fh);
			WriteStateFile (buf, num_bytes, 1, fp);

			flen -= num_bytes;
		}
		CloseStateFile (fp);
	}

	// Copy the random groupinfo file from the compressed stream
	fp = OpenStateFile (RANDGRPINFO_FILE, "wb");
	if (fp)
	{
		DWORD flen;

		cread_32 (fh, &flen);
		while (flen)
		{
			COUNT num_bytes;

			num_bytes = flen >= sizeof (buf) ? sizeof (buf) : (COUNT)flen;
			cread (buf, num_bytes, 1, fh);
			WriteStateFile (buf, num_bytes, 1, fp);

			flen -= num_bytes;
		}
		CloseStateFile (fp);
	}

	LoadStarDesc (&SD, fh);	
	
	if(optInfiniteRU){
		oldRU = GlobData.SIS_state.ResUnits;
	} else {
		oldRU = 0;
	}			
	if (optUnlockShips){
		ActivateStarShip (ARILOU_SHIP, ALLOW_BUILD);
		ActivateStarShip (CHMMR_SHIP, ALLOW_BUILD);
		ActivateStarShip (ORZ_SHIP, ALLOW_BUILD);
		ActivateStarShip (PKUNK_SHIP, ALLOW_BUILD);
		ActivateStarShip (SHOFIXTI_SHIP, ALLOW_BUILD);
		ActivateStarShip (SPATHI_SHIP, ALLOW_BUILD);
		ActivateStarShip (SUPOX_SHIP, ALLOW_BUILD);
		ActivateStarShip (THRADDASH_SHIP, ALLOW_BUILD);
		ActivateStarShip (UTWIG_SHIP, ALLOW_BUILD);
		ActivateStarShip (VUX_SHIP, ALLOW_BUILD);
		ActivateStarShip (YEHAT_SHIP, ALLOW_BUILD);
		ActivateStarShip (MELNORME_SHIP, ALLOW_BUILD);
		ActivateStarShip (DRUUGE_SHIP, ALLOW_BUILD);
		ActivateStarShip (ILWRATH_SHIP, ALLOW_BUILD);
		ActivateStarShip (MYCON_SHIP, ALLOW_BUILD);
		ActivateStarShip (SLYLANDRO_SHIP, ALLOW_BUILD);
		ActivateStarShip (UMGAH_SHIP, ALLOW_BUILD);
		ActivateStarShip (URQUAN_SHIP, ALLOW_BUILD);
		ActivateStarShip (ZOQFOTPIK_SHIP, ALLOW_BUILD);
		ActivateStarShip (SYREEN_SHIP, ALLOW_BUILD);
		ActivateStarShip (BLACK_URQUAN_SHIP, ALLOW_BUILD);
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

	cclose (fh);
	res_CloseResFile (in_fp);

	EncounterGroup = 0;
	EncounterRace = -1;

	ReinitQueue (&race_q[0]);
	ReinitQueue (&race_q[1]);
	CurStarDescPtr = FindStar (NULL, &SD.star_pt, 0, 0);
	if (!(NextActivity & START_ENCOUNTER)
			&& LOBYTE (NextActivity) == IN_INTERPLANETARY)
		NextActivity |= START_INTERPLANETARY;

	return TRUE;
}


