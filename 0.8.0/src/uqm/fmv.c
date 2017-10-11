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

#include "fmv.h"

#include "controls.h"
#include "hyper.h"
#include "options.h"
#include "master.h"
#include "resinst.h"
#include "nameref.h"
#include "settings.h"
#include "setup.h"
#include "libs/vidlib.h"
#include "libs/graphics/gfx_common.h"
#include "libs/inplib.h"

void
DoShipSpin (COUNT index, MUSIC_REF hMusic)
{
#ifdef WANT_SHIP_SPINS
	char vnbuf[32];
	RECT old_r;

	LoadIntoExtraScreen (NULL);
#if 0
	/* This is cut out right now but should be part of the 3DO side */
	SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND / 4));
	FlushColorXForms ();
#endif
	
	if (hMusic)
		StopMusic ();

	FreeHyperData ();

	// TODO: It would be nice to have better resource names for these.
	sprintf (vnbuf, "slides.spins.%02u", (unsigned)index);
	ShowPresentation (vnbuf);

	SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND / 4));
	FlushColorXForms ();

	GetContextClipRect (&old_r);
	SetContextClipRect (NULL);
	DrawFromExtraScreen (NULL);
	SetContextClipRect (&old_r);

	if (hMusic)
		PlayMusic (hMusic, TRUE, 1);
		
	SleepThreadUntil (FadeScreen (FadeAllToColor, ONE_SECOND / 4));
	FlushColorXForms ();
#else
	(void) index;  /* Satisfy compiler */
	(void) hMusic;  /* Satisfy compiler */
#endif  /* WANT_SHIP_SPINS */
}

void
SplashScreen (void (* DoProcessing)(DWORD TimeOut))
{
	STAMP s;
	DWORD TimeOut;

	SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND / 120));
	SetContext (ScreenContext);
	s.origin.x = s.origin.y = 0;
	s.frame = CaptureDrawable (LoadGraphic (TITLE_ANIM));
	DrawStamp (&s);
	DestroyDrawable (ReleaseDrawable (s.frame));

	TimeOut = FadeScreen (FadeAllToColor, ONE_SECOND / 2);

	if (DoProcessing)
		DoProcessing (TimeOut);
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
	{
		return;
	}
	
	/* There was a forcible setting of CHECK_ABORT here.  I cannot
	 * find any purpose for this that DoRestart doesn't handle
	 * better (forcing all other threads but this one to quit out,
	 * I believe), and have thus removed it.  It was interfering
	 * with the proper operation of the quit operation.
	 * --Michael */

	WaitForAnyButton (FALSE, ONE_SECOND * 3, TRUE);
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
	{
		return;
	}
	GLOBAL (CurrentActivity) &= ~CHECK_ABORT;

	SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND / 2));
}

void
Introduction (void)
{
	ShowPresentation (INTROPRES_STRTAB);
	SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND / 2));
}

void
Victory (void)
{
	SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND / 2));

	/* by default we do 3DO cinematics; or PC slides when 3DO files are
	 * not present */
	ShowPresentation (FINALPRES_STRTAB);
		
	FadeScreen (FadeAllToBlack, 0);
}



