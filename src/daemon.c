#include <fcntl.h>
#include <sys/stat.h>

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

    pid = fork();  // first fork 
    if (pid == -1) {
        log_error("daemon.c", "parrot_daemon", "fork", 20);
        return -1; // if fork returned -1, return error
    }

    if (pid > 0) 
        exit(0);         // if pid is greater than 0 than we are in the parent
                         // process and exit gracefully

    umask(0);            // set up process file mode
    pid_t sid;   
    sid = setsid();      // create session ID
    if (sid == -1) {
        log_error("daemon.c", "parrot_daemon", "setsid", 32);
        return -1;
    }

    if ((chdir("/")) == -1) { // change directory to root "/"
        log_error("daemon.c", "parrot_daemon", "chdir", 38);
        return -1;
    }

    pid = fork();  // second fork
    if (pid == -1) {
        log_error("daemon.c", "parrot_daemon", "fork", 43);
        return -1;
    }

    if (pid > 0)
        exit(0);

    pid_file_ret = create_pid_file();
    if (pid_file_ret == -1) {
        log_error("daemon.c", "parrot_daemon", "create_pid_file", 45);
        return -1;
    }

    close(STDIN_FILENO); // close stdin, stdout, stderr
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
        log_error("daemon.c", "create_pid_file", "getenv", 68);
        return -1;
    }

    pathlen = strlen(runtime_dir) + strlen(PARROT_PID_PATH) + 1;
    user_path = malloc(sizeof(char) * pathlen);
    if (user_path == NULL) {
        log_error("daemon.c", "create_pid_file", "malloc", 84);
        return -1;
    }

    snprintf(user_path, pathlen, "%s%s", runtime_dir, PARROT_PID_PATH);

    pid_file_fd = open(user_path, O_CREAT | O_RDWR, 0666);
    if (pid_file_fd == -1) {
        log_error("daemon.c", "create_pid_file", "open", 92);
        return -1;
    }

    snprintf(pid_str, max_pidstr_len, "%d", parrot_pid);
    write(pid_file_fd, (void *) pid_str, strlen(pid_str));
    close(pid_file_fd);

    free(user_path);

    return 0;
}
