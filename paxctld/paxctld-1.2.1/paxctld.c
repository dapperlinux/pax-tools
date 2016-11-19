/*
 * Copyright 2012-2016 Open Source Security, Inc.
 *
 * This file is part of paxctld.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/xattr.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <syslog.h>
#include <limits.h>
#include <signal.h>
#include <dirent.h>

#define DEFAULT_PAXCTLD_CONF "/etc/paxctld.conf"
#define DEFAULT_PAXCTLD_CONF_DIR "/etc/paxctld.d"

#define MAX_CONFIG_ENTRIES 16384

#define INOTIFY_FLAGS (IN_DONT_FOLLOW | IN_ATTRIB | IN_CREATE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVED_TO)
#define DIR_INOTIFY_FLAGS (INOTIFY_FLAGS &~ IN_ATTRIB)


#define PAX_PAGEEXEC_ON  0x00000001
#define PAX_SEGMEXEC_ON  0x00000002
#define PAX_MPROTECT_ON  0x00000004
#define PAX_ASLR_ON      0x00000008
#define PAX_EMUTRAMP_ON  0x00000010

#define PAX_DEFAULT_FLAGS	\
	(PAX_PAGEEXEC_ON|PAX_SEGMEXEC_ON|PAX_MPROTECT_ON|PAX_ASLR_ON)

struct conf_entry {
	char *requested_path;
	char *existing_path;
	unsigned int pax_flags;
	int watch_id;
	int nonroot;
};

struct paxctld_config {
	struct conf_entry *entries;
	unsigned int count;
};

static struct paxctld_config config;
static int ino = -1;

static int do_daemonize;
static char *pidfile;

static int quiet;
#define gr_syslog(level, ...) do {				\
	if (!quiet) {						\
		if (do_daemonize)				\
			syslog(level, ## __VA_ARGS__);		\
		else						\
			fprintf(stderr, ## __VA_ARGS__);	\
	}							\
} while (0)

static char *gr_strdup(const char *str)
{
	char *ret = strdup(str);
	if (ret == NULL) {
		fprintf(stderr, "Unable to allocate memory.\n");
		exit(EXIT_FAILURE);
	}	return ret;
}

static void decode_pax_flags(unsigned int flags, char *buf)
{
	buf[0] = '\0';
	if (!(flags & PAX_EMUTRAMP_ON))
		strcat(buf, "e");
	if (!(flags & PAX_PAGEEXEC_ON))
		strcat(buf, "p");
	if (!(flags & PAX_SEGMEXEC_ON))
		strcat(buf, "s");
	if (!(flags & PAX_MPROTECT_ON))
		strcat(buf, "m");
	if (!(flags & PAX_ASLR_ON))
		strcat(buf, "r");
}

static unsigned int encode_pax_flags(const char *conf)
{
	unsigned int ret = PAX_DEFAULT_FLAGS;
	const char *p = conf;
	while (*p) {
		switch (*p) {
		case 'E':
			ret |= PAX_EMUTRAMP_ON;
			break;
		case 'p':
			ret &= ~PAX_PAGEEXEC_ON;
			break;
		case 'm':
			ret &= ~PAX_MPROTECT_ON;
			break;
		case 'r':
			ret &= ~PAX_ASLR_ON;
			break;
		case 's':
			ret &= ~PAX_SEGMEXEC_ON;
			break;
		default:
			fprintf(stderr, "Unknown character: \"%c\" in PaX configuration string: \"%s\".  Permitted characters are: \"pEmrs\".\n", *p, conf);
			exit(EXIT_FAILURE);
		}
		p++;
	};

	return ret;
}

static char *get_parent_dir(char *path)
{
	char *tmpp = strrchr(path, '/');
	if (tmpp) {
		if (tmpp == path)
			tmpp[1] = '\0';
		else
			tmpp[0] = '\0';
	}

	return path;
}

static void init_watches(void);

static int add_watch(int inotify, const char *pathname, unsigned int flags)
{
	int ret = inotify_add_watch(inotify, pathname, flags);
	if (ret == -1 && errno == ENOSPC) {
		init_watches();
		ret = inotify_add_watch(inotify, pathname, flags);
	}
	return ret;
}

static void add_watch_to_closest_path(struct conf_entry *entry)
{
	char tmp[4096];
	strncpy(tmp, entry->requested_path, sizeof(tmp));
	int id = add_watch(ino, entry->requested_path, INOTIFY_FLAGS);
	while (id == -1 && errno == ENOENT && strcmp(tmp, "/")) {
		// keep stripping path components until we reach an existing directory
		get_parent_dir(tmp);
		id = add_watch(ino, tmp, DIR_INOTIFY_FLAGS);
	}
	entry->watch_id = id;
	if (entry->existing_path)
		free(entry->existing_path);
	entry->existing_path = gr_strdup(tmp);
}

/* do safe setting of user xattrs */
static int set_xattr(struct conf_entry *entry)
{
	struct stat st;
	uid_t linkuid;
	char path[PATH_MAX+1];
	int ret;
	char pax_flags_str[16];

	if (lstat(entry->requested_path, &st)) {
		if (errno == ENOENT)
			return 0;
		goto error;
	}

	linkuid = st.st_uid;
	if (!entry->nonroot && linkuid)
		goto error2;

	if (!realpath(entry->requested_path, path))
		return 0;

	if (lstat(path, &st)) {
		if (errno == ENOENT)
			return 0;
		goto error;
	}

	if (entry->nonroot && st.st_uid != linkuid && linkuid)
		goto error2;

	// prevent duplicate events
	inotify_rm_watch(ino, entry->watch_id);
	// create or replace as necessary

	decode_pax_flags(entry->pax_flags, pax_flags_str);

	ret = lsetxattr(path, "user.pax.flags", pax_flags_str, strlen(pax_flags_str), 0);
	add_watch_to_closest_path(entry);
	if (ret == -1) {
		if (errno == ENOENT)
			return 0;
		goto error;
	}

	return 1;
error:
	gr_syslog(LOG_ERR, "Unable to set extended attribute on \"%s\".  Error: %s\n", entry->requested_path, strerror(errno));
	exit(EXIT_FAILURE);
error2:
	gr_syslog(LOG_ERR, "Unable to set extended attribute on \"%s\".  Error: owner of symlink did not match that of target.\n", entry->requested_path);
	exit(EXIT_FAILURE);
}

static char *append_path(char *path, char *last)
{
	char *ret = calloc(1, strlen(path) + strlen(last) + 2);
	if (ret == NULL) {
		fprintf(stderr, "Unable to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if (strcmp(path, "/"))
		sprintf(ret, "%s/%s", path, last);
	else
		sprintf(ret, "/%s", last);

	return ret;
}

static void init_watches(void)
{
	unsigned int i;

	if (ino >= 0)
		close(ino);

	ino = inotify_init();
	if (ino < 0) {
		gr_syslog(LOG_ERR, "Fatal: Unable to initialize inotify system: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < config.count; i++) {
		struct conf_entry *entry = &config.entries[i];
		add_watch_to_closest_path(entry);
		/* mark the binary if possible */
		set_xattr(entry);
	}
}

static char* quoted_scan_string_nonroot = "\"%4095[^\"]\" %15s nonroot";
static char* quoted_scan_string         = "\"%4095[^\"]\" %15s";

static char* unquoted_scan_string_nonroot = "%4095s %15s nonroot";
static char* unquoted_scan_string         = "%4095s %15s";

static void parse_config(const char *confpath, struct paxctld_config *config)
{
	FILE *f = fopen(confpath, "r");
	char buf[8192] = { 0 };
	char *p;
	char path[4096] = { 0 };
	char flags[16] = { 0 };
	unsigned long lineno = 0;
	int nonroot;
	int ret;

	char *scan_string;
	char *scan_string_nonroot;

	if (f == NULL) {
		fprintf(stderr, "Unable to open configuration file: %s\nError: %s\n", confpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	while(fgets(buf, sizeof(buf) - 1, f)) {
		lineno++;

		p = buf;
		while (*p == ' ' || *p == '\t')
			p++;
		// ignore comment and empty lines
		if (*p == '#' || *p == '\n')
			continue;

		// if the path is quoted (i.e. has spaces), accomodate that
		if (*p == '"') {
			scan_string         = quoted_scan_string;
			scan_string_nonroot = quoted_scan_string_nonroot;
		} else {
			scan_string         = unquoted_scan_string;
			scan_string_nonroot = unquoted_scan_string_nonroot;
		}

		ret = sscanf(p, scan_string_nonroot, path, flags);
		if (ret != 2) {
			nonroot = 0;
			ret = sscanf(p, scan_string, path, flags);
		} else {
			nonroot = 1;
		}

		if (ret != 2) {
			fprintf(stderr, "Invalid configuration on line %lu of %s.\nSyntax is: </absolute/path> <PaX flags> [nonroot]\n", lineno, confpath);
			exit(EXIT_FAILURE);
		}

		if (config->count >= MAX_CONFIG_ENTRIES) {
			fprintf(stderr, "Exceeded maximum number of config entries.\n");
			exit(EXIT_FAILURE);
		}

		config->entries[config->count].nonroot = nonroot;
		config->entries[config->count].requested_path = gr_strdup(path);
		config->entries[config->count].pax_flags = encode_pax_flags(flags);

		config->count++;
	}

	fclose(f);
}

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-c config_file] [-d] [-p pid_file] [-q]\n", name);
	exit(EXIT_FAILURE);
}

static void handle_event(struct inotify_event *event, struct conf_entry *confentry)
{
	if ((event->mask & (IN_CREATE | IN_MOVED_TO)) && strcmp(confentry->existing_path, confentry->requested_path)) {
		char *p = append_path(confentry->existing_path, event->name);
		unsigned int plen = strlen(p);
		if (!strncmp(confentry->requested_path, p, plen) && (confentry->requested_path[plen] == '/' ||
								     confentry->requested_path[plen] == '\0')) {
			confentry->watch_id = add_watch(ino, p, INOTIFY_FLAGS);
			free(confentry->existing_path);
			confentry->existing_path = p;
			if (!strcmp(confentry->requested_path, p))
				gr_syslog(LOG_INFO, "File %s created.\n", confentry->existing_path);
		} else
			free(p);
	} else if (event->mask & (IN_DELETE_SELF | IN_MOVE_SELF)) {
		if (!strcmp(confentry->requested_path, confentry->existing_path))
			gr_syslog(LOG_INFO, "File %s deleted.\n", confentry->existing_path);
		get_parent_dir(confentry->existing_path);
		confentry->watch_id = add_watch(ino, confentry->existing_path, DIR_INOTIFY_FLAGS);
	} else if ((event->mask & IN_ATTRIB) && !strcmp(confentry->existing_path, confentry->requested_path)) {
		struct stat st;
		if ((lstat(confentry->existing_path, &st)) == -1 && errno == ENOENT) {
			// file was deleted
			gr_syslog(LOG_INFO, "File %s deleted.\n", confentry->existing_path);
			get_parent_dir(confentry->existing_path);
			confentry->watch_id = add_watch(ino, confentry->existing_path, DIR_INOTIFY_FLAGS);
		} else {
			gr_syslog(LOG_INFO, "File %s had its attributes changed.\n", confentry->existing_path);
		}
	}

	/* if after processing the existing file matches the requested file, then set
	   extended attributes on the file */
	if (!strcmp(confentry->existing_path, confentry->requested_path)) {
		// create or replace as necessary
		int ret = set_xattr(confentry);
		if (ret)
			gr_syslog(LOG_INFO, "Restored PaX flags on \"%s\" after update.\n", confentry->existing_path);
	}
}

static void daemonize(void)
{
	pid_t pid;

	pid = fork();
	if (pid == 0) {
		if (setsid() < 0)
			exit(EXIT_FAILURE);
		signal(SIGCHLD, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		pid = fork();
		if (pid == 0) {
			FILE *f;
			int i;
			if (chdir("/"))
				exit(EXIT_FAILURE);
			for (i = 0; i <= sysconf(_SC_OPEN_MAX); i++)
				if (i != ino)
					close(i);
			if (pidfile) {
				f = fopen(pidfile, "w");
				fprintf(f, "%u\n", getpid());
				fclose(f);
			}

			openlog("paxctld", 0, LOG_DAEMON);

			return;
		} else
			exit(EXIT_SUCCESS);
	} else
		exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	int opt;
	char *config_path = DEFAULT_PAXCTLD_CONF;
	unsigned int i;
	struct inotify_event *event;
	char flags[16] = { 0 };
	DIR *dir;

	if (argc < 1)
		usage("paxctld");

	while ((opt = getopt(argc, argv, "c:dp:q")) != -1) {
		switch (opt) {
		case 'c':
			config_path = gr_strdup(optarg);
			break;
		case 'd':
			do_daemonize = 1;
			break;
		case 'p':
			pidfile = gr_strdup(optarg);
			break;
		case 'q':
			quiet = 1;
			break;
		default:
			fprintf(stderr, "Unknown option: \"%c\".", opt);
			usage(argv[0]);
		}
	}

	if (getxattr("/proc/self/exe", "user.pax.flags", flags, sizeof(flags)-1) == -1 && errno == ENOTSUP) {
		fprintf(stderr, "Fatal: Filesystem extended attribute support is not enabled on the current running kernel.\n");
		exit(EXIT_FAILURE);
	}

	config.count = 0;
	config.entries = calloc(MAX_CONFIG_ENTRIES, sizeof(struct conf_entry));
	if (config.entries == NULL) {
		fprintf(stderr, "Unable to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	parse_config(config_path, &config);

	dir = opendir(DEFAULT_PAXCTLD_CONF_DIR);
	if (dir) {
		struct dirent *conf_file;
		char tmppath[PATH_MAX];

		while ((conf_file = readdir(dir))) {
			struct stat st;
			if (conf_file->d_name[0] == '.')
				continue;
			strcpy(tmppath, DEFAULT_PAXCTLD_CONF_DIR "/");
			strncat(tmppath, conf_file->d_name, sizeof(tmppath)-1);
			if (stat(tmppath, &st))
				continue;
			if (!S_ISREG(st.st_mode))
				continue;

			parse_config(tmppath, &config);
		}
		closedir(dir);
	}

	event = calloc(1, sizeof(struct inotify_event) + 4096);
	if (event == NULL) {
		fprintf(stderr, "Fatal: Unable to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if (do_daemonize)
		daemonize();

	gr_syslog(LOG_INFO, "paxctld initialized.\n");

	init_watches();

	while (read(ino, event, sizeof(struct inotify_event) + 4096) > 0) {
		// if it's a delete, we need to remove the watch on the file and add it to its parent directory
		for (i = 0; i < config.count; i++) {
			struct conf_entry *entry = &config.entries[i];
			if (event->wd == -1) {
				init_watches();
				break;
			}
			if (event->wd != entry->watch_id)
				continue;
			handle_event(event, entry);
		}
	}

	return 0;
}
