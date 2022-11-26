==========
Quickstart
==========

Installation
============

You can install polyleven from PyPI::

	$ pip install polyleven

Compute Levenshtein Distance
============================

Here is the first step to use polyleven:

>>> import polyleven
>>> polyleven.levenshtein("aiueo", "abcde")
4

Polyleven can handle multi-byte characters properly:

>>> polyleven.levenshtein("文字", "漢字")
1

That's all! Now you can start using polyleven in your project.

Perform fuzzy search on Wikipedia
=================================

Here is a quick demo that you can test at home: Implement a fast fuzzy
title search on Wikipedia articles.

First, download the title list from Wikiipedia (90MB with Gzip compression)::

   $ wget http://dumps.wikimedia.org/enwiki/latest/enwiki-latest-all-titles-in-ns0.gz
   $ gzip -d enwiki-latest-all-titles-in-ns0.gz

Now you can perform fuzzy search on Wikipedia titles:

>>> from polyleven import levenshtein
>>> titles = [t.strip() for t in open('enwiki-latest-all-titles-in-ns0')]
>>> [t for t in titles if levenshtein(x, "Mark_Twain") < 3]
['Mark_Tedin', 'Mark_Twang', 'Marc_Train', 'Mark_Trail', 'Mark_Tuan', 'Mark_Twain', 'Marc_Twain', 'Mark_Krain', 'Mark_twain', 'Mack_Swain', 'Mark_Tobin', 'Mark_Brain', 'Mark_Turin', 'Mark_Tulin', 'Mark_Tan', 'Mark_Fain', 'Dark_Train', 'Mark_Spain']

Easy, huf?

What's next?
============

* In this document, you learned how to install polyleven and compute Levenshtein distance.
* The full source code of polyleven is available on `GitHub <https://github.com/fujimotos/polyleven>`_.
* If you have any feedback, please `submit an issue <https://github.com/fujimotos/polyleven/issues>`_.
