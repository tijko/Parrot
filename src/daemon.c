#include <stdlib.h>
#include <sys/stat.h>

#include "parrot.h"


int parrot_daemon(void) 
{

    /*
     * Creates a daemon out the Parrot process.  Parrot will continue to run
     * threads separated from any controlling terminal.  This is the standard
     * method for creating a daemon in a linux/gnu enviroment.  Fork twice,
     * setting the file mode, create a new session ID, and finally change to the 
     * "/" or root directory.
     */
   
    pid_t pid;
    pid = fork();  // first fork 
    if (pid == -1) 
        return -1; // if fork returned -1, return error

    if (pid > 0) 
        exit(0);         // if pid is greater than 0 than we are in the parent
                         // process and exit gracefully

    umask(0);            // set up process file mode
    pid_t sid;   
    sid = setsid();      // create session ID
    if (sid == -1) 
        return -1;

    if ((chdir("/")) == -1) 
        return -1;       // change directory to root "/"

    close(STDIN_FILENO); // close stdin, stdout, stderr
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return 0;
}

