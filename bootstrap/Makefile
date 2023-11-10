#   Copyright (C) 2023 Free Software Foundation, Inc.
#
#   This file is part of the GNU Hurd.
#
#   The GNU Hurd is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation; either version 2, or (at
#   your option) any later version.
#
#   The GNU Hurd is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with the GNU Hurd.  If not, see http://www.gnu.org/licenses/

dir            = bootstrap
makemode       = server

SRCS           = main.c fs.c proc.c misc.c stubs.c exec_startupServer.o ioServer.o fsServer.o processServer.o termServer.c

MIGSRCS        =
OBJS           = $(patsubst %.S,%.o,$(patsubst %.c,%.o, $(SRCS) $(MIGSRCS)))

HURDLIBS = fshelp ports shouldbeinlibc

target = boostrap.static

MIGSFLAGS = -imacros $(srcdir)/mig-mutate.h

include ../Makeconf
