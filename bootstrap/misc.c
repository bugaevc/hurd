#include <stdio.h>

#include "exec_startup_S.h"
#include "bootstrap.h"

struct port_class *bootstrap_task_class;
struct port_bucket *bootstrap_bucket;

kern_return_t
S_exec_startup_get_info (struct bootstrap_task *task,
                         vm_address_t *user_entry,
                         vm_address_t *phdr_data,
                         vm_size_t *phdr_size,
                         vm_address_t *stack_base,
                         vm_size_t *stack_size,
                         int *flags,
                         data_t *argv,
                         mach_msg_type_number_t *argvlen,
                         data_t *envp,
                         mach_msg_type_number_t *envplen,
                         mach_port_t **dtableP,
                         mach_msg_type_name_t *dtablePoly,
                         mach_msg_type_number_t *dtableCnt,
                         mach_port_t **portarrayP,
                         mach_msg_type_name_t *portarrayPoly,
                         mach_msg_type_number_t *portarrayCnt,
                         int **intarrayP,
                         mach_msg_type_number_t *intarrayCnt)
{
  mach_port_t *dtable = *dtableP;
  mach_port_t *portarray = *portarrayP;

  if (!task)
    {
      if (bootstrap_verbose)
        fprintf (stderr, "Received S_exec_startup_get_info with a "
                         "NULL task, refusing\n");
      return EOPNOTSUPP;
    }

  if (bootstrap_verbose || bootstrap_confirm)
    fprintf (stderr, "Received S_exec_startup_get_info from %s\n",
             task->name);
  if (bootstrap_confirm)
    {
      fprintf (stderr, "Press return to continue...\n");
      getchar ();
    }

  *user_entry = 0;
  *phdr_data = *stack_base = 0;
  *phdr_size = *stack_size = 0;

  /* We have no args for it.  Tell it to look on its stack
     for the args placed there by the boot loader.  */
  *argvlen = *envplen = 0;
  *flags = EXEC_STACK_ARGS;

  assert_backtrace (*portarrayCnt >= INIT_PORT_MAX);
  *portarrayCnt = INIT_PORT_MAX;
  /* Provide everyone with root and current directories pointing
     to our bootstrap FS.  */
  portarray[INIT_PORT_CWDIR] = ports_get_right (bootstrap_fs_root);
  portarray[INIT_PORT_CRDIR] = ports_get_right (bootstrap_fs_root);
  /* No auth yet.  */
  portarray[INIT_PORT_AUTH] = MACH_PORT_NULL;
  /* The same task port as its proc and bootstrap ports.  */
  portarray[INIT_PORT_PROC] = ports_get_right (task);
  portarray[INIT_PORT_BOOTSTRAP] = ports_get_right (task);
  *portarrayPoly = MACH_MSG_TYPE_MAKE_SEND;

  /* No interesting ints to pass, let the client use their defaults.  */
  *intarrayCnt = 0;

  assert_backtrace (*dtableCnt >= 3);
  *dtableCnt = 3;
  dtable[0] = ports_get_right (bootstrap_stdio);
  dtable[1] = ports_get_right (bootstrap_stdio);
  dtable[2] = ports_get_right (bootstrap_stdio);
  *dtablePoly = MACH_MSG_TYPE_MAKE_SEND;

  return 0;
}
