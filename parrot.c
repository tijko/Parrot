#include <signal.h>

#include "src/parrot.h"


int main(int argc, char *argv[]) {

    int notify_flag;
    if ((signal(SIGINT, cleanup)) == SIG_ERR) {
        log_error("main.c", "main", "signal", 9);
        return 0;
    }
    
    notify_flag = parrot_daemon();

    if (notify_flag == -1) {
        log_error("main.c", "main", "parrot_daemon", 12);
        return 0;
    }

    RUNNING = true;
    notify_flag = notify_parrot_init();

    if (notify_flag == -1)
        log_error("main.c", "main", "notify_parrot_init", 18);

    return 0;
}

void cleanup(int signo)
{
    RUNNING = false;
    sleep(1); // a small sleep to allow enough time for cleanup.

    signal(SIGINT, SIG_DFL);
    raise(signo);
}
