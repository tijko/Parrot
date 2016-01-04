#define _GNU_SOURCE

#ifndef PARROT_H
#define PARROT_H

#include <glib.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dbus/dbus.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <dbus/dbus-glib-lowlevel.h>


bool running;

/*
    Setting the MAX_WATCHES will depend on how many watches are currently set
    on the system and how many are allowed.

    You can find the max number of watches:

        '/proc/sys/fs/inotify/max_user_watches'

    or by calling:

        `sysctl fs.inotify.max_user_watches`

    from your shell.
*/

// XXX Default max watches
#define MAX_WATCHES 4096

#define MAX_PATH 255

// Parrot mask for either type of watch to set, directory or file watch.
enum {
    W_DIR=1,
    W_FIL
};

#define W_FLAG 0x1

struct parrot_watch {
    int parrot_wd;
    int watch_type;
    int watch_flag;
    int backup_path_len;
    void (*backup)(struct parrot_watch *accessed);
    char *watch_path;
    char *backup_path;
    char *evfile;
};

struct parrot_watch *current_watch[MAX_WATCHES];

int parrot_inotify_instance;
int watch_num;

// Macro to set the size of an event
#define EVT_SIZE sizeof(struct inotify_event) + NAME_MAX + 1

// Macro to set a size for the buffer, that will contain all the events
// XXX: if you know you will have longer uptimes or are likely to have alot of
//      events you could increase this value
#define EVT_BUF_SIZE (EVT_SIZE) 

#define PARROT_DBUS_PATH "/org/Parrot"
#define PARROT_DBUS_OBJECT "org.Parrot"
#define PARROT_DBUS_INTERFACE "org.Parrot.Inotify"

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

gboolean parrot_obj_add_watch(ParrotObject *p_obj, char *watch, 
                              char *backup_path, int watch_mask,
			                  DBusGMethodInvocation *ctxt);

gboolean parrot_obj_remove_watch(ParrotObject *p_obj, char *watch,
                                 DBusGMethodInvocation *ctxt);

struct ParrotGDBusObj {
    GMainLoop *mainloop;
    DBusGConnection *conn;
    DBusConnection *dconn;
    DBusGProxy *proxy;
    ParrotObject *p_obj;
};

// Creates the daemon.
int parrot_daemon(void);

#define XDG_RUNTIME_DIR "XDG_RUNTIME_DIR"
#define PARROT_PID_PATH "/parrot/parrot.pid"

int create_pid_file(void);

// This is the function that sets the inotify event loop.  The watch is added for 
// the `file_dir_to_parrot` and will signal to backup on events.
int notify_parrot_init(void);

void parrot_cleanup(struct ParrotGDBusObj *parrot_gdbus_obj);

int parrot_add_watch(char *path, char *backup_path, int watch_mask);

void set_evfile(struct parrot_watch *watch);

void parrot_remove_watch(char *path);

void close_watch(struct parrot_watch *watch);

void parrot_mainloop(struct ParrotGDBusObj *parrot_gdbus_obj);

// Function to parse the inotify_event structs and pass on the relavent 
// members.
void parse_events(int e_status, char e_buf[], ParrotObject *p_obj);

// Backs up a single file type watch.
void find_file(struct parrot_watch *accessed);

// Walks the directory being monitored and calls the backup function when a 
// file is found.
void find_files(struct parrot_watch *accessed);

char *create_pathname(char *dirname, char *filename, size_t pathsize);

// Reads the old files and then writes the file data to the backup.
int backup_files(char *file_path, char *backup_path);

// Checks if events are able to be read from watch descriptor.
int events_in(int highest_fd, fd_set *watchfds);

// Logs any errors and their names to parrot log.
void log_error(const char *file_name, const char *func, 
               const char *call, int line, int error); 

#define FMT "\%s"
#define FMTSIZE 4

char *fmt_event(int descriptors);

#define EVENT(buf, fmt, ...) asprintf(buf, fmt, __VA_ARGS__)

// Logs any event and the mask of that event to the parrot log.
void log_event(char *event_msg);

// Signal handler to complete cleanup if any interrupts are received.
void cleanup(int signal_number, siginfo_t *sigaction_info, void *ctxt);

// Register the D-Bus parrot object
void register_parrot_obj(struct ParrotGDBusObj *parrot_gdbus_obj);

#endif
