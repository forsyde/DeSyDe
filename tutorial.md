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
optionally some constraints that the solution must obey. In terms of input, we then need four different files:

  1. `platform.xml` which describes the platform being mapped.
  2. `applications.xml`, `application1.xml`, `application2.xml`, etc that describes the applications being mapped.
  The applications can be separated in different files if desired, as DeSyDe read all SDF3 xmls given and build ups
  a model via the union of the provided applications.
  3. `desConst.xml` which describes the extra functional constraints of the final design.
  4. `WCETs.xml` which describes the worst case scenario execution time for any actor in any
  processor.

We will write these files in the order provided, starting with the platform.

### Platform

The knobs available for now are the processor types and size of the overall platform. For instance, checkout the
following `xml` platform description file and let us build upon it:

    <?xml version="1.0" encoding="UTF-8"?>
    <platform name="demo_platform">
        <processor model="simple" number="3">	
            <mode name="default" cycle="1" mem="8000" dynPower="10" staticPower="10" area="4" monetary="4"/>
        </processor>
        <processor model="powerful" number="1">	
            <mode name="eco" cycle="1.1" mem="24000" dynPower="6" staticPower="6" area="8" monetary="20"/>
            <mode name="default" cycle="1" mem="30000" dynPower="15" staticPower="15" area="8" monetary="20"/>
        </processor>
        <interconnect>
            <TDN_NoC name="2x2TDN" topology="mesh" x-dimension="2" y-dimension="2" routing="Y-X" 
                    flitSize="128" cycles="6" maxCyclesPerProc="1">
              <mode name="default" cycleLength="10" 
                    dynPower_link="7" dynPower_NI="7" dynPower_switch="7" 
                    staticPower_link="7" staticPower_NI="7" staticPower_switch="7" 
                    area_link="7" area_NI="7" area_switch="7" 
                    monetary_link="7" monetary_NI="7" monetary_switch="7"/>
              <mode name="fast" cycleLength="6" 
                    dynPower_link="18" dynPower_NI="18" dynPower_switch="18"
                    staticPower_link="18" staticPower_NI="18" staticPower_switch="18" 
                    area_link="18" area_NI="18" area_switch="18" 
                    monetary_link="18" monetary_NI="18" monetary_switch="18"/>
            </TDN_NoC>
        </interconnect>
    </platform>

Aside from the header, we starting reading the description from the processor listings. The
first processor declaration has a name of `simple` to symbolize that it is a simple and cheap processor,
with a number of `3` to indicate that there are 3 processors of this kind in the platform. 
The second has a name of `powerful` to symbolize the opposite. A glance at their children declaration reveals
that we also must specify at least one mode, while the name `default` was used simply by convention. The
mode that the processor can operate will give its memory, speed, power, area and monetary characteristics so
that the exploration tool can work with them. Remember that the mode is chosen statically for a given implementation
and does not change dynamically at runtime by assumption. There are no hard units assumed for the given 
extra function characteristics, for instance, `8000` memory for `simple` could mean `8000` bits, bytes or even Kbytes.

Next, the interconnection is specified. Like processors, the interconnect must have at least one mode of operation specified
so that DeSyDe can choose which mode, if the choice exists, should be used for the implementation. The major difference now
is that some additional tags exists for the interconnection, such as the dimensions of the mesh (the only available topology for now),
the routing algorithm to be used, the flit size, TDN slots as `cycles` and the maximum number of TDN Slots per Processor.
the `link` terminology in the mode specification represents how much of that number is applied to each real
interconnect link, e.g. a 2 x 2 grid has 4 links.

### Applications

We shall model two applications that must be run on the platform of our choice before and just for fun we'll do it
in two different ways: in a single file with both applications and with each one in its own file. We'll start with
the single file approach.

The applications here are SDF descriptions of Sobel and of Susan as follows:

* Dot File:

      digraph {
        getPx -> gx[label="6 -> 6"];
        getPx -> gy[label="6 -> 6"];
        gx -> abs[label="1 -> 1"];
        gy -> abs[label="1 -> 1"];
      }

      digraph {
        getLm -> usan[label="1 -> 1"];
        usan -> dir[label="1 -> 1"];
        dir -> thin[label="1 -> 1"];
        thin -> putLm[label="1 -> 1"];
      }

* Visual ASCII representation:

    - Sobel:

    ```
    +----+  6 -> 6   +---------+
    | gy | <-------- |  getPx  |
    +----+           +---------+
      |                |
      |                | 6 -> 6
      |                v
      |              +---------+
      |              |   gx    |
      |              +---------+
      |                |
      |                | 1 -> 1
      |                v
      |    1 -> 1    +---------+
      +------------> |   abs   |
                    +---------+
    ```

    - Susan:
   
    ```
        +---------+
        |  getLm  |
        +---------+
          |
          | 1 -> 1
          v
        +---------+
        |  usan   |
        +---------+
          |
          | 1 -> 1
          v
        +---------+
        |   dir   |
        +---------+
          |
          | 1 -> 1
          v
        +---------+
        |  thin   |
        +---------+
          |
          | 1 -> 1
          v
        +---------+
        |  putLm  |
        +---------+
    ```

So, if we want to write a file `applications.xml`, we can start by adding the actors with ports and then connecting
everything with channels. This is the content of the file:

    <?xml version="1.0"?>
    <sdf3 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0" type="sdf" xsi:noNamespaceSchemaLocation="http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd">
      <applicationGraph name="a_sobel">
        <sdf name="a_sobel" type="SOBEL">
          <actor name="get_pixel" type="getPixel">
            <port name="p0_0" type="out" rate="1"/>
            <port name="p0_1" type="out" rate="1"/>
            <port name="p0_2" type="out" rate="1"/>
            <port name="p0_3" type="out" rate="1"/>
            <port name="p0_4" type="out" rate="1"/>
            <port name="p0_5" type="out" rate="1"/>
            <port name="p1_0" type="out" rate="1"/>
            <port name="p1_1" type="out" rate="1"/>
            <port name="p1_2" type="out" rate="1"/>
            <port name="p1_3" type="out" rate="1"/>
            <port name="p1_4" type="out" rate="1"/>
            <port name="p1_5" type="out" rate="1"/>
          </actor>
          <actor name="gx" type="GX">
            <port name="p0_0" type="in" rate="1"/>
            <port name="p0_1" type="in" rate="1"/>
            <port name="p0_2" type="in" rate="1"/>
            <port name="p0_3" type="in" rate="1"/>
            <port name="p0_4" type="in" rate="1"/>
            <port name="p0_5" type="in" rate="1"/>
            <port name="p1_0" type="out" rate="1"/>
          </actor>
          <actor name="gy" type="GY">
            <port name="p0_0" type="in" rate="1"/>
            <port name="p0_1" type="in" rate="1"/>
            <port name="p0_2" type="in" rate="1"/>
            <port name="p0_3" type="in" rate="1"/>
            <port name="p0_4" type="in" rate="1"/>
            <port name="p0_5" type="in" rate="1"/>
            <port name="p1_0" type="out" rate="1"/>
          </actor>
          <actor name="abs" type="ABS">
            <port name="p0_0" type="in" rate="1"/>
            <port name="p1_0" type="in" rate="1"/>
          </actor>
          <channel name="chSo1_0" srcActor="get_pixel" srcPort="p0_0" dstActor="gx" dstPort="p0_0"/>
          <channel name="chSo1_1" srcActor="get_pixel" srcPort="p0_1" dstActor="gx" dstPort="p0_1"/>
          <channel name="chSo1_2" srcActor="get_pixel" srcPort="p0_2" dstActor="gx" dstPort="p0_2"/>
          <channel name="chSo1_3" srcActor="get_pixel" srcPort="p0_3" dstActor="gx" dstPort="p0_3"/>
          <channel name="chSo1_4" srcActor="get_pixel" srcPort="p0_4" dstActor="gx" dstPort="p0_4"/>
          <channel name="chSo1_5" srcActor="get_pixel" srcPort="p0_5" dstActor="gx" dstPort="p0_5"/>
          <channel name="chSo2_0" srcActor="get_pixel" srcPort="p1_0" dstActor="gy" dstPort="p0_0"/>
          <channel name="chSo2_1" srcActor="get_pixel" srcPort="p1_1" dstActor="gy" dstPort="p0_1"/>
          <channel name="chSo2_2" srcActor="get_pixel" srcPort="p1_2" dstActor="gy" dstPort="p0_2"/>
          <channel name="chSo2_3" srcActor="get_pixel" srcPort="p1_3" dstActor="gy" dstPort="p0_3"/>
          <channel name="chSo2_4" srcActor="get_pixel" srcPort="p1_4" dstActor="gy" dstPort="p0_4"/>
          <channel name="chSo2_5" srcActor="get_pixel" srcPort="p1_5" dstActor="gy" dstPort="p0_5"/>
          <channel name="chSo3_0" srcActor="gx" srcPort="p1_0" dstActor="abs" dstPort="p0_0"/>
          <channel name="chSo4_0" srcActor="gy" srcPort="p1_0" dstActor="abs" dstPort="p1_0"/>
        </sdf>
        <sdf name="b_susan" type="SUSAN">
          <actor name="getImage" type="GI">
            <port name="p0_0" type="out" rate="1"/>
          </actor>
          <actor name="usan" type="USAN">
            <port name="p0_0" type="in" rate="1"/>
            <port name="p0_1" type="out" rate="1"/>
            <port name="p0_2" type="out" rate="1"/>
          </actor>
          <actor name="direction" type="DIR">
            <port name="p0_0" type="in" rate="1"/>
            <port name="p0_1" type="in" rate="1"/>
            <port name="p0_2" type="out" rate="1"/>
            <port name="p0_3" type="out" rate="1"/>
            <port name="p0_4" type="out" rate="1"/>
          </actor>
          <actor name="thin" type="THIN">
            <port name="p0_0" type="in" rate="1"/>
            <port name="p0_1" type="in" rate="1"/>
            <port name="p0_2" type="in" rate="1"/>
            <port name="p0_3" type="out" rate="1"/>
            <port name="p0_4" type="out" rate="1"/>
          </actor>
          <actor name="putImage" type="PI">
            <port name="p0_0" type="in" rate="1"/>
            <port name="p0_1" type="in" rate="1"/>
          </actor>
          <channel name="chSu0_0" srcActor="getImage" srcPort="p0_0" dstActor="usan" dstPort="p0_0"/>
          <channel name="chSu0_1" srcActor="usan" srcPort="p0_1" dstActor="direction" dstPort="p0_0"/>
          <channel name="chSu0_2" srcActor="usan" srcPort="p0_2" dstActor="direction" dstPort="p0_1"/>
          <channel name="chSu0_3" srcActor="direction" srcPort="p0_2" dstActor="thin" dstPort="p0_0"/>
          <channel name="chSu0_4" srcActor="direction" srcPort="p0_3" dstActor="thin" dstPort="p0_1"/>
          <channel name="chSu0_5" srcActor="direction" srcPort="p0_4" dstActor="thin" dstPort="p0_2"/>
          <channel name="chSu0_6" srcActor="thin" srcPort="p0_3" dstActor="putImage" dstPort="p0_0"/>
          <channel name="chSu0_7" srcActor="thin" srcPort="p0_4" dstActor="putImage" dstPort="p0_1"/>
        </sdf>
      </applicationGraph>
    </sdf3>

note that to transcribe a channel with non-homogeneous production and consumption token rates, it is necessary to declare multiple ports and connect
then accordingly. In essence, a channel in the SDF3 format is reduce to a single port pair, so a channel on the pure SDF model correspond
to many channels in the SDF3 description format.

To make this same exact set of applications in different files, we just need to extract each `sdf` tag into its own file while maintaining the two parent
tags `sdf3` and `applicationGraph`, and DeSyDe will interpolate these into the final true application graph to be solved. Like this:

* sobel.xml:

      <?xml version="1.0"?>
      <sdf3 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0" type="sdf" xsi:noNamespaceSchemaLocation="http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd">
        <applicationGraph name="sobel">
          <sdf name="a_sobel" type="SOBEL">
          ...
          </sdf>
        </applicationGraph>
      </sdf3>

* susan.xml:

      <?xml version="1.0"?>
      <sdf3 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.0" type="sdf" xsi:noNamespaceSchemaLocation="http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd">
        <applicationGraph name="susan">
          <sdf name="susan" type="SUSAN">
          ...
          </sdf>
        </applicationGraph>
      </sdf3>

Then in the `config.cgf` we add an additional line to instruct DeSyDe in where to find these files we just wrote:

    inputs=template/sdfs/
    inputs=template/xmls/
    
There can be as many `inputs` directives as necessary in the configuration file, DeSyDe simply scans through all of them.
