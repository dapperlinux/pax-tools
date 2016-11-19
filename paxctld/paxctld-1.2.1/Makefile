# Copyright 2014-2015 Open Source Security, Inc.
# paxctld is licensed under the GNU GPL v2 only http://www.gnu.org
# see COPYRIGHT and LICENSE files for more information

CC=/usr/bin/gcc
CFLAGS?=-Wall -O2 -pie -fPIC -D_FORTIFY_SOURCE=2 -fstack-protector-all
LDFLAGS=
STRIP=/usr/bin/strip
MANDIR=/usr/share/man
INSTALL=/usr/bin/install -c
DESTDIR=

all: paxctld

paxctld: paxctld.c

install: paxctld paxctld.8 paxctld.conf
	@echo "Installing paxctld.conf..."
	@mkdir -p $(DESTDIR)/etc
	@$(INSTALL) -m 0644 paxctld.conf $(DESTDIR)/etc
	@echo "Installing paxctld..."
	@mkdir -p $(DESTDIR)/sbin
	@$(INSTALL) -m 0755 paxctld $(DESTDIR)/sbin
	@$(STRIP) $(DESTDIR)/sbin/paxctld
	@echo "Installing paxctld manpage..."
	@mkdir -p $(DESTDIR)$(MANDIR)/man8
	@$(INSTALL) -m 0644 paxctld.8 $(DESTDIR)$(MANDIR)/man8/paxctld.8

clean:
	rm -rf core *.o paxctld debian/paxctld debian/paxctld.debhelper.log
