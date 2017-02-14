# paxctl

##About
The paxctl package contains the paxctl tool used to set PaX flags on executables. 

##Building
To build this package, first install an RPM development chain:

```bash
$ sudo dnf install fedora-packager fedora-review

```

Next, setup rpmbuild directories with

```bash
$ rpmdev-setuptree
```
And place the file paxctl.spec in the SPECS directory, and move the paxctl-0.9 directory to SOURCES and compress it:
```bash
$ mv paxctl.spec ~/rpmbuild/SPECS/
$ tar -czvf paxctl-0.9.tar.gz paxctl-0.9
$ mv paxctl-0.9.tar.gz ~/rpmbuild/SOURCES/
```

and finally, you can build RPMs and SRPMs with:
```bash
$ cd ~/rpmbuild/SPECS
$ rpmbuild -ba paxctl.spec
```


