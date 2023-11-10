#   Copyright (C) 2023
#     Free Software Foundation, Inc.
#
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation; either version 2, or (at
#   your option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with the GNU Hurd.  If not, see <http://www.gnu.org/licenses/>.

dir := libfuse
makemode := library
libname = libfuse

IOSRCS = io-read.c
FSYSSRCS = fsys-getroot.c
SESSIONSRCS = session-destroy.c session-new.c session-mount.c session-unmount.c session-loop.c
OTHERSRCS = demuxer.c info.c add-direntry.c
SRCS = $(OTHERSRCS) $(IOSRCS) $(FSYSSRCS) $(SESSIONSRCS)
installhdrs = fuse_common.h fuse_lowlevel.h fuse_opt.h

MIGSTUBS = fsServer.o ioServer.o fsysServer.o
OBJS = $(sort $(SRCS:.c=.o) $(MIGSTUBS))

HURDLIBS = fshelp iohelp ports shouldbeinlibc ihash
LDLIBS += -lpthread

CFLAGS = -fvisibility=hidden

fsys-MIGSFLAGS = -imacros $(srcdir)/fsmutations.h
fs-MIGSFLAGS = -imacros $(srcdir)/fsmutations.h
io-MIGSFLAGS = -imacros $(srcdir)/fsmutations.h
ifsock-MIGSFLAGS = -imacros $(srcdir)/fsmutations.h
# exec_startup-MIGSFLAGS = -imacros $(srcdir)/fsmutations.h
MIGCOMSFLAGS = -prefix fuse_

include ../Makeconf
