#define EXEC_STARTUP_INTRAN bootstrap_task_t bootstrap_task_lookup (mach_port_t)
#define EXEC_STARTUP_INTRAN_PAYLOAD bootstrap_task_t bootstrap_task_lookup_payload
#define EXEC_STARTUP_DESTRUCTOR bootstrap_task_deref (bootstrap_task_t)
#define EXEC_STARTUP_IMPORTS import "bootstrap.h";

#define PROCESS_INTRAN bootstrap_task_t bootstrap_task_lookup (mach_port_t)
#define PROCESS_INTRAN_PAYLOAD bootstrap_task_t bootstrap_task_lookup_payload
#define PROCESS_DESTRUCTOR bootstrap_task_deref (bootstrap_task_t)
#define PROCESS_IMPORTS import "bootstrap.h";

#define IO_INTRAN bootstrap_file_t bootstrap_file_lookup (mach_port_t)
#define IO_INTRAN_PAYLOAD bootstrap_file_t bootstrap_file_lookup_payload
#define IO_OUTTRAN mach_port_t bootstrap_file_get_send_right (bootstrap_file_t)
#define IO_DESTRUCTOR bootstrap_file_deref (bootstrap_file_t)
#define IO_IMPORTS import "bootstrap.h";

#define FILE_INTRAN bootstrap_file_t bootstrap_file_lookup (mach_port_t)
#define FILE_INTRAN_PAYLOAD bootstrap_file_t bootstrap_file_lookup_payload
#define FILE_OUTTRAN mach_port_t bootstrap_file_get_send_right (bootstrap_file_t)
#define FILE_DESTRUCTOR bootstrap_file_deref (bootstrap_file_t)
#define FILE_IMPORTS import "bootstrap.h";

#define TERM_INTRAN bootstrap_file_t bootstrap_file_lookup (mach_port_t)
#define TERM_INTRAN_PAYLOAD bootstrap_file_t bootstrap_file_lookup_payload
#define TERM_DESTRUCTOR bootstrap_file_deref (bootstrap_file_t)
#define TERM_IMPORTS import "bootstrap.h";
