/* radare - LGPL - Copyright 2008-2018 - pancake */

#include "r_io.h"
#include "config.h"
#include <stdio.h>

volatile static RIOPlugin *DEFAULT = NULL;

static RIOPlugin *io_static_plugins[] = {
	R_IO_STATIC_PLUGINS
};

R_API bool r_io_plugin_add(RIO *io, RIOPlugin *plugin) {
	if (!io || !io->plugins || !plugin || !plugin->name) {
		return false;
	}
	ls_append (io->plugins, plugin);
	return true;
}

R_API bool r_io_plugin_init(RIO *io) {
	RIOPlugin *static_plugin;
	int i;
	if (!io) {
		return false;
	}
	io->plugins = ls_newf (free);
	for (i = 0; io_static_plugins[i]; i++) {
		if (!io_static_plugins[i]->name) {
			continue;
		}
		static_plugin = R_NEW0 (RIOPlugin);
		if (!static_plugin) {
			return false;
		}
		memcpy (static_plugin, io_static_plugins[i], sizeof (RIOPlugin));
		r_io_plugin_add (io, static_plugin);
	}
	return true;
}

R_API RIOPlugin *r_io_plugin_get_default(RIO *io, const char *filename, bool many) {
	if (!DEFAULT || !DEFAULT->check || !DEFAULT->check (io, filename, many) ) {
		return NULL;
	}
	return (RIOPlugin*) DEFAULT;
}

R_API RIOPlugin *r_io_plugin_resolve(RIO *io, const char *filename, bool many) {
	SdbListIter *iter;
	RIOPlugin *ret;
	ls_foreach (io->plugins, iter, ret) {
		if (!ret || !ret->check) {
			continue;
		}
		if (ret->check (io, filename, many)) {
			return ret;
		}
	}
	return r_io_plugin_get_default (io, filename, many);
}

R_API RIOPlugin *r_io_plugin_byname(RIO *io, const char *name) {
	SdbListIter *iter;
	RIOPlugin *iop;
	ls_foreach (io->plugins, iter, iop) {
		if (!strcmp (name, iop->name)) {
			return iop;
		}
	}
	return r_io_plugin_get_default (io, name, false);
}

R_API int r_io_plugin_list(RIO *io) {
	RIOPlugin *plugin;
	SdbListIter *iter;
	char str[4];
	int n = 0;

	ls_foreach (io->plugins, iter, plugin) {
		str[0] = 'r';
		str[1] = plugin->write ? 'w' : '_';
		str[2] = plugin->isdbg ? 'd' : '_';
		str[3] = 0;
		io->cb_printf ("%s  %-8s %s (%s)",
				str, plugin->name,
			plugin->desc, plugin->license);
		if (plugin->version) {
			io->cb_printf (" v%s", plugin->version);
		}
		if (plugin->author) {
			io->cb_printf (" %s", plugin->author);
		}
		io->cb_printf ("\n");
		n++;
	}
	return n;
}

R_API int r_io_plugin_list_json(RIO *io) {
	RIOPlugin *plugin;
	SdbListIter *iter;
	char str[4];
	int n = 0;
	io->cb_printf("{\"IO_Plugins\":[");
	ls_foreach (io->plugins, iter, plugin) {
		str[0] = 'r';
		str[1] = plugin->write ? 'w' : '_';
		str[2] = plugin->isdbg ? 'd' : '_';
		str[3] = 0;

		io->cb_printf ("%s{\"Permissions\":\"%s\",\"Name\":\"%s\",\"Description\":\"%s\",\"License\":\"%s\"",
				n? "," : "", str, plugin->name,
			plugin->desc, plugin->license);
		if (plugin->version) {
			io->cb_printf (",\"version\":\"%s\"", plugin->version);
		}
		if (plugin->author) {
			io->cb_printf (",\"plugin\":\"%s\"", plugin->author);
		}
		io->cb_printf ("}");
		n++;
	}
	io->cb_printf("]}");
	return n;
}
