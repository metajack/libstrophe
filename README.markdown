# libcouplet

libcouplet is a fork of libstrophe - a lightweight XMPP client library
written in C. Goal of the fork is make library suitable for multi-thread
applications.

## Build Instructions

If you are building from a source control checkout, run:

    ./bootstrap.sh

to generate the `configure` script.

From the top-level directory, run the following commands:

    ./configure
    make

The `examples` directory contains some examples of how to
use the library; these may be helpful in addition to the
API documentation

## Requirements

libcouplet requires:

- expat or libxml2 - expat is the default; use --with-libxml2 to
  switch
- libresolv on UNIX systems - make sure you include -lresolv
  if you are compiling by hand.

In addition, if you wish to run the unit tests, you will need the
check package.

Windows systems aren't supported for now.

### OS X (with Homebrew package manager)

You can install the requirements with:

    brew install expat
    brew install check

## Documentation

API documentation is inline with the code and conforms to Doxygen
standards. You can generate an HTML version of the API documentation
by running:

    doxygen

Then open `docs/html/index.html`.

