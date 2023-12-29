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

#include "priv.h"
#include "fuse_lowlevel.h"

#include <argp.h>
#include <version.h>

const char *argp_program_version = STANDARD_HURD_VERSION (libfuse);

static const char args_doc[] = "[MOUNTPOINT]";

static const struct argp_option startup_options[] =
{
  /* None at the moment.  */
  { 0, 0 }
};

static error_t
parse_startup_opt (int key, char *arg, struct argp_state *state)
{
  error_t err;
  mach_port_t bootstrap_port;
  struct fuse_cmdline_opts *opts = state->input;

  switch (key)
    {
    case ARGP_KEY_ARG:
      /* Only allow one positional argument.  */
      if (state->arg_num > 0)
	{
	  argp_usage (state);
	  return EINVAL;
	}
      /* See if we have been started as a translator.  */
      err = task_get_bootstrap_port (mach_task_self (), &bootstrap_port);
      assert_perror_backtrace (err);

      if (bootstrap_port == MACH_PORT_NULL)
	{
	  /* OK, accept an explicit mount point.  */
	  opts->mountpoint = strdup (arg);
	  return 0;
	}
      else
	{
	  mach_port_deallocate (mach_task_self (), bootstrap_port);
	  argp_error (state, "started as a translator but passed a mountpoint");
	  return EINVAL;
	}

    case ARGP_KEY_NO_ARGS:
      /* See if we have been started as a translator.  */
      err = task_get_bootstrap_port (mach_task_self (), &bootstrap_port);
      assert_perror_backtrace (err);
      if (bootstrap_port == MACH_PORT_NULL)
	{
	  argp_error (state, "must be either started as a translator, or passed a mountpoint");
	  return EINVAL;
	}
      /* Make up a fake mountpint to appease the program.  We use an empty string.  */
      opts->mountpoint = strdup ("");
      mach_port_deallocate (mach_task_self (), bootstrap_port);
      return 0;

    default:
      return ARGP_ERR_UNKNOWN;
    }
};

static const struct argp startup_argp =
{
  startup_options,
  parse_startup_opt,
  args_doc,
  NULL,
  NULL
};

void
fuse_cmdline_help (void)
{
  argp_help (&startup_argp, stdout, ARGP_HELP_LONG,
	     program_invocation_name);
}

int
fuse_parse_cmdline (struct fuse_args *args,
		    struct fuse_cmdline_opts *opts)
{
  error_t err;

  memset (opts, 0, sizeof (*opts));
  err = argp_parse (&startup_argp, args->argc, args->argv,
		    ARGP_NO_EXIT, NULL, opts);

  if (err)
    return 1;
  return 0;
}

void
fuse_lowlevel_help (void)
{
  fuse_cmdline_help ();
}
