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
#include "../lander.h"
#include "../planets.h"
#include "../../globdata.h"
#include "../../nameref.h"
#include "../../resinst.h"
#include "libs/mathlib.h"


static bool GenerateBurvixese_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateBurvixese_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
static bool GenerateBurvixese_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static COUNT GenerateBurvixese_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT whichNode);
static bool GenerateBurvixese_pickupEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT whichNode);


const GenerateFunctions generateBurvixeseFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateBurvixese_generatePlanets,
	/* .generateMoons    = */ GenerateBurvixese_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateBurvixese_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateBurvixese_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateBurvixese_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};


static bool
GenerateBurvixese_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	solarSys->PlanetDesc[0].data_index = REDUX_WORLD;
	solarSys->PlanetDesc[0].NumPlanets = 1;
	solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 39L / 100;
	angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
			solarSys->PlanetDesc[0].location.y);
	solarSys->PlanetDesc[0].location.x =
			COSINE (angle, solarSys->PlanetDesc[0].radius);
	solarSys->PlanetDesc[0].location.y =
			SINE (angle, solarSys->PlanetDesc[0].radius);
	return true;
}

static bool
GenerateBurvixese_generateMoons (SOLARSYS_STATE *solarSys, PLANET_DESC *planet)
{
	GenerateDefault_generateMoons (solarSys, planet);

	if (matchWorld (solarSys, planet, 0, MATCH_PLANET))
	{
		COUNT angle;
		DWORD rand_val;

		solarSys->MoonDesc[0].data_index = SELENIC_WORLD;
		solarSys->MoonDesc[0].radius = MIN_MOON_RADIUS
				+ (MAX_MOONS - 1) * MOON_DELTA;
		rand_val = TFB_Random ();
		angle = NORMALIZE_ANGLE (LOWORD (rand_val));
		solarSys->MoonDesc[0].location.x =
				COSINE (angle, solarSys->MoonDesc[0].radius);
		solarSys->MoonDesc[0].location.y =
				SINE (angle, solarSys->MoonDesc[0].radius);
	}
	return true;
}

static bool
GenerateBurvixese_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	COUNT i;
	DWORD rand_val;

	rand_val = DoPlanetaryAnalysis (&solarSys->SysInfo, world);

	solarSys->SysInfo.PlanetInfo.ScanSeed[BIOLOGICAL_SCAN] = rand_val;
	i = (COUNT)~0;
	rand_val = GenerateLifeForms (&solarSys->SysInfo, &i);

	solarSys->SysInfo.PlanetInfo.ScanSeed[MINERAL_SCAN] = rand_val;
	i = (COUNT)~0;
	GenerateMineralDeposits (&solarSys->SysInfo, &i);

	solarSys->SysInfo.PlanetInfo.ScanSeed[ENERGY_SCAN] = rand_val;

	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->PlanetSideFrame[1] =
				CaptureDrawable (
				LoadGraphic (RUINS_MASK_PMAP_ANIM));
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (
						LoadStringTable (BURV_RUINS_STRTAB));
		solarSys->SysInfo.PlanetInfo.Weather = 0;
		solarSys->SysInfo.PlanetInfo.Tectonics = 0;
	}
	else if (matchWorld (solarSys, world, 0, 0)
			&& !GET_GAME_STATE (BURVIXESE_BROADCASTERS))
	{
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->PlanetSideFrame[1] = CaptureDrawable (
				LoadGraphic (BURV_BCS_MASK_PMAP_ANIM));
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (LoadStringTable (BURV_BCS_STRTAB));
	}

	LoadPlanet (NULL);

	return true;
}

static COUNT
GenerateBurvixese_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		return GenerateDefault_generateRuins (solarSys, whichNode);
	}

	if (matchWorld (solarSys, world, 0, 0))
	{
		// This check is redundant since the retrieval bit will keep the
		// node from showing up again
		if (GET_GAME_STATE (BURVIXESE_BROADCASTERS))
		{	// already picked up
			return 0;
		}
		
		return GenerateDefault_generateArtifact (solarSys, whichNode);
	}

	return 0;
}

static bool
GenerateBurvixese_pickupEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		// Standard ruins report
		GenerateDefault_landerReportCycle (solarSys);
		return false;
	}

	if (matchWorld (solarSys, world, 0, 0))
	{
		assert (!GET_GAME_STATE (BURVIXESE_BROADCASTERS) && whichNode == 0);

		GenerateDefault_landerReport (solarSys);
		SetLanderTakeoff ();

		SET_GAME_STATE (BURVIXESE_BROADCASTERS, 1);
		SET_GAME_STATE (BURV_BROADCASTERS_ON_SHIP, 1);

		return true; // picked up
	}

	(void) whichNode;
	return false;
}
