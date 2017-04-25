# paxctld

## About
The paxctld package contains a daemon which ensures the PaX flags for all executables are correct when the operating system is running.

## Building
To build this package, first install an RPM development chain:

```bash
$ sudo dnf install fedora-packager fedora-review

```

Next, setup rpmbuild directories with

```bash
$ rpmdev-setuptree
```
And place the file paxctld.spec in the SPECS directory, and move the paxctld folder to the SOURCES directory:
```bash
$ mv paxctld.spec ~/rpmbuild/SPECS/
$ tar -czvf paxctld-1.2.1.tar.gz paxctld-1.2.1
$ mv paxctld-1.2.1.tar.gz ~/rpmbuild/SOURCES/
```

and finally, you can build RPMs and SRPMs with:
```bash
$ cd ~/rpmbuild/SPECS
$ rpmbuild -ba paxctld.spec
```


