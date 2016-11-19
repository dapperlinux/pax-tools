Name: paxtest
Version: 0.9.15
Release: 1
Summary: PaX test suite
Group: admin
License: GPLv2
BuildArch: x86_64

URL: https://pax.grsecurity.net
Source0: https://www.grsecurity.net/~spender/paxtest-0.9.15.tar.gz
Source1: paxtest.1.gz

%description
paxtest is a program that attempts to test kernel enforcements over memory 
usage. Some attacks benefit from kernels that do not impose limitations. 
For example, execution in some  memory  segments makes buffer overflows 
possible. It is used as a regression test suite for PaX, but might be useful 
to test other memory protection patches for the kernel.



%prep
%autosetup

%build
make linux

%install
mkdir -p %{buildroot}%{_sbindir}/%{name}-files
mkdir -p %{buildroot}%{_mandir}/man1
make -f Makefile.psm install DESTDIR=%{buildroot} BINDIR=%{_sbindir}/%{name}-files RUNDIR=%{_sbindir}/%{name}-files
cp %{SOURCE1} %{buildroot}%{_mandir}/man1
ln -s %{_sbindir}/%{name}-files/%{name} %{buildroot}%{_sbindir}

%post

%files
%{_sbindir}/%{name}
%{_sbindir}/%name-files/*
%{_mandir}/man1/%name.1.gz