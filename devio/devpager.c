/* A pager interface for raw mach devices.

   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by Miles Bader <miles@gnu.ai.mit.edu>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <hurd.h>
#include <hurd/pager.h>
#include <device/device.h>
#include <assert.h>

#include "dev.h"

/* ---------------------------------------------------------------- */
/* Pager library callbacks; see <hurd/pager.h> for more info.  */

/* For pager PAGER, read one page from offset PAGE.  Set *BUF to be the
   address of the page, and set *WRITE_LOCK if the page must be provided
   read-only.  The only permissable error returns are EIO, EDQUOT, and
   ENOSPC. */
error_t
pager_read_page(struct user_pager_info *upi,
		vm_offset_t page, vm_address_t *buf, int *writelock)
{
  error_t err;
  int read;			/* bytes actually read */
  int want = vm_page_size;	/* bytes we want to read */
  struct dev *dev = (struct dev *)upi;

  if (page + want > dev->size)
    /* Read a partial page if necessary to avoid reading off the end.  */
    want = dev->size - page;

  err = device_read(dev->port, 0, page / dev->dev_block_size, want,
		    (io_buf_ptr_t *)buf, &read);

  if (!err && want < vm_page_size)
    /* Zero anything we didn't read.  Allocation only happens in page-size
       multiples, so we know we can write there.  */
    bzero((char *)*buf + want, vm_page_size - want);

  *writelock = (dev->flags & DEV_READONLY);

  if (err || read < want)
    return EIO;
  else
    return 0;
}

/* For pager PAGER, synchronously write one page from BUF to offset PAGE.  In
   addition, vm_deallocate (or equivalent) BUF.  The only permissable error
   returns are EIO, EDQUOT, and ENOSPC. */
error_t
pager_write_page(struct user_pager_info *upi,
		 vm_offset_t page, vm_address_t buf)
{
  struct dev *dev = (struct dev *)upi;

  if (dev->flags & DEV_READONLY)
    return EROFS;
  else
    {
      error_t err;
      int written;
      int want = vm_page_size;

      if (page + want > dev->size)
	/* Write a partial page if necessary to avoid reading off the end.  */
	want = dev->size - page;

      err = device_write(dev->port, 0, page / dev->dev_block_size,
			 (io_buf_ptr_t)buf, want, &written);

      vm_deallocate(mach_task_self(), buf, vm_page_size);

      if (err || written < want)
	return EIO;
      else
	return 0;
    }
}

/* A page should be made writable. */
error_t
pager_unlock_page(struct user_pager_info *upi, vm_offset_t address)
{
  struct dev *dev = (struct dev *)upi;

  if (dev->flags & DEV_READONLY)
    return EROFS;
  else
    return 0;
}

/* The user must define this function.  It should report back (in
   *OFFSET and *SIZE the minimum valid address the pager will accept
   and the size of the object.   */
error_t
pager_report_extent(struct user_pager_info *upi,
		    vm_address_t *offset, vm_size_t *size)
{
  *offset = 0;
  *size = ((struct dev *)upi)->size;
  return 0;
}

/* This is called when a pager is being deallocated after all extant send
   rights have been destroyed.  */
void
pager_clear_user_data(struct user_pager_info *upi)
{
}

/* ---------------------------------------------------------------- */

/* A top-level function for the paging thread that just services paging
   requests.  */
static void
service_paging_requests (any_t arg)
{
  struct dev *dev = (struct dev *)arg;
  for (;;)
    ports_manage_port_operations_multithread (dev->pager_port_bucket,
					      pager_demuxer,
					      1000 * 30, 1000 * 60 * 5,
					      1, MACH_PORT_NULL);
}

/* Initialize paging for this device.  */
static void
init_dev_paging (struct dev *dev)
{
  dev->pager_port_bucket = ports_create_bucket ();

  /* Make a thread to service paging requests.  */
  cthread_detach (cthread_fork ((cthread_fn_t)service_paging_requests,
				(any_t)dev));
}

void
pager_dropweak (struct user_pager_info *upi __attribute__ ((unused)))
{
}

/* ---------------------------------------------------------------- */

/* Try to stop all paging activity on DEV, returning true if we were
   successful.  If NOSYNC is true, then we won't write back any (kernel)
   cached pages to the device.  */
int
dev_stop_paging (struct dev *dev, int nosync)
{
  int success = 1;		/* Initially assume success.  */

  io_state_lock(&dev->io_state);

  if (dev->pager != NULL)
    {
      int num_pagers = ports_count_bucket (dev->pager_port_bucket);
      if (num_pagers > 0 && !nosync)
	{
	  error_t block_cache (void *arg)
	    {
	      struct pager *p = arg;
	      pager_change_attributes (p, 0, MEMORY_OBJECT_COPY_DELAY, 1);
	      return 0;
	    }
	  error_t enable_cache (void *arg)
	    {
	      struct pager *p = arg;
	      pager_change_attributes (p, 1, MEMORY_OBJECT_COPY_DELAY, 0);
	      return 0;
	    }

	  /* Loop through the pagers and turn off caching one by one,
	     synchronously.  That should cause termination of each pager. */
	  ports_bucket_iterate (dev->pager_port_bucket, block_cache);
      
	  /* Give it a second; the kernel doesn't actually shutdown
	     immediately.  XXX */
	  sleep (1);
      
	  num_pagers = ports_count_bucket (dev->pager_port_bucket);
	  if (num_pagers > 0)
	    /* Darn, there are actual honest users.  Turn caching back on,
	       and return failure. */
	    {
	      ports_bucket_iterate (dev->pager_port_bucket, enable_cache);
	      success = 0;
	    }
	}

      if (success && !nosync)
	/* shutdown the pager on DEV.  If NOSYNC is set, we don't bother, for
	   fear that this may result in I/O.  In this case we've disabled
	   rpcs on the pager's ports, so this will result in hanging...  What
	   do we do??? XXXX */
	pager_shutdown (dev->pager);
    }

  if (success)
    dev->pager = NULL;

  io_state_lock(&dev->io_state);

  return success;
}

/* ---------------------------------------------------------------- */

/* Returns in MEMOBJ the port for a memory object backed by the storage on
   DEV.  Returns 0 or the error code if an error occurred.  */
error_t
dev_get_memory_object(struct dev *dev, memory_object_t *memobj)
{
  if (dev_is(dev, DEV_SERIAL))
    return ENODEV;

  io_state_lock(&dev->io_state);
  if (dev->pager_port_bucket == NULL)
    init_dev_paging (dev);
  if (dev->pager == NULL)
    dev->pager =
      pager_create((struct user_pager_info *)dev, dev->pager_port_bucket,
		   1, MEMORY_OBJECT_COPY_DELAY);
  else
    ports_port_ref (dev->pager);
  io_state_unlock(&dev->io_state);

  if (dev->pager == NULL)
    return ENODEV;		/* XXX ??? */

  *memobj = pager_get_port(dev->pager);
  ports_port_deref (dev->pager); /* Drop our original ref on PAGER.  */

  if (*memobj != MACH_PORT_NULL)
    return
      mach_port_insert_right(mach_task_self(),
			     *memobj, *memobj,
			     MACH_MSG_TYPE_MAKE_SEND);

  return 0;
}
