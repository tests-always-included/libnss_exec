#include "nss_exec.h"

static long ent_index = 0;
static pthread_mutex_t NSS_EXEC_MUTEX = PTHREAD_MUTEX_INITIALIZER;

static int pack_shadow_struct(struct spwd *result, char *buffer, size_t buffer_length, char *output) {
    field_parse_info info;

    field_parse_init(&info, buffer, buffer_length, output);
    result->sp_namp = field_parse_string(&info);
    result->sp_pwdp = field_parse_string(&info);
    result->sp_lstchg = field_parse_long(&info);
    result->sp_min = field_parse_long(&info);
    result->sp_max = field_parse_long(&info);
    result->sp_warn = field_parse_long(&info);

    // Set defaults for integers because they parse differently
    result->sp_inact = -1;
    result->sp_expire = -1;
    result->sp_flag = ~0ul;

    if (field_parse_more(&info)) {
        result->sp_inact = field_parse_long(&info);
    }

    if (field_parse_more(&info)) {
        result->sp_expire = field_parse_long(&info);
    }

    if (field_parse_more(&info)) {
        result->sp_flag = field_parse_long(&info);
    }

    return info.return_code;
}

enum nss_status _nss_exec_setspent_locked(int stayopen) {
    enum nss_status result_status;
   
    result_status = nss_exec_script(NULL, "setspent", NULL);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index = 0;
    }

    return result_status;
}


// Called to open the shadow file
enum nss_status _nss_exec_setspent(int stayopen) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_setspent_locked(stayopen);
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_endspent_locked(void) {
    enum nss_status result_status;
   
    result_status = nss_exec_script(NULL, "endspent", NULL);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index = 0;
    }

    return result_status;
}


// Called to close the shadow file
enum nss_status _nss_exec_endspent(void) {
    enum nss_status ret;
    
    NSS_EXEC_LOCK();
    ret = _nss_exec_endspent_locked();
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_getspent_r_locked(struct spwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status result_status;
    char *output;
    int pack_result;

    result_status = nss_exec_script_long(&output, "getspent", ent_index);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index += 1;
        pack_result = pack_shadow_struct(result, buffer, buffer_length, output);
        result_status = handle_pack_result(pack_result, errnop);
    }

    return result_status;
}


// Called to look up next entry in shadow file
enum nss_status _nss_exec_getspent_r(struct spwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_getspent_r_locked(result, buffer, buffer_length, errnop);
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_getspnam_r_locked(const char *name, struct spwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status result_status;
    char *output;
    int pack_result;

    result_status = nss_exec_script(&output, "getspnam", name);

    if (result_status == NSS_STATUS_SUCCESS) {
        pack_result = pack_shadow_struct(result, buffer, buffer_length, output);
        result_status = handle_pack_result(pack_result, errnop);

        if (pack_result) {
            result_status = NSS_STATUS_UNAVAIL;
        }
    }

    return result_status;
}


// Find a shadow by name
enum nss_status _nss_exec_getspnam_r(const char *name, struct spwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_getspnam_r_locked(name, result, buffer, buffer_length, errnop);
    NSS_EXEC_UNLOCK();

    return ret;
}

