#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/inotify.h>

#include "parrot.h"


int notify_parrot_init(void) 
{
    ParrotObject *p_obj;
    pthread_t parrot_dbus;

    log_parrot();

    p_obj = g_object_new(VALUE_TYPE_OBJECT, NULL);

    // thread to register the parrot object to dbus and contain the dbus
    // mainloop
    pthread_create(&parrot_dbus, NULL, (void *) register_parrot_obj, p_obj);

    if ((parrot_inotify_instance = inotify_init()) == -1) {
        log_err("INOTIFY_INIT");
        return -1;
    }

    watch_num = 0;
    parrot_mainloop(p_obj);

    return 0;
}

void parrot_mainloop(ParrotObject *p_obj)
{
    int change, status;
    char buffer[EVT_BUF_SIZE];

    fd_set watchfds;
    FD_ZERO(&watchfds);

    while (true) {

	FD_SET(parrot_inotify_instance, &watchfds);
	change = events_in(parrot_inotify_instance + 1, &watchfds);
	FD_ZERO(&watchfds);

	if (change == -1) {
	    log_err("SELECT CALL");
	    return;
	} else if (change) {
	    if ((status = read(parrot_inotify_instance, 
                               buffer, EVT_BUF_SIZE)) < 0) {
		log_err("READ CALL");
		return;
	    }

	    parse_events(status, buffer, p_obj);

	}
    }
}

void parrot_add_watch(char *path)
{
    int watch;
    struct parrot_watch *new_watch;

    new_watch = malloc(sizeof *new_watch);

    if ((watch = inotify_add_watch(parrot_inotify_instance, 
                                   path, IN_ACCESS)) == -1) {
        log_err("INOTIFY_ADD_WATCH");
        return;
    }

    new_watch->dir = malloc(sizeof(char) * strlen(path));
    strcpy(new_watch->dir, path);
    new_watch->parrot_wd = watch;

    current_watch[watch_num++] = new_watch;
    // XXX -> add log function
}

void parrot_remove_watch(char *path)
{
    int watch, idx;

    for (watch=0; watch < watch_num; watch++) {
        if (!strcmp(path, current_watch[watch]->dir)) {
            free(current_watch[watch]->dir);
            free(current_watch[watch]);
            break;
        }
    }

    if (watch != watch_num) {
        for (watch=0, idx=0; watch < watch_num; watch++) {
            if (current_watch[watch])
                current_watch[idx++] = current_watch[watch];
        }                 

        watch_num--;
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

    while (events < e_status) { 
        event = (struct inotify_event *) &e_buf[events];
        access_time = time(NULL);
        if (event->mask != IN_ACCESS)
            return;

        log_evt(event->name, event->mask);

        for (cur_watch=0; cur_watch < watch_num; cur_watch++) {
            if (current_watch[cur_watch]->parrot_wd == event->wd) {
                accessed->dir = current_watch[cur_watch]->dir;
                accessed->file = event->name;
                break;
            }
        }

        pthread_create(&backup_file, NULL, (void *) find_files, accessed);
        pthread_join(backup_file, &backup);
        parrot_obj_accessed(p_obj, (int) access_time);            

        if (backup) 
            log_err("BACKUP");

        events += EVT_SIZE + event->len;
    }
}
