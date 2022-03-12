:orphan:

==============================================
Polyleven -- Fast Pythonic Levenshtein Library
==============================================

:Author: Fujimoto Seiji
:Published: 2018-12-15
:Updated: 2021-02-05
:Copyright: This document has been placed in the public domain.

1. Introduction
---------------

:dfn:`Polyleven` is a Levenshtein distance library for Python, with
a special attention to efficiency.

The basic idea behind this library is that we can gain the best of
different algorithms by switching between them depending on the
kinds of input strings.

2. How to Install
-----------------

2.1. PyPI
+++++++++

::

   $ pip install polyleven

2.2. Source Distribution
++++++++++++++++++++++++

- `polyleven-0.7.zip <https://github.com/fujimotos/polyleven/archive/refs/tags/0.7.zip>`_

  - Optimize the implementation of ``myers1999_block()``.
  - Migrate the license to MIT License `#2`_

- `polyleven-0.6.zip <https://github.com/fujimotos/polyleven/archive/refs/tags/0.6.zip>`_

  - Fix the implementation of blockhash

- `polyleven-0.5.zip <https://github.com/fujimotos/polyleven/archive/refs/tags/0.5.zip>`_

  - Add Windows platform support.

.. _#2: https://github.com/fujimotos/polyleven/pull/2

2.3. GIT Repository
+++++++++++++++++++

::

   $ git clone https://github.com/fujimotos/polyleven

3. Usage
--------

3.1. Basic Usage
++++++++++++++++

Polyleven provides a single interface function ``levenshtein()``. You
can use this function to measure the similarity of two strings.

>>> from polyleven import levenshtein
>>> levenshtein('aaa', 'ccc')
3

3.2. Advanced Usage
+++++++++++++++++++

If you only care about distances under a certain threshold, you can
pass the max threshold to the third argument.

>>> levenshtein('acc', 'ccc', 1)
1
>>> levenshtein('aaa', 'ccc', 1)
2

In general, you can gain a noticeable speed boost with threshold
:math:`k < 3`.

4. Benchmark
------------

4.1  English Words
++++++++++++++++++

To compare Polyleven with other Pythonic edit distance libraries,
a million word pairs was generated from `SCOWL`_.

.. _SCOWL: http://wordlist.aspell.net/

Each library was measured how long it takes to evaluate all of
these words. The following table summarises the result:

============================== ============ ================
Function Name                    TIME[sec]    SPEED[pairs/s]
============================== ============ ================
edlib                                4.763           208216
editdistance                         1.943           510450
jellyfish.levenshtein_distance       0.722          1374081
distance.levenshtein                 0.623          1591396
Levenshtein.distance                 0.500          1982764
polyleven.levenshtein                0.431          2303420
============================== ============ ================

You may notice that edlib and editdistance appear to be slower than
other libraries. This is because both internally use Myers' algorithm
for computing the Levenshtein distance.

The idea behind Myers' algorithm is to use bit parallelism to speed
up the computation. In order to perform the computation efficiently,
we need to pre-compute a table of bit patterns for each character.
For short inputs like English words (the average length is just 7-8
characters), the cost of pre-computation dwarfs the benefit of
parallelism.

4.2. Longer Inputs
++++++++++++++++++

To evaluate the efficiency for longer inputs, I careted 5000 pairs
of random strings of size 16, 32, 64, 128, 256, 512 and 1024.

Each library was measured how fast it can process these entries. [#fn1]_

============ =====  =====  =====  =====  =====  =====  ======
Library      N=16   N=32   N=64   N=128  N=256  N=512  N=1024
============ =====  =====  =====  =====  =====  =====  ======
edlib        0.040  0.063  0.094  0.205  0.432  0.908   2.089
editdistance 0.027  0.049  0.086  0.178  0.336  0.740  58.139
jellyfish    0.009  0.032  0.118  0.470  1.874  8.877  42.848
distance     0.007  0.029  0.109  0.431  1.726  6.950  27.998
Levenshtein  0.006  0.022  0.085  0.336  1.328  5.286  21.097
polyleven    0.003  0.005  0.010  0.043  0.149  0.550   2.109
============ =====  =====  =====  =====  =====  =====  ======

You may notice that edlib and editdistance are good at handling
longer inputs. Other libraries have a steep cost curve as the string
length increases.

The good thing about polyleven is that it can process both short
and long inputs efficiently.

4.3. List of Libraries
++++++++++++++++++++++

============ ======= ==========================================
Library      Version URL
============ ======= ==========================================
edlib        v1.2.1  https://github.com/Martinsos/edlib
editdistance v0.4    https://github.com/aflc/editdistance
jellyfish    v0.5.6  https://github.com/jamesturk/jellyfish
distance     v0.1.3  https://github.com/doukremt/distance
Levenshtein  v0.12   https://github.com/ztane/python-Levenshtein
polyleven    v0.3    https://github.com/fujimotos/polyleven
============ ======= ==========================================

.. [#fn1] Measured using Python 3.5.3 on Debian Jessie with Intel Core
  i3-4010U (1.70GHz)

5. Implementation Note
----------------------

As of version 0.5, polyleven uses the following heuristics to
choose an algorithm::

    +===============+    Yes    +---------------------+
    ||   k = 0     ||---------->| PyUnicode_Compare() |
    +===============+           +---------------------+
          | No
          V
    +===============+    Yes    +-------------------+
    ||   k < 4     ||---------->| mbleven algorithm |
    +===============+           +-------------------+
          | No
          V
    +===============+    Yes    +------------------+
    || len(s) < 65 ||---------->| Myers' algorithm |
    +===============+           +------------------+
          | No
          V
    +-------------------------------+
    | Myers algorithm (with blocks) |
    +-------------------------------+

Before 0.4, polyleven used the Wagner-Fischer algorithm for shorter
strings, but, alas, it turned out that reasonably optimized Myers'
algorithm almost always performs better.
