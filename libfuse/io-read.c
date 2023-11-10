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
#include "io_S.h"

#include <fcntl.h>

error_t
fuse_S_io_read (struct protid *user,
		mach_port_t reply_port,
		mach_msg_type_name_t reply_port_type,
		data_t *data,
		mach_msg_type_number_t *datalen,
		off_t offset,
		vm_size_t amount)
{
  struct peropen *po;
  struct node *np;
  fuse_req_t req;
  off_t effective_offset;

  if (!user)
    return EOPNOTSUPP;

  po = user->po;
  np = po->np;
  // pthread_mutex_lock (&np->lock);

  if (!(po->fi.flags & O_READ))
    {
      // pthread_mutex_unlock (&np->lock);
      return EBADF;
    }

  effective_offset = (offset == -1) ? po->filepointer : offset;
  if (effective_offset < 0)
    {
      // pthread_mutex_unlock (&np->lock);
      return EINVAL;
    }

  if (S_ISLNK (np->stat.st_mode))
    {
      // TODO
      assert_backtrace (0);
    }
  else
    {
      req = fuse_req_new (reply_port, reply_port_type, REPLY_TO_IO_READ);
      np->session->ops->read (req, np->stat.st_ino, amount,
			      effective_offset, &po->fi);
      return MIG_NO_REPLY;
    }
}
