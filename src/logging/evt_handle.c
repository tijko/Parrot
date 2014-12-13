#include <systemd/sd-journal.h>


void log_evt(char *path, int mask)
{
    sd_journal_print(LOG_INFO, "File: %s TYPE: %x", path, mask);
}

void log_parrot(void)
{
    sd_journal_print(LOG_INFO, "Parrot Started");
}

void log_dbus(char *addr)
{
    sd_journal_print(LOG_INFO, "Connect to DBus Address: %s", addr);
}

void log_backup(char *fn)
{
    sd_journal_print(LOG_INFO, "Backup: %s", fn);
}
