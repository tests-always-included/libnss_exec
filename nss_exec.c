#define NSS_EXEC_C
#include "nss_exec.h"
#include <math.h>
#include <limits.h>
#include <signal.h>

#define IS_WHITESPACE(c) ((c) == ' ' || (c) == '\t')
#define IS_FIELD_SEPARATOR(c) ((c) == ':' || (c) == '\0')

/**
 * Initialize a field_parse_info struct
 */
void field_parse_init(field_parse_info *info, char *buffer, size_t buffer_length, char *output) {
    memset(buffer, '\0', buffer_length);
    info->buffer_start = buffer;
    info->buffer_left = buffer_length;
    info->output = output;
    info->output_offset = 0;
    info->return_code = 0;
}


/**
 * Parse out a single string, copy it to the buffer and return a pointer
 * to where this string starts in the buffer.
 */
char *field_parse_string(field_parse_info *info) {
    size_t i, len;
    char *copy;

    // Take no action if we have already hit an error
    if (info->return_code) {
        return NULL;
    }

    // Determine length of data to copy
    i = info->output_offset;
    len = 0;

    while (! IS_FIELD_SEPARATOR(info->output[i])) {
        i ++;
        len ++;
    }

    // Confirm space is available in the buffer
    if (len + 1 > info->buffer_left) {
        info->return_code = -2;
        
        return NULL;
    }

    // Copy
    copy = info->buffer_start;
    i = info->output_offset;

    while (len --) {
        info->buffer_start[0] = info->output[i];
        info->buffer_start ++;
        i += 1;
    }

    // Null terminate the copy
    info->buffer_start[0] = '\0';
    info->buffer_start ++;

    // Update output_offset
    if (info->output[i] == ':') {
        i += 1;
    }

    info->output_offset = i;

    return copy;
}


/**
 * Take a string field and split it into separate strings, delimited by
 * spaces.  Then return a pointer to a list of pointers, like how one does
 * array things like this in C.
 */
char **field_parse_string_array(field_parse_info *info) {
    int i, field_count, fields_length, size;
    char **split, **return_value;

    field_count = 0;  // Number of elements in the array
    fields_length = 0;  // Total string size of array elements including nulls.
    i = info->output_offset;  // Generic character counter

    // Bypass initial whitespace
    while (IS_WHITESPACE(info->output[i])) {
        i += 1;
    }

    while (! IS_FIELD_SEPARATOR(info->output[i])) {
        field_count += 1;

        // Grab field
        while (! IS_FIELD_SEPARATOR(info->output[i]) && ! IS_WHITESPACE(info->output[i])) {
            fields_length += 1;
            i += 1;
        }

        fields_length += 1;  // For the eventual NULL byte

        // Bypass whitespace
        while (IS_WHITESPACE(info->output[i])) {
            i += 1;
        }
    }

    // Size is the amount of data used for the pointers to strings.
    size = (field_count + 1) * sizeof(char *);  // Add one for the null pointer

    if (fields_length + size > info->buffer_left) {
        info->return_code = -2;
        return NULL;
    }

    // Just take care of buffer_left right away
    info->buffer_left -= size + fields_length;

    // Allocate the array of pointers
    split = (char **) info->buffer_start;
    return_value = split;  // We move `split` forward
    info->buffer_start += size;
    i = info->output_offset;

    // Bypass initial whitespace
    while (IS_WHITESPACE(info->output[i])) {
        i += 1;
    }

    while (! IS_FIELD_SEPARATOR(info->output[i])) {
        *split = info->buffer_start;
        split ++;

        // Grab field
        while (! IS_FIELD_SEPARATOR(info->output[i]) && ! IS_WHITESPACE(info->output[i])) {
            info->buffer_start[0] = info->output[i];
            info->buffer_start += 1;
            i += 1;
        }

        info->buffer_start[0] = '\0';
        info->buffer_start += 1;

        // Bypass whitespace
        while (IS_WHITESPACE(info->output[i])) {
            i += 1;
        }
    }

    return return_value;
}


/**
 * Parse out a long integer and return the value.
 */
long field_parse_long(field_parse_info *info) {
    int i;
    long sign = 1, result = 0;

    // Take no action if we have already hit an error
    if (info->return_code) {
        return -1;
    }

    i = info->output_offset;

    // Detect a sign - not sure if it is valid in other parts of the system.
    if (info->output[i] == '-') {
        sign = -1;
        i ++;
    }

    // Consume all numbers
    while (! IS_FIELD_SEPARATOR(info->output[i]) && info->output[i] >= '0' && info->output[i] <= '9') {
        result *= 10;
        result += info->output[i] - '0';
        i ++;
    }

    // Skip letters, symbols, etc that are left in the field
    while (! IS_FIELD_SEPARATOR(info->output[i])) {
        i ++;
    }

    // Get past the colon
    if (info->output[i] == ':') {
        i ++;
    }

    info->output_offset = i;

    return result * sign;
}


/**
 * Returns a truthy value if there is data for another field (even if
 * that means the field is empty)
 */
int field_parse_more(field_parse_info *info) {
    // Check for errors
    if (info->return_code) {
        return 0;
    }

    // This field is empty, but we progress past
    if (info->output[info->output_offset] == ':') {
        info->output_offset += 1;
        return 0;
    }

    // This field is not empty
    if (info->output[info->output_offset] != '\0') {
        return 1;
    }

    // At NULL so there are no more fields
    return 0;
}


/**
 * Common function to change the return code and set errnop based on
 * field parsing results.
 */
enum nss_status handle_pack_result(int pack_result, int *errnop) {
    if (pack_result == -1) {
        if (errnop) {
            *errnop = ENOENT;
        }

        return NSS_STATUS_UNAVAIL;
    }

    if (pack_result != 0) {
        if (errnop) {
            *errnop = ERANGE;
        }

        return NSS_STATUS_TRYAGAIN;
    }

    return NSS_STATUS_SUCCESS;
}


/**
 * Execute a script.
 *
 * This version can pass additional data as a string.
 */
enum nss_status nss_exec_script(char **output, char *command_code, const char *data) {
    char command[1024];
    static char line[1024];
    FILE *fp;
    int i, resultCode;
    struct sigaction oldSignalAction;

    // Initialize
    if (output) {
        *output = NULL;
    }

    snprintf(command, 1024, "%s %s %s", NSS_EXEC_SCRIPT, command_code, data ? data : "");
    command[1024] = '\0'; // Ensure there's a null at the end

    fp = popen(command, "r");
    fgets(line, 1024, fp);
    sigaction(SIGCHLD, NULL, &oldSignalAction);
    signal(SIGCHLD, SIG_DFL);
    resultCode = WEXITSTATUS(pclose(fp));
    sigaction(SIGCHLD, &oldSignalAction, NULL);

    if (output) {
        // Cleanse out newlines and copy
        for (i = 0; line[i]; i += 1) {
            if (line[i] == '\n' || line[i] == '\r') {
                line[i] = '\0';
            }
        }

        *output = line;
    }

    if (resultCode == 0) {
        return NSS_STATUS_SUCCESS;
    }

    if (resultCode == 1) {
        return NSS_STATUS_NOTFOUND;
    }

    if (resultCode == 2) {
        return NSS_STATUS_TRYAGAIN;
    }

    return NSS_STATUS_UNAVAIL;
}


/**
 * Same as nss_exec_script, but this version takes a long integer and will
 * convert it to a string and then call the original function.
 */
enum nss_status nss_exec_script_long(char **output, char *command, long data) {
    char dataStr[(int) ceil(log10(ULONG_MAX)) + 1];

    sprintf(dataStr, "%ld", data);
    return nss_exec_script(output, command, dataStr);
}

