#!/bin/sh
# usage: b.sh PRODUCT SCEN [MAXQ]
set -e
export DEVKITPRO=/tmp/dkproot/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export PATH=$DEVKITARM/bin:$PATH
cd /tmp/vp
{ echo "#define PRODUCT $1"; echo "#define SCEN $2"; [ -n "$3" ] && echo "#define MAXQ $3"; } > source/config.h
make -s clean >/dev/null
make -s 2>&1 | grep -E "error" || true
$DEVKITPRO/tools/bin/dlditool /tmp/melon.dldi vp.nds >/dev/null
scp -F /tmp/lima-home/fvds/ssh.config -P 60022 -q vp.nds lima-fvds:/tmp/vp.nds
