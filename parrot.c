#include <signal.h>

#include "src/parrot.h"


int main(int argc, char *argv[]) {

    int notify_flag;

    if ((signal(SIGINT, cleanup)) == SIG_ERR) {
        log_error(__FILE__, "main", "signal", __LINE__, errno);
        return 0;
    }
    
    notify_flag = parrot_daemon();

    if (notify_flag == -1) {
        log_error(__FILE__, "main", "parrot_daemon", __LINE__, errno);
        return 0;
    }

    running = true;
    notify_flag = notify_parrot_init();

    if (notify_flag == -1)
        log_error(__FILE__, "main", "notify_parrot_init", __LINE__, errno);

    return 0;
}

void cleanup(int signo)
{
    running = false;
    sleep(1); // a small sleep to allow enough time for cleanup.
    log_event("signal interrupt");
    signal(SIGINT, SIG_DFL);
    raise(signo);
}
