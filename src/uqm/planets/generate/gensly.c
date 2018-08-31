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
#include "../../comm.h"


static bool GenerateSlylandro_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateSlylandro_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);


const GenerateFunctions generateSlylandroFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateSlylandro_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateSlylandro_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateDefault_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};


static bool
GenerateSlylandro_generatePlanets (SOLARSYS_STATE *solarSys)
{
	solarSys->SunDesc[0].NumPlanets = (BYTE)~0;
	solarSys->SunDesc[0].ExtraByte = 3;

	if(SeedA != PrimeA){
		solarSys->SunDesc[0].NumPlanets = (RandomContext_Random (SysGenRNG) % (9 - 1) + 1);
		solarSys->SunDesc[0].ExtraByte = (RandomContext_Random (SysGenRNG) % solarSys->SunDesc[0].NumPlanets);
	}

	FillOrbits (solarSys, solarSys->SunDesc[0].NumPlanets, solarSys->PlanetDesc, FALSE);
	GeneratePlanets (solarSys);

	solarSys->PlanetDesc[solarSys->SunDesc[0].ExtraByte].data_index = RED_GAS_GIANT;
	solarSys->PlanetDesc[solarSys->SunDesc[0].ExtraByte].alternate_colormap = NULL;
	solarSys->PlanetDesc[solarSys->SunDesc[0].ExtraByte].NumPlanets = 1;

	if(SeedA != PrimeA){
		solarSys->PlanetDesc[solarSys->SunDesc[0].ExtraByte].data_index = (RandomContext_Random (SysGenRNG) % (YEL_GAS_GIANT - BLU_GAS_GIANT) + BLU_GAS_GIANT);
		solarSys->PlanetDesc[solarSys->SunDesc[0].ExtraByte].NumPlanets = (RandomContext_Random (SysGenRNG) % 4);
	}

	return true;
}

static bool
GenerateSlylandro_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, solarSys->SunDesc[0].ExtraByte, MATCH_PLANET))
	{
		InitCommunication (SLYLANDRO_HOME_CONVERSATION);
		return true;
	}

	GenerateDefault_generateOrbital (solarSys, world);
	return true;
}

