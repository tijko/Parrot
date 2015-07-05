#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/inotify.h>

#include "parrot.h"


int notify_parrot_init(void) 
{
    struct ParrotGDBusObj *parrot_gdbus_obj;
    pthread_t parrot_dbus;

    log_event("initializing parrot loop", 0);

    parrot_gdbus_obj = malloc(sizeof *parrot_gdbus_obj);
    if (parrot_gdbus_obj == NULL) {
        log_error("notify_parrot.c", "notify_parrot_init", "malloc", 18);
        return -1;
    }

    parrot_gdbus_obj->p_obj = g_object_new(VALUE_TYPE_OBJECT, NULL);

    // thread to register the parrot object to dbus and contain the dbus
    // mainloop
    pthread_create(&parrot_dbus, NULL, (void *) register_parrot_obj, 
                                                   parrot_gdbus_obj);

    if ((parrot_inotify_instance = inotify_init()) == -1) {
        log_error("notify_parrot.c", "notify_parrot_init", "inotify_init", 31);
        return -1;
    }

    watch_num = 0;
    parrot_mainloop(parrot_gdbus_obj);

    return 0;
}

void parrot_cleanup(struct ParrotGDBusObj *parrot_gdbus_obj)
{
    g_main_loop_unref(parrot_gdbus_obj->mainloop);
    g_object_unref(parrot_gdbus_obj->proxy);
    dbus_g_connection_unref(parrot_gdbus_obj->conn);
    dbus_connection_unref(parrot_gdbus_obj->dconn);
    g_object_unref(parrot_gdbus_obj->p_obj);
    free(parrot_gdbus_obj);
    log_event("clean up and close connections", 0);
}

void parrot_mainloop(struct ParrotGDBusObj *parrot_gdbus_obj)
{
    int change, status;
    char buffer[EVT_BUF_SIZE];

    fd_set watchfds;
    FD_ZERO(&watchfds);

    while (RUNNING) {

        FD_SET(parrot_inotify_instance, &watchfds);
        change = events_in(parrot_inotify_instance + 1, &watchfds);
        FD_ZERO(&watchfds);

        if (change == -1) {
            log_error("notify_parrot.c", "parrot_mainloop", "events_in", 64);
            return;
        } else if (change) {
            if ((status = read(parrot_inotify_instance, 
                                   buffer, EVT_BUF_SIZE)) < 0) {
                log_error("notify_parrot.c", "parrot_mainloop", "read", 71);
                return;
            }

            parse_events(status, buffer, parrot_gdbus_obj->p_obj);

        }
    }

    parrot_cleanup(parrot_gdbus_obj);
}

int parrot_add_watch(char *path)
{
    int watch;
    struct parrot_watch *new_watch;

    // add watch check ....

    new_watch = malloc(sizeof *new_watch);
    if (new_watch == NULL) {
        log_error("notify_parrot.c", "parrot_add_watch",
                  "malloc", 92);
        return -1;
    }

    if ((watch = inotify_add_watch(parrot_inotify_instance, 
                                   path, IN_ACCESS)) == -1) {
        free(new_watch);
        log_error("notify_parrot.c", "parrot_add_watch", 
                  "inotify_add_watch", 99);
        return -1;
    }

    new_watch->dir = malloc(sizeof(char) * strlen(path));
    strcpy(new_watch->dir, path);
    new_watch->parrot_wd = watch;

    current_watch[watch_num++] = new_watch;
    log_event("watch added => ", 1, path);
    return 0;
}

void parrot_remove_watch(char *path)
{
    int watch, idx;

    for (watch=0; watch < watch_num; watch++) {
        if (!strcmp(path, current_watch[watch]->dir)) {
            free(current_watch[watch]->dir);
            free(current_watch[watch]);
            current_watch[watch] = NULL;
            log_event("watch removed => ", 1, path);
            for (idx=watch; watch < watch_num; watch++) {
                if (current_watch[watch])
                    current_watch[idx++] = current_watch[watch];
            }                 

            watch_num--;
            break;
        }
    }


}

int events_in(int highest_fd, fd_set *watchfds)
{
    int available;

    struct timeval timeout = {.tv_sec = 4, .tv_usec = 0};
    available = select(highest_fd, watchfds, NULL, NULL, &timeout);

    return available;
}

void parse_events(int e_status, char e_buf[], ParrotObject *p_obj)
{
    int events, cur_watch;
    struct inotify_event *event;
    struct watch_trigger *accessed;

    void *backup;
    time_t access_time;

    pthread_t backup_file;

    events = 0;
    accessed = malloc(sizeof *accessed);
    if (accessed == NULL) {
        log_error("notify_parrot.c", "parse_events", "malloc", 161);
        return;
    }

    while (events < e_status) { 
        event = (struct inotify_event *) &e_buf[events];
        access_time = time(NULL);
        if (event->mask != IN_ACCESS)
            return;

        log_event("event => ", 1, event->name);

        for (cur_watch=0; cur_watch < watch_num; cur_watch++) {
            if (current_watch[cur_watch]->parrot_wd == event->wd) {
                accessed->dir = current_watch[cur_watch]->dir;
                accessed->file = event->name;
                break;
            }
        }

        pthread_create(&backup_file, NULL, (void *) find_file, accessed);
        pthread_join(backup_file, &backup);
        parrot_obj_accessed(p_obj, (int) access_time);            

        if (backup) 
            log_error("notify_parrot.c", "parse_events", "find_file", 183);

        events += EVT_SIZE + event->len;
    }
}
