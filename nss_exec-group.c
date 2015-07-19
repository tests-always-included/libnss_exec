#include "nss_exec.h"

static int ent_index = 0;
static pthread_mutex_t NSS_EXEC_MUTEX = PTHREAD_MUTEX_INITIALIZER;


static int pack_group_struct(struct group *result, char *buffer, size_t buffer_length, char *output) {
    field_parse_info info;

    field_parse_init(&info, buffer, buffer_length, output);
    result->gr_name = field_parse_string(&info);
    result->gr_passwd = field_parse_string(&info);
    result->gr_gid = field_parse_long(&info);
    result->gr_mem = field_parse_string_array(&info);
    
    return info.return_code;
}


enum nss_status _nss_exec_setgrent_locked(int stayopen) {
    enum nss_status result_status;

    result_status = nss_exec_script(NULL, "setgrent", NULL);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index = 0;
    }

    return result_status;
}


// Called to open the group file
enum nss_status _nss_exec_setgrent(int stayopen) {
    enum nss_status ret;
    
    NSS_EXEC_LOCK();
    ret = _nss_exec_setgrent_locked(stayopen);
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_endgrent_locked(void) {
    enum nss_status result_status;

    result_status = nss_exec_script(NULL, "endgrent", NULL);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index = 0;
    }

    return result_status;
}


// Called to close the group file
enum nss_status _nss_exec_endgrent(void) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_endgrent_locked();
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_getgrent_r_locked(struct group *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status result_status;
    char *output;
    int pack_result;

    result_status = nss_exec_script_long(&output, "getgrent", ent_index);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index += 1;
        pack_result = pack_group_struct(result, buffer, buffer_length, output);
        result_status = handle_pack_result(pack_result, errnop);
    }

    return result_status;
}


// Called to look up next entry in group file
enum nss_status _nss_exec_getgrent_r(struct group *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_getgrent_r_locked(result, buffer, buffer_length, errnop);
    NSS_EXEC_UNLOCK();

    return ret;
}


// Find a group by gid
enum nss_status _nss_exec_getgrgid_r_locked(gid_t gid, struct group *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status result_status;
    char *output;
    int pack_result;

    result_status = nss_exec_script_long(&output, "getgrgid", gid);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index += 1;
        pack_result = pack_group_struct(result, buffer, buffer_length, output);
        result_status = handle_pack_result(pack_result, errnop);
    }

    return result_status;
}


enum nss_status _nss_exec_getgrgid_r(gid_t gid, struct group *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_getgrgid_r_locked(gid, result, buffer, buffer_length, errnop);
    NSS_EXEC_UNLOCK();

    return ret;
}


enum nss_status _nss_exec_getgrnam_r_locked(const char *name, struct group *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status result_status;
    char *output;
    int pack_result;

    result_status = nss_exec_script(&output, "getgrnam", name);

    if (result_status == NSS_STATUS_SUCCESS) {
        ent_index += 1;
        pack_result = pack_group_struct(result, buffer, buffer_length, output);
        result_status = handle_pack_result(pack_result, errnop);
    }

    return result_status;
}


// Find a group by name
enum nss_status _nss_exec_getgrnam_r(const char *name, struct group *result, char *buffer, size_t buffer_length, int *errnop) {
    enum nss_status ret;

    NSS_EXEC_LOCK();
    ret = _nss_exec_getgrnam_r_locked(name, result, buffer, buffer_length, errnop);
    NSS_EXEC_UNLOCK();

    return ret;
}

