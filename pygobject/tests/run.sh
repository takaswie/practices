#!/bin/bash

# NOTE:
# Current working directory should be in root of this repository.

export LD_LIBRARY_PATH=src/.libs/:/usr/lib:/lib
export GI_TYPELIB_PATH=src/:/usr/lib/girepository-1.0

./tests/test.py
