## RAPID(C)

*this is the source repo for RAPID(C) library (linux)

### - Contents

RAPID(C) source contains 2 main parts:

1) The C++ static library being used later to instrument applications [RAPID(C)_root]

2) The Python script to run the training [RAPID(C)_root/modelConstr/source]

### - BUILD

The python script does not need to be installed, python interpreter (2.7) will be enough

Steps to build the C++ library:

1) install necessary libs:

```
$ sudo apt-get install libcurl-dev
```

2) make sure the path to libcurl is updated in [Makefile](https://github.com/niuye8911/rapidlib-linux/blob/master/makefile)

3) make

```
$ make
```

The output should contain a static library, **rsdg.a**


### - LEARN MORE

Please refer to the [IO page](https://niuye8911.github.com/rapidlib-linux) for this project to learn how to use RAPID(C) to write your own approximate applications.
