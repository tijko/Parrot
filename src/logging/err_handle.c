#include <systemd/sd-journal.h>


void log_err(const char *path)
{
    sd_journal_print(LOG_ERR, "LOGERR: %s: %m", path);
}

void sig_err(void)
{
    sd_journal_print(LOG_ERR, "SIGERR: %m");
}

void notify_err(void)
{
    sd_journal_print(LOG_ERR, "NOTIFY_ERR: %m");
}

void dbus_conn_err(int err)
{
    sd_journal_print(LOG_ERR, "DBUS_ERR: %d", err);
}

void dbus_addr_err(void)
{
    sd_journal_print(LOG_ERR, "DBUS_ADDR_ERR");
}
