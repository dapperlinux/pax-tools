Name: paxctld
Version: 1.2.1
Release: 22
Summary: PaX flags maintenance daemon
Group: admin
License: GPLv2
Requires(post): chkconfig
Requires(preun): chkconfig
Requires(preun): initscripts
Requires(postun): initscripts
URL: https://grsecurity.net
Source: paxctld-1.2.1.tar.xz

%description
paxctld is a daemon that automatically applies PaX flags to binaries on
the system.  These flags are applied via user extended attributes and are
refreshed on any update to the binaries specified in its configuration file.

%package systemd
Summary: PaX flags maintenance daemon
Group: admin
Requires(post): systemd
Requires(preun): systemd
Requires(postun): systemd
%description systemd
paxctld is a daemon that automatically applies PaX flags to binaries on
the system.  These flags are applied via user extended attributes and are
refreshed on any update to the binaries specified in its configuration file.
This package supports those who have been forced to run only systemd by their
distro.

%define debug_package %{nil}

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
%make_install
install -d $RPM_BUILD_ROOT/etc/rc.d/init.d
install -d $RPM_BUILD_ROOT/etc/systemd/system
install -m755 rpm/paxctld.init $RPM_BUILD_ROOT/etc/rc.d/init.d/paxctld
install -m755 rpm/paxctld.service $RPM_BUILD_ROOT/etc/systemd/system

%post
# This adds the proper /etc/rc*.d links for the script
/sbin/chkconfig --add paxctld
/sbin/service paxctld start >/dev/null 2>&1

%post systemd
/usr/bin/systemctl enable paxctld.service >/dev/null 2>&1 || :

%preun
if [ $1 -eq 0 ] ; then
    /sbin/service paxctld stop >/dev/null 2>&1
    /sbin/chkconfig --del paxctld
fi

%preun systemd
%systemd_preun paxctld.service

%postun
if ["$!" -ge "1" ] ; then
    /sbin/service paxctld condrestart >/dev/null 2>&1 || :
fi

%postun systemd
%systemd_postun paxctld.service

%files
%defattr(-,root,root)
%attr(0755,root,root) /sbin/paxctld
%attr(0644,root,root) %{_mandir}/man8/paxctld.8.gz
%attr(0644,root,root) %config(noreplace) %{_sysconfdir}/paxctld.conf
%attr(0755,root,root) %config %{_sysconfdir}/rc.d/init.d/paxctld
%doc

%files systemd
%defattr(-,root,root)
%attr(0755,root,root) /sbin/paxctld
%attr(0644,root,root) %{_mandir}/man8/paxctld.8.gz
%attr(0644,root,root) %config(noreplace) %{_sysconfdir}/paxctld.conf
%attr(0755,root,root) %config %{_sysconfdir}/systemd/system/paxctld.service
%doc

%changelog
* Sat Jul 23 2016 Brad Spengler <spender@grsecurity.net> 1.2.1-1
- Updated default paxctld.conf to improve Ubuntu usability
- Updated chrome/firefox settings now that the anti-defense
  "features" of firefox in particular have been fixed

* Sun Jul 17 2016 Brad Spengler <spender@grsecurity.net> 1.2-1
- Added support for filesystems that don't provide d_type, from Damir Vandic:
  https://forums.grsecurity.net/viewtopic.php?f=1&t=4484

* Tue Feb 9 2016 Brad Spengler <spender@grsecurity.net> 1.1-1
- Added support for pathnames with spaces, from Austin Seipp

* Wed Dec 23 2015 Brad Spengler <spender@grsecurity.net> 1.0-4
- Fixed a non-security-relevant crash on too-large paxctld.conf files,
  reported by Jurriaan Bremer
- Fixed the xattrs set by paxctld -- due to the nonintuitivity of PaX
  xattr parsing which differed from some previous marking methods, it
  ended up being the case that if EMUTRAMP was enabled in the kernel,
  all binaries marked with xattrs via paxctld were running with EMUTRAMP
  enabled.  Running the updated version of paxctld will fix the issue
  on binaries listed in paxctld.conf.  This issue was found via an internal
  audit.

* Thu Oct 29 2015 Brad Spengler <spender@grsecurity.net> 1.0-3
- Added support for reading extra config files from /etc/paxctld.d
  Files beginning with '.' are ignored, as are any non-regular files
  in the directory

* Thu Mar 12 2015 Brad Spengler <spender@grsecurity.net> 1.0
- Added systemd-specific subpackage from 'tweek' on the forums,

* Wed Dec 17 2014 Brad Spengler <spender@grsecurity.net> 1.0
- Initial release
