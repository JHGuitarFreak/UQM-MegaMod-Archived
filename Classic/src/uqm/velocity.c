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

#include "velocity.h"

#include "units.h"
#include "libs/compiler.h"


#define VELOCITY_REMAINDER(v) ((v) & (VELOCITY_SCALE - 1))

void
GetCurrentVelocityComponents (VELOCITY_DESC *velocityptr, SIZE *pdx, SIZE *pdy)
{
	*pdx = WORLD_TO_VELOCITY (velocityptr->vector.width)
			+ (velocityptr->fract.width - (SIZE)HIBYTE (velocityptr->incr.width));
	*pdy = WORLD_TO_VELOCITY (velocityptr->vector.height)
			+ (velocityptr->fract.height - (SIZE)HIBYTE (velocityptr->incr.height));
}

void
GetNextVelocityComponents (VELOCITY_DESC *velocityptr, SIZE *pdx, SIZE *pdy,
		COUNT num_frames)
{
	COUNT e;

	e = (COUNT)((COUNT)velocityptr->error.width +
			((COUNT)velocityptr->fract.width * num_frames));
	*pdx = (velocityptr->vector.width * num_frames)
			+ ((SIZE)((SBYTE)LOBYTE (velocityptr->incr.width))
			* (e >> VELOCITY_SHIFT));
	velocityptr->error.width = VELOCITY_REMAINDER (e);

	e = (COUNT)((COUNT)velocityptr->error.height +
			((COUNT)velocityptr->fract.height * num_frames));
	*pdy = (velocityptr->vector.height * num_frames)
			+ ((SIZE)((SBYTE)LOBYTE (velocityptr->incr.height))
			* (e >> VELOCITY_SHIFT));
	velocityptr->error.height = VELOCITY_REMAINDER (e);
}

void
SetVelocityVector (VELOCITY_DESC *velocityptr, SIZE magnitude, COUNT facing)
{
	COUNT angle;
	SIZE dx, dy;

	angle = velocityptr->TravelAngle =
			FACING_TO_ANGLE (NORMALIZE_FACING (facing));
	magnitude = WORLD_TO_VELOCITY (magnitude);
	dx = COSINE (angle, magnitude);
	dy = SINE (angle, magnitude);
	if (dx >= 0)
	{
		velocityptr->vector.width = VELOCITY_TO_WORLD (dx);
		velocityptr->incr.width = MAKE_WORD ((BYTE)1, (BYTE)0);
	}
	else
	{
		dx = -dx;
		velocityptr->vector.width = -VELOCITY_TO_WORLD (dx);
		velocityptr->incr.width =
				MAKE_WORD ((BYTE)0xFF, (BYTE)(VELOCITY_REMAINDER (dx) << 1));
	}
	if (dy >= 0)
	{
		velocityptr->vector.height = VELOCITY_TO_WORLD (dy);
		velocityptr->incr.height = MAKE_WORD ((BYTE)1, (BYTE)0);
	}
	else
	{
		dy = -dy;
		velocityptr->vector.height = -VELOCITY_TO_WORLD (dy);
		velocityptr->incr.height =
				MAKE_WORD ((BYTE)0xFF, (BYTE)(VELOCITY_REMAINDER (dy) << 1));
	}

	velocityptr->fract.width = VELOCITY_REMAINDER (dx);
	velocityptr->fract.height = VELOCITY_REMAINDER (dy);
	velocityptr->error.width = velocityptr->error.height = 0;
}

void
SetVelocityComponents (VELOCITY_DESC *velocityptr, SIZE dx, SIZE dy)
{
	COUNT angle;

	if ((angle = ARCTAN (dx, dy)) == FULL_CIRCLE)
	{
		ZeroVelocityComponents (velocityptr);
	}
	else
	{
		if (dx >= 0)
		{
			velocityptr->vector.width = VELOCITY_TO_WORLD (dx);
			velocityptr->incr.width = MAKE_WORD ((BYTE)1, (BYTE)0);
		}
		else
		{
			dx = -dx;
			velocityptr->vector.width = -VELOCITY_TO_WORLD (dx);
			velocityptr->incr.width =
					MAKE_WORD ((BYTE)0xFF, (BYTE)(VELOCITY_REMAINDER (dx) << 1));
		}
		if (dy >= 0)
		{
			velocityptr->vector.height = VELOCITY_TO_WORLD (dy);
			velocityptr->incr.height = MAKE_WORD ((BYTE)1, (BYTE)0);
		}
		else
		{
			dy = -dy;
			velocityptr->vector.height = -VELOCITY_TO_WORLD (dy);
			velocityptr->incr.height =
					MAKE_WORD ((BYTE)0xFF, (BYTE)(VELOCITY_REMAINDER (dy) << 1));
		}

		velocityptr->fract.width = VELOCITY_REMAINDER (dx);
		velocityptr->fract.height = VELOCITY_REMAINDER (dy);
		velocityptr->error.width = velocityptr->error.height = 0;
	}

	velocityptr->TravelAngle = angle;
}

void
DeltaVelocityComponents (VELOCITY_DESC *velocityptr, SIZE dx, SIZE dy)
{

	dx += WORLD_TO_VELOCITY (velocityptr->vector.width)
			+ (velocityptr->fract.width - (SIZE)HIBYTE (velocityptr->incr.width));
	dy += WORLD_TO_VELOCITY (velocityptr->vector.height)
			+ (velocityptr->fract.height - (SIZE)HIBYTE (velocityptr->incr.height));

	SetVelocityComponents (velocityptr, dx, dy);
}

