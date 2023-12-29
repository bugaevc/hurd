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

struct protid *
fuse_make_protid (struct peropen *po, struct iouser *cred)
{
  error_t err;
  struct protid *pi;

  err = ports_create_port (fuse_protid_class, fuse_port_bucket,
			   sizeof (struct protid), &pi);

  if (err)
    {
      errno = err;
      return NULL;
    }

  /* Consume the reference.  */
  pi->po = po;
  pi->user = cred;

  return po;
}
