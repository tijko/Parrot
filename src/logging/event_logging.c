#include <systemd/sd-journal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


void log_event(char *event_msg, int descriptors, ...)
{
    if (descriptors) {

        int i;
        va_list log_args;
        char *event, *msg;

        msg = malloc(sizeof(char) * strlen(event_msg));
        strcat(msg, event_msg);
        va_start(log_args, descriptors);

        for (i=0; i < descriptors; i++) {
            event = va_arg(log_args, char *);                
            msg = (char *) realloc(msg, strlen(msg) + strlen(event));
            strcat(msg, event);
        }

        va_end(log_args);

        sd_journal_print(LOG_INFO, "PARROT: %s\n", msg);
        free(msg);

    } else {

        sd_journal_print(LOG_INFO, "PARROT: %s\n", event_msg);

    }
}
