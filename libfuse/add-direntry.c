/*
   Copyright (C) 2023 Free Software Foundation, Inc.
   Written by Sergey Bugaev.

   This file is part of the GNU Hurd.

   The GNU Hurd is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   The GNU Hurd is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA. */

#include "fuse_lowlevel.h"

#include <assert-backtrace.h>

#include <string.h>
#include <dirent.h>
#include <sys/param.h>

size_t
fuse_add_direntry (fuse_req_t req,
                   char *buf, size_t buf_size,
                   const char *name, const struct stat *stat,
                   off_t off)
{
  size_t namelen, reclen, recalign, recpad;
  struct dirent64 *dirent;

  /* A single record takes up as much space as struct dirent64, plus the space
     for the name, rounded up to the alignment.  Note that d_name is declared
     as a single-character array, which takes up exactly as much space as we
     need for the null terminator.  */
  recalign = __alignof__ (struct dirent64);
  namelen = strlen (name);
  reclen = roundup (sizeof (struct dirent64) + namelen, recalign);
  recpad = reclen - namelen - sizeof (struct dirent64);

  /* It is valid to call this function without a buffer, just to get the
     required size. */
  if (!buf || buf_size < reclen)
    return reclen;

  /* Otherwise, we expect things to always be aligned.  This will be the case
     if the buffer is only ever modified though these two functions.  */
  assert_backtrace (((uintptr_t) buf) % recalign == 0);

  dirent = (struct dirent64 *) buf;
  dirent->d_ino = stat->st_ino;
  dirent->d_reclen = reclen;
  dirent->d_type = IFTODT (stat->st_mode);
  dirent->d_namlen = namelen;

  memcpy (&dirent->d_name[0], name, namelen);
  /* Zero-fill the remainder, both to nul-terminate the name, and nto avoid
     leaking memory contents.  */
  memset (&dirent->d_name[namelen], 0, recpad);

  return reclen;
}

size_t
fuse_add_direntry_plus (fuse_req_t req,
                        char *buf, size_t buf_size,
                        const char *name,
                        struct fuse_entry_param *param,
                        off_t off)
{
  return fuse_add_direntry (req, buf, buf_size, name, &param->attr, off);
}
