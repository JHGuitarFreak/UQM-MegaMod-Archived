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

#ifndef SIS_H_INCL__
#define SIS_H_INCL__

#include "libs/compiler.h"
#include "libs/gfxlib.h"
#include "planets/elemdata.h"
		// for NUM_ELEMENT_CATEGORIES

#define CLEAR_SIS_RADAR (1 << 2)
#define DRAW_SIS_DISPLAY (1 << 3)

#define UNDEFINED_DELTA 0x7FFF

#define NUM_DRIVE_SLOTS 11
#define NUM_JET_SLOTS 8
#define NUM_MODULE_SLOTS 16

#define CREW_POD_CAPACITY 50
#define STORAGE_BAY_CAPACITY 500 /* km cubed */
#define FUEL_TANK_SCALE 100
#define FUEL_TANK_CAPACITY (50 * FUEL_TANK_SCALE)
#define HEFUEL_TANK_CAPACITY (100 * FUEL_TANK_SCALE)
#define MODULE_COST_SCALE 50

#define CREW_EXPENSE_THRESHOLD 1000

#define CREW_PER_ROW 5
#define SBAY_MASS_PER_ROW 50

#define MAX_FUEL_BARS 10
#define FUEL_VOLUME_PER_ROW (HEFUEL_TANK_CAPACITY / MAX_FUEL_BARS)
#define FUEL_RESERVE FUEL_VOLUME_PER_ROW

#define IP_SHIP_THRUST_INCREMENT 8
#define IP_SHIP_TURN_WAIT 17
#define IP_SHIP_TURN_DECREMENT 2

#define BIO_CREDIT_VALUE 2

enum
{
	PLANET_LANDER = 0,
		/* thruster types */
	FUSION_THRUSTER,
		/* jet types */
	TURNING_JETS,
		/* module types */
	CREW_POD,
	STORAGE_BAY,
	FUEL_TANK,
	HIGHEFF_FUELSYS,
	DYNAMO_UNIT,
	SHIVA_FURNACE,
	GUN_WEAPON,
	BLASTER_WEAPON,
	CANNON_WEAPON,
	TRACKING_SYSTEM,
	ANTIMISSILE_DEFENSE,
	
	NUM_PURCHASE_MODULES,

	BOMB_MODULE_0 = NUM_PURCHASE_MODULES,
	BOMB_MODULE_1,
	BOMB_MODULE_2,
	BOMB_MODULE_3,
	BOMB_MODULE_4,
	BOMB_MODULE_5,

	NUM_MODULES /* must be last entry */
};

#define EMPTY_SLOT NUM_MODULES
#define NUM_BOMB_MODULES 10

#define DRIVE_SIDE_X 31
#define DRIVE_SIDE_Y 56
#define DRIVE_TOP_X 33
#define DRIVE_TOP_Y (65 + 21)

#define JET_SIDE_X 71
#define JET_SIDE_Y 48
#define JET_TOP_X 70
#define JET_TOP_Y (73 + 21)

#define MODULE_SIDE_X 17
#define MODULE_SIDE_Y 14
#define MODULE_TOP_X 17
#define MODULE_TOP_Y (96 + 21)

#define SHIP_PIECE_OFFSET 12

#define MAX_BUILT_SHIPS 12
		/* Maximum number of ships escorting the SIS */
#define MAX_LANDERS 10

#define SUPPORT_SHIP_PTS \
	{3 +  0, 30 + (2 * 16)}, \
	{3 + 42, 30 + (2 * 16)}, \
	{3 +  0, 30 + (3 * 16)}, \
	{3 + 42, 30 + (3 * 16)}, \
	{3 +  0, 30 + (1 * 16)}, \
	{3 + 42, 30 + (1 * 16)}, \
	{3 +  0, 30 + (4 * 16)}, \
	{3 + 42, 30 + (4 * 16)}, \
	{3 +  0, 30 + (0 * 16)}, \
	{3 + 42, 30 + (0 * 16)}, \
	{3 +  0, 30 + (5 * 16)}, \
	{3 + 42, 30 + (5 * 16)},

#define SIS_NAME_SIZE 16

typedef struct
{
	SDWORD log_x, log_y;

	DWORD ResUnits;

	DWORD FuelOnBoard;
	COUNT CrewEnlisted;
			// Number of crew on board, not counting the captain.
			// Set to (COUNT) ~0 to indicate game over.
	COUNT TotalElementMass, TotalBioMass;

	BYTE ModuleSlots[NUM_MODULE_SLOTS];
	BYTE DriveSlots[NUM_DRIVE_SLOTS];
	BYTE JetSlots[NUM_JET_SLOTS];

	BYTE NumLanders;

	COUNT ElementAmounts[NUM_ELEMENT_CATEGORIES];

	UNICODE ShipName[SIS_NAME_SIZE];
	UNICODE CommanderName[SIS_NAME_SIZE];
	UNICODE PlanetName[SIS_NAME_SIZE];
} SIS_STATE;

// XXX: Theoretically, a player can have 17 devices on board without
//   cheating. We only provide
//   room for 16 below, which is not really a problem since this
//   is only used for displaying savegame summaries. There is also
//   room for only 16 devices on screen.
#define MAX_EXCLUSIVE_DEVICES 16

typedef struct
{
	SIS_STATE SS;
	BYTE Activity;
	BYTE Flags;
	BYTE day_index, month_index;
	COUNT year_index;
	BYTE MCreditLo, MCreditHi;
	BYTE NumShips, NumDevices;
	BYTE ShipList[MAX_BUILT_SHIPS];
	BYTE DeviceList[MAX_EXCLUSIVE_DEVICES];
} SUMMARY_DESC;

#define OVERRIDE_LANDER_FLAGS (1 << 7)
#define AFTER_BOMB_INSTALLED (1 << 7)

extern void RepairSISBorder (void);
extern void InitSISContexts (void);
extern void DrawSISFrame (void);
extern void ClearSISRect (BYTE ClearFlags);
extern void SetFlashRect (RECT *pRect);
#define SFR_MENU_3DO ((RECT*)~0L)
#define SFR_MENU_ANY ((RECT*)~1L)
extern void DrawHyperCoords (POINT puniverse);
extern void DrawSISTitle (UNICODE *pStr);

// Flags for DrawSISMessageEx (may be OR'ed):
#define DSME_NONE     0
#define DSME_SETFR    (1 << 0)
		// Set the flash rectangle to the message area.
#define DSME_CLEARFR  (1 << 1)
		// Disable the flash rectangle.
#define DSME_BLOCKCUR (1 << 2)
		// Use a block cursor instead of an insertion point cursor,
		// when editing in the message field.
#define DSME_MYCOLOR  (1 << 3)
		// Use the current foreground color, instead of the default.
extern BOOLEAN DrawSISMessageEx (const UNICODE *pStr, SIZE CurPos,
		SIZE ExPos, COUNT flags);

extern void DrawSISMessage (const UNICODE *pStr);
extern void DateToString (char *buf, size_t bufLen,
		BYTE month_index, BYTE day_index, COUNT year_index);

// Returned RECT is relative to the StatusContext
extern void GetStatusMessageRect (RECT *r);
extern void DrawStatusMessage (const UNICODE *pStr);
typedef enum
{
	SMM_UNDEFINED = 0,
	SMM_DATE,
	SMM_RES_UNITS,
	SMM_CREDITS,

	SMM_DEFAULT = SMM_DATE,
} StatMsgMode;
// Sets the new mode and return the previous
extern StatMsgMode SetStatusMessageMode (StatMsgMode);

extern void DrawLanders (void);
extern void DrawStorageBays (BOOLEAN Refresh);
extern void GetGaugeRect (RECT *pRect, BOOLEAN IsCrewRect);
extern void DrawFlagshipStats (void);
void DrawAutoPilotMessage (BOOLEAN Reset);

extern void DeltaSISGauges (SIZE crew_delta, SIZE fuel_delta, int
		resunit_delta);

extern COUNT GetCrewCount (void);
extern COUNT GetModuleCrewCapacity (BYTE moduleType);

extern COUNT GetCrewPodCapacity (void);
extern COUNT GetCPodCapacity (POINT *ppt);

extern COUNT GetModuleStorageCapacity (BYTE moduleType);
extern COUNT GetStorageBayCapacity (void);
extern COUNT GetSBayCapacity (POINT *ppt);

extern DWORD GetModuleFuelCapacity (BYTE moduleType);
extern DWORD GetFuelTankCapacity (void);
extern DWORD GetFTankCapacity (POINT *ppt);

extern COUNT CountSISPieces (BYTE piece_type);

extern void DrawFlagshipName (BOOLEAN InStatusArea);
extern void DrawCaptainsName (void);

#endif /* SIS_H_INCL__ */

