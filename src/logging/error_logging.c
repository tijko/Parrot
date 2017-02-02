#include <systemd/sd-journal.h>

#include "../parrot.h"


void log_error(const char *file_name, const char *func, 
               const char *call, int line, int error)
{
    /* 
     * Generic error logging function.
     *
     * Propagating the file that the error originated from, the function that 
     * the error was called in, the function call that was the cause of the 
     * error and the line number at which that error occured.
     *
     */

    sd_journal_print(LOG_ERR, "ERROR :: FILE: %s FUNC: %s CALL: %s LINE: %d MSG: %s", 
                               file_name, func, call, line, strerror(error));
}

// use sd_journal_perror for fatals ...
// use sd_journal_print with differing priority levels
// for all other
