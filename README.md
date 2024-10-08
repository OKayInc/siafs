# siafs
SIA FUSE Filesystem, sponsored by the SIA Foundation.

## Status
The following has been tested:

* Mounting volumes work.
* Listing directories work.
* Creating directories work.
* Reading files work.
* Uploading small files (< 4kB or 128kb).
* Uploading big files (=> 4kB or 128kb).
* Deleting files and directories.
* Rewriting on pre-existing files.
* Renaming files and directories.
* Mount wrappers.
* L1 Memcached cache.
* L2 Disk cache.

## Installing
siafs uses `CMake`.

### From source

    git clone https://github.com/OKayInc/siafs
    cd siafs
    mkdir build
    cd buid
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Debug ..
    make
    make install

#### Requirements
* A working renterd (you may use testnet for testing)
* libmemcached
* libcjson
* libcurl
* libfuse 2.9+ (3 recommended)

### From RPM (RedHat based Linux)
(not yet)


This project looks for libfuse3 first and it will fall back looking for libfuse2.

## How to Use It
### Command Line
Just type:
`siafs http://:yourpassword@yourIP:port/bucket mountpoint`
If the bucket is omitted, it will use the default one.

### FSTAB
(not yet)

### How to Debug
Type:
`siafs -v -d http://:yourpassword@yourIP:port/bucket mountpoint`
Daemon won't fork and a lot of debug info will be displayed.

## Known Issues
* Performance issues when writting big files with libfuse 2.9.x due to lack of the big_write feature. This doesn't (a lot) happen with libfuse 3+.
* The L2 cache directory needs babysitting. It is safe to delete the directories within as you see fit.

## TODO
* Test under diferent conditions. So far, under laboratory, all went good.

## Limitations
The following limitations are due the SIA RenterD api.
* No extended attributes.
* No partial writting operations.
* Filesize limitation due to a combination of Renterd multipart limitation (10,000 parts) and a libfuse fixed bufer size (4kb for libfuse2, 128kb for libfuse3). For libfuse 3, maximum filesize is 1.2GB; for libfuse 2, maximum filesize is 40MB. (working on this)

## More Information
(not yet)
