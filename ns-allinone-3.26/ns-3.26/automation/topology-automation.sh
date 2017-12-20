#!/bin/bash
cd ..
./waf --run "queue-discs-benchmark --queueDiscType=RED"
./waf --run "queue-discs-benchmark --queueDiscType=ARED"
./waf --run "queue-discs-benchmark --queueDiscType=HRED"
./waf --run "queue-discs-benchmark --queueDiscType=FRED"
./waf --run "queue-discs-benchmark --queueDiscType=SFLRED"
./waf --run "queue-discs-benchmark --queueDiscType=CSFLRED"
cd automation
