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
#include "../../build.h"
#include "../../comm.h"
#include "../../globdata.h"
#include "../../nameref.h"
#include "../../state.h"
#include "libs/mathlib.h"

static bool GenerateZoqColony0_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateZoqColony0_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateZoqColony1_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static COUNT GenerateZoqColony0_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT whichNode);
static COUNT GenerateZoqColony1_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT whichNode);
static bool GenerateZoqColony0_pickupEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT whichNode);
static bool GenerateZoqColony1_pickupEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT whichNode);


const GenerateFunctions generateZoqFotPikColony0Functions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateZoqColony0_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateZoqColony0_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateZoqColony0_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateZoqColony0_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};

const GenerateFunctions generateZoqFotPikColony1Functions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateDefault_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateZoqColony1_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateZoqColony1_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateZoqColony1_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};

static bool
GenerateZoqColony0_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	solarSys->PlanetDesc[0].data_index = REDUX_WORLD;
	solarSys->PlanetDesc[0].NumPlanets = 0;
	solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 125L / 100;
	angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
			solarSys->PlanetDesc[0].location.y);
	solarSys->PlanetDesc[0].location.x =
			COSINE (angle, solarSys->PlanetDesc[0].radius);
	solarSys->PlanetDesc[0].location.y =
			SINE (angle, solarSys->PlanetDesc[0].radius);
	ComputeSpeed(&solarSys->PlanetDesc[0], FALSE, 1);

	return true;
}

static bool
GenerateZoqColony0_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->PlanetSideFrame[1] =
				CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
		
		if ((GET_GAME_STATE(MET_ZOQFOT)) == 1)
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (LoadStringTable (ZOQCOLONY_RUINS2_STRTAB));
		else
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (LoadStringTable (ZOQCOLONY_RUINS1_STRTAB));
	}

	GenerateDefault_generateOrbital (solarSys, world);
	return true;
}

static bool
GenerateZoqColony1_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, 1))
	{
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->PlanetSideFrame[1] =
			CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
		
		if ((GET_GAME_STATE(MET_ZOQFOT)) == 1)
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
			CaptureStringTable (LoadStringTable (ZOQCOLONY_RUINS2_STRTAB));
		else
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
			CaptureStringTable (LoadStringTable (ZOQCOLONY_RUINS1_STRTAB));
	}
	
	GenerateDefault_generateOrbital (solarSys, world);
	return true;
}

static COUNT
GenerateZoqColony0_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		return GenerateDefault_generateRuins (solarSys, whichNode);
	}

	return 0;
}

static COUNT
GenerateZoqColony1_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
								   COUNT whichNode)
{
	if (matchWorld (solarSys, world, 0, 1))
	{
		return GenerateDefault_generateRuins (solarSys, whichNode);
	}
	
	return 0;
}

static bool
GenerateZoqColony0_pickupEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		// Standard ruins report
		GenerateDefault_landerReportCycle (solarSys);
		return false;
	}

	(void) whichNode;
	return false;
}

static bool
GenerateZoqColony1_pickupEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
								 COUNT whichNode)
{
	if (matchWorld (solarSys, world, 0, 1))
	{
		// Standard ruins report
		GenerateDefault_landerReportCycle (solarSys);
		return false;
	}
	
	(void) whichNode;
	return false;
}
