========================================
TinyFastSS -- An Implementatin of FastSS
========================================

:Author: Fujimoto Seiji
:Published: 2012-03-01
:Copyright: This document has been placed in the public domain.

1. Introduction
---------------

`FastSS`_ is a data structure for approximate string matching that
allows you to retrieve a set of similar words very quickly.
FastSS was invented by researchers at Zurich University in 2007
(See [Bocek2007]_ for more details).

.. _FastSS: http://fastss.csg.uzh.ch/

TinyFastSS is a simple Python implementation of FastSS, written
in less than 300 LoC.

2. Installation
---------------

You can install TinyFastSS from PyPI::

    $ pip install TinyFastSS

The source code is available on https://github.com/fujimotos/TinyFastSS

3. Usage
--------

3.1. Basic Usage
++++++++++++++++

Create an index from your word sets.

.. code-block:: python

    import fastss

    with fastss.open('fastss.dat') as index:
        index.add("test")
        index.add("text")

Perform a similarity search using ``index.query()``.

.. code-block:: python

    with fastss.open('fastss.dat') as index:
        print(index.query('test'))

3.2. Use TinyFastSS from Command Line
+++++++++++++++++++++++++++++++++++++

You also can use TinyFastSS from console. Here is an example::

    $ head -n 3 /usr/share/dict/words
    aardvark
    abacus
    aerial
    $ python -m fastss -c index.dat /usr/share/dict/words
    $ python -m fastss -q index.dat adaptive
    {"0": ["adaptive"], "1": ["adoptive"], "2": ["additive"]}

4. Benchmark
------------

4.1. Dataset
++++++++++++

I benchmarked TinyFastSS using `SCOWL`_ v2015-08-24 (english-50).

.. _SCOWL: http://wordlist.aspell.net/

- The word set contained 98,986 words.
- The test machine was Intel Core i3-4010U (1.70GHz) with 4GB mem.

4.2. Index Creation Perrmance
+++++++++++++++++++++++++++++

It took 3 minutes to create an index file::

    $ time python -m fastss -c fastss.dat dictonary.txt
    3m0.71s real     2m44.35s user     0m16.43s system

The resulting index was 161 MB in size.

4.3. Query Perrmance
++++++++++++++++++++

I tested the query performance using randomly choosen words.

=========== =============== ===================
WORD           TIME [msec]  SPEED [query/sec]
=========== =============== ===================
sterner            7.57              132.1
spotlighted        2.43              411.5
burn              11.70               85.4
nirvana            1.16              862.0
conveyor           1.99              502.5
=========== =============== ===================

It took ~5ms on average to perform a query.
