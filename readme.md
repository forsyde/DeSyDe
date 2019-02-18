# DeSyDe

DeSyDe is a design space exploration tool developed at KTH (ForSyDe research group).

### Releases:
* latest: 
  [Release for our accepted DSD article]()
* previous:
  * [Release for our accepted TODAES article](https://github.com/forsyde/DeSyDe/tree/v0.2.1-todaes)
  * [Release for our RAPIDO'17 publication](https://github.com/forsyde/DeSyDe/tree/v0.1.1-rapido)

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

# Running the Experiments

The experiments provided in the `examples` folder represent those that are still functional and were
used as proof of concepts into previous papers this project was involved. For a step-by-step tutorial
on how to setup your own experiment, check out the tutorial provided in this repo.

<!-- 1. Run `make` in the DeSyDe root folder to build the tool. -->
<!-- 2. All files for the experiments are in the directory **examples/TODAES/** -->
<!-- 3. The directory also contains a script, `run_experiments.sh`. Run the script, it will prompt you for which experiment from the TODAES article you want to run (1-6). *Note that for Experiments 3-6, the script will run all scenarious, which each are in a seperate directory.* -->
<!-- 4. The results of the experiment will be put into **examples/TODAES/exp_&ast;/out/** -->

# Publications

[K. Rosvall and I. Sander. A constraint-based design space exploration framework for real-time applications on MPSoCs. In Design Automation and Test in Europe (DATE '14), Dresden, Germany, Mar. 2014.](http://dx.doi.org/10.7873/DATE.2014.339)

[Rosvall, Kathrin, Tage Mohammadat, George Ungureanu, Johnny Öberg, and Ingo Sander. “Exploring Power and Throughput for Dataflow Applications on Predictable NoC Multiprocessors,” 719–26, 2018.](https://doi.org/10.1109/DSD.2018.00011.)

[Kathrin Rosvall, Nima Khalilzad, George Ungureanu, and Ingo Sander. Throughput propagation in constraint-based design space exploration for mixed-criticality systems. In Proceedings of the 2017 Workshop on Rapid Simulation and Performance Evaluation: Methods and Tools (RAPIDO '17), Stockholm, Sweden. ACM, January 2017.](https://doi.org/10.1145/3023973.3023977)

[Nima Khalilzad, Kathrin Rosvall, and Ingo Sander. A modular design space exploration framework for multiprocessor real-time systems. In Forum on specification & Design Languages (FDL '16), Bremen, Germany. IEEE, September 2016.](https://doi.org/10.1109/FDL.2016.7880377)

