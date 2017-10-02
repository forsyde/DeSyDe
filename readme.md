# DeSyDe

DeSyDe is a design space exploration tool developed at KTH (ForSyDe research group).

# Dependencies

To build the DeSyDe tool, the following dependencies have to be met:

 * a C++ compiler: developed and tested with `g++-5.4.1`
 * the `libxml2` library: developed and tested with 2.9.3
 * the `boost` libraries `boost_system`, `boost_filesystem`, `boost_program_options`, `boost_graph`: developed and tested with 1.58
 * complete [Gecode](http://www.gecode.org/download.html), including Gist: developed with v4.4.0 

# Running the Experiments
1. Run `make` in the DeSyDe root folder to build the tool.
2. All files for the experiments are in the directory **examples/TODAES/**
3. The directory also contains a script, `run_experiments.sh`. Run the script, it will prompt you for which experiment from the TODAES article you want to run (1-6). *Note that for Experiments 3-6, the script will run all scenarious, which each are in a seperate directory.*
4. The results of the experiment will be put into **examples/TODAES/exp_&ast;/out/**

# The Input Files for DeSyDe
This is a very brief introduction to DeSyDe's input files.
* **exp_&ast;/config.cfg**: Configuration of the experiment. Specifies e.g. time-out (optional), type of solver, optimization criteria, output print frequency, input and output paths. A default config file is produced by the DeSyDe tool if the command line argument `-dump-cfg` is provided.
* **exp_&ast;/sdfs/&ast;.&ast;sdf.xml**: The application graphs in [SDF3's xml format](http://www.es.ele.tue.nl/sdf3/manuals/xml/sdf/). The graphs can be SDFs or HSDFs.
* **exp_&ast;/xmls/platform.xml**: Description of the platform with modes for processors.
* **exp_&ast;/xmls/WCETs.xml**: worst-case execution time values for all valid combinations of actors and processors.
* **exp_&ast;/xmls/desConst.xml**: Design constraints for the applications (e.g. iteration period) and system (e.g. power).
* **(exp_&ast;/xmls/mappingRules.xml)**: not used in the TODAES experiments. Disallows or forces combinations of specified actors and processors for the mapping.

# Publications
[Kathrin Rosvall, Nima Khalilzad, George Ungureanu, and Ingo Sander. Throughput propagation in constraint-based design space exploration for mixed-criticality systems. In Proceedings of the 2017 Workshop on Rapid Simulation and Performance Evaluation: Methods and Tools (RAPIDO '17), Stockholm, Sweden. ACM, January 2017.](https://doi.org/10.1145/3023973.3023977)

[Nima Khalilzad, Kathrin Rosvall, and Ingo Sander. A modular design space exploration framework for multiprocessor real-time systems. In Forum on specification & Design Languages (FDL '16), Bremen, Germany. IEEE, September 2016.](https://doi.org/10.1109/FDL.2016.7880377)

[Kathrin Rosvall and Ingo Sander. A constraint-based design space exploration framework for real-time applications on MPSoCs. In Design Automation and Test in Europe (DATE '14), Dresden, Germany, Mar. 2014.](http://dx.doi.org/10.7873/DATE.2014.339)

