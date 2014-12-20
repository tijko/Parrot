#include <dbus/dbus-glib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>


// Macro to set a directory to backup the files from the directory above being 
// monitored.
// Edit to specify which directory to backup to.
#define BACKUP_PATH "/home/tijko/documents/backups/backup_c/"

#define BACKUP_SIZE strlen(BACKUP_PATH) + 1

struct parrot_watch {
    int parrot_wd;
    char *dir;
};

struct watch_trigger {
    char *file;
    char *dir;
};

struct parrot_watch *current_watch[4092];

int parrot_inotify_instance;
int watch_num;

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

gboolean parrot_obj_accessed(ParrotObject *p_obj, int access_time);

gboolean parrot_obj_current_watches(ParrotObject *p_obj,
                                    DBusGMethodInvocation *ctxt);

gboolean parrot_obj_method_arg(ParrotObject *p_obj, char *fn,
			       DBusGMethodInvocation *ctxt);

// Creates the daemon.
int parrot_daemon(void);

// This is the function that sets the inotify event loop.  The watch is added for 
// the `file_dir_to_parrot` and will signal to backup on events.
int notify_parrot_init(void);

void parrot_add_watch(char *path);

void parrot_mainloop(ParrotObject *p_obj);

// Function to parse the inotify_event structs and pass on the relavent 
// members.
void parse_events(int e_status, char e_buf[], ParrotObject *p_obj);

// Walks the directory being monitored and calls the backup function when a 
// file is found.
void find_files(struct watch_trigger *accessed);

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
void dbus_conn_err(int err);

// Logs any errors while returning the session bus address.
void dbus_addr_err(void);

// Logs any errors that occur during inotify_loop.
void notify_err(void);

// Logs any event and the mask of that event to the parrot log.
void log_evt(char *file, int mask);

// Logs parrot being started.
void log_parrot(void);

// Logs successful backup of file.
void log_backup(char *fn);

// Logs the a successful dbus session connection
void log_dbus(char *addr);

void log_method(char *fn);

// Signal handler to complete cleanup if any interrupts are received.
void cleanup(int signo);

// Register the D-Bus parrot object
void register_parrot_obj(ParrotObject *p_obj);
