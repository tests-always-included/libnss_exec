#ifndef NSS_EXEC_H
#define NSS_EXEC_H

#include <errno.h>
#include <grp.h>
#include <nss.h>
#include <pthread.h>
#include <pwd.h>
#include <shadow.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NSS_EXEC_LOCK()    do { pthread_mutex_lock(&NSS_EXEC_MUTEX); } while (0)
#define NSS_EXEC_UNLOCK()  do { pthread_mutex_unlock(&NSS_EXEC_MUTEX); } while (0)
#ifndef NSS_EXEC_SCRIPT
#define NSS_EXEC_SCRIPT "/sbin/nss_exec"
#endif

typedef struct {
    char *bufferStart;  // Where the NSS buffer's free space starts (changes)
    size_t bufferLeft;  // Number of bytes left in the buffer (changes)
    char *output;  // Starting point of unparsed output (static)
    size_t outputOffset;  // What point we are at in the output (changes)
    int returnCode;  // The return code after processing all fields
} field_parse_info;


#ifdef NSS_EXEC_C
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN void field_parse_init(field_parse_info *info, char *buffer, size_t bufferLength, char *output);
EXTERN char *field_parse_string(field_parse_info *info);
EXTERN char **field_parse_string_array(field_parse_info *info);
EXTERN long field_parse_long(field_parse_info *info);
EXTERN int field_parse_more(field_parse_info *info);
EXTERN enum nss_status handle_pack_result(int packResult, int *errnop);
EXTERN enum nss_status nss_exec_script(char **output, char *command, const char *data);
EXTERN enum nss_status nss_exec_script_long(char **output, char *command, long data);

#endif /* NSS_EXEC_H */
