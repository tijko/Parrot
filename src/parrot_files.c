#define _GNU_SOURCE

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "parrot.h"


void find_file(struct parrot_watch *accessed)
{
    char *backupname = create_pathname(accessed->backup_path, accessed->evfile,
                                       accessed->backup_path_len);

    int backup_err = backup_files(accessed->watch_path, backupname);
    free(backupname);

    if (backup_err) 
        log_error(__FILE__, "find_file", "backup_files", __LINE__, errno);

    char *fmt = fmt_event(2);
    char *logevent_buffer;
    EVENT(&logevent_buffer, fmt, "backing up => ", accessed->evfile);
    log_event(logevent_buffer);
    free(fmt);
    free(logevent_buffer);
}

void find_files(struct parrot_watch *accessed)
{
    char *pathname, *backupname;
    DIR *drect;
    
    size_t dirent_size = offsetof(struct dirent, d_name) + MAX_PATH + 1;
    struct dirent *dir_results;
    struct dirent *dir_entries = malloc(sizeof(char) * dirent_size);

    if ((drect = opendir(accessed->watch_path)) == NULL) 
        log_error(__FILE__, "find_file", "opendir", __LINE__, errno);

    while ((readdir_r(drect, dir_entries, &dir_results)) != -1) {

        if (dir_results == NULL)
            break;

        if (dir_entries->d_type == DT_REG && 
            !strcmp(dir_entries->d_name, accessed->evfile)) {
            pathname = create_pathname(accessed->watch_path, dir_entries->d_name,
                                       strlen(accessed->watch_path) + 1);
            backupname = create_pathname(accessed->backup_path, 
                                         dir_entries->d_name, 
                                         accessed->backup_path_len);
            if (backup_files(pathname, backupname))
                log_error(__FILE__, "find_files", "backup_files", 
                          __LINE__, errno);
            else {
                char *fmt = fmt_event(2);
                char *logevent_buffer;
                EVENT(&logevent_buffer, fmt, "backingup => ", dir_entries->d_name);
                log_event(logevent_buffer);
                free(fmt);
                free(logevent_buffer);
            }

            free(pathname);
            free(backupname);
       }
    }

    free(dir_entries);
    closedir(drect);
}

char *create_pathname(char *dirname, char *filename, size_t pathsize)
{
    char *pathname;
    size_t pathname_size;
    pathname_size = strlen(filename) + pathsize;
    if ((pathname = malloc(sizeof(char) * pathname_size)) == NULL) {
        log_error(__FILE__, "create_pathname", "malloc", __LINE__, errno);
        return NULL;
    }

    snprintf(pathname, pathname_size, "%s%s", dirname, filename);
    return pathname;    
}

int backup_files(char *file_path, char *backup_path) 
{
    struct stat f_buffer;
    stat(file_path, &f_buffer);
    int file_size = (int) f_buffer.st_size;
    int r_file, f_read, w_file;

    char *fmt = fmt_event(2);
    char *logevent_buffer;
    EVENT(&logevent_buffer, fmt, "path ", file_path);
    log_event(logevent_buffer);
    free(fmt);
    free(logevent_buffer);

    if ((r_file = open(file_path, O_RDONLY | O_NOATIME)) == -1) {
        log_error(__FILE__, "backup_files", "open", __LINE__, errno);
        return errno;
    }

    void *fd_buffer = malloc(sizeof(char) * file_size);
    if (fd_buffer == NULL) {
        log_error(__FILE__, "backup_files", "malloc", __LINE__, errno);
        close(r_file);
        return errno;
    }

    if ((f_read = read(r_file, fd_buffer, file_size)) == -1) {
        log_error(__FILE__, "backup_files", "read", __LINE__, errno);
        close(r_file);
        return errno;
    }

    if ((w_file = open(backup_path, O_CREAT | O_RDWR, f_buffer.st_mode)) == -1) {
        log_error(__FILE__, "backup_files", "open", __LINE__, errno);
        close(r_file);
        close(f_read);
        return errno;
    }

    set_parrot_lock(w_file);
    write(w_file, fd_buffer, file_size);
    release_parrot_lock(w_file);

    close(w_file);
    close(r_file);
    close(f_read);

    free(fd_buffer);

    return 0;
}

void set_parrot_lock(int fd)
{
    struct flock parrot_lock;

    parrot_lock.l_type = F_WRLCK;
    parrot_lock.l_whence = SEEK_SET;
    parrot_lock.l_start = 0;
    parrot_lock.l_len = 0;

    while ( 1 ) {

        int ret = fcntl(fd, F_GETLK, &parrot_lock);

        if (ret < 0)
            return;
        // handle errors

        if (parrot_lock.l_type == F_UNLCK) {
            parrot_lock.l_type = F_WRLCK;
            fcntl(fd, F_SETLK, &parrot_lock);
            return;
        }
    }
}

void release_parrot_lock(int fd)
{
    struct flock parrot_lock;

    parrot_lock.l_type = F_UNLCK;
    parrot_lock.l_whence = SEEK_SET;
    parrot_lock.l_start = 0;
    parrot_lock.l_len = 0;

    fcntl(fd, F_SETLK, &parrot_lock);
}
