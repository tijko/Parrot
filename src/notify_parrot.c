#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/inotify.h>

#include "parrot.h"


int notify_parrot(void) 
{
    int fd, status, watch, change;
    
    char buffer[EVT_BUF_SIZE];
    ParrotObject *p_obj;

    fd_set watchfds;
    FD_ZERO(&watchfds);
    pthread_t parrot_dbus;
    log_parrot();

    p_obj = g_object_new(VALUE_TYPE_OBJECT, NULL);

    // thread to register the parrot object to dbus and contain the dbus
    // mainloop
    pthread_create(&parrot_dbus, NULL, (void *) register_parrot_obj, p_obj);

    if ((fd = inotify_init()) == -1) {
        log_err(PARROT_PATH);
        return -1;
    }

    if ((watch = inotify_add_watch(fd, PARROT_PATH, IN_ACCESS)) == -1) {
        log_err(PARROT_PATH);
        return -1;
    }

    while (true) {
        FD_SET(fd, &watchfds);
        change = events_in(fd + 1, &watchfds);
        FD_ZERO(&watchfds);

        if (change == -1) {
            log_err("SELECT CALL");
            return -1;
        } else if (change) {
            if ((status = read(fd, buffer, EVT_BUF_SIZE)) < 0) {
                log_err(PARROT_PATH);
                return -1;
            }

            parse_events(status, buffer, p_obj);

        }
    }

    return 0;
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
    int events;
    struct inotify_event *event;
    void *backup;
    time_t access_time;

    pthread_t backup_file;

    events = 0;

    while (events < e_status) { 
        event = (struct inotify_event *) &e_buf[events];
        access_time = time(NULL);
        if (event->mask != IN_ACCESS)
            return;

        log_evt(event->name, event->mask);

        pthread_create(&backup_file, NULL, (void *) find_files, event->name);
        pthread_join(backup_file, &backup);
        parrot_obj_accessed(p_obj, (int) access_time);            

        if (backup) 
            log_err(PARROT_PATH);

        events += EVT_SIZE + event->len;
    }
}
