/* cda.c 
 * 
 * Minimalistic command line CD player using libcda.  Written for Linux
 * systems, but should work on others with a little help.
 *
 * Peter Wang <tjaden@psynet.net>
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libcda.h>


#define VERSION    "cda -- Copyright 2000 Peter Wang (tjaden@psynet.net)"


static void fatal (const char *fmt, ...)
{
    va_list ap;
    
    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    
    exit (EXIT_FAILURE);
}


static void cd_fatal (const char *s)
{
    fatal ("%s: %s\n", s, cd_error);
}


static int t0, t1;

static void get_track_info ()
{
    if (cd_get_tracks (&t0, &t1)) 
	cd_fatal ("cd_get_tracks");
}


static void show_usage ()
{
    printf (VERSION "\n"
	    "Interface pinched from dcd by dave@technopagan.org\n"
	    "Usage: cda [command | tracks]\n"
	    "\t d   dir      lists the CD directory\n"
	    "\t e   eject    opens the CD tray\n"
	    "\t c   close    closes the CD tray\n"
	    "\t h   help     displays this message\n"
	    "\t i   info     minimal info on the current CD\n"
	    "\t p   pause    pause/resume the CD\n"
	    "\t s   stop     stops the CD player\n"
	    "\t v   version  displays the software revision/patchlevel\n"
	    "\t f   forward  play the next track on CD\n"
	    "\t b   back     play the previous track on CD\n"
	    "\t r   random   plays random track from the CD\n");
}


static void show_version ()
{
    printf (VERSION "\n");
}


static void show_toc ()
{
    int i;
    
    get_track_info ();
        
    printf ("Track  Type  (%d tracks)\n", t1 - t0 + 1);
    
    for (i = t0; i <= t1; i++) 
	printf ("  %2d   %s\n", i, cd_is_audio (i) ? "Audio" : "Data");
}


static void show_info ()
{
    int i;
    
    get_track_info ();
    i = cd_current_track ();
    
    if (i > 0)
	printf ("Playing track %d (%d tracks)\n", i, t1 - t0 + 1);
    else
	printf ("Not playing (%d tracks)\n", t1 - t0 + 1);
}


static void pause_resume ()
{
    if (cd_is_paused ())
	cd_resume ();
    else if (cd_current_track ())
	cd_pause ();
}


static void forward_track ()
{
    int i, j;
    
    get_track_info ();
    
    j = cd_current_track ();
    if (!j)
	i = j = t0;
    else {
	i = j + 1;
	if (i > t1) i = t0;
    }
    
    while (1) {
	if (cd_is_audio (i)) {
	    cd_play_from (i);
	    break;
	}

	if (++i > t1) i = t0;
	if (i == j)
	    fatal ("No audio tracks on CD.\n");
    }
}


static void back_track ()
{
    int i, j;
    
    get_track_info ();

    j = i = cd_current_track ();
    if (!j) j = i = t0;
    
    while (1) {
	if (--i < t0) i = t1;
	if (i == j) break;
	
	if (cd_is_audio (i)) {
	    cd_play_from (i);
	    break;
	}
    }

    if (i == j)
	fatal ("No audio tracks on CD.\n");
}


static void random_track ()
{
    int i;
    
    get_track_info ();
    
    for (i = t0; i <= t1; i++)
	if (cd_is_audio (i))
	    break;
    
    if (i == t1 + 1) 
	fatal ("No audio tracks on CD.\n");
    
    srand (time (NULL));
    
    while (1) 
	if (cd_is_audio ((i = (rand () % (t1 + 1 - t0)) + t0))) {
	    cd_play_from (i);
	    break;
	}
}


static int option (const char *arg, const char *o1, const char *o2) 
{
    return (o1 && !strcmp (arg, o1)) || (o2 && !strcmp (arg, o2));
}


int main (int argc, char *argv[])
{
    int i;
    char *arg;
    
    if (cd_init ()) 
	cd_fatal ("cd_init");

    /* Command line options.  */
    for (i = 1; i < argc; i++) {
	arg = argv[i];
	while (*arg && *arg == '-') 
	    arg++;
	
	if (option (arg, "d", "dir"))
	    show_toc ();
	else if (option (arg, "e", "eject"))
	    cd_eject ();
	else if (option (arg, "c", "close"))
	    cd_close ();
	else if (option (arg, "h", "help"))
	    show_usage ();
	else if (option (arg, "i", "info"))
	    show_info ();
	else if (option (arg, "p", "pause"))
	    pause_resume ();
	else if (option (arg, "s", "stop"))
	    cd_stop ();
	else if (option (arg, "v", "version"))
	    show_version ();
	else if (option (arg, "f", "forward"))
	    forward_track ();
	else if (option (arg, "b", "back"))
	    back_track ();
	else if (option (arg, "r", "random"))
	    random_track ();
	else {
	    int i = atoi (arg);
	    
	    if (i) {
		if (cd_is_audio (i) > 0)
		    cd_play_from (i);
		else 
		    fatal ("Track %d is not audio or out of range\n", i);
	    }
	    else 
		fatal ("Unrecognised option `%s'\n", arg);
	}
    }

    /* No command line options: play first audio track.  */
    if (argc < 2) {
	cd_stop ();
	forward_track ();
    }
    
    cd_exit ();
    
    return EXIT_SUCCESS;
}


/*
 * end of cda.c
 * 
 *  9 June 2000 - First version
 * 14 June 2000 - Fixed problem trying to play no-audio CDs
 *  4 Nov  2000 - "Forward" skipped first track if not already playing
 */
