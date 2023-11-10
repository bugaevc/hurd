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

#include <stdio.h>
#include <hurd/fsys.h>

int
fuse_session_mount (struct fuse_session *session, const char *mount_point)
{
  error_t err;
  mach_port_t bootstrap;
  file_t underlying;

  err = task_get_bootstrap_port (mach_task_self (), &bootstrap);
  assert_perror_backtrace (err);

  if (bootstrap != MACH_PORT_NULL)
    {
      /* Started as a translator, ignore the passed-in mount point, and
         contact the parent filesystem in the usual manner.  Warn the user
         about this, unless the mount point is dummy: namely, NULL, empty,
         or root, in which case we assume the user is aware.  */

      if (mount_point && mount_point[0] && strcmp (mount_point, "/"))
	fprintf (stderr,
		"%s: Started as a translator, "
		"ignoring mountpoint \"%s\"\n",
		program_invocation_name, mount_point);

      err = fsys_startup (bootstrap, 0, ports_get_right (session),
			  MACH_MSG_TYPE_MAKE_SEND, &underlying);
      if (MACH_PORT_VALID (bootstrap))
        mach_port_deallocate (mach_task_self (), bootstrap);
    }
  else
    {
      underlying = file_name_lookup (mount_point, O_NORW, 0);
      if (!underlying)
	return errno;

      err = file_set_translator (underlying, 0, FS_TRANS_SET | FS_TRANS_EXCL,
				 0, 0, 0, ports_get_right (session),
				 MACH_MSG_TYPE_MAKE_SEND);
    }

  if (MACH_PORT_VALID (underlying))
    mach_port_deallocate (mach_task_self (), underlying);
  return err;
}
