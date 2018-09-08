#!/bin/bash

set -e

PACKAGENAME=$(basename $(pwd))

if [ $# -eq 0 ] ; then
    echo 'At least, one argument is required for tag as release version.'
    exit 1
fi

if git tag | grep -q $1 ; then
    match=1
else
    match=0
fi

if [ $match -ne 1 ] ; then
    echo "Unexistent tag: ${1}"
    exit 1
fi
tag=$1

version=''
if [[ "${tag}" =~ ([0-9]+\.[0-9]+\.[0-9]+)$ ]] ; then
    version="${BASH_REMATCH[1]}"
fi

if [ -z "${version}" ] ; then
    echo "illegal format of release version: ${version}"
    exit 1
fi
prefix="${PACKAGENAME}-${version}/"
filename="${PACKAGENAME}-${version}.tar.xz"
ascname="${filename}.asc"

if [ -e "${filename}" ] ; then
    rm "${filename}"
fi
if [ -e "${ascname}" ] ; then
    rm "${ascname}"
fi

git archive --format=tar --prefix="${prefix}" "${tag}" | xz > "${filename}"

if tar tvf "${filename}" | grep -q "^${prefix}" ; then
    echo "missing prefix: ${prefix}"
    exit 1
fi

gpg --local-user 0xB5A586C7D66FD341 --armor --detach-sign "${filename}"
gpg --verify "${ascname}" "${filename}"

if [ $# -gt 1 ] ; then
    file-roller "${filename}"
fi
