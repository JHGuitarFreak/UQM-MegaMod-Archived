static void
DrawInnerPlanets (PLANET_DESC *planet)
{
	STAMP s;
	COUNT i;
	PLANET_DESC *moon;

	s.origin.x = SIS_SCREEN_WIDTH >> 1;
	s.origin.y = SIS_SCREEN_HEIGHT >> 1;

	if (TEXTURED_PLANETS)
	{
		// Draw the planet image
		DrawTexturedBody (planet, s);
		
		// Draw the moon images
		for (i = planet->NumPlanets, moon = pSolarSysState->MoonDesc;
		     i; --i, ++moon)
		{
			if (moon->data_index & WORLD_TYPE_SPECIAL)
				DrawStamp (&moon->image);
			else
				DrawTexturedBody(moon, moon->image);
		}
	}
	else
	{
		// Draw the planet image
		SetPlanetColorMap (planet);
		s.frame = planet->image.frame;

		i = planet->data_index & ~WORLD_TYPE_SPECIAL;
		if (i < NUMBER_OF_PLANET_TYPES
			&& (planet->data_index & PLANET_SHIELDED))
		{	// Shielded world looks "shielded" in inner view
			s.frame = SetAbsFrameIndex (SpaceJunkFrame, 17);
		}
		DrawStamp (&s);

		// Draw the moon images
		for (i = planet->NumPlanets, moon = pSolarSysState->MoonDesc;
			i; --i, ++moon)
		{
			if (!(moon->data_index & WORLD_TYPE_SPECIAL))
				SetPlanetColorMap (moon);
			DrawStamp (&moon->image);
		}
	}
}