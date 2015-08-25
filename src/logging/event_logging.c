#include <systemd/sd-journal.h>
#include <stdarg.h>

#include "../parrot.h"


char *fmt_event(int descriptors)
{
    char *fmt = calloc(sizeof(char) * (FMTSIZE * descriptors), sizeof(char));

    if (fmt == NULL) {
        log_error(__FILE__, "fmt_event", "calloc", __LINE__, errno);
        return NULL;
    }
        
    int i;
    for (i=0; i < descriptors; i++)
        strcat(fmt, FMT);

    return fmt;
}

void log_event(char *event_msg)
{
    sd_journal_print(LOG_INFO, "PARROT: %s\n", event_msg);
}
