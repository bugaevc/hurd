#include <hurd/ports.h>

#ifndef _BOOTSTRAP_H_
#define _BOOTSTRAP_H_

extern device_t bootstrap_mach_console;

extern int bootstrap_verbose;
extern int bootstrap_confirm;

typedef struct bootstrap_task
{
  struct port_info pi;
  task_t kernel_task;
  char *name;
  mach_port_t msgport;
} *bootstrap_task_t;

struct bootstrap_dir_entry
{
  char *name;
  /* strong ref */
  struct bootstrap_file *file;
};

typedef struct bootstrap_file
{
  struct port_info pi;
  /* weak ref */
  struct bootstrap_file *parent;
  fsys_t translator;

  struct bootstrap_dir_entry *children;
  size_t nchildren;
} *bootstrap_file_t;

extern struct port_class *bootstrap_task_class;
extern struct port_class *bootstrap_file_class;
extern struct port_bucket *bootstrap_bucket;

extern struct bootstrap_file *bootstrap_fs_root;
extern struct bootstrap_file *bootstrap_stdio;

#ifdef mig_unlikely

#define MIG_EOPNOTSUPP EOPNOTSUPP

static inline struct bootstrap_task * __attribute__ ((unused))
bootstrap_task_lookup (mach_port_t port)
{
  return ports_lookup_port (bootstrap_bucket, port, bootstrap_task_class);
}

static inline struct bootstrap_task * __attribute__ ((unused))
bootstrap_task_lookup_payload (unsigned long payload)
{
  return ports_lookup_payload (bootstrap_bucket, payload,
                               bootstrap_task_class);
}

static inline void __attribute__ ((unused))
bootstrap_task_deref (struct bootstrap_task *task)
{
  if (task)
    ports_port_deref (task);
}

static inline struct bootstrap_file * __attribute__ ((unused))
bootstrap_file_lookup (mach_port_t port)
{
  return ports_lookup_port (bootstrap_bucket, port, bootstrap_file_class);
}

static inline struct bootstrap_file * __attribute__ ((unused))
bootstrap_file_lookup_payload (unsigned long payload)
{
  return ports_lookup_payload (bootstrap_bucket, payload,
                               bootstrap_file_class);
}

static inline mach_port_t __attribute__ ((unused))
bootstrap_file_get_send_right (struct bootstrap_file *file)
{
  if (!file)
    return MACH_PORT_NULL;
  return ports_get_send_right (file);
}

static inline void __attribute__ ((unused))
bootstrap_file_deref (struct bootstrap_file *file)
{
  if (file)
    ports_port_deref (file);
}

#endif /* mig_unlikely */

#endif /* _BOOTSTRAP_H_ */
