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

#include "fuse_opt.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <assert-backtrace.h>

int
fuse_opt_add_arg (struct fuse_args *args, const char *arg)
{
  size_t new_size;
  char **new_argv;

  /* We'll need this much space for the new argv plus a NULL terminator.  */
  new_size = (args->argc + 2) * sizeof (char *);

  if (args->allocated)
    new_argv = realloc (args->argv, new_size);
  else
    new_argv = malloc (new_size);

  if (!new_argv)
    goto nomem;

  if (!args->allocated && args->argv != NULL)
    memcpy (new_argv, args->argv, args->argc * sizeof (char *));
  else if (args->allocated)
    /* We have sucessfully realloc'ed, so the old pointer is no longer usable;
       switch to the new pointer even if we fail the strdup () below.  */
    args->argv = new_argv;

  new_argv[args->argc] = strdup (arg);
  new_argv[args->argc + 1] = NULL;

  /* Check if that strdup () above worked.  */
  if (new_argv[args->argc] == NULL)
    {
      if (!args->allocated)
	free (new_argv);
      goto nomem;
    }

  /* It all worked, commit it.  */
  if (!args->allocated)
    {
      args->argv = new_argv;
      args->allocated = 1;
    }
  args->argc++;
  return 0;

 nomem:
  error (0, errno, "failed to allocate memory");
  return -1;
}

void
fuse_opt_free_args (struct fuse_args *args)
{
  size_t i;

  if (!args)
    return;

  if (args->allocated)
    {
      for (i = 0; i < args->argc; i++)
	free (args->argv[i]);
      free (args->argv);
    }

  args->argc = 0;
  args->argv = NULL;
  args->allocated = 0;
}

int
fuse_opt_parse (struct fuse_args *args, void *data,
                const struct fuse_opt opts[],
                fuse_opt_proc_t proc)
{
  size_t i;
  const char *arg;
  int seen_dash_dash = 0;
  struct fuse_args out_args = FUSE_ARGS_INIT (0, NULL);

  /* Loop over the input arguments.  */
  for (i = 1; i < args->argc; i++)
    {
      arg = args->argv[i];
      assert_backtrace (arg != NULL);

      if (seen_dash_dash || arg[0] != '-') // TODO act on proc () result
        proc (data, arg, FUSE_OPT_KEY_NONOPT, &out_args);
        // todo continue
      if (arg[1] == '-' && arg[2] == 0)
	{
	  seen_dash_dash = 1;
	  continue;
	}

      /* Loop over the options.  */
      for (const struct fuse_opt *opt = opts; opt->templ; opt++)
	{
	  // TODO
	}
    }

  return 0;
}
