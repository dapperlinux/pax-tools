Summary:    Dapper Linux Pax Flags
Name:       dapper-paxset
Version:    27
Release:    5

Group:      System Environment/Base
License:    GPLv3+
Url:        http://github.com/dapperlinux/pax-tools/
Source0:    dapper-paxset
BuildArch:  noarch
Requires:   paxctl
Requires:   attr

%description
Dapper-Paxset is a script which correctly sets Pax flags for Dapper Linux.
Used for initial install media creation and initial install only. Use extended
attributes to maintain your Pax flags after install.

%prep

%build

%install
mkdir -p %{buildroot}%{_bindir}
install -m 755 %{SOURCE0} %{buildroot}%{_bindir}

%posttrans
dapper-paxset

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}

%changelog
* Sat Oct 28 2017 Matthew Ruffell <msr50@uclive.ac.nz>
- Adding Xorg and Xwayland

* Wed Sep 27 2017 Matthew Ruffell <msr50@uclive.ac.nz>
- Adding polari

* Sat Sep 23 2017 Matthew Ruffell <msr50@uclive.ac.nz>
- Moving from paxctl to extended file attributes with setfattr

* Sun Sep  3 2017 Matthew Ruffell <msr50@uclive.ac.nz>
- Adding in file existence checks

* Mon Aug 14 2017 Matthew Ruffell <msr50@uclive.ac.nz>
- Adding krita for DL26

* Fri Dec  2 2016 Matthew Ruffell <msr50@uclive.ac.nz>
- Created dapper-paxset
