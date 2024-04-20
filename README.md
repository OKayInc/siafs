# siafs
SIA FUSE Filesystem, sponsored by the SIA Foundation.

## Status
The following has been tested:

* Mounting volumes work.
* Listing directories work.
* Creating directories work.
* Reading files work.
* Uploading small files (< 4kB).
* Uploading big files (=> 4kB).
* Deleting files and directories.
* Rewriting on pre-existing files.
* Renaming files and directories.
* Mount wrappers.

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
* libcjson
* libcurl
* libfuse 2.9+ (3 recommended)

This project looks for libfus3 first and it will fall back looking for libfuse2.

## How to Use It
Just type:
`siafs http://:yourpassword@yourIP:port/bucket mountpoint`
If the bucket is omitted, it will use the default one.

### Debug
Type:
`siafs -v -d http://:yourpassword@yourIP:port/bucket mountpoint`
Daemon won't fork and a lot of debug info will be displayed.

## Known Issues
* Extremelly slow for now. Needs caching and optimization review. The first release will focus on the logic only.
* Performance issues when writting big files with libfuse 2.9.x due to lack of the big_write feature. This doesn't happen with libfuse 3+.

## TODO
* Caching.
* Test under diferent conditions.

## Limitations
The following limitations are due the SIA RenterD api.
* No extended attributes.
* No partial writting operations.

## More Information
(not yet)
