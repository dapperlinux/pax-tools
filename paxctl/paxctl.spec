Name: paxctl
Version: 0.9
Release: 1%{?dist}
Summary: PaX flags maintenance
Group: admin
License: GPLv2
BuildArch: x86_64

URL: https://pax.grsecurity.net
Source: https://pax.grsecurity.net/paxctl-0.9.tar.xz

%description
  This is paxctl for controlling PaX flags on a per binary basis. PaX
  is an intrusion prevention system that provides the best protection
  mechanisms against memory corruption bugs. Some applications are not
  compatible with certain features (due to design or bad engineering)
  and therefore they have to be exempted from certain enforcements. It
  is also possible to use PaX in soft mode where none of the protection
  mechanisms are active by default - here paxctl can be used to turn
  them on for selected programs (e.g., network daemons, programs that
  process network data such as mail clients, web browsers, etc).

  PaX and paxctl work on ELF executables, both of the standard ET_EXEC
  and the newer ET_DYN kind (older PaX releases referred to the latter
  as ET_DYN executables, these days they are called Position Independent
  Executables or PIEs for short).


%prep
%autosetup

%build
make

%install
mkdir -p %{buildroot}/sbin
mkdir -p %{buildroot}%{_mandir}/man1
make install DESTDIR=$RPM_BUILD_ROOT


%post

%files
/sbin/%name
%{_mandir}/man1/%name.1.gz

%changelog
* Tue Aug 30 2016 PaX Team <pageexec@freemail.hu>
- added support for files larger than 2G on 64 bit archs
  see https://bugs.gentoo.org/show_bug.cgi?id=520198

* Sun May 04 2014 PaX Team <pageexec@freemail.hu>
- fixed handling of ELF files whose ELF/PT headers aren't covered by a PT_LOAD segment, e.g., memcheck-amd64-linux from valgrind 
  thanks to Mathias Krause <minipli@googlemail.com>

* Fri Mar 23 2012 PaX Team <pageexec@freemail.hu>
- fixed gentoo bug #408377
  thanks to <wbrana@gmail.com> for reporting

* Sun Feb 19 2012 PaX Team <pageexec@freemail.hu>
- fixed EMUTRMAP typo
  thanks to Francisco Blas Izquierdo Riera (klondike) <klondike@gentoo.org>

* Thu May 20 2010 PaX Team <pageexec@freemail.hu>
- introduced paxctl-elf.c

* Thu Jun 25 2009 PaX Team <pageexec@freemail.hu>
- changed macro functions into normal ones

* Mon Apr 16 2007 PaX Team <pageexec@freemail.hu>
- fixed section header offset handling in stripped binaries,
  thanks to Ned Ludd <solar@gentoo.org>

* Tue Dec 12 2006 PaX Team <pageexec@freemail.hu>
- use /usr/bin/install for install target
  macro'ize elf_modify_phdr to unify Elf32/Elf64 handling

* Sat Jul 22 2006 PaX Team <pageexec@freemail.hu>
- added program header extension (chpax is no longer neeeded),
  thanks to Kevin F. Quinn <co@kevquinn.com> & Ned Ludd <solar@gentoo.org>

* Sat Jun 17 2006 PaX Team <pageexec@freemail.hu>
- fixed code for gcc 2.95, thanks to Ned Ludd <solar@gentoo.org>

* Sat Apr 29 2006 PaX Team <pageexec@freemail.hu>
- fixed compilation on systems where PT_GNU_STACK is not defined,
  thanks to Ned Ludd <solar@gentoo.org>

* Sun May 29 2005 PaX Team <pageexec@freemail.hu>
- cleaned up error code reporting, thanks to Kevin F. Quinn <co@kevquinn.com>

* Tue May 03 2005 PaX Team <pageexec@freemail.hu>
- added -c option to convert PT_GNU_STACK into PT_PAX_FLAGS
- minor typo fixes in the code/manpage

* Tue Feb 10 2004 PaX Team <pageexec@freemail.hu>
- initial release
