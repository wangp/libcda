/* libcda; Linux component.
 *
 * Peter Wang <tjaden@psynet.net>
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include "libcda.h"


#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX((x), MIN((y), (z)))


static int fd = -1;


static int get_subchnl(struct cdrom_subchnl *s)
{
    memset(s, 0, sizeof(struct cdrom_subchnl));
    s->cdsc_format = CDROM_MSF;
    return ioctl(fd, CDROMSUBCHNL, s);
}


/* cd_init:
 *  Initialise library.  Return zero on success.
 */
int cd_init()
{
    char *device;

    device = getenv("CDAUDIO");
    if (!device) device = "/dev/cdrom";

    if (fd != -1) close(fd);

    fd = open(device, O_RDONLY | O_NONBLOCK);
    return fd ? 0 : -1;
}


/* cd_exit:
 *  Shutdown.
 */
void cd_exit()
{
    if (fd != -1) {
	close(fd);
	fd = -1;
    }
}


static int play(int t1, int t2)
{
    struct cdrom_ti idx;

    memset(&idx, 0, sizeof(idx));
    idx.cdti_trk0 = t1;
    idx.cdti_trk1 = t2;
    return ioctl(fd, CDROMPLAYTRKIND, &idx);
}


/* cd_play:
 *  Play specified track.  Return zero on success.
 */
int cd_play(int track)
{
    return play(track, track);
}


/* cd_play_range:
 *  Play from START to END tracks.  Return zero on success.
 */
int cd_play_range(int start, int end)
{
    return play(start, end);
}


/* cd_play_from:
 *  Play from track to end of disc.
 *  Return zero on success.
 */
int cd_play_from(int track)
{
    int t1, t2;
    if ((cd_get_tracks(&t1, &t2) != 0) || (track > t2))
	return -1;
    return play(track, t2);
}


/* cd_current_track:
 *  Return track currently in playback, or zero if stopped.
 */
int cd_current_track()
{
    struct cdrom_subchnl s;

    get_subchnl(&s);
    if (s.cdsc_audiostatus == CDROM_AUDIO_PLAY)
	return s.cdsc_trk;
    else
	return 0;
}


/* cd_pause:
 *  Pause playback.
 */
void cd_pause()
{
    ioctl(fd, CDROMPAUSE);
}


/* cd_resume:
 *  Resume playback.
 */
void cd_resume()
{
    if (cd_is_paused())
	ioctl(fd, CDROMRESUME);
}


/* cd_is_paused:
 *  Return non-zero if playback is paused.
 */
int cd_is_paused()
{
    struct cdrom_subchnl s;

    get_subchnl(&s);
    return (s.cdsc_audiostatus == CDROM_AUDIO_PAUSED);
}


/* cd_stop:
 *  Stop playback.
 */
void cd_stop()
{
    ioctl(fd, CDROMSTOP);
}


/* cd_get_tracks:
 *  Get first and last tracks of CD.
 *  Return zero on success.
 */
int cd_get_tracks(int *first, int *last)
{
    struct cdrom_tochdr toc;

    if (ioctl(fd, CDROMREADTOCHDR, &toc) < 0) {
	*first = *last = 0;
	return -1;
    }

    *first = toc.cdth_trk0;
    *last = toc.cdth_trk1;
    return 0;
}


/* cd_is_audio:
 *  Return 1 if track specified is audio,
 *  zero if it is data, -1 if an error occurs.
 */
int cd_is_audio(int track)
{
    struct cdrom_tocentry e;
    
    memset(&e, 0, sizeof(e));
    e.cdte_track = track;
    e.cdte_format = CDROM_LBA;
    if (ioctl(fd, CDROMREADTOCENTRY, &e) < 0)
	return -1;
    return (e.cdte_ctrl & CDROM_DATA_TRACK) ? 0 : 1;
}


/* cd_get_volume:
 *  Return volumes of left and right channels.
 */
void cd_get_volume(int *c0, int *c1)
{
    struct cdrom_volctrl vol;

    ioctl(fd, CDROMVOLREAD, &vol);
    *c0 = vol.channel0;
    *c1 = vol.channel1;
}


/* cd_set_volume:
 *  Set left and right channel volumes (0 - 255).
 */
void cd_set_volume(int c0, int c1)
{
    struct cdrom_volctrl vol;

    vol.channel0 = MID(0, c0, 255);
    vol.channel1 = MID(0, c1, 255);
    vol.channel2 = 0;
    vol.channel3 = 0;
    ioctl(fd, CDROMVOLCTRL, &vol);
}


/* cd_eject:
 *  Eject CD drive (if possible).
 */
void cd_eject()
{
    ioctl(fd, CDROMEJECT);
}


/* cd_close:
 *  Close CD drive (if possible).
 */
void cd_close()
{
    ioctl(fd, CDROMCLOSETRAY);
}
