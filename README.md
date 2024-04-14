# siafs
SIA FUSE Filesystem, sponsored by the SIA Foundation.

## Status
* Mounting volumes work.
* Listing directories work.
* Creating directories work.
* Reading files work.
* Uploading small files (< 4kB).

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
(not yet)

## TODO
* Uploading big files (> 4kb).
* Renaming files and directories.
* Rewriting on pre-existing files.
* Mount wrappers.

## More Information
(not yet)
