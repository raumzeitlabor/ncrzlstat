ncrzlstat
=========

What is it?
-----------

This is a simple tool to display information about the current state of the
german hackerspace [RaumZeitLabor](https://raumzeitlabor.de/).

How does it work?
-----------------

This tool fetches information about the current state of the RaumZeitlabor from
[status.raumzeitlabor.de](http://status.raumzeitlabor.de/) and
[cosm](https://cosm.com/feeds/42055) and displays a set of selected values in a
ncurses based interface.

Prerequisites
-------------

In order to fetch the cosm datafeed, you need a cosm API key. `ncrzlstat` reads
the environment variable `RZLCOSMKEY` to obtain this key.

Dependencies
------------

 * [Jansson](http://www.digip.org/jansson/), a C library for encoding, decoding
   and manipulating JSON data.
 * [libcurl](http://curl.haxx.se/libcurl/), a free and easy-to-use client-side
   URL transfer library.
 * [ncurses](http://invisible-island.net/ncurses/ncurses.html), a free software
   emulation of curses in System V Release 4.0, and more.

