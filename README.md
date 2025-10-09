# Build
```
cd src-liberty_parse-2.6
./configure 
make clean
make
./makelib (required package csh)
cd ..
mkdir build
cd build
cmake ..
make
```

You need to install packages (based on your distro packages name may are different):
    cmake
    gperf
    libtcl8.6
    tcl8.6
    tcl8.6-dev
    gtk2-devel
    boost-devel
    libreadline
    git
    bison
    csh

Note:
    On some distros you may need to change #include <tcl.h> to #include <tcl/tcl.h> or #include <tcl8.6/tcl.h>, same for tclDecls.h