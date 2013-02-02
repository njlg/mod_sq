# mod_sq

Apache module for running Squirrel files

## Overview

Mod_sq is an Apache web server module that will handle creating a Squirrel
virtual machine, setting up a few globals and functions, and running the
request Squirrel script (e.g. nut) through the virtual machine.

## Build

To build and install mod_sq:

* Make sure you have Apache and Squirrel installed and all the headers and libraries installed.
* Run 'autoreconf -vfi'
* Run './configure'
* Run 'make'
* Run 'make install'

## Setup

* Add the following lines to your Apache configuration:
        LoadModule squirrel_module modules/mod_sq.so
        <IfModule mod_mime.c>
           AddHandler application/x-httpd-sq .nut
        </IfModule>
        DirectoryIndex index.nut
* Restart Apache
* Copy the example nuts (either from Squirrel or mod_sq) to somewhere in your htdocs area
* Point your browser to one of the example nuts

# Addendum

mod_sq Homepage: [http://github.com/njlg/mod_sq](http://github.com/njlg/mod_sq)

Squirrel Homepage: [http://squirrel-lang.org](http://squirrel-lang.org)
