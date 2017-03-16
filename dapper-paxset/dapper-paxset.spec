Summary:    Dapper Linux Pax Flags
Name:       dapper-paxset
Version:    25
Release:    7

Group:      System Environment/Base
License:    GPLv3+
Url:        http://github.com/dapperlinux/pax-tools/
Source0:    dapper-paxset
BuildArch:  noarch
Requires:   paxctl

%description
Dapper-Paxset is a script which correctly sets Pax flags for Dapper Linux.
Used for initial install media creation and initial install only. Use paxctld
to maintain your Pax flags after install.

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
* Fri Dec  2 2016 Matthew Ruffell <msr50@uclive.ac.nz>
- Created dapper-paxset
