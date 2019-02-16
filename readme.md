# DeSyDe

DeSyDe is a design space exploration tool developed at KTH (ForSyDe research group).

# (Almost) hassle-free installation

The main way in which DeSyDe is meant to be 'installed' is via compilation, which means that
we, the ForSyDe group, tried as much as possible to provide automated scripts for compilation
of DeSyDe, specially for linux machines. The idea is quite simple: the script downloads almost
everything necessary so that nothing on your system is touched and then proceeds to
compile everything. This sandboxing comes with the cost of added compilation time, but since
this should be a one-time process, the larger time frame is a good trade-off for flexibility.

The only dependency that is not cloned directly from its repo and compiled alongside DeSyDe
is Qt, as DeSyDe currently does not make Gecode's Gist optional. Please ensure that you have
the basic development files for Qt installed and reachable in your machine. In future releases
this necessity will be removed.

If you are on any debian-based distro with reasonably updated packages, you should be
good to go by issuing the following install command (do not forget to prepend sudo if necessary):

    apt install automake libtool qt5-default

Then, a `make` followed by `make install` should do the trick. Tested on Linux Mint 18.3 and Debian 10.

# Publications

[K. Rosvall and I. Sander. A constraint-based design space exploration framework for real-time applications on MPSoCs. In Design Automation and Test in Europe (DATE '14), Dresden, Germany, Mar. 2014.](http://dx.doi.org/10.7873/DATE.2014.339)

[Rosvall, Kathrin, Tage Mohammadat, George Ungureanu, Johnny Öberg, and Ingo Sander. “Exploring Power and Throughput for Dataflow Applications on Predictable NoC Multiprocessors,” 719–26, 2018.](https://doi.org/10.1109/DSD.2018.00011.)
