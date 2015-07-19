#include "nss_exec.h"

static long ent_index = 0;
static pthread_mutex_t NSS_EXEC_MUTEX = PTHREAD_MUTEX_INITIALIZER;


static int pack_passwd_struct(struct passwd *result, char *buffer, size_t buffer_length, char *output) {
    field_parse_info info;

    field_parse_init(&info, buffer, buffer_length, output);
    result->pw_name = field_parse_string(&info);
    result->pw_passwd = field_parse_string(&info);
    result->pw_uid = field_parse_long(&info);
    result->pw_gid = field_parse_long(&info);
    result->pw_gecos = field_parse_string(&info);
    result->pw_dir = field_parse_string(&info);
    result->pw_shell = field_parse_string(&info);

    return info.return_code;
}


enum nss_status _nss_exec_setpwent_locked(int stayopen) {
    enum nss_status result_status;

    result_status = nss_exec_script(NULL, "setpwent", NULL);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index = 0;
    }

    return result_status;
}


// Called to open the passwd file
enum nss_status _nss_exec_setpwent(int stayopen) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_setpwent_locked(stayopen);
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_endpwent_locked(void) {
    enum nss_status result_status;

    result_status = nss_exec_script_long(NULL, "endpwent", ent_index);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index = 0;
    }

    return result_status;
}


// Called to close the passwd file
enum nss_status _nss_exec_endpwent(void) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_endpwent_locked();
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_getpwent_r_locked(struct passwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status result_status;
    char *output;
    int pack_result;

    result_status = nss_exec_script_long(&output, "getpwent", ent_index);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index += 1;
        pack_result = pack_passwd_struct(result, buffer, buffer_length, output);
        result_status = handle_pack_result(pack_result, errnop);
    }

    return result_status;
}


// Called to look up next entry in passwd file
enum nss_status _nss_exec_getpwent_r(struct passwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_getpwent_r_locked(result, buffer, buffer_length, errnop);
    NSS_EXEC_UNLOCK();

    return ret;
}


// Find a passwd by uid
enum nss_status _nss_exec_getpwuid_r_locked(uid_t uid, struct passwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status result_status;
    char *output;
    int pack_result;

    result_status = nss_exec_script_long(&output, "getpwuid", uid);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index += 1;
        pack_result = pack_passwd_struct(result, buffer, buffer_length, output);
        result_status = handle_pack_result(pack_result, errnop);
    }

    return result_status;
}


enum nss_status _nss_exec_getpwuid_r(uid_t uid, struct passwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_getpwuid_r_locked(uid, result, buffer, buffer_length, errnop);
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_getpwnam_r_locked(const char *name, struct passwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status result_status;
    char *output;
    int pack_result;

    result_status = nss_exec_script(&output, "getpwnam", name);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index += 1;
        pack_result = pack_passwd_struct(result, buffer, buffer_length, output);
        result_status = handle_pack_result(pack_result, errnop);
    }

    return result_status;
}


// Find a passwd by name
enum nss_status _nss_exec_getpwnam_r(const char *name, struct passwd *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_getpwnam_r_locked(name, result, buffer, buffer_length, errnop);
    NSS_EXEC_UNLOCK();

    return ret;
}

