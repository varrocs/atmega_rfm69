#IN=tracefile.txt
IN=trace.in
simulavr -f rfm -F 16000000 -d atmega328 -v -c vcd:$IN:trace.out.vcd:rw
