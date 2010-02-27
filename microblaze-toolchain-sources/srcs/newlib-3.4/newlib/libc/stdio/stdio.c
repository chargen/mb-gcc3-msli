/* No user fns here.  Pesch 15apr92. */

/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/unistd.h>
#include "local.h"

/*
 * Small standard I/O/seek/close functions.
 * These maintain the `known seek offset' for seek optimisation.
 */

int
__sread (cookie, buf, n)
     _PTR cookie;
     char *buf;
     int n;
{
  register FILE *fp = (FILE *) cookie;
  register int ret;

#ifdef __SCLE
  int oldmode = 0;
  if (fp->_flags & __SCLE)
    oldmode = setmode(fp->_file, O_BINARY);
#endif

#ifdef MICROBLAZE_NEWLIB
  ret = read(fp->_file, buf, n);
#else
  ret = _read_r (fp->_data, fp->_file, buf, n);
#endif

#ifdef __SCLE
  if (oldmode)
    setmode(fp->_file, oldmode);
#endif

  /* If the read succeeded, update the current offset.  */

  if (ret >= 0)
    fp->_offset += ret;
  else
    fp->_flags &= ~__SOFF;	/* paranoia */
  return ret;
}

int
__swrite (cookie, buf, n)
     _PTR cookie;
     char _CONST *buf;
     int n;
{
  register FILE *fp = (FILE *) cookie;
  int w;
#ifdef __SCLE
  int oldmode=0;
#endif

  if (fp->_flags & __SAPP)
#ifdef MICROBLAZE_NEWLIB
    (void) lseek(fp->_file, (off_t) 0, SEEK_END);
#else
    (void) _lseek_r (fp->_data, fp->_file, (off_t) 0, SEEK_END);
#endif
  fp->_flags &= ~__SOFF;	/* in case O_APPEND mode is set */

#ifdef __SCLE
  if (fp->_flags & __SCLE)
    oldmode = setmode(fp->_file, O_BINARY);
#endif

#ifdef MICROBLAZE_NEWLIB
  w = write(fp->_file, buf, n);
#else
  w = _write_r (fp->_data, fp->_file, buf, n);
#endif

#ifdef __SCLE
  if (oldmode)
    setmode(fp->_file, oldmode);
#endif

  return w;
}

fpos_t
__sseek (cookie, offset, whence)
     _PTR cookie;
     fpos_t offset;
     int whence;
{
  register FILE *fp = (FILE *) cookie;
  register off_t ret;

#ifdef MICROBLAZE_NEWLIB
  ret = lseek( fp->_file, (off_t) offset, whence);
#else
  ret = _lseek_r (fp->_data, fp->_file, (off_t) offset, whence);
#endif

  if (ret == -1L)
    fp->_flags &= ~__SOFF;
  else
    {
      fp->_flags |= __SOFF;
      fp->_offset = ret;
    }
  return ret;
}

int
__sclose (cookie)
     _PTR cookie;
{
  FILE *fp = (FILE *) cookie;
#ifdef MICROBLAZE_NEWLIB
  return close(fp->_file);
#else
  return _close_r (fp->_data, fp->_file);
#endif

}

#ifdef __SCLE
int
__stextmode (int fd)
{
#ifdef __CYGWIN__
  return _cygwin_istext_for_stdio (fd);
#else
  return 0;
#endif
}
#endif