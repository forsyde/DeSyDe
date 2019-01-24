# Tutorial

## CLI Invocation

This short tutorial will guide you through testing a small SDF application on a TDN NoC frabic as discussed
on the 2018 paper -MUST REFER LATER-.

First, make sure that you have DeSyDe installed and that it is callable in your system in someway. Now,
if you issue in your command line:

    adse --help
    
You will see that there are quite a handful of options to tweak the exploration. Fortunately you don't have to
specify these knobs on the invocation everytime! DeSyDe also provides the option of reading a conf. File so
that these tweaks are parsed in the same manner as if they were given at the CLI. Thus, issue the following
command so that the binary can give an empty configuration file named `config.cfg`:

    adse --dump-cfg
    
Once all the configuration aspects are set up, running DeSyDe in that experiment is a matter of invoking `adse`
in the experiment/example folder.

## Setting up an exploration problem

What we need is a description of the platform that the system is going to be mapped into, the execution
and communication times of the SDF actors on each mapped processor so the DSE solver can reason about it and
optionally some constraints that the solution must obey. In terms of input, we then need three different files:

  1. `platform.xml` which describes the platform being mapped.
  2. `applications.xml`, `application1.xml`, `application2.xml`, etc that describes the applications being mapped.
  The applications can be separated in different files if desired, as DeSyDe read all SDF3 xmls given and build ups
  a model via the union of the provided applications.
  3. `desConst.xml` which describes the extra functional constraints of the final design.
  4. `WCETs.xml` which describes the worst case scenario execution time for any actor in any
  processor.
