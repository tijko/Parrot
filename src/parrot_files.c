#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "parrot.h"


void find_file(struct watch_trigger *accessed)
{
    char *pathname, *backupname;
    DIR *drect;
    int backup_err;
    
    struct dirent *dir_files;

    if ((drect = opendir(accessed->dir)) == NULL) 
        log_error("parrot_files.c", "find_files", "opendir", 18); 

    while ((dir_files = readdir(drect))) {

        if (dir_files->d_type == DT_REG && !strcmp(dir_files->d_name, accessed->file)) {
            pathname = create_pathname(accessed->dir, dir_files->d_name, 
                                                   strlen(accessed->dir) + 1);
            backupname = create_pathname(BACKUP_PATH, dir_files->d_name, BACKUP_SIZE);            
            backup_err = backup_files(pathname, backupname);
            if (backup_err) 
                log_error("parrot_files.c", "find_files", "backup_files", 27);
            else
                log_backup(dir_files->d_name);

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
    pathname = malloc(sizeof(char) * pathname_size);
    snprintf(pathname, pathname_size, "%s%s", dirname, filename);
    return pathname;    
}

int backup_files(char *file_path, char *backup_path) 
{
    struct stat f_buffer;
    stat(file_path, &f_buffer);
    int file_size = (int) f_buffer.st_size;
    int r_file, f_read, w_file;

    if ((r_file = open(file_path, O_RDONLY)) == -1) {
        log_error("parrot_files.c", "create_pathname", "open", 56);
        return errno;
    }

    void *fd_buffer = malloc(sizeof(char) * file_size);

    if ((f_read = read(r_file, fd_buffer, file_size)) == -1) {
        log_error("parrot_files.c", "backup_files", "read", 63);
        return errno;
    }

    if ((w_file = open(backup_path, O_CREAT | O_RDWR, 0644)) == -1) {
        log_error("parrot_files.c", "backup_files", "open", 68);
        return errno;
    }

    write(w_file, fd_buffer, file_size);
    close(w_file);
    close(r_file);
    free(fd_buffer);

    return 0;
}
