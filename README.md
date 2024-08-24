# PostGTK

An simple and lightweight API Client built with C, GTK, and libcurl.


## Requirements:

GTK >= 4.0
libcurl >= 8.9.1
jansson >= 2.14
make >= 4.4.1
meson >= 1.5.1

## Compiling

After intall the dependencies, To compile the code, run:


```
git clone https://github.com/victor-renan/postgtk
cd postgtk
meson setup builddir
meson compile -C builddir
./builddir/postgtk
