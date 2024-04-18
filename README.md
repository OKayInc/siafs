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
    cmake ..
    make
    make install

#### Requirements
* libcjson
* libcurl
* libfuse 2.9+

## How to Use It
Just type:
`siafs http://:yourpassword@yourIP:port/bucket mountpoint`
If the bucket is omitted, it will use the default one.

## Known Issues
Extremelly slow for now. Needs caching and optimization review. The first release will focus on the logic only.

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
