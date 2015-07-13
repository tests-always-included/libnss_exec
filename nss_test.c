#include <stdio.h>
#include "nss_exec.h"

extern enum nss_status _nss_exec_setgrent_locked(int stayopen);
extern enum nss_status _nss_exec_setgrent(int stayopen);
extern enum nss_status _nss_exec_endgrent_locked(void);
extern enum nss_status _nss_exec_endgrent(void);
extern enum nss_status _nss_exec_getgrent_r_locked(struct group *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getgrent_r(struct group *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getgrgid_r_locked(gid_t gid, struct group *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getgrgid_r(gid_t gid, struct group *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getgrnam_r_locked(const char *name, struct group *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getgrnam_r(const char *name, struct group *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_setpwent_locked(int stayopen);
extern enum nss_status _nss_exec_setpwent(int stayopen);
extern enum nss_status _nss_exec_endpwent_locked(void);
extern enum nss_status _nss_exec_endpwent(void);
extern enum nss_status _nss_exec_getpwent_r_locked(struct passwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getpwent_r(struct passwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getpwuid_r_locked(uid_t uid, struct passwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getpwuid_r(uid_t uid, struct passwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getpwnam_r_locked(const char *name, struct passwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getpwnam_r(const char *name, struct passwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_setspent_locked(int stayopen);
extern enum nss_status _nss_exec_setspent(int stayopen);
extern enum nss_status _nss_exec_endspent_locked(void);
extern enum nss_status _nss_exec_endspent(void);
extern enum nss_status _nss_exec_getspent_r_locked(struct spwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getspent_r(struct spwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getspnam_r_locked(const char *name, struct spwd *result, char *buffer, size_t bufferLength, int *errnop);
extern enum nss_status _nss_exec_getspnam_r(const char *name, struct spwd *result, char *buffer, size_t bufferLength, int *errnop);

int main(void) {
    char buffer[2048];
    enum nss_status status;
    struct passwd result;
    int err;

    errno = 0;
    status = _nss_exec_getpwnam_r("testuser", &result, buffer, 2048, &err);
    printf("errno: %d\n", err);

    switch (status) {
        case NSS_STATUS_SUCCESS:
            printf("pw_name %s\n", result.pw_name);
            printf("pw_passwd %s\n", result.pw_passwd);
            printf("pw_uid %ld\n", (long) result.pw_uid);
            printf("pw_gid %ld\n", (long) result.pw_gid);
            printf("pw_gecos %s\n", result.pw_gecos);
            printf("pw_dir %s\n", result.pw_dir); 
            printf("pw_shell %s\n", result.pw_shell);
            printf("NSS_STATUS_SUCCESS\n");
            break;

        case NSS_STATUS_NOTFOUND:
            printf("NSS_STATUS_NOTFOUND\n");
            break;

        case NSS_STATUS_TRYAGAIN:
            printf("NSS_STATUS_TRYAGAIN\n");
            break;

        case NSS_STATUS_UNAVAIL:
            printf("NSS_STATUS_UNAVAIL\n");
            break;

        default:
            printf("Unknown status code: %ld", (long) status);
    }

    return 0;
}
