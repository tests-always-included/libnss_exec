#include <stdio.h>
#include "nss_exec.h"

#define BUFFER_LENGTH 2048

extern enum nss_status _nss_exec_endgrent(void);
extern enum nss_status _nss_exec_endpwent(void);
extern enum nss_status _nss_exec_endspent(void);
extern enum nss_status _nss_exec_getgrent_r(struct group *result, char *buffer, size_t buffer_length, int *errnop);
extern enum nss_status _nss_exec_getgrgid_r(gid_t gid, struct group *result, char *buffer, size_t buffer_length, int *errnop);
extern enum nss_status _nss_exec_getgrnam_r(const char *name, struct group *result, char *buffer, size_t buffer_length, int *errnop);
extern enum nss_status _nss_exec_getpwent_r(struct passwd *result, char *buffer, size_t buffer_length, int *errnop);
extern enum nss_status _nss_exec_getpwnam_r(const char *name, struct passwd *result, char *buffer, size_t buffer_length, int *errnop);
extern enum nss_status _nss_exec_getpwuid_r(uid_t uid, struct passwd *result, char *buffer, size_t buffer_length, int *errnop);
extern enum nss_status _nss_exec_getspent_r(struct spwd *result, char *buffer, size_t buffer_length, int *errnop);
extern enum nss_status _nss_exec_getspnam_r(const char *name, struct spwd *result, char *buffer, size_t buffer_length, int *errnop);
extern enum nss_status _nss_exec_setgrent(int stayopen);
extern enum nss_status _nss_exec_setpwent(int stayopen);
extern enum nss_status _nss_exec_setspent(int stayopen);

typedef struct {
    int argc;
    char **argv;
    char *buffer;
    int buffer_length;
} call_params;

typedef struct {
    char *name;
    int (*fn)(call_params *);
} function_map;

int show_nssgetent_status(enum nss_status status, int err) {
    printf("errno: %d\n", err);

    if (status == 0) {
        printf("SUCCESS (%d)\n", (int) status);
        return 0;
    }

    if (status == ENOENT) {
        printf("ENOENT (%d)\n", (int) status);
    } else if (status == ERANGE) {
        printf("ERANGE (%d)\n", (int) status);
    } else {
        printf("Unknown status (%d)\n", (int) status);
    }

    return 1;
}

int show_nss_status(enum nss_status status, int err) {
    printf("errno: %d\n", err);

    if (status == NSS_STATUS_SUCCESS) {
        printf("NSS_STATUS_SUCCESS (%d)\n", (int) status);
        return 0;
    }

    if (status == NSS_STATUS_NOTFOUND) {
        printf("NSS_STATUS_NOTFOUND (%d)\n", (int) status);
    } else if (status == NSS_STATUS_TRYAGAIN) {
        printf("NSS_STATUS_TRYAGAIN (%d)\n", (int) status);
    } else if (status == NSS_STATUS_UNAVAIL) {
        printf("NSS_STATUS_UNAVAIL (%d)\n", (int) status);
    } else if (status == NSS_STATUS_RETURN) {
        printf("NSS_STATUS_RETURN (%d)\n", (int) status);
    } else {
        printf("Unknown status (%d)\n", (int) status);
    }

    return 1;
}

void show_group(struct group *grent) {
    int i;

    printf("group.gr_name: %s\n", grent->gr_name);
    printf("group.gr_passwd: %s\n", grent->gr_passwd);
    printf("group.gr_gid: %ld\n", (long) grent->gr_gid);
    i = 0;

    if (grent->gr_mem) {
        while (grent->gr_mem[i]) {
            printf("group.gr_mem[%d]: %s\n", i, grent->gr_mem[i]);
            i += 1;
        }
    }
}

void show_passwd(struct passwd *pwent) {
    printf("passwd.pw_name: %s\n", pwent->pw_name);
    printf("passwd.pw_passwd: %s\n", pwent->pw_passwd);
    printf("passwd.pw_uid: %ld\n", (long) pwent->pw_uid);
    printf("passwd.pw_gid: %ld\n", (long) pwent->pw_gid);
    printf("passwd.pw_gecos: %s\n", pwent->pw_gecos);
    printf("passwd.pw_dir: %s\n", pwent->pw_dir); 
    printf("passwd.pw_shell: %s\n", pwent->pw_shell);
}

void show_spwd(struct spwd *spent) {
    printf("spwd.pw_name: %s\n", spent->sp_namp);
    printf("spwd.pw_spwd: %s\n", spent->sp_pwdp);
    printf("spwd.sp_lstchg: %ld\n", spent->sp_lstchg);
    printf("spwd.sp_min: %ld\n", spent->sp_min);
    printf("spwd.sp_max: %ld\n", spent->sp_max);
    printf("spwd.sp_warn: %ld\n", spent->sp_warn);
    printf("spwd.sp_inact: %ld\n", spent->sp_inact);
    printf("spwd.sp_expire: %ld\n", spent->sp_expire);
    printf("spwd.sp_flag: %lX\n", spent->sp_flag);
}

int call_getgrgid(call_params *params) {
    enum nss_status status;
    struct group grent;
    int err;
    long gid;

    if (params->argc < 3) {
        fprintf(stderr, "getgrgid requires a group id\n");
        return 1;
    }

    gid = atol(params->argv[2]);
    printf("getgrgid(%ld)\n", gid);
    err = 0;
    status = _nss_exec_getgrgid_r((gid_t) gid, &grent, params->buffer, params->buffer_length, &err);

    if (!show_nss_status(status, err)) {
        show_group(&grent);
    }

    return 0;
}

int call_getgrnam(call_params *params) {
    enum nss_status status;
    struct group grent;
    int err;

    if (params->argc < 3) {
        fprintf(stderr, "getgrnam requires a name\n");
        return 1;
    }

    printf("getgrnam(\"%s\")\n", params->argv[2]);
    err = 0;
    status = _nss_exec_getgrnam_r(params->argv[2], &grent, params->buffer, params->buffer_length, &err);

    if (!show_nss_status(status, err)) {
        show_group(&grent);
    }

    return 0;
}

int call_getpwnam(call_params *params) {
    enum nss_status status;
    struct passwd pwent;
    int err;

    if (params->argc < 3) {
        fprintf(stderr, "getpwnam requires a username\n");
        return 1;
    }

    printf("getpwnam(\"%s\")\n", params->argv[2]);
    err = 0;
    status = _nss_exec_getpwnam_r(params->argv[2], &pwent, params->buffer, params->buffer_length, &err);

    if (!show_nss_status(status, err)) {
        show_passwd(&pwent);
    }

    return 0;
}

int call_getpwuid(call_params *params) {
    enum nss_status status;
    struct passwd pwent;
    int err;
    long uid;

    if (params->argc < 3) {
        fprintf(stderr, "getpwuid requires a user id\n");
        return 1;
    }

    uid = atol(params->argv[2]);
    printf("getpwuid(%ld)\n", uid);
    err = 0;
    status = _nss_exec_getpwuid_r((uid_t) uid, &pwent, params->buffer, params->buffer_length, &err);

    if (!show_nss_status(status, err)) {
        show_passwd(&pwent);
    }

    return 0;
}

int call_getspnam(call_params *params) {
    enum nss_status status;
    struct spwd spent;
    int err;

    if (params->argc < 3) {
        fprintf(stderr, "getspnam requires a username\n");
        return 1;
    }

    printf("getspnam(\"%s\")\n", params->argv[2]);
    err = 0;
    status = _nss_exec_getspnam_r(params->argv[2], &spent, params->buffer, params->buffer_length, &err);

    if (!show_nss_status(status, err)) {
        show_spwd(&spent);
    }

    return 0;
}

int call_listgrent(call_params *params) {
    enum nss_status status;
    struct group grent;
    int err, return_code;
    unsigned long entry_number;

    printf("setgrent(0)\n");
    err = 0;
    return_code = 0;
    entry_number = 0;
    status = _nss_exec_setgrent(0);

    if (show_nss_status(status, err)) {
        return 1;
    }

    status = 0;

    while (!status) {
        printf("getgrent(%lu)\n", entry_number);
        err = 0;
        status = _nss_exec_getgrent_r(&grent, params->buffer, params->buffer_length, &err);

        if (!show_nssgetent_status(status, err)) {
            show_group(&grent);
        }

        if (err != 0) {
            return_code = 2;
        }

        entry_number += 1;
    }

    printf("endgrent()\n");
    err = 0;
    status = _nss_exec_endgrent();
    show_nss_status(status, err);

    return return_code;
}

int call_listpwent(call_params *params) {
    enum nss_status status;
    struct passwd pwent;
    int err, return_code;
    unsigned long entry_number;

    printf("setpwent(0)\n");
    err = 0;
    return_code = 0;
    entry_number = 0;
    status = _nss_exec_setpwent(0);

    if (show_nss_status(status, err)) {
        return 1;
    }

    status = 0;

    while (!status) {
        printf("getpwent(%lu)\n", entry_number);
        err = 0;
        status = _nss_exec_getpwent_r(&pwent, params->buffer, params->buffer_length, &err);

        if (!show_nssgetent_status(status, err)) {
            show_passwd(&pwent);
        }

        if (err != 0) {
            return_code = 2;
        }

        entry_number += 1;
    }

    printf("endpwent()\n");
    err = 0;
    status = _nss_exec_endpwent();
    show_nss_status(status, err);

    return return_code;
}

int call_listspent(call_params *params) {
    enum nss_status status;
    struct spwd spent;
    int err, return_code;
    unsigned long entry_number;

    printf("setspent(0)\n");
    err = 0;
    return_code = 0;
    entry_number = 0;
    status = _nss_exec_setspent(0);

    if (show_nss_status(status, err)) {
        return 1;
    }

    status = 0;

    while (!status) {
        printf("getspent(%lu)\n", entry_number);
        err = 0;
        status = _nss_exec_getspent_r(&spent, params->buffer, params->buffer_length, &err);

        if (!show_nssgetent_status(status, err)) {
            show_spwd(&spent);
        }

        if (err != 0) {
            return_code = 2;
        }

        entry_number += 1;
    }

    printf("endspent()\n");
    err = 0;
    status = _nss_exec_endspent();
    show_nss_status(status, err);

    return return_code;
}

void help(void) {
    printf("Calls nss_exec functions.  Available functions, sorted by category.\n"
        "The list__ent functions all start with one call to set__ent, zero or more\n"
        "calls to get__ent, and one final call to end__ent.  (Replace __ with the two\n"
        "letter code for the thing you want.)"
        "\n"
        "group:\n"
        "    getgrnam [NAME]\n"
        "    getgrgid [GID]\n"
        "    listgrent\n"
        "\n"
        "passwd:\n"
        "    getpwnam [NAME]\n"
        "    getpwuid [UID]\n"
        "    listpwent\n"
        "\n"
        "shadow:\n"
        "    getspnam [NAME]\n"
        "    listwpent\n");
}

int main(int argc, char **argv) {
    call_params params;
    char buffer[BUFFER_LENGTH];  // Used when we pretend to be NSS
    function_map fnmap[] = {
        { "getgrnam", call_getgrnam },
        { "getgrgid", call_getgrgid },
        { "getpwnam", call_getpwnam },
        { "getpwuid", call_getpwuid },
        { "getspnam", call_getspnam },
        { "listgrent", call_listgrent },
        { "listpwent", call_listpwent },
        { "listspent", call_listspent },
        { NULL, NULL }
    };
    function_map *fnmap_ptr;

    if (argc < 2 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
        help();
        return 0;
    }

    fnmap_ptr = fnmap;
    params.argc = argc;
    params.argv = argv;
    params.buffer = buffer;
    params.buffer_length = BUFFER_LENGTH;

    while (fnmap_ptr->name != NULL) {
        if (strcmp(fnmap_ptr->name, argv[1]) == 0) {
            return fnmap_ptr->fn(&params);
        }

        fnmap_ptr ++;
    }

    fprintf(stderr, "Invalid command: %s\n", argv[1]);
    return 1;
}
