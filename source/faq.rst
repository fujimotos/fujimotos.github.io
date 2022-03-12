===
FAQ
===

.. contents:: Frequently Asked Questions
   :local:
   :backlinks: none

General Questions
=================

What is Levenshtein Distance?
-----------------------------

.. epigraph::

   Levenshtein distance is a string metric for measuring the
   difference between two sequences.

   Informally, the Levenshtein distance between two words is
   the minimum number of single-character edits (insertions,
   deletions or substitutions) required to change one word
   into the other.

   It is named after the Soviet mathematician Vladimir Levenshtein,
   who considered this distance in 1965.

   -- https://en.wikipedia.org/wiki/Levenshtein_distance
   

Who uses it?
------------

- Programmers use Levenshtein distance for simple spell correction.
  For example, `Git uses it to suggest the most similar command.
  <https://github.com/git/git/blob/v2.35.1/help.c#L605-L606>`_ 

  .. code-block:: bash

    $ git checkou
    git: 'checkou' is not a git command. See 'git --help'.
    
    The most similar command is
            checkout

- Biologists use Levenshtein distance to study mutation of DNA/RNA
  sequences. (...)

- Linguists use Levenshtein distance to classify languages into
  a tree. See [Serva2008]_ for example.

- Other usages include hand-writing recognition. You can check my
  toy implementation :any:`Charec <20191127-charec>`.

How to compute it?
------------------

There is two general algorithms for Levenshtein distance:

- Wagner-Fischer Algorithm (1974) [Wagner1974]_
- Myers' Bit-parallel Algorithm (1999) [Myers1999]_

Read :any:`20200815-Comparison` to learn which suites for
your use case.

Note that Myers algorithm cannot be applied to variants of 
Leveshtein distance (e.g. edit costs are not fixed to 1).

Polyleven Questions
===================

mbleven Questions
=================


