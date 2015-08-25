#define _GNU_SOURCE

#include <pthread.h>
#include <sys/inotify.h>

#include "parrot.h"


int notify_parrot_init(void) 
{
    struct ParrotGDBusObj *parrot_gdbus_obj;
    pthread_t parrot_dbus;

    log_event("initializing parrot loop");

    parrot_gdbus_obj = malloc(sizeof *parrot_gdbus_obj);
    if (parrot_gdbus_obj == NULL) {
        log_error(__FILE__, "notify_parrot_init", "malloc", __LINE__, errno);
        return -1;
    }

    parrot_gdbus_obj->p_obj = g_object_new(VALUE_TYPE_OBJECT, NULL);

    // thread to register the parrot object to dbus and contain the dbus
    // mainloop
    pthread_create(&parrot_dbus, NULL, (void *) register_parrot_obj, 
                                                   parrot_gdbus_obj);

    if ((parrot_inotify_instance = inotify_init()) == -1) {
        log_error(__FILE__, "notify_parrot_init", "inotify_init", 
                  __LINE__, errno);
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
    log_event("clean up and close connections");
}

void close_watch(struct parrot_watch *watch)
{
    // XXX read off rest of events 
    inotify_rm_watch(parrot_inotify_instance, watch->parrot_wd);
    free(watch->watch_path);
    free(watch->backup_path);
    free(watch->evfile);
}

void parrot_mainloop(struct ParrotGDBusObj *parrot_gdbus_obj)
{
    int change, status;
    char buffer[EVT_BUF_SIZE];

    fd_set watchfds;
    FD_ZERO(&watchfds);

    while (running) {

        FD_SET(parrot_inotify_instance, &watchfds);
        change = events_in(parrot_inotify_instance + 1, &watchfds);
        FD_ZERO(&watchfds);

        if (change == -1) {
            log_error(__FILE__, "parrot_mainloop", "events_in", __LINE__, errno);
            return;
        } else if (change) {
            if ((status = read(parrot_inotify_instance, 
                                   buffer, EVT_BUF_SIZE - 1)) < 0) {
                log_error(__FILE__, "parrot_mainloop", "read", __LINE__, errno);
                return;
            }

            parse_events(status, buffer, parrot_gdbus_obj->p_obj);

        }
    }

    parrot_cleanup(parrot_gdbus_obj);
}

int parrot_add_watch(char *watch_path, char *backup_path, int watch_mask)
{
    int watch, f_open, d_open;
    struct parrot_watch *new_watch;

    if (watch_mask != W_FIL && watch_mask != W_DIR) {
        log_error(__FILE__, "parrot_add_watch", "bad watch mask", __LINE__, EINVAL);
        return -1;
    }

    d_open = open(backup_path, O_RDONLY | O_DIRECTORY);
    if (d_open == -1)
        return -1;
    else
        close(d_open);
 
    if (watch_mask & W_FIL) {
        f_open = open(watch_path, O_RDONLY);
        d_open = open(watch_path, O_RDONLY | O_DIRECTORY);    
        if (f_open == -1 || d_open != -1) {
            close(f_open);
            close(d_open);
            log_error(__FILE__, "parrot_add_watch", "open", __LINE__, errno);
            return -1;
        }
    } else {
        d_open = open(watch_path, O_RDONLY | O_DIRECTORY);
        if (d_open == -1) {
            log_error(__FILE__, "parrot_add_watch", "open", __LINE__, errno);
            return -1;
        }
    }

    new_watch = malloc(sizeof *new_watch);
    if (new_watch == NULL) {
        log_error(__FILE__, "parrot_add_watch",
                  "malloc", __LINE__, errno);
        return -1;
    }

    if ((watch = inotify_add_watch(parrot_inotify_instance, 
                                   watch_path, IN_ACCESS)) == -1) {
        free(new_watch);
        log_error(__FILE__, "parrot_add_watch", 
                  "inotify_add_watch", __LINE__, errno);
        return -1;
    }

    int path_len = strlen(watch_path);
    if (watch_path[path_len - 1] != '/' && (watch_mask & W_DIR))
        path_len += 1;

    new_watch->watch_path = malloc(sizeof(char) * path_len + 1);
    if (new_watch->watch_path == NULL) {
        log_error(__FILE__, "parrot_add_watch",
                  "malloc", __LINE__, errno);
        return -1;
    }

    strcpy(new_watch->watch_path, watch_path);
    path_len = strlen(watch_path);
    if (watch_path[path_len - 1] != '/' && (watch_mask & W_DIR)) {
        new_watch->watch_path[path_len] = '/';
        new_watch->watch_path[path_len + 1] = '\0';
    }

    int backup_path_len = strlen(backup_path);
    if (backup_path[backup_path_len - 1] != '/')
        new_watch->backup_path_len = backup_path_len + 2;
    else
        new_watch->backup_path_len = backup_path_len + 1;

    new_watch->backup_path = malloc(sizeof(char) * new_watch->backup_path_len);
    if (new_watch->backup_path == NULL) {
        log_error(__FILE__, "parrot_add_watch",
                  "malloc", __LINE__, errno);
        return -1;
    }

    strcpy(new_watch->backup_path, backup_path);
    if (backup_path[backup_path_len - 1] != '/') {
        new_watch->backup_path[backup_path_len] = '/';
        new_watch->backup_path[backup_path_len + 1] = '\0';
    }

    new_watch->parrot_wd = watch;
    new_watch->watch_type = watch_mask;
    new_watch->backup = watch_mask & W_FIL ? find_file : find_files;

    if (new_watch->watch_type & W_FIL) 
        set_evfile(new_watch);
    else
        new_watch->evfile = NULL;

    current_watch[watch_num++] = new_watch;

    char *fmt = fmt_event(2);
    if (fmt == NULL)
        return 0;

    char *logevent_buffer;

    EVENT(&logevent_buffer, fmt, "watch added => ", watch_path);

    log_event(logevent_buffer);
    free(fmt);
    free(logevent_buffer);

    return 0;
}

void parrot_remove_watch(char *watch_path)
{
    int watch, idx;

    for (watch=0; watch < watch_num; watch++) {
        if (!strcmp(watch_path, current_watch[watch]->watch_path)) {
            close_watch(current_watch[watch]);
            current_watch[watch] = NULL;
            char *fmt = fmt_event(2);
            if (fmt == NULL)
                return;
            char *logevent_buffer;
            EVENT(&logevent_buffer, fmt, "watch removed => ", watch_path);
            log_event(logevent_buffer);
            free(fmt);
            free(logevent_buffer);
            for (idx=watch; watch < watch_num; watch++) 
                if (current_watch[watch])
                    current_watch[idx++] = current_watch[watch];
            watch_num--;
            return;
        }
    }
}

void set_evfile(struct parrot_watch *watch)
{
    char *path, *prev_path, *temp_path, *watch_path;

    watch_path = malloc(sizeof(char) * (strlen(watch->watch_path) + 1));
    strcpy(watch_path, watch->watch_path);
    prev_path = NULL;
    strtok(watch_path, "/");

    while ((path = strtok(NULL, "/"))) {
        temp_path = realloc(prev_path, sizeof(char) * (strlen(path) + 1));
        strcpy(temp_path, path);
        prev_path = temp_path;
    } 

    watch->evfile = malloc(sizeof(char) * (strlen(prev_path) + 1));
    strcpy(watch->evfile, prev_path);
    watch->watch_flag = 0;
    free(prev_path);
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
    struct parrot_watch *accessed;

    void *backup;
    time_t access_time;

    pthread_t backup_file;

    events = 0;
    accessed = malloc(sizeof *accessed);
    if (accessed == NULL) {
        log_error(__FILE__, "parse_events", "malloc", __LINE__, errno);
        goto stop_events;
    }

    while (events < e_status) { 
        event = (struct inotify_event *) &e_buf[events];
        access_time = time(NULL);
        if (event->mask != IN_ACCESS) 
            goto stop_events;            

        for (cur_watch=0; cur_watch < watch_num; cur_watch++) {
            if (current_watch[cur_watch]->parrot_wd == event->wd) {
                memcpy(accessed, current_watch[cur_watch], sizeof(*accessed));
                if (!(accessed->watch_type & W_FIL)) {
                    if (accessed->evfile)
                        free(accessed->evfile);
                    accessed->evfile = malloc(sizeof(char) * 
                                              strlen(event->name) + 1);
                    strcpy(accessed->evfile, event->name);
                    char *fmt = fmt_event(2);
                    char *logevent_buffer;
                    EVENT(&logevent_buffer, fmt, "event => ", accessed->evfile);
                    log_event(logevent_buffer);
                    free(fmt);
                    free(logevent_buffer);
                } else {
                    if (current_watch[cur_watch]->watch_flag & W_FLAG) {
                        current_watch[cur_watch]->watch_flag ^= W_FLAG;
                        goto stop_events;
                    } else {
                        current_watch[cur_watch]->watch_flag |= W_FLAG;
                        char *fmt = fmt_event(2);
                        char *logevent_buffer;
                        EVENT(&logevent_buffer, fmt, "event => ", 
                                                 accessed->evfile);
                        log_event(logevent_buffer);
                        free(fmt);
                        free(logevent_buffer);
                    }
                }

                break;
            }
        }

        pthread_create(&backup_file, NULL, (void *) accessed->backup, accessed);
        pthread_join(backup_file, &backup);
        parrot_obj_accessed(p_obj, (int) access_time);            

        if (backup) 
            log_error(__FILE__, "parse_events", "find_file", __LINE__, errno);

        events += EVT_SIZE + event->len;
    }

    stop_events:
        free(accessed);
}
