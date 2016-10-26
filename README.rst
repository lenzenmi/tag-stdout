==========
README.rst
==========

``tag_stdout`` is a simple wrapper that adds a tag to the output of the process it runs. It reads in both stdout and stderr but only outputs to stdout. A marker is prepended to the tag to indicate if the line originally came from stdout or stderr.

This is usefull for situations where you have multiple programs writing to the same terminal and you wish to know which program is responsible for each line of output.

It was primarily designed for use inside a docker container where multiple processes were being run simultaniously. This could be accomplished using supervisord with a configuration that forwards child process output to stdout.

It is written in modern C++ and has been tested to compile on arch linux and raspbery pi. The resulting binary is statically linked, so it can be included in a docker image without the compile time dependencies.

Building
========

.. code:: bash

    make
    ls -l bin/tag_stdout

Usage
=====

``tag_stdout`` takes one argument, the tag, followed by the program you are running and it's arguments

Example
-------

Here's my root partition from ``ls -l /``

.. code:: bash

    #              tag    command
    bin/tag_stdout my-tag ls -l /
    [O]:my-tag: total 16
    [O]:my-tag: lrwxrwxrwx   1 root root    7 Oct  1  2015 bin -> usr/bin
    [O]:my-tag: drwxr-xr-x   1 root root  368 Oct  2 20:25 boot
    [O]:my-tag: drwxr-xr-x  21 root root 3320 Oct 26 15:56 dev
    [O]:my-tag: drwxr-xr-x   1 root root 4368 Oct 26 09:55 etc
    [O]:my-tag: drwxr-xr-x   1 root root   20 Nov 12  2015 home
    [O]:my-tag: lrwxrwxrwx   1 root root    7 Oct  1  2015 lib -> usr/lib
    [O]:my-tag: lrwxrwxrwx   1 root root    7 Oct  1  2015 lib64 -> usr/lib
    [O]:my-tag: drwxr-xr-x   1 root root   74 Dec 14  2014 mnt
    [O]:my-tag: drwxr-xr-x   1 root root  210 Oct  9 09:37 opt
    [O]:my-tag: dr-xr-xr-x 247 root root    0 Oct 26 15:56 proc
    [O]:my-tag: drwxr-x---   1 root root 1558 Oct 24 10:33 root
    [O]:my-tag: drwxr-xr-x  25 root root  700 Oct 26 15:57 run
    [O]:my-tag: lrwxrwxrwx   1 root root    7 Oct  1  2015 sbin -> usr/bin
    [O]:my-tag: drwxr-xr-x   1 root root   32 May 11 15:40 srv
    [O]:my-tag: dr-xr-xr-x  13 root root    0 Oct 26 16:09 sys
    [O]:my-tag: drwxrwxrwt  11 root root  280 Oct 26 16:11 tmp
    [O]:my-tag: drwxr-xr-x   1 root root  156 Oct  2 18:28 usr
    [O]:my-tag: drwxr-xr-x   1 root root  146 Oct 14  2015 var

You can see, each line of output is pre-pended with the following:

    * '[O]' or '[E]' - The letter O indicates output was on stdout, the letter E indicates output was on stderr
    * ':' - delimiter
    * 'tag' - the first argument to ``tag_stdout`` in this case ``my-tag``
    * ': ' - delimiter. Notice the extra space after the colon. It's there to make the output easier to read.

Here's the same example with the output redirected to stdout

.. code:: bash

    bin/tag_stdout my-tag bash -c "ls -l / 1>&2"
    [E]:my-tag: total 16
    [E]:my-tag: lrwxrwxrwx   1 root root    7 Oct  1  2015 bin -> usr/bin
    [E]:my-tag: drwxr-xr-x   1 root root  368 Oct  2 20:25 boot
    [E]:my-tag: drwxr-xr-x  21 root root 3320 Oct 26 15:56 dev
    [E]:my-tag: drwxr-xr-x   1 root root 4368 Oct 26 09:55 etc
    [E]:my-tag: drwxr-xr-x   1 root root   20 Nov 12  2015 home
    [E]:my-tag: lrwxrwxrwx   1 root root    7 Oct  1  2015 lib -> usr/lib
    [E]:my-tag: lrwxrwxrwx   1 root root    7 Oct  1  2015 lib64 -> usr/lib
    [E]:my-tag: drwxr-xr-x   1 root root   74 Dec 14  2014 mnt
    [E]:my-tag: drwxr-xr-x   1 root root  210 Oct  9 09:37 opt
    [E]:my-tag: dr-xr-xr-x 246 root root    0 Oct 26 15:56 proc
    [E]:my-tag: drwxr-x---   1 root root 1558 Oct 24 10:33 root
    [E]:my-tag: drwxr-xr-x  25 root root  700 Oct 26 15:57 run
    [E]:my-tag: lrwxrwxrwx   1 root root    7 Oct  1  2015 sbin -> usr/bin
    [E]:my-tag: drwxr-xr-x   1 root root   32 May 11 15:40 srv
    [E]:my-tag: dr-xr-xr-x  13 root root    0 Oct 26 16:09 sys
    [E]:my-tag: drwxrwxrwt  11 root root  280 Oct 26 16:11 tmp
    [E]:my-tag: drwxr-xr-x   1 root root  156 Oct  2 18:28 usr
    [E]:my-tag: drwxr-xr-x   1 root root  146 Oct 14  2015 var

License
=======

GPLv3