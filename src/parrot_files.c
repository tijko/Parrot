#define _GNU_SOURCE

#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "parrot.h"


void find_file(struct parrot_watch *accessed)
{
    char *backupname;
    int backup_err;

    backupname = create_pathname(accessed->backup_path, accessed->evfile,
                                 accessed->backup_path_len);
    backup_err = backup_files(accessed->watch_path, backupname);
    if (backup_err)
        log_error("parrot_files.c", "find_file", "backup_files", 18);
    else
        log_event("backing up => ", 1, accessed->evfile);
    free(backupname);
}

void find_files(struct parrot_watch *accessed)
{
    char *pathname, *backupname;
    DIR *drect;
    int backup_err;
    
    struct dirent *dir_files;

    if ((drect = opendir(accessed->watch_path)) == NULL) 
        log_error("parrot_files.c", "find_file", "opendir", 35);

    while ((dir_files = readdir(drect))) {

        if (dir_files->d_type == DT_REG && !strcmp(dir_files->d_name, accessed->evfile)) {
            pathname = create_pathname(accessed->watch_path, dir_files->d_name, 
                                                   strlen(accessed->watch_path) + 1);
            backupname = create_pathname(accessed->backup_path, dir_files->d_name, accessed->backup_path_len);
            backup_err = backup_files(pathname, backupname);
            if (backup_err) 
                log_error("parrot_files.c", "find_files", "backup_files", 44);
            else
                log_event("backing up => ", 1, dir_files->d_name);

            free(pathname);
            free(backupname);
       }
    }
}

char *create_pathname(char *dirname, char *filename, size_t pathsize)
{
    char *pathname;
    size_t pathname_size;
    pathname_size = strlen(filename) + pathsize;
    if ((pathname = malloc(sizeof(char) * pathname_size)) == NULL) {
        log_error("parrot_files.c", "create_pathname", "malloc", 63);
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

    if ((r_file = open(file_path, O_RDONLY | O_NOATIME)) == -1) {
        log_error("parrot_files.c", "backup_files", "open", 79);
        return errno;
    }

    void *fd_buffer = malloc(sizeof(char) * file_size);
    if (fd_buffer == NULL) {
        log_error("parrot_files.c", "backup_files", "malloc", 84);
        return errno;
    }

    if ((f_read = read(r_file, fd_buffer, file_size)) == -1) {
        log_error("parrot_files.c", "backup_files", "read", 90);
        return errno;
    }

    if ((w_file = open(backup_path, O_CREAT | O_RDWR, 0644)) == -1) {
        log_error("parrot_files.c", "backup_files", "open", 95);
        return errno;
    }

    write(w_file, fd_buffer, file_size);
    close(w_file);
    close(r_file);
    free(fd_buffer);

    return 0;
}
