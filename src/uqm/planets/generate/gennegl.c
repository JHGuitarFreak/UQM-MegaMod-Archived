/*
 *  Copyright (C) 2008  Nicolas Simonds <uqm@submedia.net>
 *
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
#include "../lander.h"
#include "../planets.h"
#include "../../comm.h"
#include "../../build.h"
#include "../../gamestr.h"
#include "../../gendef.h"
#include "../../nameref.h"
#include "../../sounds.h"
#include "../../starmap.h"

static bool GenerateDestroyedStarbase_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateDestroyedStarbase_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
static bool GenerateDestroyedStarbase_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);


const GenerateFunctions generateDestroyedStarbaseFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateDestroyedStarbase_generatePlanets,
	/* .generateMoons    = */ GenerateDestroyedStarbase_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateDestroyedStarbase_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateDefault_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};


static bool
GenerateDestroyedStarbase_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT p;

	solarSys->SunDesc[0].NumPlanets = (BYTE)~0;

	if (!PrimeSeed && EXTENDED)
		solarSys->SunDesc[0].NumPlanets = (RandomContext_Random(SysGenRNG) % (MAX_GEN_PLANETS - 1) + 1);

	FillOrbits (solarSys, solarSys->SunDesc[0].NumPlanets, solarSys->PlanetDesc, FALSE);
	GeneratePlanets (solarSys);	

	if (EXTENDED) {
		if (CurStarDescPtr->Index == DESTROYED_STARBASE_DEFINED) {
			solarSys->SunDesc[0].PlanetByte = 0;
			solarSys->SunDesc[0].MoonByte = 0;

			if (!PrimeSeed)
				solarSys->SunDesc[0].PlanetByte = (RandomContext_Random(SysGenRNG) % solarSys->SunDesc[0].NumPlanets);

			solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].data_index = PLANET_SHIELDED;
			solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].alternate_colormap = NULL;
			solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets = 1;

			if (!PrimeSeed) {
				solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].data_index = PLANET_SHIELDED;
				solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets = (RandomContext_Random(SysGenRNG) % (MAX_GEN_MOONS - 1) + 1);
				solarSys->SunDesc[0].MoonByte = (RandomContext_Random(SysGenRNG) % solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets);
			}
		} else {
			for (p = 0; p < solarSys->SunDesc[0].NumPlanets; p++) {
				if (solarSys->PlanetDesc[p].NumPlanets <= 1)
					break;
			}
			solarSys->SunDesc[0].PlanetByte = p;
			solarSys->SunDesc[0].MoonByte = 0;

			solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].alternate_colormap = NULL;
			solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets = 1;

			if (!PrimeSeed) {
				solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets = (RandomContext_Random(SysGenRNG) % (MAX_GEN_MOONS - 1) + 1);
				solarSys->SunDesc[0].MoonByte = (RandomContext_Random(SysGenRNG) % solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets);
			}
		}
	}

	return true;
}

static bool
GenerateDestroyedStarbase_generateMoons (SOLARSYS_STATE *solarSys, PLANET_DESC *planet)
{
	GenerateDefault_generateMoons (solarSys, planet);

	if (matchWorld (solarSys, planet, solarSys->SunDesc[0].PlanetByte, MATCH_PLANET) && EXTENDED)
	{
		solarSys->MoonDesc[solarSys->SunDesc[0].MoonByte].data_index = DESTROYED_STARBASE;
		solarSys->MoonDesc[solarSys->SunDesc[0].MoonByte].alternate_colormap = NULL;
	}

	return true;
}

static bool
GenerateDestroyedStarbase_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, solarSys->SunDesc[0].PlanetByte, solarSys->SunDesc[0].MoonByte) && EXTENDED)
	{
		/* Starbase */
		LoadStdLanderFont(&solarSys->SysInfo.PlanetInfo);


		if (CurStarDescPtr->Index == DESTROYED_STARBASE_DEFINED) {
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					CaptureStringTable (
						LoadStringTable (DESTROYED_BASE_STRTAB));

			// use alternate text if the player
			// hasn't freed the Earth starbase yet
			if (!GET_GAME_STATE(STARBASE_AVAILABLE))
				solarSys->SysInfo.PlanetInfo.DiscoveryString =
					SetRelStringTableIndex (
						solarSys->SysInfo.PlanetInfo.DiscoveryString, 1);
		} else {
			BYTE Index = CurStarDescPtr->Index == URQUAN_DEFINED ? 0 : 1;

			solarSys->SysInfo.PlanetInfo.DiscoveryString =
				SetRelStringTableIndex(
					CaptureStringTable(
						LoadStringTable(URQUAN_BASE_STRTAB)), Index);

		}

		DoDiscoveryReport(MenuSounds);

		DestroyStringTable(ReleaseStringTable(
			solarSys->SysInfo.PlanetInfo.DiscoveryString));
		solarSys->SysInfo.PlanetInfo.DiscoveryString = 0;
		FreeLanderFont(&solarSys->SysInfo.PlanetInfo);

		return true;
	}

	GenerateDefault_generateOrbital (solarSys, world);

	return true;
}

