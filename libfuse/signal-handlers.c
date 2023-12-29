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

#include "fuse_common.h"
#include "fuse_lowlevel.h"

#include <signal.h>
#include <string.h>
#include <assert-backtrace.h>

static struct fuse_session *global_session;

static void
exit_handler (int signum)
{
  /* Doing things in a signal handler is kind of problematic,
     but here goes.  */
  assert_backtrace (global_session);
  fuse_session_exit (global_session);
}

static void
sigpipe_handler (int signum)
{
  assert_backtrace (signum == SIGPIPE);
  /* Ignore it.  */
}

static int
do_sigaction (int signum, const struct sigaction *act, void *expected)
{
  int rc;
  struct sigaction prev_act;

  rc = sigaction (signum, NULL, &prev_act);
  if (rc < 0)
    return rc;
  if (prev_act.sa_handler != expected)
    return 0;
  return sigaction (signum, act, NULL);
}

int
fuse_set_signal_handlers (struct fuse_session *session)
{
  struct sigaction act;

  assert_backtrace (global_session == NULL);
  global_session = session;

  memset (&act, 0, sizeof (act));
  act.sa_handler = exit_handler;

  do_sigaction (SIGINT, &act, SIG_DFL);
  do_sigaction (SIGTERM, &act, SIG_DFL);
  do_sigaction (SIGHUP, &act, SIG_DFL);

  act.sa_handler = sigpipe_handler;
  do_sigaction (SIGPIPE, &act, SIG_DFL);

  return 0;
}

void
fuse_remove_signal_handlers (struct fuse_session *session)
{
  struct sigaction act;

  assert_backtrace (session == global_session);
  global_session = NULL;

  memset (&act, 0, sizeof (act));
  act.sa_handler = SIG_DFL;

  do_sigaction (SIGINT, &act, exit_handler);
  do_sigaction (SIGTERM, &act, exit_handler);
  do_sigaction (SIGHUP, &act, exit_handler);
  do_sigaction (SIGINT, &act, sigpipe_handler);
}
