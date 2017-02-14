# dapper-paxset

##About
The Dapper PaXset package contains the correct PaX flag settings for the Dapper Linux distribution. 


##Building
To build this package, first install an RPM development chain:

```bash
$ sudo dnf install fedora-packager fedora-review

```

Next, setup rpmbuild directories with

```bash
$ rpmdev-setuptree
```
And place the file dapper-paxset.spec in the SPECS directory, add the file dapper-paxset to the SOURCES directory:
```bash
$ mv dapper-backgrounds.spec ~/rpmbuild/SPECS/
$ mv dapper-paxset ~/rpmbuild/SOURCES/
```

and finally, you can build RPMs and SRPMs with:
```bash
$ cd ~/rpmbuild/SPECS
$ rpmbuild -ba dapper-paxset.spec
```


