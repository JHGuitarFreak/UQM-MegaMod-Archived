/* This file contains some compile-time configuration options for MS Windows
 * systems when building using MSVC.
 * Change the values below if you want anything other than the defaults.
 * For *nix systems, config_unix.h is used, which is generated by build.sh
 * from src/config_unix.h.in.
 * When building on MS Windows using build.sh (MinGW, Cygwin),
 * config_win.h is generated from src/config_win.h.in.
 */

#ifndef _CONFIG_VC6_H
#define _CONFIG_VC6_H

/* Directory where the UQM game data is located */
#define CONTENTDIR "../content/"

/* Directory where game data will be stored */
//#define USERDIR "../userdata/"
#define USERDIR "%APPDATA%/uqm-megamod/"

/* Directory where config files will be stored */
#define CONFIGDIR USERDIR

/* Directory where supermelee teams will be stored */
#define MELEEDIR "%UQM_CONFIG_DIR%/teams/"

/* Directory where save games will be stored */
#define SAVEDIR "%UQM_CONFIG_DIR%/save/"

/* Define if words are stored with the most significant byte first */
#undef WORDS_BIGENDIAN

/* Defined if your system has readdir_r of its own */
#undef HAVE_READDIR_R

/* Defined if your system has setenv of its own */
#undef HAVE_SETENV

/* Defined if your system has strupr of its own */
#define HAVE_STRUPR

/* Defined if your system has strcasecmp of its own */
#undef HAVE_STRCASECMP_UQM
		// Not using "HAVE_STRCASECMP" as that conflicts with SDL.

/* Defined if your system has stricmp of its own */
#define HAVE_STRICMP

/* Defined if your system has getopt_long */
#undef HAVE_GETOPT_LONG

/* Defined if your system has iswgraph of its own*/
#define HAVE_ISWGRAPH

/* Defined if your system has wchar_t of its own */
#define HAVE_WCHAR_T

/* Defined if your system has wint_t of its own */
#define HAVE_WINT_T

#endif /* _CONFIG_VC6_H */

