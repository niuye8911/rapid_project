## RAPID_Project

project dir for LP generation (.lp) from Description file (.desc)

### - Contents

This dir contains 2 main parts:

1) The python script to generate the XML representation of a KDG

2) The Cpp code to generate the LP file to be further interpreted and solved by Gurobi

### - BUILD

The python script does not need to be installed, python interpreter (2.7) will be enough

Steps to build the sample C++ code:

1) The header file to parse XML is already in the source

2) make

```
$ make
```

The output should be an executable, **lp_generator**


### - example calls


1) generate the XML
```
$ python rapid.py --desc ../example_input/Small.desc --outdir ../example_output/
```
it should generate a ***Small.xml*** in ../example_output

2) generate the LP
```
$ ./lp_generator --budget 99 --xml ../example_output/Small.xml --app Small
```
it should generate a ***Small.lp*** in ../example_output
