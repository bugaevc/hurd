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

#ifndef _LIBFUSE_PRIV_H
#define _LIBFUSE_PRIV_H

#include <hurd/hurd_types.h>
#include <hurd/ports.h>
#include <hurd/fshelp.h>
#include "fuse_lowlevel.h"

struct protid
{
  struct port_info pi;

  struct iouser *user;
  struct fuse_ctx ctx;

  struct peropen *po;
};

struct peropen
{
  loff_t filepointer;
  refcount_t refcnt;

  struct node *np;
  struct fuse_file_info fi;

  // mach_port_t shadow_root_parent;
  // struct node *shadow_root;
};

struct node
{
  // pthread_mutex_t lock;
  refcounts_t refcounts;
  struct fuse_session *session;
  io_statbuf_t stat;
  struct transbox transbox;
};

struct fuse_session
{
  struct port_info pi;
  const struct fuse_lowlevel_ops *ops;
  void *userdata;
  struct node *root_np;
};

enum reply_to
{
  REPLY_TO_IO_READ,
};

struct fuse_req
{
  // struct fuse_session *session;
  mach_port_t reply_port;
  mach_msg_type_name_t reply_port_type;
  enum reply_to reply_to;
};

static inline fuse_req_t
fuse_req_new (mach_port_t reply_port,
	      mach_msg_type_name_t reply_port_type,
	      enum reply_to reply_to)
{
  fuse_req_t req = malloc (sizeof (struct fuse_req));
  if (req)
    {
      req->reply_port = reply_port;
      req->reply_port_type = reply_port_type;
      req->reply_to = reply_to;
    }
  return req;
}

struct peropen *fuse_make_peropen (struct node *np, int flags);
struct protid *fuse_make_protid (struct peropen *po, struct iouser *cred);

/* For MIG.  */
typedef struct protid *protid_t;
typedef struct fuse_session *session_t;

extern struct port_bucket *fuse_port_bucket;
extern struct port_class *fuse_session_class;
extern struct port_class *fuse_protid_class;

int fuse_demuxer (mach_msg_header_t *inp, mach_msg_header_t *outp);

static inline struct protid * __attribute__ ((unused))
begin_using_protid_port (file_t port)
{
  return ports_lookup_port (fuse_port_bucket, port, fuse_protid_class);
}

static inline struct protid * __attribute__ ((unused))
begin_using_protid_payload (uintptr_t payload)
{
  return ports_lookup_payload (fuse_port_bucket, payload, fuse_protid_class);
}

static inline void __attribute__ ((unused))
end_using_protid_port (struct protid *cred)
{
  if (cred)
    ports_port_deref (cred);
}

static inline struct fuse_session * __attribute__ ((unused))
begin_using_session_port (fsys_t port)
{
  return ports_lookup_port (fuse_port_bucket, port, fuse_session_class);
}

static inline struct fuse_session * __attribute__ ((unused))
begin_using_session_payload (uintptr_t payload)
{
  return ports_lookup_payload (fuse_port_bucket, payload, fuse_session_class);
}

static inline void __attribute__ ((unused))
end_using_session_port (struct fuse_session *session)
{
  if (session)
    ports_port_deref (session);
}

#endif
