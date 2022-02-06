==================================
Wagner-Fischer vs Myers' Algorithm
==================================

:Author: Fujimoto Seiji
:Published: 2020-08-15 
:Copyright: This document has been placed in the public domain.

Abstract
--------

This article compares the two most common algorithms for computing
Levenshtein Distance.

- Wagner-Fischer Algorithm (1974) [Wagner1974]_
- Myers' Bit-parallel Algorithm (1999) [Myers1999]_

The key result is that Myers' algorithm almost always outperforms
the Wagner-Fischer algorithm (the exception is very short inputs
of ``length < 6``).  The performance difference becomes striking
when input strings become longer.

.. figure:: 20200815/benchmark.svg
   :width: 600

   Comparison of Wagner-Fischer vs Myers Algorithm

1. Introduction
---------------

Algorithm selection is the corner-stone of problem solving. It can
result in a major difference in performance depending on which
algorithm is applied. Yet, the empirical evidence on which algorithm
should be applied (and when) is often limited.

To fill the gap, I wrote an implementation of the Wagner-Fischer
algorithm and Myers' bit-parallel algorithm, and tested each
implementation against the same data set.

The test data is random hex strings ranging from 2 characters to
64 characters. The computation time was measured by processing 1
million pairs of strings.

2. Result
---------

The figure 1 above shows the raw result data. The figure 2 below
shows the same result using a log scale, to better illustrate the
performance difference for shorter strings.

The "break-even" point seems to be six characters. Longer than
that, Myers's algorithm works better than the Wagner-Fischer
algorithm. Shorter than that, the converse holds true.

.. figure:: 20200815/benchmark-log.svg
   :width: 600

   Comparison of Wagner-Fischer vs Myers Algorithm (logscale)

The numbers are retrieved on Intel Xeon(R) E5-2660 (2.60Ghz) with
GCC 8.3.0.

You can download the benchmark script from :download:`benchmark.c <20200815/benchmark.c>`
and run it as follows:

.. code-block::

    $ cc -o benchmark -O2 benchmark.c
    $ ./benchmark

3. Implementation Notes
-----------------------

To the Wagner-Fischer algorithm, I applied Ukkonen's optimization
to improve the performance. My article :any:`wagner-fischer/index`
contains a detailed explanation of this technique. To my knowledge,
this is the best generic implementation of this algorithm.

To Myers' bit-parallel algorithm, I applied an unpublished
optimization technique to speed up the computation. In particular,
the original paper [Myers1999]_ required the computation of a lookup
table for every alphabet (e.g. 52 letters for ASCII alphabets, or
10k+ letters for Unicode). This requirement is often impractical for
real-world applications.

The basic idea of my improvement is that the lookup table can be
pre-computed in :math:`O(n)` time, rather than :math:`O(Σ)`, where n is the length
of strings and :math:`Σ` is the total number of alphabets. I found this
technique adds a good speedup to the algorithm (See `82a08e04`_ for details).

.. _82a08e04: https://github.com/fujimotos/polyleven/commit/82a08e04
