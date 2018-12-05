# DeSyDe

DeSyDe is a design space exploration tool developed at KTH (ForSyDe research group).

# Branch motivation

This branch exists in order to make the tool compilation-portable across different machines. Specifically,
this branch aims to enable easy compilation of DeSyDe right after cloning, judging that all necessary
build tools are installed.

# (Almost) hassle-free installation

If you are on any debian-based distro with reasonably updated packages, you should be
good to go by issuing the following install command (do not forget to prepend sudo if necessary):

    apt install libxml1-dev qt5-default libboost-graph-dev

Then, a `make install` should do the trick. Tested on Linux Mint 18.3 and Debian 10.

# Publication
[K. Rosvall and I. Sander. A constraint-based design space exploration framework for real-time applications on MPSoCs. In Design Automation and Test in Europe (DATE '14), Dresden, Germany, Mar. 2014.](http://dx.doi.org/10.7873/DATE.2014.339)

