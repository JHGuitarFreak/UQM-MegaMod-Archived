static void
GenerateMoons (SOLARSYS_STATE *system, PLANET_DESC *planet)
{
	COUNT i;
	COUNT facing;
	PLANET_DESC *pMoonDesc;
	DWORD old_seed;

	old_seed = TFB_SeedRandom (planet->rand_seed);

	for (i = 0, pMoonDesc = &system->MoonDesc[0];
			i < MAX_MOONS; ++i, ++pMoonDesc)
	{
		pMoonDesc->pPrevDesc = planet;
		if (i >= planet->NumPlanets)
			continue;
		
		pMoonDesc->temp_color = planet->temp_color;
	}

	(*system->genFuncs->generateName) (system, planet);
	(*system->genFuncs->generateMoons) (system, planet);

	facing = NORMALIZE_FACING (ANGLE_TO_FACING (
			ARCTAN (planet->location.x, planet->location.y)));

	TFB_SeedRandom (old_seed);
}