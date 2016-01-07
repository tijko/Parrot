#include <signal.h>

#include "src/parrot.h"


int main(int argc, char *argv[]) {

    struct sigaction sa = { .sa_sigaction = cleanup, .sa_flags = SA_RESETHAND };

    if ((sigaction(SIGINT, &sa, NULL)) < 0) {
        log_error(__FILE__, "main", "signal", __LINE__, errno);
        return 0;
    }
    
    int notify_flag = parrot_daemon();

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

void cleanup(int signal_number, siginfo_t *sigaction_info, void *ctxt)
{
    running = false;
    sleep(1); // a small sleep to allow enough time for cleanup.

    log_event("signal interrupt");

    raise(signal_number);
}
