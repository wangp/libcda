/* This file is part of libcda.  See COPYING for licence.
 *
 * Peter Wang <tjaden@psynet.net>
 */

#ifndef __included_libcda_h
#define __included_libcda_h

#ifdef __cplusplus
extern "C" {
#endif


/* High-byte is major version, low byte is minor. */
#define LIBCDA_VERSION		0x0002
#define LIBCDA_VERSION_STR	"0.2"


int cd_init();
void cd_exit();

int cd_play(int track);
int cd_play_range(int start, int end);
int cd_play_from(int track);
int cd_current_track();
void cd_pause();
void cd_resume();
int cd_is_paused();
void cd_stop();

int cd_get_tracks(int *first, int *last);
int cd_is_audio(int track);

void cd_get_volume(int *c0, int *c1);
void cd_set_volume(int c0, int c1);

void cd_eject();
void cd_close();


#ifdef __cplusplus
}
#endif

#endif
