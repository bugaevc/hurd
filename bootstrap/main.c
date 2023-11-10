#include <hurd.h>
#include <hurd/fd.h>
#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <version.h>
#include <assert-backtrace.h>
#include <device/device.h>
#include "bootstrap.h"

int bootstrap_verbose;
int bootstrap_confirm;

device_t bootstrap_mach_console;

const char *argp_program_version = STANDARD_HURD_VERSION (bootstrap);

enum
{
  OPT_DEVICE_MASTER = -1,
  OPT_HOST_PRIV = -2,
  OPT_VERBOSE = -3,
  OPT_CONFIRM = -4,
};

static struct argp_option
options[] =
{
  {"device-master-port", OPT_DEVICE_MASTER, "PORT"},
  {"host-priv-port", OPT_HOST_PRIV, "PORT"},
  {"verbose", OPT_VERBOSE},
  {"confirm", OPT_CONFIRM},
  {0}
};

static error_t
open_console (void)
{
  error_t err;

  /* If we're being run in a normal Hurd environment (perhaps to
     view --help), don't attempt to open the console.  */
  extern int _hurd_dtablesize;
  if (_hurd_dtablesize > 0)
    return 0;

  err = device_open (_hurd_device_master, D_READ|D_WRITE,
                     "console", &bootstrap_mach_console);
  if (err)
    return err;

  stdin = mach_open_devstream (bootstrap_mach_console, "r");
  stdout = stderr = mach_open_devstream (bootstrap_mach_console, "w");
  setbuf (stdout, NULL);

  return 0;
}

static int
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case OPT_DEVICE_MASTER:
      _hurd_device_master = atoi (arg);
      /* Open the Mach console as soon as possible, so we can output
         an error in case there is a mistake in following arguments.  */
      open_console ();
      break;

    case OPT_HOST_PRIV:
      _hurd_host_priv = atoi (arg);
      break;

    case OPT_VERBOSE:
      bootstrap_verbose = 1;
      break;

    case OPT_CONFIRM:
      bootstrap_confirm = 1;
      break;

    case ARGP_KEY_INIT:
    case ARGP_KEY_SUCCESS:
    case ARGP_KEY_ERROR:
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

int
main (int argc, char *argv[])
{
  struct argp argp = { options, parse_opt, 0, "Hurd bootstrap server" };
  argp_parse (&argp, argc, argv, 0, 0, 0);

  if (bootstrap_verbose)
    fprintf (stderr, "Hurd bootstrap server started\n");
}
