#!/bin/sh

set -u
set -e

if [ ! -e m4 ] ; then
	mkdir -p m4
fi

autoreconf --install
