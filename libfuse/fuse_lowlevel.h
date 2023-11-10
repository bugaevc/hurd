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

#ifndef _FUSE_LOWLEVEL_H
#define _FUSE_LOWLEVEL_H

#include "fuse_common.h"

#include <sys/stat.h>
#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct fuse_session;
typedef struct fuse_req *fuse_req_t;

typedef ino_t fuse_ino_t;
#define FUSE_ROOT_ID 1

struct fuse_ctx
{
  uid_t uid;
  gid_t gid;
  pid_t pid;
  mode_t umask;
};

struct fuse_lowlevel_ops
{
  void (*init)(void *userdata, struct fuse_conn_info *conn);
  void (*destroy)(void *userdata);

  void (*lookup)(fuse_req_t req, fuse_ino_t parent, const char *name);
  void (*forget)(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup);

  void (*open)(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

  void (*getattr)(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

  void (*read)(fuse_req_t req, fuse_ino_t ino, size_t size,
	       off_t off, struct fuse_file_info *fi);

  void (*flush)(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

  void (*release)(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

  void (*readdir)(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
		  struct fuse_file_info *fi);

  void (*getxattr)(fuse_req_t req, fuse_ino_t ino,
		   const char *name, size_t size);

  void (*setxattr)(fuse_req_t req, fuse_ino_t ino,
		   const char *name, const char *value,
		   size_t size, int flags);

  void (*removexattr)(fuse_req_t req, fuse_ino_t ino, const char *name);
};

struct fuse_entry_param
{
  fuse_ino_t ino;
  uint64_t generation;
  struct stat attr;
  double attr_timeout;
  double entry_timeout;
};

struct fuse_cmdline_opts
{
  char *mountpoint;

  unsigned int max_idle_threads;
  unsigned int max_threads;

  int singlethread : 1;
  int foreground : 1;
  int debug : 1;
  int nodefault_subtype : 1;

  int show_version : 1;
  int show_help : 1;
  int clone_fd : 1;
};

/* Replies.  */

FUSE_API int fuse_reply_err (fuse_req_t req, int err);
FUSE_API void fuse_reply_none (fuse_req_t req);
FUSE_API int fuse_reply_entry (fuse_req_t req,
                               const struct fuse_entry_param *entry);
FUSE_API int fuse_reply_open (fuse_req_t req, const struct fuse_file_info *fi);
FUSE_API int fuse_reply_buf (fuse_req_t req, const char *buf, size_t size);
FUSE_API int fuse_reply_iov (fuse_req_t req,
			     const struct iovec *iov,
			     int count);
FUSE_API int fuse_reply_attr (fuse_req_t req, const struct stat *attr,
                              double attr_timeout);

/* Request metadata.  */

FUSE_API void *fuse_req_userdata (fuse_req_t req);
FUSE_API const struct fuse_ctx *fuse_req_ctx (fuse_req_t req);
FUSE_API int fuse_req_getgroups (fuse_req_t req, int size, gid_t list[]);

FUSE_API struct fuse_session *fuse_session_new (struct fuse_args *args,
						const struct fuse_lowlevel_ops *ops,
						size_t ops_size, void *userdata);

/* Dirent utilities.  */

FUSE_API size_t fuse_add_direntry (fuse_req_t req,
                                   char *buf, size_t buf_size,
                                   const char *name, const struct stat *stat,
                                   off_t off);

FUSE_API size_t fuse_add_direntry_plus (fuse_req_t req,
                                        char *buf, size_t buf_size,
                                        const char *name,
                                        struct fuse_entry_param *param,
                                        off_t off);

/* Session & loop management.  */

FUSE_API void fuse_session_destroy (struct fuse_session *session);

FUSE_API int fuse_session_mount (struct fuse_session *session,
				 const char *mount_point);
FUSE_API void fuse_session_unmount (struct fuse_session *session);

FUSE_API int fuse_session_loop (struct fuse_session *session);
FUSE_API int fuse_session_loop_mt (struct fuse_session *session,
				   struct fuse_loop_config *config);

FUSE_API void fuse_session_exit (struct fuse_session *session);
FUSE_API int fuse_session_exited (struct fuse_session *session);

/* Misc.  */

FUSE_API void fuse_lowlevel_version (void);
FUSE_API void fuse_lowlevel_help (void);
FUSE_API void fuse_cmdline_help (void);

#ifdef __cplusplus
}
#endif

#endif
