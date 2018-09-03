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

#include "genall.h"
#include "../planets.h"
#include "../../gendef.h"
#include "../../starmap.h"
#include "../../globdata.h"
#include "libs/mathlib.h"


static bool GenerateRainbowWorld_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateRainbowWorld_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);


const GenerateFunctions generateRainbowWorldFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateRainbowWorld_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateRainbowWorld_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateDefault_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};


static bool
GenerateRainbowWorld_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	solarSys->SunDesc[0].NumPlanets = (BYTE)~0;
	solarSys->SunDesc[0].PlanetByte = 0;

	if(!PrimeSeed){
		solarSys->SunDesc[0].NumPlanets = (RandomContext_Random (SysGenRNG) % (9 - 1) + 1);
	}

	FillOrbits (solarSys, solarSys->SunDesc[0].NumPlanets, solarSys->PlanetDesc, FALSE);
	GeneratePlanets (solarSys);

	solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].data_index = RAINBOW_WORLD;
	solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].alternate_colormap = NULL;
	solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets = 0;
	solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].radius = EARTH_RADIUS * 50L / 100;
	angle = ARCTAN (solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].location.x,
			solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].location.y);
	if (angle <= QUADRANT)
		angle += QUADRANT;
	else if (angle >= FULL_CIRCLE - QUADRANT)
		angle -= QUADRANT;
	solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].location.x =
			COSINE (angle, solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].radius);
	solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].location.y =
			SINE (angle, solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].radius);
	ComputeSpeed(&solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte], FALSE, 1);

	if(!PrimeSeed){
		solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets = (RandomContext_Random (SysGenRNG) % 4);
	}

	return true;
}

static bool
GenerateRainbowWorld_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, solarSys->SunDesc[0].PlanetByte, MATCH_PLANET))
	{
		BYTE which_rainbow;
		UWORD rainbow_mask;
		STAR_DESC *SDPtr;

		rainbow_mask = MAKE_WORD (
				GET_GAME_STATE (RAINBOW_WORLD0),
				GET_GAME_STATE (RAINBOW_WORLD1));

		which_rainbow = 0;
		SDPtr = &star_array[0];
		while (SDPtr != CurStarDescPtr)
		{
			if (SDPtr->Index == RAINBOW_DEFINED)
				++which_rainbow;
			++SDPtr;
		}
		rainbow_mask |= 1 << which_rainbow;
		SET_GAME_STATE (RAINBOW_WORLD0, LOBYTE (rainbow_mask));
		SET_GAME_STATE (RAINBOW_WORLD1, HIBYTE (rainbow_mask));
	}

	GenerateDefault_generateOrbital (solarSys, world);
	return true;
}

