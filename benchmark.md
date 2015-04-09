# Introduction #

Test with xmlbench<http://xmlbench.sourceforge.net/> framework.


# Details #

Test result:

expat-2.0.0 + Sablot-1.0.3
```
--->Running <expat-sablotron-dom> benchmarks:
-> xmlgen 4 KB
Initialisation time 0.106 + 0.613(6.48%) ms, Parsing Time 1.620(2.45%) ms
-> xmlgen 256 KB
Initialisation time 0.137 + 1.121(114.60%) ms, Parsing Time 126.102(1.02%) ms
```

spxml
```
--->Running <spxml-dom> benchmarks:
-> xmlgen 4 KB
Initialisation time 0.002 + 0.388(16.89%) ms, Parsing Time 1.601(4.09%) ms
-> xmlgen 256 KB
Initialisation time 0.002 + 5.474(30.01%) ms, Parsing Time 164.281(1.00%) ms
```