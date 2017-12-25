#!/bin/bash
cd ..
./waf --run "queue-discs-benchmark --queueDiscType=RED"
./waf --run "queue-discs-benchmark --queueDiscType=ARED"
./waf --run "queue-discs-benchmark --queueDiscType=HRED"
./waf --run "queue-discs-benchmark --queueDiscType=FRED"
./waf --run "queue-discs-benchmark --queueDiscType=SCRED"
./waf --run "queue-discs-benchmark --queueDiscType=FSCRED"
./waf --run "topology --red=RED"
./waf --run "topology --red=ARED"
./waf --run "topology --red=HRED"
./waf --run "topology --red=FRED"
./waf --run "topology --red=SCRED"
./waf --run "topology --red=FSCRED"
cd automation
