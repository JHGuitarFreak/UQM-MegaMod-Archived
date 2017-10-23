// Copyright Michael Martin, 2004.

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

#include "setupmenu.h"

#include "controls.h"
#include "options.h"
#include "setup.h"
#include "sounds.h"
#include "colors.h"
#include "libs/gfxlib.h"
#include "libs/graphics/gfx_common.h"
#include "libs/graphics/widgets.h"
#include "libs/graphics/tfb_draw.h"
#include "libs/strlib.h"
#include "libs/reslib.h"
#include "libs/inplib.h"
#include "libs/vidlib.h"
#include "libs/sound/sound.h"
#include "libs/resource/stringbank.h"
#include "libs/log.h"
#include "libs/memlib.h"
#include "resinst.h"
#include "nameref.h"
#include <math.h>


static STRING SetupTab;

typedef struct setup_menu_state {
	BOOLEAN (*InputFunc) (struct setup_menu_state *pInputState);

	BOOLEAN initialized;
	int anim_frame_count;
	DWORD NextTime;
} SETUP_MENU_STATE;

static BOOLEAN DoSetupMenu (SETUP_MENU_STATE *pInputState);
static BOOLEAN done;
static WIDGET *current, *next;

static int quit_main_menu (WIDGET *self, int event);
static int quit_sub_menu (WIDGET *self, int event);
static int do_graphics (WIDGET *self, int event);
static int do_audio (WIDGET *self, int event);
static int do_engine (WIDGET *self, int event);
static int do_resources (WIDGET *self, int event);
static int do_keyconfig (WIDGET *self, int event);
static int do_advanced (WIDGET *self, int event);
static int do_editkeys (WIDGET *self, int event);
static void change_template (WIDGET_CHOICE *self, int oldval);
static void rename_template (WIDGET_TEXTENTRY *self);
static void rebind_control (WIDGET_CONTROLENTRY *widget);
static void clear_control (WIDGET_CONTROLENTRY *widget);

#ifdef HAVE_OPENGL
#define RES_OPTS 4
#else
#define RES_OPTS 2
#endif

#define MENU_COUNT          8
#define CHOICE_COUNT       24
#define SLIDER_COUNT        4
#define BUTTON_COUNT       10
#define LABEL_COUNT         4
#define TEXTENTRY_COUNT     1
#define CONTROLENTRY_COUNT  7

/* The space for our widgets */
static WIDGET_MENU_SCREEN menus[MENU_COUNT];
static WIDGET_CHOICE choices[CHOICE_COUNT];
static WIDGET_SLIDER sliders[SLIDER_COUNT];
static WIDGET_BUTTON buttons[BUTTON_COUNT];
static WIDGET_LABEL labels[LABEL_COUNT];
static WIDGET_TEXTENTRY textentries[TEXTENTRY_COUNT];
static WIDGET_CONTROLENTRY controlentries[CONTROLENTRY_COUNT];

/* The hardcoded data that isn't strings */

typedef int (*HANDLER)(WIDGET *, int);

static int choice_widths[CHOICE_COUNT] = {
	3, 2, 3, 3, 2, 2, 2, 2, 2, 2, 
	2, 2, 3, 2, 2, 3, 3, 2,	3, 3, 
	3, 2, 2, 2 };

static HANDLER button_handlers[BUTTON_COUNT] = {
	quit_main_menu, quit_sub_menu, do_graphics, do_engine,
	do_audio, do_resources, do_keyconfig, do_advanced, do_editkeys, 
	do_keyconfig };

/* These refer to uninitialized widgets, but that's OK; we'll fill
 * them in before we touch them */
static WIDGET *main_widgets[] = {
	(WIDGET *)(&buttons[2]),
	(WIDGET *)(&buttons[3]),
	(WIDGET *)(&buttons[4]),
	(WIDGET *)(&buttons[5]),
	(WIDGET *)(&buttons[6]),
	(WIDGET *)(&buttons[7]),
	(WIDGET *)(&buttons[0]),
	NULL };

static WIDGET *graphics_widgets[] = {
	(WIDGET *)(&choices[0]),
	(WIDGET *)(&choices[23]),
	(WIDGET *)(&choices[10]),
	(WIDGET *)(&sliders[3]),
	(WIDGET *)(&choices[2]),
	(WIDGET *)(&choices[3]),
	(WIDGET *)(&buttons[1]),
	NULL };

static WIDGET *audio_widgets[] = {
	(WIDGET *)(&sliders[0]),
	(WIDGET *)(&sliders[1]),
	(WIDGET *)(&sliders[2]),
	(WIDGET *)(&choices[14]),
	(WIDGET *)(&choices[9]),
	(WIDGET *)(&choices[21]),
	(WIDGET *)(&choices[22]),
	(WIDGET *)(&buttons[1]),
	NULL };

static WIDGET *engine_widgets[] = {
	(WIDGET *)(&choices[4]),
	(WIDGET *)(&choices[5]),
	(WIDGET *)(&choices[6]),
	(WIDGET *)(&choices[7]),
	(WIDGET *)(&choices[8]),
	(WIDGET *)(&choices[13]),
	(WIDGET *)(&choices[11]),
	(WIDGET *)(&choices[17]),
	(WIDGET *)(&buttons[1]),
	NULL };

static WIDGET *advanced_widgets[] = {
#ifdef HAVE_OPENGL
	(WIDGET *)(&choices[1]),
#endif
	(WIDGET *)(&choices[12]),
	(WIDGET *)(&choices[15]),
	(WIDGET *)(&choices[16]),
	(WIDGET *)(&buttons[1]),
	NULL };
	
static WIDGET *keyconfig_widgets[] = {
	(WIDGET *)(&choices[18]),
	(WIDGET *)(&choices[19]),
	(WIDGET *)(&labels[1]),
	(WIDGET *)(&buttons[8]),
	(WIDGET *)(&buttons[1]),
	NULL };

static WIDGET *editkeys_widgets[] = {
	(WIDGET *)(&choices[20]),
	(WIDGET *)(&labels[2]),
	(WIDGET *)(&textentries[0]),
	(WIDGET *)(&controlentries[0]),
	(WIDGET *)(&controlentries[1]),
	(WIDGET *)(&controlentries[2]),
	(WIDGET *)(&controlentries[3]),
	(WIDGET *)(&controlentries[4]),
	(WIDGET *)(&controlentries[5]),
	(WIDGET *)(&controlentries[6]),
	(WIDGET *)(&buttons[9]),
	NULL };

static WIDGET *incomplete_widgets[] = {
	(WIDGET *)(&labels[0]),
	(WIDGET *)(&buttons[1]),
	NULL };

static const struct
{
	WIDGET **widgets;
	int bgIndex;
}
menu_defs[] =
{
	{main_widgets, 0},
	{graphics_widgets, 1},
	{audio_widgets, 1},
	{engine_widgets, 2},
	{incomplete_widgets, 3},
	{keyconfig_widgets, 1},
	{advanced_widgets, 2},
	{editkeys_widgets, 1},
	{NULL, 0}
};

// Start with reasonable gamma bounds. These will get updated
// as we find out the actual bounds.
static float minGamma = 0.4f;
static float maxGamma = 2.5f;
// The gamma slider uses an exponential curve
// We use y = e^(2.1972*(x-1)) curve to give us a nice spread of
// gamma values 0.11 < g < 9.0 centered at g=1.0
#define GAMMA_CURVE_B  2.1972f
static float minGammaX;
static float maxGammaX;


static int
quit_main_menu (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = NULL;
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static int
quit_sub_menu (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = (WIDGET *)(&menus[0]);
		(*next->receiveFocus) (next, WIDGET_EVENT_SELECT);
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static int
do_graphics (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = (WIDGET *)(&menus[1]);
		(*next->receiveFocus) (next, WIDGET_EVENT_DOWN);
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static int
do_audio (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = (WIDGET *)(&menus[2]);
		(*next->receiveFocus) (next, WIDGET_EVENT_DOWN);
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static int
do_engine (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = (WIDGET *)(&menus[3]);
		(*next->receiveFocus) (next, WIDGET_EVENT_DOWN);
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static int
do_resources (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = (WIDGET *)(&menus[4]);
		(*next->receiveFocus) (next, WIDGET_EVENT_DOWN);
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static int
do_keyconfig (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = (WIDGET *)(&menus[5]);
		(*next->receiveFocus) (next, WIDGET_EVENT_DOWN);
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static int
do_advanced (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = (WIDGET *)(&menus[6]);
		(*next->receiveFocus) (next, WIDGET_EVENT_DOWN);
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static void
populate_editkeys (int templat)
{
	int i, j;
	
	strncpy (textentries[0].value, input_templates[templat].name, textentries[0].maxlen);
	textentries[0].value[textentries[0].maxlen-1] = 0;
	
	for (i = 0; i < NUM_KEYS; i++)
	{
		for (j = 0; j < 2; j++)
		{
			InterrogateInputState (templat, i, j, controlentries[i].controlname[j], WIDGET_CONTROLENTRY_WIDTH);
		}
	}
}

static int
do_editkeys (WIDGET *self, int event)
{
	if (event == WIDGET_EVENT_SELECT)
	{
		next = (WIDGET *)(&menus[7]);
		/* Prepare the components */
		choices[20].selected = 0;
		
		populate_editkeys (0);
		(*next->receiveFocus) (next, WIDGET_EVENT_DOWN);
		return TRUE;
	}
	(void)self;
	return FALSE;
}

static void
change_template (WIDGET_CHOICE *self, int oldval)
{
	(void) oldval;
	populate_editkeys (self->selected);
}

static void
rename_template (WIDGET_TEXTENTRY *self)
{
	/* TODO: This will have to change if the size of the
	   input_templates name is changed.  It would probably be nice
	   to track this symbolically or ensure that self->value's
	   buffer is always at least this big; this will require some
	   reworking of widgets */
	strncpy (input_templates[choices[20].selected].name, self->value, 30);
	input_templates[choices[20].selected].name[29] = 0;
}

#define NUM_STEPS 20
#define X_STEP (SCREEN_WIDTH / NUM_STEPS)
#define Y_STEP (SCREEN_HEIGHT / NUM_STEPS)
#define MENU_FRAME_RATE (ONE_SECOND / 20)

static void
SetDefaults (void)
{
	GLOBALOPTS opts;
	
	GetGlobalOptions (&opts);
	if (opts.res == OPTVAL_CUSTOM)
	{
		choices[0].numopts = RES_OPTS + 1;
	}
	else
	{
		choices[0].numopts = RES_OPTS;
	}
	choices[0].selected = opts.res;
	choices[1].selected = opts.driver;
	choices[2].selected = opts.scaler;
	choices[3].selected = opts.scanlines;
	choices[4].selected = opts.menu;
	choices[5].selected = opts.text;
	choices[6].selected = opts.cscan;
	choices[7].selected = opts.scroll;
	choices[8].selected = opts.subtitles;
	choices[9].selected = opts.music3do;
	choices[10].selected = opts.fullscreen;
	choices[11].selected = opts.intro;
	choices[12].selected = opts.fps;
	choices[13].selected = opts.meleezoom;
	choices[14].selected = opts.stereo;
	choices[15].selected = opts.adriver;
	choices[16].selected = opts.aquality;
	choices[17].selected = opts.shield;
	choices[18].selected = opts.player1;
	choices[19].selected = opts.player2;
	choices[20].selected = 0;
	choices[21].selected = opts.musicremix;
	choices[22].selected = opts.speech;
	choices[23].selected = opts.keepaspect;

	sliders[0].value = opts.musicvol;
	sliders[1].value = opts.sfxvol;
	sliders[2].value = opts.speechvol;
	sliders[3].value = opts.gamma;
}

static void
PropagateResults (void)
{
	GLOBALOPTS opts;
	opts.res = choices[0].selected;
	opts.driver = choices[1].selected;
	opts.scaler = choices[2].selected;
	opts.scanlines = choices[3].selected;
	opts.menu = choices[4].selected;
	opts.text = choices[5].selected;
	opts.cscan = choices[6].selected;
	opts.scroll = choices[7].selected;
	opts.subtitles = choices[8].selected;
	opts.music3do = choices[9].selected;
	opts.fullscreen = choices[10].selected;
	opts.intro = choices[11].selected;
	opts.fps = choices[12].selected;
	opts.meleezoom = choices[13].selected;
	opts.stereo = choices[14].selected;
	opts.adriver = choices[15].selected;
	opts.aquality = choices[16].selected;
	opts.shield = choices[17].selected;
	opts.player1 = choices[18].selected;
	opts.player2 = choices[19].selected;
	opts.musicremix = choices[21].selected;
	opts.speech = choices[22].selected;
	opts.keepaspect = choices[23].selected;

	opts.musicvol = sliders[0].value;
	opts.sfxvol = sliders[1].value;
	opts.speechvol = sliders[2].value;
	opts.gamma = sliders[3].value;
	SetGlobalOptions (&opts);
}

static BOOLEAN
DoSetupMenu (SETUP_MENU_STATE *pInputState)
{
	/* Cancel any presses of the Pause key. */
	GamePaused = FALSE;

	if (!pInputState->initialized) 
	{
		SetDefaultMenuRepeatDelay ();
		pInputState->NextTime = GetTimeCounter ();
		SetDefaults ();
		Widget_SetFont (StarConFont);
		Widget_SetWindowColors (SHADOWBOX_BACKGROUND_COLOR,
				SHADOWBOX_DARK_COLOR, SHADOWBOX_MEDIUM_COLOR);

		current = NULL;
		next = (WIDGET *)(&menus[0]);
		(*next->receiveFocus) (next, WIDGET_EVENT_DOWN);
		
		pInputState->initialized = TRUE;
	}
	if (current != next)
	{
		SetTransitionSource (NULL);
	}
	
	BatchGraphics ();
	(*next->draw)(next, 0, 0);

	if (current != next)
	{
		ScreenTransition (3, NULL);
		current = next;
	}

	UnbatchGraphics ();

	if (PulsedInputState.menu[KEY_MENU_UP])
	{
		Widget_Event (WIDGET_EVENT_UP);
	}
	else if (PulsedInputState.menu[KEY_MENU_DOWN])
	{
		Widget_Event (WIDGET_EVENT_DOWN);
	}
	else if (PulsedInputState.menu[KEY_MENU_LEFT])
	{
		Widget_Event (WIDGET_EVENT_LEFT);
	}
	else if (PulsedInputState.menu[KEY_MENU_RIGHT])
	{
		Widget_Event (WIDGET_EVENT_RIGHT);
	}
	if (PulsedInputState.menu[KEY_MENU_SELECT])
	{
		Widget_Event (WIDGET_EVENT_SELECT);
	}
	if (PulsedInputState.menu[KEY_MENU_CANCEL])
	{
		Widget_Event (WIDGET_EVENT_CANCEL);
	}
	if (PulsedInputState.menu[KEY_MENU_DELETE])
	{
		Widget_Event (WIDGET_EVENT_DELETE);
	}

	SleepThreadUntil (pInputState->NextTime + MENU_FRAME_RATE);
	pInputState->NextTime = GetTimeCounter ();
	return !((GLOBAL (CurrentActivity) & CHECK_ABORT) || 
		 (next == NULL));
}

static void
redraw_menu (void)
{
	BatchGraphics ();
	(*next->draw)(next, 0, 0);
	UnbatchGraphics ();
}

static BOOLEAN
OnTextEntryChange (TEXTENTRY_STATE *pTES)
{
	WIDGET_TEXTENTRY *widget = (WIDGET_TEXTENTRY *) pTES->CbParam;

	widget->cursor_pos = pTES->CursorPos;
	if (pTES->JoystickMode)
		widget->state |= WTE_BLOCKCUR;
	else
		widget->state &= ~WTE_BLOCKCUR;
	
	// XXX TODO: Here, we can examine the text entered so far
	// to make sure it fits on the screen, for example,
	// and return FALSE to disallow the last change
	
	return TRUE; // allow change
}

static BOOLEAN
OnTextEntryFrame (TEXTENTRY_STATE *pTES)
{
	redraw_menu ();

	SleepThreadUntil (pTES->NextTime);
	pTES->NextTime = GetTimeCounter () + MENU_FRAME_RATE;

	return TRUE; // continue
}

static int
OnTextEntryEvent (WIDGET_TEXTENTRY *widget)
{	// Going to edit the text
	TEXTENTRY_STATE tes;
	UNICODE revert_buf[256];

	// position cursor at the end of text
	widget->cursor_pos = utf8StringCount (widget->value);
	widget->state = WTE_EDITING;
	redraw_menu ();

	// make a backup copy for revert on cancel
	utf8StringCopy (revert_buf, sizeof (revert_buf), widget->value);

	// text entry setup
	tes.Initialized = FALSE;
	tes.NextTime = GetTimeCounter () + MENU_FRAME_RATE;
	tes.BaseStr = widget->value;
	tes.MaxSize = widget->maxlen;
	tes.CursorPos = widget->cursor_pos;
	tes.CbParam = widget;
	tes.ChangeCallback = OnTextEntryChange;
	tes.FrameCallback = OnTextEntryFrame;

	SetMenuSounds (MENU_SOUND_NONE, MENU_SOUND_SELECT);
	if (!DoTextEntry (&tes))
	{	// editing failed (canceled) -- revert the changes
		utf8StringCopy (widget->value, widget->maxlen, revert_buf);
	}
	else
	{
		if (widget->onChange)
		{
			(*(widget->onChange))(widget);
		}
	}
	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);

	widget->state = WTE_NORMAL;
	redraw_menu ();

	return TRUE; // event handled
}

static inline float
gammaCurve (float x)
{
	// The slider uses an exponential curve
	return exp ((x - 1) * GAMMA_CURVE_B);
}

static inline float
solveGammaCurve (float y)
{
	return log (y) / GAMMA_CURVE_B + 1;
}

static int
gammaToSlider (float gamma)
{
	const float x = solveGammaCurve (gamma);
	const float step = (maxGammaX - minGammaX) / 100;
	return (int) ((x - minGammaX) / step + 0.5);
}

static float
sliderToGamma (int value)
{
	const float step = (maxGammaX - minGammaX) / 100;
	const float x = minGammaX + step * value;
	const float g = gammaCurve (x);
	// report any value that is close enough as 1.0
	return (fabs (g - 1.0f) < 0.001f) ? 1.0f : g;
}

static void
updateGammaBounds (bool useUpper)
{
	float g, x;
	int slider;
	
	// The slider uses an exponential curve.
	// Calculate where on the curve the min and max gamma values are
	minGammaX = solveGammaCurve (minGamma);
	maxGammaX = solveGammaCurve (maxGamma);

	// We have 100 discrete steps through the range, so the slider may
	// skip over a 1.0 gamma. We need to ensure that there always is
	// a 1.0 on the slider by tweaking the range (expanding/contracting).
	slider = gammaToSlider (1.0f);
	g = sliderToGamma (slider);
	if (g == 1.0f)
		return; // no adjustment needed

	x = solveGammaCurve (g);
	if (useUpper)
	{	// Move the upper bound up or down to land on 1.0
		const float d = (x - 1.0f) * 100 / slider;
		maxGammaX -= d;
		maxGamma = gammaCurve (maxGammaX);
	}
	else
	{	// Move the lower bound up or down to land on 1.0
		const float d = (x - 1.0f) * 100 / (100 - slider);
		minGammaX -= d;
		minGamma = gammaCurve (minGammaX);
	}
}

static int
gamma_HandleEventSlider (WIDGET *_self, int event)
{
	WIDGET_SLIDER *self = (WIDGET_SLIDER *)_self;
	int prevValue = self->value;
	float gamma;
	bool set;

	switch (event)
	{
	case WIDGET_EVENT_LEFT:
		self->value -= self->step;
		break;
	case WIDGET_EVENT_RIGHT:
		self->value += self->step;
		break;
	default:
		return FALSE;
	}

	// Limit the slider to values accepted by gfx subsys
	gamma = sliderToGamma (self->value);
	set = TFB_SetGamma (gamma);
	if (!set)
	{	// revert
		self->value = prevValue;
		gamma = sliderToGamma (self->value);
	}

	// Grow or shrink the range based on accepted values
	if (gamma < minGamma || (!set && event == WIDGET_EVENT_LEFT))
	{
		minGamma = gamma;
		updateGammaBounds (true);
		// at the lowest end
		self->value = 0;
	}
	else if (gamma > maxGamma || (!set && event == WIDGET_EVENT_RIGHT))
	{
		maxGamma = gamma;
		updateGammaBounds (false);
		// at the highest end
		self->value = 100;
	}
	return TRUE;
}

static void
gamma_DrawValue (WIDGET_SLIDER *self, int x, int y)
{
	TEXT t;
	char buf[16];
	float gamma = sliderToGamma (self->value);
	snprintf (buf, sizeof buf, "%.4f", gamma);

	t.baseline.x = x;
	t.baseline.y = y;
	t.align = ALIGN_CENTER;
	t.CharCount = ~0;
	t.pStr = buf;

	font_DrawText (&t);
}

static void
rebind_control (WIDGET_CONTROLENTRY *widget)
{
	int templat = choices[20].selected;
	int control = widget->controlindex;
	int index = widget->highlighted;

	FlushInput ();
	DrawLabelAsWindow (&labels[3], NULL);
	RebindInputState (templat, control, index);
	populate_editkeys (templat);
	FlushInput ();
}

static void
clear_control (WIDGET_CONTROLENTRY *widget)
{
	int templat = choices[20].selected;
	int control = widget->controlindex;
	int index = widget->highlighted;
      
	RemoveInputState (templat, control, index);
	populate_editkeys (templat);
}	

static int
count_widgets (WIDGET **widgets)
{
	int count;

	for (count = 0; *widgets != NULL; ++widgets, ++count)
		;
	return count;
}

static stringbank *bank = NULL;
static FRAME setup_frame = NULL;

static void
init_widgets (void)
{
	const char *buffer[100], *str, *title;
	int count, i, index;

	if (bank == NULL)
	{
		bank = StringBank_Create ();
	}
	
	if (setup_frame == NULL)
	{
		setup_frame = CaptureDrawable (LoadGraphic (MENUBKG_PMAP_ANIM));
	}

	count = GetStringTableCount (SetupTab);

	if (count < 3)
	{
		log_add (log_Fatal, "PANIC: Setup string table too short to even hold all indices!");
		exit (EXIT_FAILURE);
	}

	/* Menus */
	title = StringBank_AddOrFindString (bank, GetStringAddress (SetAbsStringTableIndex (SetupTab, 0)));
	if (SplitString (GetStringAddress (SetAbsStringTableIndex (SetupTab, 1)), '\n', 100, buffer, bank) != MENU_COUNT)
	{
		/* TODO: Ignore extras instead of dying. */
		log_add (log_Fatal, "PANIC: Incorrect number of Menu Subtitles");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < MENU_COUNT; i++)
	{
		menus[i].tag = WIDGET_TYPE_MENU_SCREEN;
		menus[i].parent = NULL;
		menus[i].handleEvent = Widget_HandleEventMenuScreen;
		menus[i].receiveFocus = Widget_ReceiveFocusMenuScreen;
		menus[i].draw = Widget_DrawMenuScreen;
		menus[i].height = Widget_HeightFullScreen;
		menus[i].width = Widget_WidthFullScreen;
		menus[i].title = title;
		menus[i].subtitle = buffer[i];
		menus[i].bgStamp.origin.x = 0;
		menus[i].bgStamp.origin.y = 0;
		menus[i].bgStamp.frame = SetAbsFrameIndex (setup_frame, menu_defs[i].bgIndex);
		menus[i].num_children = count_widgets (menu_defs[i].widgets);
		menus[i].child = menu_defs[i].widgets;
		menus[i].highlighted = 0;
	}
	if (menu_defs[i].widgets != NULL)
	{
		log_add (log_Error, "Menu definition array has more items!");
	}
		
	/* Options */
	if (SplitString (GetStringAddress (SetAbsStringTableIndex (SetupTab, 2)), '\n', 100, buffer, bank) != CHOICE_COUNT)
	{
		log_add (log_Fatal, "PANIC: Incorrect number of Choice Options");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < CHOICE_COUNT; i++)
	{
		choices[i].tag = WIDGET_TYPE_CHOICE;
		choices[i].parent = NULL;
		choices[i].handleEvent = Widget_HandleEventChoice;
		choices[i].receiveFocus = Widget_ReceiveFocusChoice;
		choices[i].draw = Widget_DrawChoice;
		choices[i].height = Widget_HeightChoice;
		choices[i].width = Widget_WidthFullScreen;
		choices[i].category = buffer[i];
		choices[i].numopts = 0;
		choices[i].options = NULL;
		choices[i].selected = 0;
		choices[i].highlighted = 0;
		choices[i].maxcolumns = choice_widths[i];
		choices[i].onChange = NULL;
	}

	/* Fill in the options now */
	index = 3;  /* Index into string table */
	for (i = 0; i < CHOICE_COUNT; i++)
	{
		int j, optcount;

		if (index >= count)
		{
			log_add (log_Fatal, "PANIC: String table cut short while reading choices");
			exit (EXIT_FAILURE);
		}
		str = GetStringAddress (SetAbsStringTableIndex (SetupTab, index++));
		optcount = SplitString (str, '\n', 100, buffer, bank);
		choices[i].numopts = optcount;
		choices[i].options = HMalloc (optcount * sizeof (CHOICE_OPTION));
		for (j = 0; j < optcount; j++)
		{
			choices[i].options[j].optname = buffer[j];
			choices[i].options[j].tooltip[0] = "";
			choices[i].options[j].tooltip[1] = "";
			choices[i].options[j].tooltip[2] = "";
		}
		for (j = 0; j < optcount; j++)
		{
			int k, tipcount;

			if (index >= count)
			{
				log_add (log_Fatal, "PANIC: String table cut short while reading choices");
				exit (EXIT_FAILURE);
			}
			str = GetStringAddress (SetAbsStringTableIndex (SetupTab, index++));
			tipcount = SplitString (str, '\n', 100, buffer, bank);			
			if (tipcount > 3)
			{
				tipcount = 3;
			}
			for (k = 0; k < tipcount; k++)
			{
				choices[i].options[j].tooltip[k] = buffer[k];
			}
		}
	}

	/* The first choice is resolution, and is handled specially */
	choices[0].numopts = RES_OPTS;

	/* Choices 18-20 are also special, being the names of the key configurations */
	for (i = 0; i < 6; i++)
	{
		choices[18].options[i].optname = input_templates[i].name;
		choices[19].options[i].optname = input_templates[i].name;
		choices[20].options[i].optname = input_templates[i].name;
	}

	/* Choice 20 has a special onChange handler, too. */
	choices[20].onChange = change_template;

	/* Sliders */
	if (index >= count)
	{
		log_add (log_Fatal, "PANIC: String table cut short while reading sliders");
		exit (EXIT_FAILURE);
	}

	if (SplitString (GetStringAddress (SetAbsStringTableIndex (SetupTab, index++)), '\n', 100, buffer, bank) != SLIDER_COUNT)
	{
		/* TODO: Ignore extras instead of dying. */
		log_add (log_Fatal, "PANIC: Incorrect number of Slider Options");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < SLIDER_COUNT; i++)
	{
		sliders[i].tag = WIDGET_TYPE_SLIDER;
		sliders[i].parent = NULL;
		sliders[i].handleEvent = Widget_HandleEventSlider;
		sliders[i].receiveFocus = Widget_ReceiveFocusSimple;
		sliders[i].draw = Widget_DrawSlider;
		sliders[i].height = Widget_HeightOneLine;
		sliders[i].width = Widget_WidthFullScreen;
		sliders[i].draw_value = Widget_Slider_DrawValue;
		sliders[i].min = 0;
		sliders[i].max = 100;
		sliders[i].step = 5;
		sliders[i].value = 75;
		sliders[i].category = buffer[i];
		sliders[i].tooltip[0] = "";
		sliders[i].tooltip[1] = "";
		sliders[i].tooltip[2] = "";
	}
	// gamma is a special case
	sliders[3].step = 1;
	sliders[3].handleEvent = gamma_HandleEventSlider;
	sliders[3].draw_value = gamma_DrawValue;

	for (i = 0; i < SLIDER_COUNT; i++)
	{
		int j, tipcount;
		
		if (index >= count)
		{
			log_add (log_Fatal, "PANIC: String table cut short while reading sliders");
			exit (EXIT_FAILURE);
		}
		str = GetStringAddress (SetAbsStringTableIndex (SetupTab, index++));
		tipcount = SplitString (str, '\n', 100, buffer, bank);
		if (tipcount > 3)
		{
			tipcount = 3;
		}
		for (j = 0; j < tipcount; j++)
		{
			sliders[i].tooltip[j] = buffer[j];
		}
	}

	/* Buttons */
	if (index >= count)
	{
		log_add (log_Fatal, "PANIC: String table cut short while reading buttons");
		exit (EXIT_FAILURE);
	}

	if (SplitString (GetStringAddress (SetAbsStringTableIndex (SetupTab, index++)), '\n', 100, buffer, bank) != BUTTON_COUNT)
	{
		/* TODO: Ignore extras instead of dying. */
		log_add (log_Fatal, "PANIC: Incorrect number of Button Options");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < BUTTON_COUNT; i++)
	{
		buttons[i].tag = WIDGET_TYPE_BUTTON;
		buttons[i].parent = NULL;
		buttons[i].handleEvent = button_handlers[i];
		buttons[i].receiveFocus = Widget_ReceiveFocusSimple;
		buttons[i].draw = Widget_DrawButton;
		buttons[i].height = Widget_HeightOneLine;
		buttons[i].width = Widget_WidthFullScreen;
		buttons[i].name = buffer[i];
		buttons[i].tooltip[0] = "";
		buttons[i].tooltip[1] = "";
		buttons[i].tooltip[2] = "";
	}

	for (i = 0; i < BUTTON_COUNT; i++)
	{
		int j, tipcount;
		
		if (index >= count)
		{
			log_add (log_Fatal, "PANIC: String table cut short while reading buttons");
			exit (EXIT_FAILURE);
		}
		str = GetStringAddress (SetAbsStringTableIndex (SetupTab, index++));
		tipcount = SplitString (str, '\n', 100, buffer, bank);
		if (tipcount > 3)
		{
			tipcount = 3;
		}
		for (j = 0; j < tipcount; j++)
		{
			buttons[i].tooltip[j] = buffer[j];
		}
	}

	/* Labels */
	if (index >= count)
	{
		log_add (log_Fatal, "PANIC: String table cut short while reading labels");
		exit (EXIT_FAILURE);
	}

	if (SplitString (GetStringAddress (SetAbsStringTableIndex (SetupTab, index++)), '\n', 100, buffer, bank) != LABEL_COUNT)
	{
		/* TODO: Ignore extras instead of dying. */
		log_add (log_Fatal, "PANIC: Incorrect number of Label Options");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < LABEL_COUNT; i++)
	{
		labels[i].tag = WIDGET_TYPE_LABEL;
		labels[i].parent = NULL;
		labels[i].handleEvent = Widget_HandleEventIgnoreAll;
		labels[i].receiveFocus = Widget_ReceiveFocusRefuseFocus;
		labels[i].draw = Widget_DrawLabel;
		labels[i].height = Widget_HeightLabel;
		labels[i].width = Widget_WidthFullScreen;
		labels[i].line_count = 0;
		labels[i].lines = NULL;
	}

	for (i = 0; i < LABEL_COUNT; i++)
	{
		int j, linecount;
		
		if (index >= count)
		{
			log_add (log_Fatal, "PANIC: String table cut short while reading labels");
			exit (EXIT_FAILURE);
		}
		str = GetStringAddress (SetAbsStringTableIndex (SetupTab, index++));
		linecount = SplitString (str, '\n', 100, buffer, bank);
		labels[i].line_count = linecount;
		labels[i].lines = (const char **)HMalloc(linecount * sizeof(const char *));
		for (j = 0; j < linecount; j++)
		{
			labels[i].lines[j] = buffer[j];
		}
	}

	/* Text Entry boxes */
	if (index >= count)
	{
		log_add (log_Fatal, "PANIC: String table cut short while reading text entries");
		exit (EXIT_FAILURE);
	}

	if (SplitString (GetStringAddress (SetAbsStringTableIndex (SetupTab, index++)), '\n', 100, buffer, bank) != TEXTENTRY_COUNT)
	{
		log_add (log_Fatal, "PANIC: Incorrect number of Text Entries");
		exit (EXIT_FAILURE);
	}
	for (i = 0; i < TEXTENTRY_COUNT; i++)
	{
		textentries[i].tag = WIDGET_TYPE_TEXTENTRY;
		textentries[i].parent = NULL;
		textentries[i].handleEvent = Widget_HandleEventTextEntry;
		textentries[i].receiveFocus = Widget_ReceiveFocusSimple;
		textentries[i].draw = Widget_DrawTextEntry;
		textentries[i].height = Widget_HeightOneLine;
		textentries[i].width = Widget_WidthFullScreen;
		textentries[i].handleEventSelect = OnTextEntryEvent;
		textentries[i].category = buffer[i];
		textentries[i].value[0] = 0;
		textentries[i].maxlen = WIDGET_TEXTENTRY_WIDTH-1;
		textentries[i].state = WTE_NORMAL;
		textentries[i].cursor_pos = 0;
	}

	if (index >= count)
	{
		log_add (log_Fatal, "PANIC: String table cut short while reading text entries");
		exit (EXIT_FAILURE);
	}

	if (SplitString (GetStringAddress (SetAbsStringTableIndex (SetupTab, index++)), '\n', 100, buffer, bank) != TEXTENTRY_COUNT)
	{
		/* TODO: Ignore extras instead of dying. */
		log_add (log_Fatal, "PANIC: Incorrect number of Text Entries");
		exit (EXIT_FAILURE);
	}
	for (i = 0; i < TEXTENTRY_COUNT; i++)
	{
		strncpy (textentries[i].value, buffer[i], textentries[i].maxlen);
		textentries[i].value[textentries[i].maxlen] = 0;
	}
	textentries[0].onChange = rename_template;

	/* Control Entry boxes */
	if (index >= count)
	{
		log_add (log_Fatal, "PANIC: String table cut short while reading control entries");
		exit (EXIT_FAILURE);
	}

	if (SplitString (GetStringAddress (SetAbsStringTableIndex (SetupTab, index++)), '\n', 100, buffer, bank) != CONTROLENTRY_COUNT)
	{
		log_add (log_Fatal, "PANIC: Incorrect number of Control Entries");
		exit (EXIT_FAILURE);
	}
	for (i = 0; i < CONTROLENTRY_COUNT; i++)
	{
		controlentries[i].tag = WIDGET_TYPE_CONTROLENTRY;
		controlentries[i].parent = NULL;
		controlentries[i].handleEvent = Widget_HandleEventControlEntry;
		controlentries[i].receiveFocus = Widget_ReceiveFocusControlEntry;
		controlentries[i].draw = Widget_DrawControlEntry;
		controlentries[i].height = Widget_HeightOneLine;
		controlentries[i].width = Widget_WidthFullScreen;
		controlentries[i].category = buffer[i];
		controlentries[i].highlighted = 0;
		controlentries[i].controlname[0][0] = 0;
		controlentries[i].controlname[1][0] = 0;
		controlentries[i].controlindex = i;
		controlentries[i].onChange = rebind_control;
		controlentries[i].onDelete = clear_control;
	}

	/* Check for garbage at the end */
	if (index < count)
	{
		log_add (log_Warning, "WARNING: Setup strings had %d garbage entries at the end.",
				count - index);
	}
}

static void
clean_up_widgets (void)
{
	int i;

	for (i = 0; i < CHOICE_COUNT; i++)
	{
		if (choices[i].options)
		{
			HFree (choices[i].options);
		}
	}

	for (i = 0; i < LABEL_COUNT; i++)
	{
		if (labels[i].lines)
		{
			HFree ((void *)labels[i].lines);
		}
	}

	/* Clear out the master tables */
	
	if (SetupTab)
	{
		DestroyStringTable (ReleaseStringTable (SetupTab));
		SetupTab = 0;
	}
	if (bank)
	{
		StringBank_Free (bank);
		bank = NULL;
	}
	if (setup_frame)
	{
		DestroyDrawable (ReleaseDrawable (setup_frame));
		setup_frame = NULL;
	}
}

void
SetupMenu (void)
{
	SETUP_MENU_STATE s;

	s.InputFunc = DoSetupMenu;
	s.initialized = FALSE;
	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
	SetupTab = CaptureStringTable (LoadStringTable (SETUP_MENU_STRTAB));
	if (SetupTab) 
	{
		init_widgets ();
	}
	else
	{
		log_add (log_Fatal, "PANIC: Could not find strings for the setup menu!");
		exit (EXIT_FAILURE);
	}
	done = FALSE;

	DoInput (&s, TRUE);
	GLOBAL (CurrentActivity) &= ~CHECK_ABORT;
	PropagateResults ();
	if (SetupTab)
	{
		clean_up_widgets ();
	}
}

void
GetGlobalOptions (GLOBALOPTS *opts)
{
	bool whichBound;

	if (GfxFlags & TFB_GFXFLAGS_SCALE_BILINEAR) 
	{
		opts->scaler = OPTVAL_BILINEAR_SCALE;
	}
	else if (GfxFlags & TFB_GFXFLAGS_SCALE_BIADAPT)
	{
		opts->scaler = OPTVAL_BIADAPT_SCALE;
	}
	else if (GfxFlags & TFB_GFXFLAGS_SCALE_BIADAPTADV) 
	{
		opts->scaler = OPTVAL_BIADV_SCALE;
	}
	else if (GfxFlags & TFB_GFXFLAGS_SCALE_TRISCAN) 
	{
		opts->scaler = OPTVAL_TRISCAN_SCALE;
	} 
	else if (GfxFlags & TFB_GFXFLAGS_SCALE_HQXX)
	{
		opts->scaler = OPTVAL_HQXX_SCALE;
	}
	else
	{
		opts->scaler = OPTVAL_NO_SCALE;
	}
	opts->fullscreen = (GfxFlags & TFB_GFXFLAGS_FULLSCREEN) ?
			OPTVAL_ENABLED : OPTVAL_DISABLED;
	opts->subtitles = optSubtitles ? OPTVAL_ENABLED : OPTVAL_DISABLED;
	opts->scanlines = (GfxFlags & TFB_GFXFLAGS_SCANLINES) ? 
		OPTVAL_ENABLED : OPTVAL_DISABLED;
	opts->menu = (optWhichMenu == OPT_3DO) ? OPTVAL_3DO : OPTVAL_PC;
	opts->text = (optWhichFonts == OPT_3DO) ? OPTVAL_3DO : OPTVAL_PC;
	opts->cscan = (optWhichCoarseScan == OPT_3DO) ? OPTVAL_3DO : OPTVAL_PC;
	opts->scroll = (optSmoothScroll == OPT_3DO) ? OPTVAL_3DO : OPTVAL_PC;
	opts->intro = (optWhichIntro == OPT_3DO) ? OPTVAL_3DO : OPTVAL_PC;
	opts->shield = (optWhichShield == OPT_3DO) ? OPTVAL_3DO : OPTVAL_PC;
	opts->fps = (GfxFlags & TFB_GFXFLAGS_SHOWFPS) ? 
			OPTVAL_ENABLED : OPTVAL_DISABLED;
	opts->meleezoom = (optMeleeScale == TFB_SCALE_STEP) ? 
			OPTVAL_PC : OPTVAL_3DO;
	opts->stereo = optStereoSFX ? OPTVAL_ENABLED : OPTVAL_DISABLED;
	/* These values are read in, but won't change during a run. */
	opts->music3do = opt3doMusic ? OPTVAL_ENABLED : OPTVAL_DISABLED;
	opts->musicremix = optRemixMusic ? OPTVAL_ENABLED : OPTVAL_DISABLED;
	opts->speech = optSpeech ? OPTVAL_ENABLED : OPTVAL_DISABLED;
	opts->keepaspect = optKeepAspectRatio ? OPTVAL_ENABLED : OPTVAL_DISABLED;
	switch (snddriver) {
	case audio_DRIVER_OPENAL:
		opts->adriver = OPTVAL_OPENAL;
		break;
	case audio_DRIVER_MIXSDL:
		opts->adriver = OPTVAL_MIXSDL;
		break;
	default:
		opts->adriver = OPTVAL_SILENCE;
		break;
	}
	if (soundflags & audio_QUALITY_HIGH)
	{
		opts->aquality = OPTVAL_HIGH;
	}
	else if (soundflags & audio_QUALITY_LOW)
	{
		opts->aquality = OPTVAL_LOW;
	}
	else
	{
		opts->aquality = OPTVAL_MEDIUM;
	}

	/* Work out resolution.  On the way, try to guess a good default
	 * for config.alwaysgl, then overwrite it if it was set previously. */
	opts->driver = OPTVAL_PURE_IF_POSSIBLE;
	switch (ScreenWidthActual)
	{
	case 320:
		if (GraphicsDriver == TFB_GFXDRIVER_SDL_PURE)
		{
			opts->res = OPTVAL_320_240;
		}
		else
		{
			if (ScreenHeightActual != 240)
			{
				opts->res = OPTVAL_CUSTOM;
			}
			else
			{
				opts->res = OPTVAL_320_240;
				opts->driver = OPTVAL_ALWAYS_GL;
			}
		}
		break;
	case 640:
		if (GraphicsDriver == TFB_GFXDRIVER_SDL_PURE)
		{
			opts->res = OPTVAL_640_480;
		}
		else
		{
			if (ScreenHeightActual != 480)
			{
				opts->res = OPTVAL_CUSTOM;
			}
			else
			{
				opts->res = OPTVAL_640_480;
				opts->driver = OPTVAL_ALWAYS_GL;
			}
		}
		break;
	case 800:
		if (ScreenHeightActual != 600)
		{
			opts->res = OPTVAL_CUSTOM;
		}
		else
		{
			opts->res = OPTVAL_800_600;
		}
		break;
	case 1024:
		if (ScreenHeightActual != 768)
		{
			opts->res = OPTVAL_CUSTOM;
		}
		else
		{
			opts->res = OPTVAL_1024_768;
		}		
		break;
	default:
		opts->res = OPTVAL_CUSTOM;
		break;
	}

	if (res_IsBoolean ("config.alwaysgl"))
	{
		if (res_GetBoolean ("config.alwaysgl"))
		{
			opts->driver = OPTVAL_ALWAYS_GL;
		}
		else
		{
			opts->driver = OPTVAL_PURE_IF_POSSIBLE;
		}
	}

	whichBound = (optGamma < maxGamma);
	// The option supplied by the user may be beyond our starting range
	// but valid nonetheless. We need to account for that.
	if (optGamma <= minGamma)
		minGamma = optGamma - 0.03f;
	else if (optGamma >= maxGamma)
		maxGamma = optGamma + 0.3f;
	updateGammaBounds (whichBound);
	opts->gamma = gammaToSlider (optGamma);

	opts->player1 = PlayerControls[0];
	opts->player2 = PlayerControls[1];

	opts->musicvol = (((int)(musicVolumeScale * 100.0f) + 2) / 5) * 5;
	opts->sfxvol = (((int)(sfxVolumeScale * 100.0f) + 2) / 5) * 5;
	opts->speechvol = (((int)(speechVolumeScale * 100.0f) + 2) / 5) * 5;
}

void
SetGlobalOptions (GLOBALOPTS *opts)
{
	int NewGfxFlags = GfxFlags;
	int NewWidth = ScreenWidthActual;
	int NewHeight = ScreenHeightActual;
	int NewDriver = GraphicsDriver;

	NewGfxFlags &= ~TFB_GFXFLAGS_SCALE_ANY;

	switch (opts->res) {
	case OPTVAL_320_240:
		NewWidth = 320;
		NewHeight = 240;
#ifdef HAVE_OPENGL	       
		NewDriver = (opts->driver == OPTVAL_ALWAYS_GL ? TFB_GFXDRIVER_SDL_OPENGL : TFB_GFXDRIVER_SDL_PURE);
#else
		NewDriver = TFB_GFXDRIVER_SDL_PURE;
#endif
		break;
	case OPTVAL_640_480:
		NewWidth = 640;
		NewHeight = 480;
#ifdef HAVE_OPENGL	       
		NewDriver = (opts->driver == OPTVAL_ALWAYS_GL ? TFB_GFXDRIVER_SDL_OPENGL : TFB_GFXDRIVER_SDL_PURE);
#else
		NewDriver = TFB_GFXDRIVER_SDL_PURE;
#endif
		break;
	case OPTVAL_800_600:
		NewWidth = 800;
		NewHeight = 600;
		NewDriver = TFB_GFXDRIVER_SDL_OPENGL;
		break;
	case OPTVAL_1024_768:
		NewWidth = 1024;
		NewHeight = 768;
		NewDriver = TFB_GFXDRIVER_SDL_OPENGL;
		break;
	default:
		/* Don't mess with the custom value */
		break;
	}

	res_PutInteger ("config.reswidth", NewWidth);
	res_PutInteger ("config.resheight", NewHeight);
	res_PutBoolean ("config.alwaysgl", opts->driver == OPTVAL_ALWAYS_GL);
	res_PutBoolean ("config.usegl", NewDriver == TFB_GFXDRIVER_SDL_OPENGL);

	switch (opts->scaler) {
	case OPTVAL_BILINEAR_SCALE:
		NewGfxFlags |= TFB_GFXFLAGS_SCALE_BILINEAR;
		res_PutString ("config.scaler", "bilinear");
		break;
	case OPTVAL_BIADAPT_SCALE:
		NewGfxFlags |= TFB_GFXFLAGS_SCALE_BIADAPT;
		res_PutString ("config.scaler", "biadapt");
		break;
	case OPTVAL_BIADV_SCALE:
		NewGfxFlags |= TFB_GFXFLAGS_SCALE_BIADAPTADV;
		res_PutString ("config.scaler", "biadv");
		break;
	case OPTVAL_TRISCAN_SCALE:
		NewGfxFlags |= TFB_GFXFLAGS_SCALE_TRISCAN;
		res_PutString ("config.scaler", "triscan");
		break;
	case OPTVAL_HQXX_SCALE:
		NewGfxFlags |= TFB_GFXFLAGS_SCALE_HQXX;
		res_PutString ("config.scaler", "hq");
		break;
	default:
		/* OPTVAL_NO_SCALE has no equivalent in gfxflags. */
		res_PutString ("config.scaler", "no");
		break;
	}
	if (opts->scanlines) {
		NewGfxFlags |= TFB_GFXFLAGS_SCANLINES;
	} else {
		NewGfxFlags &= ~TFB_GFXFLAGS_SCANLINES;
	}
	if (opts->fullscreen)
		NewGfxFlags |= TFB_GFXFLAGS_FULLSCREEN;
	else
		NewGfxFlags &= ~TFB_GFXFLAGS_FULLSCREEN;

	res_PutBoolean ("config.scanlines", opts->scanlines);
	res_PutBoolean ("config.fullscreen", opts->fullscreen);


	if ((NewWidth != ScreenWidthActual) ||
	    (NewHeight != ScreenHeightActual) ||
	    (NewDriver != GraphicsDriver) ||
	    (NewGfxFlags != GfxFlags)) 
	{
		FlushGraphics ();
		UninitVideoPlayer ();
		TFB_DrawScreen_ReinitVideo (NewDriver, NewGfxFlags, NewWidth, NewHeight);
		FlushGraphics ();
		InitVideoPlayer (TRUE);
	}

	// Avoid setting gamma when it is not necessary
	if (optGamma != 1.0f || sliderToGamma (opts->gamma) != 1.0f)
	{
		optGamma = sliderToGamma (opts->gamma);
		setGammaCorrection (optGamma);
	}

	optSubtitles = (opts->subtitles == OPTVAL_ENABLED) ? TRUE : FALSE;
	optWhichMenu = (opts->menu == OPTVAL_3DO) ? OPT_3DO : OPT_PC;
	optWhichFonts = (opts->text == OPTVAL_3DO) ? OPT_3DO : OPT_PC;
	optWhichCoarseScan = (opts->cscan == OPTVAL_3DO) ? OPT_3DO : OPT_PC;
	optSmoothScroll = (opts->scroll == OPTVAL_3DO) ? OPT_3DO : OPT_PC;
	optWhichShield = (opts->shield == OPTVAL_3DO) ? OPT_3DO : OPT_PC;
	optMeleeScale = (opts->meleezoom == OPTVAL_3DO) ? TFB_SCALE_TRILINEAR : TFB_SCALE_STEP;
	opt3doMusic = (opts->music3do == OPTVAL_ENABLED);
	optRemixMusic = (opts->musicremix == OPTVAL_ENABLED);
	optSpeech = (opts->speech == OPTVAL_ENABLED);
	optWhichIntro = (opts->intro == OPTVAL_3DO) ? OPT_3DO : OPT_PC;
	optStereoSFX = (opts->stereo == OPTVAL_ENABLED);
	optKeepAspectRatio = (opts->keepaspect == OPTVAL_ENABLED);
	PlayerControls[0] = opts->player1;
	PlayerControls[1] = opts->player2;

	res_PutBoolean ("config.subtitles", opts->subtitles == OPTVAL_ENABLED);
	res_PutBoolean ("config.textmenu", opts->menu == OPTVAL_PC);
	res_PutBoolean ("config.textgradients", opts->text == OPTVAL_PC);
	res_PutBoolean ("config.iconicscan", opts->cscan == OPTVAL_3DO);
	res_PutBoolean ("config.smoothscroll", opts->scroll == OPTVAL_3DO);

	res_PutBoolean ("config.3domusic", opts->music3do == OPTVAL_ENABLED);
	res_PutBoolean ("config.remixmusic", opts->musicremix == OPTVAL_ENABLED);
	res_PutBoolean ("config.speech", opts->speech == OPTVAL_ENABLED);
	res_PutBoolean ("config.3domovies", opts->intro == OPTVAL_3DO);
	res_PutBoolean ("config.showfps", opts->fps == OPTVAL_ENABLED);
	res_PutBoolean ("config.smoothmelee", opts->meleezoom == OPTVAL_3DO);
	res_PutBoolean ("config.positionalsfx", opts->stereo == OPTVAL_ENABLED); 
	res_PutBoolean ("config.pulseshield", opts->shield == OPTVAL_3DO);
	res_PutBoolean ("config.keepaspectratio", opts->keepaspect == OPTVAL_ENABLED);
	res_PutInteger ("config.gamma", (int) (optGamma * GAMMA_SCALE + 0.5));
	res_PutInteger ("config.player1control", opts->player1);
	res_PutInteger ("config.player2control", opts->player2);

	switch (opts->adriver) {
	case OPTVAL_SILENCE:
		res_PutString ("config.audiodriver", "none");
		break;
	case OPTVAL_MIXSDL:
		res_PutString ("config.audiodriver", "mixsdl");
		break;
	case OPTVAL_OPENAL:
		res_PutString ("config.audiodriver", "openal");
	default:
		/* Shouldn't happen; leave config untouched */
		break;
	}

	switch (opts->aquality) {
	case OPTVAL_LOW:
		res_PutString ("config.audioquality", "low");
		break;
	case OPTVAL_MEDIUM:
		res_PutString ("config.audioquality", "medium");
		break;
	case OPTVAL_HIGH:
		res_PutString ("config.audioquality", "high");
		break;
	default:
		/* Shouldn't happen; leave config untouched */
		break;
	}

	res_PutInteger ("config.musicvol", opts->musicvol);
	res_PutInteger ("config.sfxvol", opts->sfxvol);
	res_PutInteger ("config.speechvol", opts->speechvol);
	musicVolumeScale = opts->musicvol / 100.0f;
	sfxVolumeScale = opts->sfxvol / 100.0f;
	speechVolumeScale = opts->speechvol / 100.0f;
	// update actual volumes
	SetMusicVolume (musicVolume);
	SetSpeechVolume (speechVolumeScale);

	res_PutString ("keys.1.name", input_templates[0].name);
	res_PutString ("keys.2.name", input_templates[1].name);
	res_PutString ("keys.3.name", input_templates[2].name);
	res_PutString ("keys.4.name", input_templates[3].name);
	res_PutString ("keys.5.name", input_templates[4].name);
	res_PutString ("keys.6.name", input_templates[5].name);

	SaveResourceIndex (configDir, "uqm.cfg", "config.", TRUE);
	SaveKeyConfiguration (configDir, "flight.cfg");
}
