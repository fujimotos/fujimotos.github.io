========
Research
========

This page contains a collection of technical surveys which I
did during development of polyleven.

List of articles
================

:any:`wagner-fischer/index`
    In this expository note, I explain several techniques
    to optimize the Wagner-Fischer algorithm.

    This note contains a small benchmark test program as well.
    The result suggests that it is possible to reduce the
    computation time by 20-40% in general cases.
    
:any:`20200815-Comparison`
    This article compares the two most common algorithms for
    computing Levenshtein Distance.

:any:`fastcomp/index`
    `mbleven` is a fast algorithm to compute k-bounded Levenshtein
    distance. In general, it's one of the fastest algorithm for cases
    where the bound parameter is small (:math:`k < 3`).

.. toctree::
   :caption: Table of Contents
   :maxdepth: 1
   :hidden:

   wagner-fischer/index.rst
   20200815-Comparison.rst
   fastcomp/index.rst
   tinyfss/index.rst
