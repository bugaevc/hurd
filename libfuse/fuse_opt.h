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

#ifndef _FUSE_OPT_H
#define _FUSE_OPT_H

#include "fuse_base.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fuse_args
{
  char **argv;
  int argc;
  int allocated : 1;
};

#define FUSE_ARGS_INIT(argc, argv) { (argv), (argc), 0 }

FUSE_API int fuse_opt_add_arg (struct fuse_args *args, const char *arg);
FUSE_API void fuse_opt_free_args (struct fuse_args *args);

struct fuse_opt
{
  const char *templ;
  unsigned long offset;
  int value;
};

#define FUSE_OPT_KEY(templ, key) { (templ), -1UL, (key) }
#define FUSE_OPT_END { NULL, 0, 0 }

enum
{
  FUSE_OPT_KEY_OPT = -1,
  FUSE_OPT_KEY_NONOPT = -2,
  FUSE_OPT_KEY_KEEP = -3,
  FUSE_OPT_KEY_DISCARD = -4
};

typedef int (*fuse_opt_proc_t) (void *data, const char *arg,
				int key, struct fuse_args *out_args);

FUSE_API int fuse_opt_parse (struct fuse_args *args, void *data,
			     const struct fuse_opt opts[],
			     fuse_opt_proc_t proc);

#ifdef __cplusplus
}
#endif

#endif
