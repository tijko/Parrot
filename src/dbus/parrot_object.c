#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "../parrot.h"


// Enumerate any signals to be defined, adding any new signals in before the
// LAST_SIGNAL in the future if extending.

enum {
    NOTIFY_SIGNAL,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(ParrotObject, parrot_obj, G_TYPE_OBJECT);

gboolean parrot_obj_accessed(ParrotObject *p_obj, int access_time);
gboolean parrot_obj_current_watches(ParrotObject *p_obj, 
                                    DBusGMethodInvocation *ctxt);

gboolean parrot_obj_add_watch(ParrotObject *p_obj, char *watch,
                               DBusGMethodInvocation *ctxt);

#include "parrot_object.h"

static void parrot_obj_init(ParrotObject *p_obj)
{
}

static void parrot_obj_class_init(ParrotObjectClass *klass)
{
    signals[NOTIFY_SIGNAL] =
        g_signal_new("accessed", G_OBJECT_CLASS_TYPE (klass),
        G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL,
        g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

gboolean parrot_obj_accessed(ParrotObject *p_obj, int access_time)
{
    g_signal_emit(p_obj, signals[NOTIFY_SIGNAL], 0, access_time);
    return TRUE;
}

gboolean parrot_obj_current_watches(ParrotObject *p_obj, 
                                    DBusGMethodInvocation *ctxt)
{
    int watch;

    char *sender;
    char **current_watches;

    sender = dbus_g_method_get_sender(ctxt);
    current_watches = g_new(char *, watch_num + 1);

    for (watch=0; watch < watch_num; watch++)
        current_watches[watch] = g_strdup(current_watch[watch]->dir);

    current_watches[watch] = NULL;
    dbus_g_method_return(ctxt, current_watches);

    g_free(sender);
    return TRUE;
}

gboolean parrot_obj_add_watch(ParrotObject *p_obj, char *watch,
                              DBusGMethodInvocation *ctxt)
{
    char *sender;

    sender = dbus_g_method_get_sender(ctxt);
    g_free(sender);

    parrot_add_watch((char *) watch);
    dbus_g_method_return(ctxt);

    return TRUE;
}

void register_parrot_obj(ParrotObject *p_obj)
{
    GMainLoop *mainloop;
    DBusGConnection *conn;
    DBusConnection *dconn;
    DBusGProxy *proxy;
    GError *err;

    guint result;
    char *path, *name, *iface, *addr;
    
    mainloop = g_main_loop_new(NULL, FALSE);
    dbus_g_object_type_install_info(VALUE_TYPE_OBJECT, 
                                    &dbus_glib_parrot_obj_object_info);

    path = "/org/Parrot";
    name = "org.Parrot";
    iface = "org.Parrot.Inotify";

    err = NULL;
    conn = dbus_g_bus_get(DBUS_BUS_SESSION, &err);

    if (err != NULL) {
        dbus_conn_err(err->code);
    } else {
        addr = getenv("DBUS_SESSION_BUS_ADDRESS");

        if (addr)
            log_dbus(addr);
        else
            dbus_addr_err();
    }

    dconn = dbus_g_connection_get_connection(conn);

    dbus_bus_request_name(dconn, name, DBUS_NAME_FLAG_REPLACE_EXISTING, NULL);

    proxy = dbus_g_proxy_new_for_name(conn, name, path, iface);
    dbus_g_proxy_call(proxy, "RequestName", &err, G_TYPE_STRING, name, 
                      G_TYPE_UINT, 0, G_TYPE_INVALID, G_TYPE_UINT, &result,
                      G_TYPE_INVALID);

    dbus_g_connection_register_g_object(conn, path, G_OBJECT(p_obj));

    g_main_loop_run(mainloop);
}
