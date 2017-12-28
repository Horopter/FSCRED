#!/bin/bash
cd ..
./waf --run "queue-discs-benchmark --queueDiscType=RED"
./waf --run "queue-discs-benchmark --queueDiscType=ARED"
./waf --run "queue-discs-benchmark --queueDiscType=HRED"
./waf --run "queue-discs-benchmark --queueDiscType=FRED"
./waf --run "queue-discs-benchmark --queueDiscType=SCRED"
./waf --run "queue-discs-benchmark --queueDiscType=FSCRED"
./waf --run "queue-discs-benchmark --queueDiscType=PfifoFast"
./waf --run "queue-discs-benchmark --queueDiscType=CoDel"
./waf --run "queue-discs-benchmark --queueDiscType=FqCoDel"
./waf --run "queue-discs-benchmark --queueDiscType=PIE"
cd automation
