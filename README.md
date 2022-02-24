# CSE 322 Computer Netwowrks Sessional Term Assignment
## Implementation of SmRED in NS3
### Overview
We modified the Random Early Detection(RED)
algorithm to attain a refined Active Queue Management(AQM) scheme.In RED, average
queue size is maintained which varies between the minimum and maximum
threshold.Packet dropping probability linearly varies with the average queue size.If the
average queue size becomes larger than maximum threshold,all incoming packets are
dropped forcefully.RED performs well in most cases but is sensitive to traffic load.
In a low load scenario,small packet dropping probability should be used so that
link utilization becomes proper.In a high load scenario,higher packet dropping
probability should be used so that queue does not overflow.Thus,number of forced
dropped packets will be lessened and thus the retransmission will be reduced.The modified algorithm is called Smart RED or SmRED algorithm.The goal is to achieve
higher throughput at low load and reduce delay at highload.

### SmRED example
An example program for SmED has been provided in

> examples/traffic-control/red-vs-smred.cc

and should be executed as

* ./waf --run "red-vs-smred --queueDiscType=RED"
* ./waf --run "red-vs-smred --queueDiscType=SMRED"

### List of files modified:
* src/traffic-control/model/red-queue-disc.cc
* src/traffic-control/model/red-queue-disc.h
* src/traffic-control/test/red-queue-disc-test-suite.cc
* examples/traffic-control/wscript

### List of files newly added:
* examples/traffic-conttrol/red-vs-smred.cc

## References 
* https://ieeexplore.ieee.org/document/7726792
* http://www.nsnam.org/
