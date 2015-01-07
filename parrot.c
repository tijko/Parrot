#include <stdio.h>
#include <signal.h>

#include "src/parrot.h"


int main(int argc, char *argv[]) {

    int notify_flag;
    if ((signal(SIGINT, cleanup)) == SIG_ERR)
        sig_err();

    notify_flag = parrot_daemon();

    if (notify_flag == -1)
        notify_err();

    notify_flag = notify_parrot_init();

    if (notify_flag == -1)
        notify_err();

    return 0;
}

void cleanup(int signo)
{
    sleep(1); // a small sleep to allow enough time for cleanup.

    // call cleanup function

    signal(SIGINT, SIG_DFL);
    raise(signo);
}
