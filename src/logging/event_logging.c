#include <systemd/sd-journal.h>
#include <stdarg.h>

#include "../parrot.h"


void log_event(char *event_msg, int descriptors, ...)
{
    if (descriptors) {

        int i;
        va_list log_args;
        char *event, *msg, *tmp;

        msg = malloc(sizeof(char) * strlen(event_msg) + 1);
        tmp = NULL;

        if (msg == NULL) {
            log_error("event_logging.c", "log_event", "malloc", 15);
            return;
        }
    
        snprintf(msg, strlen(event_msg) + 1, "%s", event_msg);
        va_start(log_args, descriptors);

        for (i=0; i < descriptors; i++) {
            event = va_arg(log_args, char *);                
            tmp = realloc(msg, strlen(msg) + strlen(event) + 1);

            if (tmp == NULL) {
                log_error("event_logging.c", "log_event", "realloc", 27);
                free(msg);
                return;
            }
        
            msg = tmp;            
            tmp = NULL;
            strcat(msg, event);
        }

        va_end(log_args);

        sd_journal_print(LOG_INFO, "PARROT: %s\n", msg);
        free(msg);

    } else 
        sd_journal_print(LOG_INFO, "PARROT: %s\n", event_msg);
}
