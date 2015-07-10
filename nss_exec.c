#define NSS_EXEC_C
#include "nss_exec.h"
#include <math.h>
#include <limits.h>

#define IS_WHITESPACE(c) ((c) == ' ' || (c) == '\t')
#define IS_FIELD_SEPARATOR(c) ((c) == ':' || (c) == '\0')

/**
 * Initialize a field_parse_info struct
 */
void field_parse_init(field_parse_info *info, char *buffer, size_t bufferLength, char *output) {
    memset(buffer, '\0', bufferLength);
    info->bufferStart = buffer;
    info->bufferLeft = bufferLength;
    info->output = output;
    info->outputOffset = 0;
    info->returnCode = 0;
}


/**
 * Parse out a single string, copy it to the buffer and return a pointer
 * to where this string starts in the buffer.
 */
char *field_parse_string(field_parse_info *info) {
    size_t i, len;
    char *copy;

    // Take no action if we have already hit an error
    if (info->returnCode) {
        return NULL;
    }

    // Determine length of data to copy
    i = info->outputOffset;
    len = 0;

    while (! IS_FIELD_SEPARATOR(info->output[i])) {
        i ++;
        len ++;
    }

    // Confirm space is available in the buffer
    if (len + 1 > info->bufferLeft) {
        info->returnCode = -2;
        return NULL;
    }

    // Copy
    copy = info->bufferStart;
    i = info->outputOffset;

    while (len --) {
        info->bufferStart[0] = info->output[i];
        info->bufferStart ++;
        i += 1;
    }

    // Null terminate the copy
    info->bufferStart[0] = '\0';
    info->bufferStart ++;

    // Update outputOffset
    if (info->output[i] == ':') {
        i += 1;
    }

    info->outputOffset = i;

    return copy;
}


/**
 * Take a string field and split it into separate strings, delimited by
 * spaces.  Then return a pointer to a list of pointers, like how one does
 * array things like this in C.
 */
char **field_parse_string_array(field_parse_info *info) {
    int i, fieldCount, fieldsLength, size;
    char **split, **returnValue;

    fieldCount = 0;  // Number of elements in the array
    fieldsLength = 0;  // Total string size of array elements including nulls.
    i = info->outputOffset;  // Generic character counter

    // Bypass initial whitespace
    while (IS_WHITESPACE(info->output[i])) {
        i += 1;
    }

    while (! IS_FIELD_SEPARATOR(info->output[i])) {
        fieldCount += 1;

        // Grab field
        while (! IS_FIELD_SEPARATOR(info->output[i]) && ! IS_WHITESPACE(info->output[i])) {
            fieldsLength += 1;
            i += 1;
        }

        fieldsLength += 1;  // For the eventual NULL byte

        // Bypass whitespace
        while (IS_WHITESPACE(info->output[i])) {
            i += 1;
        }
    }

    // Size is the amount of data used for the pointers to strings.
    size = (fieldCount + 1) * sizeof(char *);  // Add one for the null pointer

    if (fieldsLength + size > info->bufferLeft) {
        info->returnCode = -2;
        return NULL;
    }

    // Just take care of bufferLeft right away
    info->bufferLeft -= size + fieldsLength;

    // Allocate the array of pointers
    split = (char **) info->bufferStart;
    returnValue = split;  // We move `split` forward
    info->bufferStart += size;
    i = info->outputOffset;

    // Bypass initial whitespace
    while (IS_WHITESPACE(info->output[i])) {
        i += 1;
    }

    while (! IS_FIELD_SEPARATOR(info->output[i])) {
        *split = info->bufferStart;
        split ++;

        // Grab field
        while (! IS_FIELD_SEPARATOR(info->output[i]) && ! IS_WHITESPACE(info->output[i])) {
            info->bufferStart[0] = info->output[i];
            i += 1;
            info->bufferStart += 1;
            i += 1;
        }

        info->bufferStart[0] = '\0';
        info->bufferStart += 1;

        // Bypass whitespace
        while (IS_WHITESPACE(info->output[i])) {
            i += 1;
        }
    }

    return returnValue;
}


/**
 * Parse out a long integer and return the value.
 */
long field_parse_long(field_parse_info *info) {
    int i;
    long sign = 1, result = 0;

    // Take no action if we have already hit an error
    if (info->returnCode) {
        return -1;
    }

    i = info->outputOffset;

    if (info->output[i] == '-') {
        sign = -1;
        i ++;
    }

    while (! IS_FIELD_SEPARATOR(info->output[i]) && info->output[i] >= '0' && info->output[i] <= '9') {
        result *= 10;
        result += info->output[i] - '0';
        i ++;
    }

    while (! IS_FIELD_SEPARATOR(info->output[i])) {
        i ++;
    }

    info->outputOffset = i;

    return result * sign;
}


/**
 * Returns a truthy value if there is data for another field (even if
 * that means the field is empty)
 */
int field_parse_more(field_parse_info *info) {
    // Check for errors
    if (info->returnCode) {
        return 0;
    }

    // This field is empty, but we progress past
    if (info->output[info->outputOffset] == ':') {
        info->outputOffset += 1;
        return 0;
    }

    // This field is not empty
    if (info->output[info->outputOffset] != '\0') {
        return 1;
    }

    // At NULL so there are no more fields
    return 0;
}


/**
 * Common function to change the return code and set errnop based on
 * field parsing results.
 */
enum nss_status handle_pack_result(int packResult, int *errnop) {
    if (packResult == -1) {
        if (errnop) {
            *errnop = ENOENT;
        }

        return NSS_STATUS_UNAVAIL;
    }

    if (packResult != 0) {
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
enum nss_status nss_exec_script(char **output, char *commandCode, const char *data) {
    char command[1024];
    char line[1024];
    FILE *fp;
    int i, resultCode;

    // Initialize
    if (output) {
        *output = NULL;
    }

    snprintf(command, 1024, "%s %s %s", NSS_EXEC_SCRIPT, commandCode, data ? data : "");
    command[1024] = '\0'; // Ensure there's a null at the end

    fp = popen(NSS_EXEC_SCRIPT, "r");
    fgets(line, 1024, fp);
    resultCode = WEXITSTATUS(pclose(fp));

    if (output) {
        // Cleanse out newlines
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
    return nss_exec_script(output, "setspent", dataStr);
}

