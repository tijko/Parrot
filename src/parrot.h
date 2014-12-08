#include <dbus/dbus-glib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <dirent.h>


// Macro to set a directory to monitor for events. Edit to specify which 
// directory to monitor.
#define PARROT_PATH "/home/tijko/documents/learning/Cee_Ceeplus/" 

#define PARROT_SIZE strlen(PARROT_PATH) + 1

// Macro to set a directory to backup the files from the directory above being 
// monitored.
// Edit to specify which directory to backup to.
#define BACKUP_PATH "/home/tijko/documents/backups/backup_c/"

#define BACKUP_SIZE strlen(BACKUP_PATH) + 1

// Macro to set the size of an event
#define EVT_SIZE sizeof(struct inotify_event) + NAME_MAX + 1

// Macro to set a size for the buffer, that will contain all the events
// XXX: if you know you will have longer uptimes or are likely to have alot of
//      events you could increase this value
#define EVT_BUF_SIZE (EVT_SIZE) 

// Gobject types 
typedef struct {
    GObject parent;
} ParrotObject;

typedef struct {
    GObjectClass parent;
} ParrotObjectClass;

GType parrot_obj_get_type(void);

#define VALUE_TYPE_OBJECT (parrot_obj_get_type())

gboolean parrot_obj_accessed(ParrotObject *p_obj);

// Creates the daemon.
int parrot_daemon(void);

// This is the function that sets the inotify event loop.  The watch is added for 
// the `file_dir_to_parrot` and will signal to backup on events.
int notify_parrot(void);

// Function to parse the inotify_event structs and pass on the relavent 
// members.
void parse_events(int e_status, char e_buf[]);

// Walks the directory being monitored and calls the backup function when a 
// file is found.
void find_files(void);

char *create_pathname(char *dirname, char *filename, size_t pathsize);

// Reads the old files and then writes the file data to the backup.
int backup_files(char *file_path, char *backup_path);

// Checks if events are able to be read from watch descriptor.
int events_in(int highest_fd, fd_set *watchfds);

// Logs any errors and their names to parrot log.
void log_err(const char *path); 

// Logs any errors dealing with setting up the interrupt signal.
void sig_err(void);

// Logs any errors associated with setting up the dbus connection.
void dbus_err(int err);

// Logs any errors that occur during inotify_loop.
void notify_err(void);

// Logs any event and the mask of that event to the parrot log.
void log_evt(char *file, int mask);

// Logs parrot being started.
void log_parrot(void);

// Signal handler to complete cleanup if any interrupts are received.
void cleanup(int signo);

// Register the D-Bus parrot object
void register_parrot_obj(ParrotObject *p_obj);
