#include <stdio.h>
#include <string.h>
#include <hurd/ports.h>
#include <hurd/fsys.h>
#include <device/device.h>

#include "bootstrap.h"
#include "io_S.h"
#include "term_S.h"

struct port_class *bootstrap_file_class;

struct bootstrap_file *bootstrap_stdio;
struct bootstrap_file *bootstrap_fs_root;

error_t
S_io_read (struct bootstrap_file *file,
           char **data,
           mach_msg_type_number_t *dataCnt,
           loff_t offset,
           vm_size_t amount)
{
  if (!file)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_io_read on NULL file\n");
      return EOPNOTSUPP;
    }

  if (file != bootstrap_stdio)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_io_read on an invalid file\n");
      return EOPNOTSUPP;
    }

  if (offset != -1)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_io_read with offset = %lld\n",
                 (long long) offset);
      return EINVAL;
    }

  if (amount == 0)
    {
      *data = NULL;
      *dataCnt = 0;
      return 0;
    }

  /* Use the semi-private glibc libio API.  */
  if (stdin->_IO_read_ptr >= stdin->_IO_read_end)
    {
      int c = getc (stdin);
      ungetc (c, stdin);
    }

  if (amount < *dataCnt)
    *dataCnt = amount;
  if (stdin->_IO_read_end - stdin->_IO_read_ptr < *dataCnt)
    *dataCnt = stdin->_IO_read_end - stdin->_IO_read_ptr;

  memcpy (*data, stdin->_IO_read_ptr, *dataCnt);
  stdin->_IO_read_ptr += *dataCnt;

  return 0;
}

error_t
S_io_write (struct bootstrap_file *file,
            const char *data,
            mach_msg_type_number_t dataCnt,
            loff_t offset,
            vm_size_t *amount)
{
  if (!file)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_io_write on NULL file\n");
      return EOPNOTSUPP;
    }

  if (file != bootstrap_stdio)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_io_write on an invalid file\n");
      return EOPNOTSUPP;
    }

  if (offset != -1)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_io_write with offset = %lld\n",
                 (long long) offset);
      return EINVAL;
    }

  *amount = fwrite (data, 1, dataCnt, stdout);

  return 0;
}

kern_return_t
S_io_stat (struct bootstrap_file *file,
           struct stat *st)
{
  if (!file)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_io_stat on NULL file\n");
      return EOPNOTSUPP;
    }

  // TODO: root, etc
  if (file != bootstrap_stdio)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_io_write on an invalid file\n");
      return EOPNOTSUPP;
    }

  memset (st, 0, sizeof (struct stat));
  st->st_blksize = 512;
  st->st_fstype = FSTYPE_DEV;
  /* Report ourselves as a character device so glibc recognizes
     us as possibly being a tty.  */
  st->st_mode = S_IFCHR | 0600;
  return 0;
}

error_t
S_term_getctty (struct bootstrap_file *file,
                mach_port_t *ctty,
                mach_msg_type_name_t *cttyPoly)
{
  if (!file)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_term_getctty on NULL file\n");
      return EOPNOTSUPP;
    }

  if (file != bootstrap_stdio)
    return EOPNOTSUPP;

  /* bootstrap_stdio has to respond to S_term_getctty in order
     for isatty to return true.  But it doesn't actually have
     to return a meaningful ctty port.  */
  *ctty = MACH_PORT_NULL;
  *cttyPoly = MACH_MSG_TYPE_COPY_SEND;

  return 0;
}

error_t
S_dir_lookup (struct bootstrap_file *start_dir,
              const char *file_name,
              int flags, mode_t mode,
              retry_type *do_retry, char *retry_name,
              mach_port_t *result, mach_msg_type_name_t *resultPoly)
{
  error_t err;
  struct bootstrap_file *file = start_dir;
  struct bootstrap_dir_entry *entry;
  const char *part = file_name, *part_end;
  size_t part_len;
  int lastcomp;

  if (bootstrap_verbose)
    fprintf (stderr, "Received request for file %s\n", file_name);

  while (1)
    {
      part_end = strchrnul (part, '/');
      part_len = part_end - part;
      lastcomp = !*part_end;
      if (part_len == 0 || (part_len == 1 && part[0] == '.'))
        goto found;
      if (part_len == 2 && part[0] == '.' && part[1] == '.')
        {
          file = file->parent;
          goto found;
        }
      for (entry = file->children; entry != file->children + file->nchildren; entry++)
        if (!strncmp (part, entry->name, part_len))
          {
            file = entry->file;
            goto found;
          }

      fprintf (stderr, "No such file %s, the missing part was %*s\n",
               file_name, part_len, part);
      return ENOENT;

     found:
      /* If this file is translated, ask the translator for its root.  */
      if (file->translator)
        {
          uid_t root = 0;
          err = fsys_getroot (file->translator,
                              ports_get_right (file->parent),
                              MACH_MSG_TYPE_MAKE_SEND,
                              &root, 1, &root, 1, flags,
                              do_retry, retry_name, result);
          /* If this is not the last path component, append the rest
             to the retry name.  This is essentially strlcpy ().  */
          if (!err && !lastcomp)
            {
              size_t retry_name_len = strlen (retry_name);
              strncpy (retry_name + retry_name_len, part_end,
                       sizeof (string_t) - retry_name_len);
            }
          *resultPoly = MACH_MSG_TYPE_MOVE_SEND;
          return err;
        }

      if (lastcomp)
        {
          retry_name[0] = 0;
          *do_retry = FS_RETRY_NORMAL;
          *result = ports_get_right (file);
          *resultPoly = MACH_MSG_TYPE_MAKE_SEND;
          return 0;
        }

      /* Otherwise, keep locking.  */
      part = part_end + 1;
    }
}
