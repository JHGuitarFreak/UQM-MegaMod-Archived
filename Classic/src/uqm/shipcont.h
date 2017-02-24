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

#ifndef _SHIPCONT_H
#define _SHIPCONT_H

#include "menustat.h"

#define FIELD_WIDTH (STATUS_WIDTH - 5)

extern void CargoMenu (void);
extern BOOLEAN RosterMenu (void);
extern BOOLEAN DevicesMenu (void);
extern BOOLEAN StarMap (void);

extern void DrawCargoStrings (BYTE OldElement, BYTE NewElement);
extern void ShowRemainingCapacity (void);

extern SIZE InventoryDevices (BYTE *pDeviceMap, COUNT Size);

#endif /* _SHIPCONT_H */

