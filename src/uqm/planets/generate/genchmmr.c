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
#include "../../build.h"
#include "../../comm.h"
#include "../../globdata.h"
#include "../../nameref.h"
#include "../../setup.h"
#include "../../sounds.h"
#include "../../state.h"
#include "libs/mathlib.h"

static bool GenerateChmmr_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateChmmr_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
static bool GenerateChmmr_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);


const GenerateFunctions generateChmmrFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateChmmr_generatePlanets,
	/* .generateMoons    = */ GenerateChmmr_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateChmmr_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateDefault_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};


static bool
GenerateChmmr_generatePlanets (SOLARSYS_STATE *solarSys)
{
	int jewelArray[] = { SAPPHIRE_WORLD, EMERALD_WORLD, RUBY_WORLD };
	solarSys->SunDesc[0].NumPlanets = (BYTE)~0;
	solarSys->SunDesc[0].PlanetByte = 1;
	solarSys->SunDesc[0].MoonByte = 0;

	if(SeedA != PrimeA){
		solarSys->SunDesc[0].NumPlanets = (RandomContext_Random (SysGenRNG) % (9 - 1) + 1);
		solarSys->SunDesc[0].PlanetByte = (RandomContext_Random (SysGenRNG) % solarSys->SunDesc[0].NumPlanets);
	}

	FillOrbits (solarSys, solarSys->SunDesc[0].NumPlanets, solarSys->PlanetDesc, FALSE);
	GeneratePlanets (solarSys);

	solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].data_index = SAPPHIRE_WORLD;
	solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets = 1;

	if(SeedA != PrimeA){
		solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].data_index = jewelArray[RandomContext_Random (SysGenRNG) % 2];
		solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets = (RandomContext_Random (SysGenRNG) % (4 - 1) + 1);
		solarSys->SunDesc[0].MoonByte = (RandomContext_Random (SysGenRNG) % solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].NumPlanets);
	}
	
	if (!GET_GAME_STATE (CHMMR_UNLEASHED))
		solarSys->PlanetDesc[solarSys->SunDesc[0].PlanetByte].data_index |= PLANET_SHIELDED;

	return true;
}

static bool
GenerateChmmr_generateMoons (SOLARSYS_STATE *solarSys, PLANET_DESC *planet)
{
	GenerateDefault_generateMoons (solarSys, planet);

	if (matchWorld (solarSys, planet, solarSys->SunDesc[0].PlanetByte, MATCH_PLANET))
	{
		COUNT angle;
		DWORD rand_val;

		solarSys->MoonDesc[solarSys->SunDesc[0].MoonByte].data_index = HIERARCHY_STARBASE;

		if(SeedA == PrimeA){
			solarSys->MoonDesc[solarSys->SunDesc[0].MoonByte].radius = MIN_MOON_RADIUS;
			rand_val = RandomContext_Random (SysGenRNG);
			angle = NORMALIZE_ANGLE (LOWORD (rand_val));
			solarSys->MoonDesc[solarSys->SunDesc[0].MoonByte].location.x =
					COSINE (angle, solarSys->MoonDesc[solarSys->SunDesc[0].MoonByte].radius);
			solarSys->MoonDesc[solarSys->SunDesc[0].MoonByte].location.y =
					SINE (angle, solarSys->MoonDesc[solarSys->SunDesc[0].MoonByte].radius);
			ComputeSpeed(&solarSys->MoonDesc[0], TRUE, 1);
		}
	}

	return true;
}

static bool
GenerateChmmr_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, solarSys->SunDesc[0].PlanetByte, MATCH_PLANET))
	{
		if (GET_GAME_STATE (CHMMR_UNLEASHED))
		{
			SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
			InitCommunication (CHMMR_CONVERSATION);

			if (GET_GAME_STATE (CHMMR_BOMB_STATE) == 2)
			{
				GLOBAL (CurrentActivity) |= END_INTERPLANETARY;
			}
		
			return true;
		}
		else if (GET_GAME_STATE (SUN_DEVICE_ON_SHIP)
				&& !GET_GAME_STATE (ILWRATH_DECEIVED)
				&& StartSphereTracking (ILWRATH_SHIP))
		{
			PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
			ReinitQueue (&GLOBAL (ip_group_q));
			assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

			CloneShipFragment (ILWRATH_SHIP,
					&GLOBAL (npc_built_ship_q), INFINITE_FLEET);

			SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 6);
			GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
			InitCommunication (ILWRATH_CONVERSATION);

			if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
			{
				GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
				ReinitQueue (&GLOBAL (npc_built_ship_q));
				GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);
			}

			return true;
		}
	}
	else if (matchWorld (solarSys, world, solarSys->SunDesc[0].PlanetByte, solarSys->SunDesc[0].MoonByte))
	{
		/* Starbase */
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (LoadStringTable (CHMMR_BASE_STRTAB));

		DoDiscoveryReport (MenuSounds);

		DestroyStringTable (ReleaseStringTable (
				solarSys->SysInfo.PlanetInfo.DiscoveryString));
		solarSys->SysInfo.PlanetInfo.DiscoveryString = 0;
		FreeLanderFont (&solarSys->SysInfo.PlanetInfo);

		return true;
	}

	GenerateDefault_generateOrbital (solarSys, world);

	return true;
}

