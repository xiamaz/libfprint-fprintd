/*
 * fprintd example to enroll right index finger
 * Copyright (C) 2008 Daniel Drake <dsd@gentoo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <dbus/dbus-glib-bindings.h>
#include "manager-dbus-glue.h"
#include "device-dbus-glue.h"
#include "marshal.h"

static DBusGProxy *manager = NULL;
static DBusGConnection *connection = NULL;

static void create_manager(void)
{
	GError *error = NULL;

	connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (connection == NULL) {
		g_print("Failed to connect to session bus: %s\n", error->message);
		exit (1);
	}

	manager = dbus_g_proxy_new_for_name(connection,
		"net.reactivated.Fprint", "/net/reactivated/Fprint/Manager",
		"net.reactivated.Fprint.Manager");
}

static DBusGProxy *open_device(const char *username)
{
	GError *error = NULL;
	gchar *path;
	DBusGProxy *dev;

	if (!net_reactivated_Fprint_Manager_get_default_device(manager, &path, &error)) {
		g_print("list_devices failed: %s\n", error->message);
		exit (1);
	}
	
	if (path == NULL) {
		g_print("No devices found\n");
		exit(1);
	}

	g_print("Using device %s\n", path);

	/* FIXME use for_name_owner?? */
	dev = dbus_g_proxy_new_for_name(connection, "net.reactivated.Fprint",
		path, "net.reactivated.Fprint.Device");

	g_free (path);

	if (!net_reactivated_Fprint_Device_claim(dev, username, &error)) {
		g_print("failed to claim device: %s\n", error->message);
		exit (1);
	}
	return dev;
}

static void enroll_result(GObject *object, const char *result, gboolean done, void *user_data)
{
	gboolean *enroll_completed = user_data;
	g_print("Enroll result: %s\n", result);
	if (done != FALSE)
		*enroll_completed = TRUE;
}

static void do_enroll(DBusGProxy *dev)
{
	GError *error;
	gboolean enroll_completed = FALSE;

	dbus_g_proxy_add_signal(dev, "EnrollStatus", G_TYPE_STRING, G_TYPE_BOOLEAN, NULL);
	dbus_g_proxy_connect_signal(dev, "EnrollStatus", G_CALLBACK(enroll_result),
				    &enroll_completed, NULL);

	g_print("Enrolling right index finger.\n");
	if (!net_reactivated_Fprint_Device_enroll_start(dev, "right-index-finger", &error)) {
		g_print("EnrollStart failed: %s\n", error->message);
		exit (1);
	}

	while (!enroll_completed)
		g_main_context_iteration(NULL, TRUE);

	dbus_g_proxy_disconnect_signal(dev, "EnrollStatus",
		G_CALLBACK(enroll_result), &enroll_completed);

	if (!net_reactivated_Fprint_Device_enroll_stop(dev, &error)) {
		g_print("VerifyStop failed: %s\n", error->message);
		exit(1);
	}
}

static void release_device(DBusGProxy *dev)
{
	GError *error = NULL;
	if (!net_reactivated_Fprint_Device_release(dev, &error)) {
		g_print("ReleaseDevice failed: %s\n", error->message);
		exit (1);
	}
}

int main(int argc, char **argv)
{
	GMainLoop *loop;
	DBusGProxy *dev;
	char *username;

	g_type_init();

	dbus_g_object_register_marshaller (fprintd_marshal_VOID__STRING_BOOLEAN,
					   G_TYPE_NONE, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_INVALID);

	loop = g_main_loop_new(NULL, FALSE);
	create_manager();

	username = NULL;
	if (argc == 2)
		username = argv[1];
	dev = open_device(username);
	do_enroll(dev);
	release_device(dev);
	return 0;
}

