/* 
 * Example program for libcda.
 *
 * Jean-Yves Levesque <jyves@nortelnetworks.com>
 */

/*----------------------------------------------------------------------*/
/*			D E F I N E S					*/
/*----------------------------------------------------------------------*/
#define true 	1
#define false 	0

#define PGM	"cda"
#define VERSION "V0.1"
#define COPYRGT "(C)2000 Peter Wang (tjaden@users.sourceforge.net)"


#define TRACK	"Track"
#define INFO	"TrackInfo"
#define FROM 	"From "
#define TO   	"To   "
#define VOL_L	"Left  Volume"
#define VOL_R	"Right Volume"
#define CANCEL	"Command Cancelled"
#define ERROR	"Error occurred (%s)\n" 

#define AUDIO(t)	( cd_is_audio(t) ? "Audio" : "Data" )

/*----------------------------------------------------------------------*/
/*			I N C L U D E S					*/
/*----------------------------------------------------------------------*/
#include <stdio.h>
#include "libcda.h"
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

/*----------------------------------------------------------------------*/
/*			T Y P E D E F S					*/
/*----------------------------------------------------------------------*/
typedef	  struct
	{
	  char c ;
	  char string[15] ;
	  void (*func)() ;
	  char helptext[30] ;
	}
	  cmdStruct ;

typedef	  struct
	{
	  int	first_track ;
	  int	last_track ;
	}
	  track_info ;

/*----------------------------------------------------------------------*/
/*			Internal Functions Prototypes			*/
/*----------------------------------------------------------------------*/
static void back_track() ;
static void done_playing() ;
static void forward_track() ;

static void pause_resume() ;

static void play_from() ;
static void play_range() ;
static void play_track() ;

static void random_track() ;

static void set_volume() ;

static void show_audio() ;
static void show_info() ;
static void show_status() ;
static void show_toc() ;
static void show_usage() ;
static void show_volume() ;
static void show_version() ;

static void show_error( const char *fmt, ...) ;

static int  input_int( const char * prompt ) ;
static int  valid_input_int( const char * prompto, int min, int max ) ;
static char read_cmd() ;
static int  valid_cmd( char c ) ;

/*----------------------------------------------------------------------*/
/*			Table of Function Pointers			*/
/*----------------------------------------------------------------------*/
cmdStruct myCmdStruct[] =
        { { 'a', "audio"    , show_audio   , "Check if track is audio" }
        , { 'c', "close"    , cd_close     , "closes the CD tray" }
	, { 'd', "dir"	    , show_toc     , "lists the CD directory" }
        , { 'e', "eject"    , cd_eject     , "opens the CD tray" }
        , { 'F', "FromTrack", play_from    , "Play from track to end of CD" }
        , { 'h', "help"	    , show_usage   , "displays this message" }
        , { 'i', "info"	    , show_info    , "minimal info on the current CD" }
        , { 'I', "Info S/W" , show_version , "Displays software version" }
        , { 'p', "play"     , play_track   , "Plays track from CD" }
        , { 'r', "random"   , random_track , "Plays random track from CD" }
        , { 'R', "Range"    , play_range   , "Plays range of tracks from CD" }
        , { 's', "stop"	    , cd_stop      , "stops the CD player" }
        , { 'S', "Status"   , show_status  , "Show Paused Status" }
        , { 'v', "volume"   , set_volume   , "Sets Volume" }
        , { 'V', "Volume"   , show_volume  , "Shows Current Volume" }
        , { 'w', "wait"     , pause_resume , "Pause/resume the CD" }
        , { '+', "forward"  , forward_track, "play the next track on CD" }
        , { '-', "back"	    , back_track   , "play the previous track on CD" }
        , { 'q', "Quit"     , done_playing , "Quit" }
	} ;

/*----------------------------------------------------------------------*/
/*			GLobal Variables                                */
/*----------------------------------------------------------------------*/
static int tLength     = sizeof( myCmdStruct ) / sizeof( myCmdStruct[0] ) ;


/*
 * Main procedure loops over user requests.
 */
int main()
{
  int	  i = 0 ;
  char	  c ; // Command Read From Input.

  // Open CD ROM Drive.
  if	( cd_init() < 0 )
	{
	  show_error( ERROR, cd_error ) ;
	  return 1;
  	}
	
  // Shows possible commands.
  show_usage();

  // While Not Quit.
  while ((c = read_cmd()) != 'q' )
	{

          if	(( i = valid_cmd( c )) == -1 )
		{
		  printf("Unrecognised command: `%c'\n", c ) ;
		}
	  else
		{
		  (*myCmdStruct[i].func)() ;
		}
    	}

  // Stop Device.
  cd_exit();
    
  return 0 ;
}


/*
 * Description: Sets Boolean to break the loop and quit.
 *
 * Returns:     c - First character in buffer.
 */
static char read_cmd()
{
  char	  cmd[20] ;

  fprintf( stdout, PGM "-> " ) ;
  fflush ( stdout ) ;

  scanf( "%s", cmd ) ;

  return cmd[0] ;
}


/*
 * Description: Sets Boolean to break the loop and quit.
 *
 * Returns:     n  - Line in table where command is.
 *              -1 - Not Found.
 */
static int valid_cmd( char c )
{
  int	  rc	= -1 ;
  int	  i	=  0 ;

  // Search Command.
  for	( i = 0; i < tLength; i++ )
	{
          if	( myCmdStruct[i].c == c )
		{
		  rc = i ;
		}
	}

  return rc ;
}



/*
 * Description: Sets Boolean to break the loop and quit.
 */
static void done_playing()
{
}



/*
 * Description: Get First and Last Track number from the CD.
 * 
 * Returns:	true if success, false other wise.
 */
static int get_track_info( track_info * ptrack_info )
{
  int	  rc		= false ;
  int	  first_track	= 0 ;
  int	  last_track	= 0 ;

  if	( cd_get_tracks ( &first_track, &last_track ) ) 
	{
	  show_error( INFO, cd_error ) ;
	}
  else
	{
	  ptrack_info->first_track = first_track ;
	  ptrack_info->last_track  = last_track ;
	  rc = true ;
	}

  return rc ;
}


/*
 * Description: Displays the various available commands.
 */
static void show_usage ()
{
  int	   i ;

  show_version() ;

  printf( "Available Commands are:\n\n" ) ;

  for	( i = 0; i < tLength; i++ )
	{
	  printf( "\t%c  %-15s  %-30s\n", myCmdStruct[i].c
				  	, myCmdStruct[i].string
				  	, myCmdStruct[i].helptext ) ;
	}
}



/*
 * Description: Displays the software version.
 */
static void show_version ()
{
    printf( PGM " - " VERSION " - " COPYRGT "\n" ) ;
}



/*
 * Description: Displays the CD Table Of Contents
 */
static void show_toc ()
{
  int  		i ;
  int		no ;
  track_info	tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}

  no	= tInfo.last_track - tInfo.first_track + 1 ;
        
  printf( TRACK "  Type  (%d tracks)\n", no ) ;
    
  for	( i = tInfo.first_track; i <= tInfo.last_track; i++ ) 
	{
          printf( "  %2d   %s\n", i, AUDIO(i) ) ;
	}
}


/*
 * Description: Displays minimal information regardin gthe current track.
 */
static void show_info ()
{
  int		i ;
  int		no ;
  track_info	tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}

  i	= cd_current_track() ;
  no	= tInfo.last_track - tInfo.first_track + 1 ;
    
  if	( i > 0 )
	{
          printf ("Playing track %d (%d tracks)\n", i, no ) ;
	}
  else
	{
          printf ("Not playing (%d tracks)\n", no ) ;
	}
}


/*
 * Description: Plays a track from the CD.
 */
static void play_track ()
{
  int		  trk ;
  track_info	  tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}
  else
  if	((trk = valid_input_int( TRACK	, tInfo.first_track
					, tInfo.last_track ) ) == -1 ) 
	{
	  show_error( CANCEL ) ;
	}
  else
  if	( cd_play( trk ) != 0 )
	{
  	  show_error( ERROR, cd_error ) ;
	}
}


/*
 * Description: Plays a range of tracks from the CD.
 */
static void play_from ()
{
  int		  trk ;
  track_info	  tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}
  else
  if	((trk = valid_input_int	( FROM TRACK
				, tInfo.first_track
				, tInfo.last_track ) ) == -1 ) 
	{
	  show_error( CANCEL ) ;
	}
  else
  if	( cd_play_from( trk ) != 0 )
	{
  	  show_error( ERROR, cd_error ) ;
	}
}


/*
 * Description: Plays a range of tracks from the CD.
 */
static void play_range ()
{
  int		  first ;
  int		  last  ;
  track_info	  tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}
  else
  if	((first = valid_input_int( FROM TRACK
				 , tInfo.first_track
				 , tInfo.last_track ) ) == -1 ) 
	{
	  show_error( CANCEL ) ;
	}
  else
  if	((last  = valid_input_int( TO TRACK
				 , tInfo.first_track
				 , tInfo.last_track ) ) == -1 ) 
	{
	  show_error( CANCEL ) ;
	}
  else
  if	( cd_play_range( first, last ) != 0 )
	{
  	  show_error( ERROR, cd_error ) ;
	}
}


/*
 * Description: Pause or Resume the Current Track.
 */
static void pause_resume ()
{
  if	( cd_is_paused ())
	{
          cd_eject() ;
	}
  else
  if	( cd_current_track ())
	{
          cd_close() ;
	}
}


/*
 * Description: Pause or Resume the Current Track.
 */
static void open_close()
{
  if	( cd_is_paused ())
	{
          cd_resume ();
	}
  else
  if	( cd_current_track ())
	{
          cd_pause ();
	}
}



/*
 * Description: Plays the next track from the CD.
 */
static void forward_track ()
{
  int		  ct ;
  track_info	  tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}
    
  ct	= ( cd_current_track() + 1 ) % tInfo.last_track ;

  if	( ct < 1 )
	{
	  ct =  tInfo.last_track ;
	}

  if	( cd_play_from( ct ) != 0 )
	{
  	  show_error( ERROR, cd_error ) ;
	}
}



/*
 * Description: Plays the previous track from the CD.
 */
static void back_track ()
{
  int		  ct ;
  track_info	tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}

  ct	= ( cd_current_track() - 1 ) % tInfo.last_track ;

  if	( ct < 1 )
	{
	  ct =  tInfo.last_track ;
	}

  if	( cd_play_from( ct ) != 0 )
	{
  	  show_error( ERROR, cd_error ) ;
	}
}


/*
 * Description: Plays a random track from the CD.
 */
static void random_track ()
{
  int		  ct ;
  track_info	  tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}
    
  srand	( time (NULL ) ) ;
    
  ct	 = (( rand () % ( tInfo.last_track + 1 - tInfo.first_track ) )
		       + tInfo.first_track ) ;

  if	( cd_play_from( ct ) != 0 )
	{
  	  show_error( ERROR, cd_error ) ;
	}
}


/*
 * Description: Shows the current volume settings.
 */
static void show_volume()
{
  int lVol ; // left volume
  int rVol ; // right volume

  cd_get_volume( &lVol, &rVol ) ;
  printf( VOL_L  ": %d\n" VOL_R ": %d\n", lVol, rVol ) ;
}


/*
 * Description: Changes the volume settings.
 */
static void set_volume()
{
  int lVol ; // left volume
  int rVol ; // right volume

  if	((lVol = valid_input_int( VOL_L	, 1, 255 ) ) == -1 ) 
	{
	  show_error( CANCEL ) ;
	}
  else
  if	((rVol = valid_input_int( VOL_R	, 1, 255 ) ) == -1 ) 
	{
	  show_error( CANCEL ) ;
	}
  else
	{
	  cd_set_volume( lVol, rVol ) ;
	}
}


/*
 * Description: Shows the current track type.
 */
static void show_audio()
{
  int		  trk ;
  track_info	  tInfo ;
    
  if	( get_track_info( &tInfo ) == false )
	{
	  return ;
	}
  else
  if	((trk = valid_input_int( TRACK	, tInfo.first_track
					, tInfo.last_track ) ) == -1 ) 
	{
	  show_error( CANCEL ) ;
	}
  else
	{
	  printf( TRACK " %d is %s\n", trk, AUDIO(trk) ) ;
	}
}


/*
 * Description: Shows the current track type.
 */
static void show_status()
{
  printf( ( cd_is_paused() ) ? "Paused\n" : "Not Paused\n" ) ;
}


/* 
 * Description: Gets Integer From User Via a Prompt.
 *
 * Parameters:  prompt - To Ask User.
 * Returns:     a      - Number imput
 */
static int input_int( const char * prompt )
{
  int	  a ;

  fprintf( stdout, "%s: ", prompt ) ;

  fflush ( stdout ) ;

  scanf("%d", &a ) ;

  return a ;
}


/* 
 * Description: Gets Integer From User Via a Prompt.
 *
 * Parameters:  prompt - To Ask User.
 *              min    - Minimum Value Requested
 *              max    - Maximum Value Requested
 * Returns:     a      - Number imput
 */
static int valid_input_int( const char * prompt, int min, int max )
{
  int	  a 	= -1 ;
  char	  cmd[20] ;

  while	( 1 )
	{
	  a	= -1 ;
	  fprintf( stdout, "%s [%d-%d] or 'q' to quit: ", prompt, min, max ) ;
	  fflush ( stdout ) ;

	  scanf( "%s", cmd ) ;

          if	( cmd[0] == 'q' )
		{
		  break ;
		}

          a	= atoi ( cmd ) ;

	  if	( a >= min
	       && a <= max )
	        {
	          break ;
	        } 
	  else
	        {
	          fprintf( stdout, "Invalid Value %d\n", a ) ;
		  fflush ( stdout ) ;
	        }
	}

  return a ;
}


/*
 * Description: Shows the error encountered.
 */
static void show_error( const char *fmt, ... )
{
  va_list ap;
    
  va_start( ap, fmt);
  vfprintf( stderr, fmt, ap ) ;
  va_end  ( ap ) ;
}
