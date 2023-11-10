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

struct port_bucket *fuse_port_bucket;
struct port_class *fuse_session_class;
struct port_class *fuse_protid_class;

static void
init_ports (void)
{
  fuse_port_bucket = ports_create_bucket ();
  assert_backtrace (fuse_port_bucket);

  fuse_session_class = ports_create_class (NULL, NULL);
  assert_backtrace (fuse_session_class);

  fuse_protid_class = ports_create_class (NULL, NULL);  // FIXME
  assert_backtrace (fuse_protid_class);
}

struct fuse_session *
fuse_session_new (struct fuse_args *args,
		  const struct fuse_lowlevel_ops *ops,
		  size_t ops_size, void *userdata)
{
  error_t err;
  struct fuse_session *session;

  assert_backtrace (ops_size == sizeof (*ops));

  if (!fuse_session_class || !fuse_port_bucket)
    init_ports ();

  err = ports_create_port (fuse_session_class, fuse_port_bucket,
			   sizeof (struct fuse_session), &session);
  if (err)
    {
      errno = err;
      return NULL;
    }

  session->userdata = userdata;
  session->ops = ops;

  return session;
}

