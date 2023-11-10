#include <stdio.h>
#include <errno.h>
#include <mach.h>

#include "bootstrap.h"
#include "process_S.h"

error_t
S_proc_getprivports (struct bootstrap_task *task,
                     mach_port_t *host_priv,
                     mach_port_t *device_master)
{
  if (!task)
    return EOPNOTSUPP;

  if (bootstrap_verbose)
    fprintf (stderr, "S_proc_getprivports from %s\n", task->name);

  *host_priv = _hurd_host_priv;
  *device_master = _hurd_device_master;

  return 0;
}

error_t
S_proc_setmsgport (struct bootstrap_task *task,
                   mach_port_t reply_port,
                   mach_msg_type_name_t reply_portPoly,
                   mach_port_t newmsgport,
                   mach_port_t *oldmsgport,
                   mach_msg_type_name_t *oldmsgportPoly)
{
  if (!task)
    return EOPNOTSUPP;

  if (bootstrap_verbose)
    fprintf (stderr, "S_proc_setmsgport for %s\n", task->name);

  *oldmsgport = task->msgport;
  *oldmsgportPoly = MACH_MSG_TYPE_MOVE_SEND;

  task->msgport = newmsgport;

  return 0;
}
