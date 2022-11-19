## Homework 3: Tar (tape archive) File Manipulation

This project implements `mytar`, which creates and manipulates tape archives. `mytar` creates an archive from a directory tree, extracts files from an archive, or prints the contents and details of an archive file.

### Usage

`mytar [cxtf:] [file] filename`

**Options**

`-c`: creates an archive of the specified directory tree.

`-x`: extracts the directory tree contained in the specified archive.

`-t`: prints the contents of the specified archive.

`-f`: the subsequent argument is the name of the archive file (to create, extract, or print).

**Examples**

`mytar -c -f a.tar a`

create an archive, `a.tar`, containing all files in the directory tree, `a`

`mytar -x -f a.tar`

extract the files in the archive, `a.tar`

`mytar -t -f a.tar`

print the details of all files in the archive, `a.tar`

`mytar -c a`
Error: No tarfile specified.
