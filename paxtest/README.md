# paxtest

##About
The paxtest package implements a suite of tests to verify that PaX protections are working in the currently running kernel.

##Building
To build this package, first install an RPM development chain:

```bash
$ sudo dnf install fedora-packager fedora-review

```

Next, setup rpmbuild directories with

```bash
$ rpmdev-setuptree
```
And place the file paxtest.spec in the SPECS directory, and place the rest of the files in the SOURCES directory:
```bash
$ mv paxtest.spec ~/rpmbuild/SPECS/
$ tar -czvf paxtest-0.9.15.tar.gz paxtest-0.9.15
$ mv * ~/rpmbuild/SOURCES/
```

and finally, you can build RPMs and SRPMs with:
```bash
$ cd ~/rpmbuild/SPECS
$ rpmbuild -ba paxtest.spec
```


