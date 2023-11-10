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
#include "fs_S.h"
#include "fsys_S.h"
#include "../libports/notify_S.h"
#include "../libports/interrupt_S.h"

int
fuse_demuxer (mach_msg_header_t *inp,
	      mach_msg_header_t *outp)
{
  mig_routine_t routine;

  if ((routine = fuse_io_server_routine (inp)) ||
      (routine = fuse_fs_server_routine (inp)) ||
      (routine = ports_notify_server_routine (inp)) ||
      (routine = fuse_fsys_server_routine (inp)) ||
      (routine = ports_interrupt_server_routine (inp)))
    {
      (*routine) (inp, outp);
      return TRUE;
    }

  return FALSE;
}
