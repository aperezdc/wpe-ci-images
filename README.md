WPE CI Images
=============

[![CircleCI](https://circleci.com/gh/aperezdc/wpe-ci-images/tree/master.svg?style=svg)](https://circleci.com/gh/aperezdc/wpe-ci-images/tree/master)

The files in this repository can be used to re-create the base Docker images
used for continuous integration of [WPE WebKit](https://wpewebkit.org):

- [aperezdc/ci-imagebuild](https://hub.docker.com/r/aperezdc/ci-imagebuild/):
  Used to build other images and upload them to a registry.
- [aperezdc/ci-wpebackend](https://hub.docker.com/r/aperezdc/ci-wpebackend/):
  Used for [WPEBackend](https://github.com/WebPlatformForEmbedded/WPEBackend).


Building Images
---------------

### Prerequisites

Images are based on [Arch Linux](https://www.archlinux.org/). Running the
[build](./script) on Arch is recommended, though any GNU/Linux system where
the following tools are available should theoretically work as well:

- [mcpp](http://mcpp.sourceforge.net/) C preprocessor.
- `sha256sum` (usually included as part of [GNU
  Coreutils](http://www.gnu.org/software/coreutils)).
- `fakeroot` & `fakechroot`.
- [GnuPG](https://www.gnupg.org/).
- [pigz](https://zlib.net/pigz/).
- `tar` ([GNU Tar](https://www.gnu.org/software/tar/) is known to work).
- [Zsh](https://www.zsh.org).

Arch users can ensure that the needed packages are installed with the
following command:

```sh
pacman -S mcpp gpg fake{ch,}root pigz tar zsh
```

### Package lists

The `.pkglist` files specify a space-or-newline separated list of packages.
The C preprocessor is ran over them, which means:

- Other package list files can be `#include`d.
- Packages can be added conditionally using e.g. `#ifdef`.
- Comments take the `/* ... /*` form.

For example, the following is simple package list which ensures that
development packages are installed, plus a few extras:

```c
/* devel-tools.pkglist */
base-devel
ninja cmake

/* Pass -DMORE_TOOLS in the command line to add these. */
#ifdef MORE_TOOLS
valgrind ccache
#endif
```


### Building

Run the `build` script, passing at least one package list in the command line
or a package name:

```sh
./build --output out --pkglist wpebackend.pkglist
```

The resulting contents of the `out/` directory will look similar to the
following:

```
out/
 ├── blobs/
 │   ├── 64173e2860cf9f4e894b6bf063d80d30f7b37454f9321dfccf98aee617974515
 │   ├── 74b247eb791abcd2405a6515ded29bd24e4621dd9ca47a0788783ff008d4bc3f
 │   ├── manifest.json
 │   └── version
 ├── config.json
 ├── config.json.sum
 ├── rootfs/
 │   ├── bin -> usr/bin/
 │   ├── boot/
 :   : 
 │   └── var/
 ├── rootfs.tar.gz
 ├── rootfs.tar.gz.sum
 └── rootfs.tar.sum
```

- The `out/rootfs/` subdirectory and the `out/rootfs.tar.gz` tarball contain
  the output from assembling the file system with `pacstrap`, with all the
  packages from the command line and package lists installed.
- The `out/config.json` is a bare-bones, minimal image configuration JSON file
  which only specifies the image contents as a single layer layer. This is
  enough for assembling Docker/OCI images which used only for their file
  system contents. Run commands, port redirections, or other goodies are *not*
  specified in the generated file.
- The `out/blobs/` subdirectory contains the file system image in
  [Skopeo](https://github.com/containers/skopeo)'s “dir” format (which is
  kind of similar to OCI and reuses some of its concepts, but is easier to
  generate) and can be used with the tool (for example for [uploading
  images to DockerHub](#uploading-images)).


### OCI Images

Only the `…/rootfs/` output from the `build` script is directly usable. Images
in other formats can be converted from the contents of the `…/blobs/`
subdirectory using the [Skopeo](https://github.com/containers/skopeo) tool.

For example, the following converts the output from the `build` script into
an OCI image:

```
% skopeo copy dir:out/blobs/ oci:oci-image:latest
Getting image source signatures
Copying blob sha256:74b247eb791abcd2405a6515ded29bd24e4621dd9ca47a0788783ff008d4bc3f
 307.19 MB / 307.19 MB [====================================================] 2s
Copying config sha256:a796de0d923938946eb06e143f66bc91280ddc47dceb70b61fceb3416399cd71
 163 B / 163 B [============================================================] 0s
Writing manifest to image destination
Storing signatures
%
```

Note how the resulting directory checks out as a valid OCI image (you will
need [image-tools](https://github.com/opencontainers/image-tools) for the
`oci-image-tool` program):

```
% oci-image-tool validate --type=image --ref name=latest oci-image/
autodetected image file type is: imageLayout
oci-image-tool: reference [name=latest]: OK
oci-image/: OK
Validation succeeded
%
```


Uploading Images
----------------

[Skopeo](https://github.com/containers/skopeo) can be used for this as well,
note that it is *not* needed to convert the image, the “dir” format can be
used directly:

```sh
skopeo copy --dest-creds user:pass \
    dir:out/blobs/ docker://docker.io/aperezdc/ci-wpebackend
```
