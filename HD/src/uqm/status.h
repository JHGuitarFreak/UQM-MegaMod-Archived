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

// JMS_GFX 2012: Merged the resolution Factor stuff from P6014.

#ifndef UQM_STATUS_H_INCL_
#define UQM_STATUS_H_INCL_

#include "races.h"
#include "libs/compiler.h"

#define CREW_XOFFS RES_STAT_SCALE(4) // JMS_GFX
#define ENERGY_XOFFS (RES_STAT_SCALE(52) + (3 * RESOLUTION_FACTOR) + (RESOLUTION_FACTOR / 2)) // JMS_GFX
#define GAUGE_YOFFS (SHIP_INFO_HEIGHT - (10 << RESOLUTION_FACTOR) + 6 * RESOLUTION_FACTOR) // JMS_GFX
#define UNIT_WIDTH RES_STAT_SCALE(2)
#define UNIT_HEIGHT (1 << RESOLUTION_FACTOR)
#define STAT_WIDTH (1 + UNIT_WIDTH + 1 + UNIT_WIDTH + 1) // JMS_GFX

#define SHIP_INFO_HEIGHT (65 << RESOLUTION_FACTOR) // JMS_GFX
#define CAPTAIN_WIDTH RES_STAT_SCALE(55) // JMS_GFX
#define CAPTAIN_HEIGHT RES_STAT_SCALE(30) // JMS_GFX
#define CAPTAIN_XOFFS ((STATUS_WIDTH - CAPTAIN_WIDTH) >> 1) // JMS_GFX
#define CAPTAIN_YOFFS (SHIP_INFO_HEIGHT + (4 << RESOLUTION_FACTOR)) // JMS_GFX
#define SHIP_STATUS_HEIGHT (STATUS_HEIGHT >> 1)
#define BAD_GUY_YOFFS 0
#define GOOD_GUY_YOFFS SHIP_STATUS_HEIGHT
#define STARCON_TEXT_HEIGHT (7 << RESOLUTION_FACTOR) // JMS_GFX
#define TINY_TEXT_HEIGHT (9 << RESOLUTION_FACTOR) // JMS_GFX
#define BATTLE_CREW_X RES_STAT_SCALE(10) // JMS_GFX
#define BATTLE_CREW_Y ((64 - SAFE_Y) << RESOLUTION_FACTOR) // JMS_GFX

extern COORD status_y_offsets[];

extern void InitStatusOffsets (void);

extern void DrawCrewFuelString (COORD y, SIZE state);
extern void ClearShipStatus (COORD y, COORD w, BOOLEAN inMeleeMenu);
extern void OutlineShipStatus (COORD y, COORD w, BOOLEAN inMeleeMenu); // JMS: now is needed elsewhere
extern void InitShipStatus (SHIP_INFO *ShipInfoPtr, STARSHIP *StarShipPtr, RECT *pClipRect, BOOLEAN inMeleeMenu);
			// StarShipPtr or pClipRect can be NULL
extern void DeltaStatistics (SHIP_INFO *ShipInfoPtr, COORD y_offs,
		SIZE crew_delta, SIZE energy_delta);
extern void DrawBattleCrewAmount (SHIP_INFO *ShipInfoPtr, COORD y_offs);

extern void DrawCaptainsWindow (STARSHIP *StarShipPtr);
extern BOOLEAN DeltaEnergy (ELEMENT *ElementPtr, SIZE energy_delta);
extern BOOLEAN DeltaCrew (ELEMENT *ElementPtr, SIZE crew_delta);

extern void PreProcessStatus (ELEMENT *ShipPtr);
extern void PostProcessStatus (ELEMENT *ShipPtr);

#endif /* UQM_STATUS_H_INCL_ */
