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

#ifndef _FUSE_COMMON_H
#define _FUSE_COMMON_H

#include "fuse_opt.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Check for this definition to conditionally compile
   code in fuse-based filesystems.  */
#define FUSE_HURD 1

#define FUSE_API __attribute__ ((visibility ("default")))

struct fuse_file_info
{
  int flags;

  uint64_t fh;
  uint32_t poll_events;
};

struct fuse_loop_config
{
  unsigned int max_idle_threads;
  int clone_fd : 1;
};

struct fuse_session;

FUSE_API int fuse_set_signal_handlers (struct fuse_session *sesession);
FUSE_API void fuse_remove_signal_handlers (struct fuse_session *session);

#ifdef __cplusplus
}
#endif

#endif
