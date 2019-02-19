# DeSyDe

DeSyDe is a design space exploration tool developed at KTH (ForSyDe research group).

### Releases:
* latest: 
  * [Release for our DSD'18 publication + user tutorial](https://github.com/forsyde/DeSyDe/tree/v0.3.0-dsd)

* previous:
  * [Release for our TODAES article](https://github.com/forsyde/DeSyDe/tree/v0.2.1-todaes)
  * [Release for our RAPIDO'17 publication](https://github.com/forsyde/DeSyDe/tree/v0.1.1-rapido)


# (Almost) hassle-free installation

You need to install DeSyDe via the automated build scripts. We have
tried assuring an (almost) fully-automated installation process,
especially for Linux machines. The idea is quite simple: the script
downloads almost everything necessary so that nothing on your system
is touched and then proceeds to compile everything. This sandboxing
comes with the cost of added compilation time, but since this should
be a one-time process, the larger time frame is a good trade-off for
flexibility.

The only dependency that is not cloned directly from its repo and
compiled alongside DeSyDe is Qt, as DeSyDe currently does not make
Gecode's Gist optional. Please ensure that you have the basic
development files for Qt installed and reachable in your machine. In
future releases this necessity will be removed.

If you are on any debian-based distro with reasonably updated
packages, you should be good to go by issuing the following install
command (do not forget to prepend sudo if necessary):

    apt install automake libtool qt5-default

Then, a `make` followed by `make install` should do the trick. Tested
on Linux Mint 18.3 and Debian 10.

# Usage

Please follow the [tutorial](docs/tutorial.md) for more details on how
to use the tool and how to interpret its output.

# Running the Experiments

The experiments provided in the `examples` folder represent those that are still functional and were
used as proof of concepts into previous papers this project was involved. For a step-by-step tutorial
on how to setup your own experiment, check out the tutorial provided in this repo.

## Included examples

* [DSD18](examples/DSD18): experiments from our [DSD'18](ttps://doi.org/10.1109/DSD.2018.00011.) dealing with TDN NoCs exploration that optimize power while respecting real time constraints.
* [ScalAnalysis](examples/ScalAnalysis): folder containing scripts that generates experiments for different sized NoCs platforms based on a template extracted from [DSD18](examples/DSD18).
* [tutorial](examples/tutorial): the files used for the user [tutorial](docs/tutorial.md).

# Publications

[Kathrin Rosvall, Tage Mohammadat, George Ungureanu, Johnny Öberg, and Ingo Sander. “Exploring Power and Throughput for Dataflow Applications on Predictable NoC Multiprocessors,” 719–26, 2018.](https://doi.org/10.1109/DSD.2018.00011.)

[Kathrin Rosvall, Nima Khalilzad, George Ungureanu, and Ingo Sander. Throughput propagation in constraint-based design space exploration for mixed-criticality systems. In Proceedings of the 2017 Workshop on Rapid Simulation and Performance Evaluation: Methods and Tools (RAPIDO '17), Stockholm, Sweden. ACM, January 2017.](https://doi.org/10.1145/3023973.3023977)

[Nima Khalilzad, Kathrin Rosvall, and Ingo Sander. A modular design space exploration framework for multiprocessor real-time systems. In Forum on specification & Design Languages (FDL '16), Bremen, Germany. IEEE, September 2016.](https://doi.org/10.1109/FDL.2016.7880377)

[Kathrin Rosvall and Ingo Sander. A constraint-based design space exploration framework for real-time applications on MPSoCs. In Design Automation and Test in Europe (DATE '14), Dresden, Germany, Mar. 2014.](http://dx.doi.org/10.7873/DATE.2014.339)
