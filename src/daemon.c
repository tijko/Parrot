#include "parrot.h"


int parrot_daemon(void) 
{
    /*
     * Creates a daemon out the Parrot process.  Parrot will continue to run
     * threads separated from any controlling terminal.  This is the standard 
     * method for creating a daemon in a linux/gnu enviroment.  Fork twice,
     * setting the file mode, create a new session ID, and finally change to 
     * the "/" or root directory.
     */
   
    pid_t pid;
    int pid_file_ret;

    pid = fork();
    if (pid == -1) {
        log_error(__FILE__, "parrot_daemon", "fork", __LINE__, errno);
        return -1;
    }

    if (pid > 0) 
        exit(0);         // if pid is greater than 0 than we are in the parent

    umask(0);
    pid_t sid;   
    sid = setsid();
    if (sid == -1) {
        log_error(__FILE__, "parrot_daemon", "setsid", __LINE__, errno);
        return -1;
    }

    if ((chdir("/")) == -1) {
        log_error(__FILE__, "parrot_daemon", "chdir", __LINE__, errno);
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        log_error(__FILE__, "parrot_daemon", "fork", __LINE__, errno);
        return -1;
    }

    if (pid > 0)
        exit(0);

    pid_file_ret = create_pid_file();
    if (pid_file_ret == -1) {
        log_error(__FILE__, "parrot_daemon", "create_pid_file", __LINE__, errno);
        return -1;
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return 0;
}

int create_pid_file(void)
{
    char *runtime_dir, *user_path;
    int pid_file_fd;
    pid_t parrot_pid;
    size_t max_pidstr_len, pathlen;

    parrot_pid = getpid();
    
    max_pidstr_len = 10;
    char pid_str[max_pidstr_len];

    runtime_dir = getenv(XDG_RUNTIME_DIR);
    if (runtime_dir == NULL) {
        log_error(__FILE__, "create_pid_file", "getenv", __LINE__, errno);
        return -1;
    }

    pathlen = strlen(runtime_dir) + strlen(PARROT_PID_PATH) + 1;
    user_path = malloc(sizeof(char) * pathlen);
    if (user_path == NULL) {
        log_error(__FILE__, "create_pid_file", "malloc", __LINE__, errno);
        return -1;
    }

    snprintf(user_path, pathlen, "%s%s", runtime_dir, PARROT_PID_PATH);

    pid_file_fd = open(user_path, O_CREAT | O_RDWR, 0666);
    if (pid_file_fd == -1) {
        log_error(__FILE__, "create_pid_file", "open", __LINE__, errno);
        return -1;
    }

    snprintf(pid_str, max_pidstr_len, "%d", parrot_pid);
    write(pid_file_fd, (void *) pid_str, strlen(pid_str));
    close(pid_file_fd);

    free(user_path);

    return 0;
}
