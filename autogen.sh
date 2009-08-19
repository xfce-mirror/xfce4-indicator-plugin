#!/bin/sh
#
# $Id: autogen.sh 2398 2007-01-17 17:47:38Z nick $
#
# Copyright (c) 2002-2007
#         The Xfce development team. All rights reserved.
#

(type xdt-autogen) >/dev/null 2>&1 || {
  cat >&2 <<EOF
autogen.sh: You don't seem to have the Xfce development tools installed on
            your system, which are required to build this software.
            Please install the xfce4-dev-tools package first, it is available
            from http://www.xfce.org/.
EOF
  exit 1
}

# substitute revision and linguas
linguas=`ls po/*.po | awk 'BEGIN { FS="[./]"; ORS=" " } { print $2 }'`
if [ -d .git ]; then
  revision=`git rev-parse --short HEAD 2>/dev/null`
fi

if [ -z "$revision" ]; then
  revision="UNKNOWN"
fi

sed -e "s/@LINGUAS@/${linguas}/g" \
    -e "s/@REVISION@/${revision}/g" \
    < "configure.in.in" > "configure.in"

exec xdt-autogen $@
