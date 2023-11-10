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

#define REPLY_PORTS

#define FILE_INTRAN protid_t begin_using_protid_port (file_t)
#define FILE_INTRAN_PAYLOAD protid_t begin_using_protid_payload
#define FILE_DESTRUCTOR end_using_protid_port (protid_t)

#define IO_INTRAN protid_t begin_using_protid_port (io_t)
#define IO_INTRAN_PAYLOAD protid_t begin_using_protid_payload
#define IO_DESTRUCTOR end_using_protid_port (protid_t)

#define FSYS_INTRAN session_t begin_using_session_port (fsys_t)
#define FSYS_INTRAN_PAYLOAD session_t begin_using_session_payload
#define FSYS_DESTRUCTOR end_using_session_port (session_t)

#define FILE_IMPORTS import "priv.h";
#define IO_IMPORTS import "priv.h";
#define FSYS_IMPORTS import "priv.h";
#define IFSOCK_IMPORTS import "priv.h";
