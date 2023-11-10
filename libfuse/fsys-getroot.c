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
#include "fsys_S.h"

#include <fcntl.h>

error_t
fuse_S_fsys_getroot (struct fuse_session *session,
		     mach_port_t reply,
		     mach_msg_type_name_t reply_type,
		     mach_port_t dotdot,
		     const id_t *uids, mach_msg_type_number_t nuids,
		     const id_t *gids, mach_msg_type_number_t ngids,
		     int flags,
		     retry_type *do_retry,
		     string_t retry_name,
		     mach_port_t *retry_port,
		     mach_msg_type_name_t *retry_port_type)
{
  error_t err;
  struct iouser *cred;
  struct peropen *po;
  struct protid *newpi;

  if (!session)
    return EOPNOTSUPP;

  err = iohelp_create_complex_iouser (&cred, uids, nuids, gids, ngids);
  if (err)
    return err;

  flags &= O_HURD;

  // TODO: check for symlinks
  // TODO: check for translators
  // TODO: check permissions

  po = fuse_make_peropen (session->root_np, flags);
  if (!po)
    {
      err = errno;
      goto out;
    }

  newpi = fuse_make_protid (po, cred);
  if (!newpi)
    {
      err = errno;
      goto out;
    }

  // TODO: We should use it.
  if (MACH_PORT_VALID (dotdot))
    mach_port_deallocate (mach_task_self (), dotdot);

  *do_retry = FS_RETRY_NORMAL;
  *retry_port = ports_get_right (newpi);
  *retry_port_type = MACH_MSG_TYPE_MAKE_SEND;
  retry_name[0] = 0;
  ports_port_deref (newpi);

 out:
  if (err)
    iohelp_free_iouser (cred);
  return err;
}
