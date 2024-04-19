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

This project assumes libfuse2 is the default fuse version and libfuse3 is under fuse3/ directory. If your Linux distribution uses libfuse3 as the default one, edit the CMakeLists.txt file and the .h headers.

## How to Use It
Just type:
`siafs http://:yourpassword@yourIP:port/bucket mountpoint`
If the bucket is omitted, it will use the default one.

### Debug
Type:
`siafs -v -d http://:yourpassword@yourIP:port/bucket mountpoint`
Daemon won't for and a lot of debug info will be displayed.

## Known Issues
* Extremelly slow for now. Needs caching and optimization review. The first release will focus on the logic only.
* Performance issues when writting big files with libfuse 2.9.x due to lack of the big_write feature. This doesn't happen with libfuse 3+.

## TODO
* Mount wrappers.
* Caching.
* Test under diferent conditions.

## Limitations
The following limitations are due the SIA RenterD api.
* No extended attributes.
* No partial writting operations.

## More Information
(not yet)
