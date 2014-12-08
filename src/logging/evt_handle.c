#include <systemd/sd-journal.h>


void log_evt(char *path, int mask)
{
    sd_journal_print(LOG_INFO, "File: %s TYPE: %x", path, mask);
}

void log_parrot(void)
{
    sd_journal_print(LOG_INFO, "Parrot Started");
}
