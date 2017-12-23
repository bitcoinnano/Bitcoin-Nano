#!/bin/sh

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

BTCNANOD=${BTCNANOD:-$SRCDIR/btcnanod}
BTCNANOCLI=${BTCNANOCLI:-$SRCDIR/btcnano-cli}
BTCNANOTX=${BTCNANOTX:-$SRCDIR/btcnano-tx}
BTCNANOQT=${BTCNANOQT:-$SRCDIR/qt/btcnano-qt}

[ ! -x $BTCNANOD ] && echo "$BTCNANOD not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
BTNVER=($($BTCNANOCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for btcnanod if --version-string is not set,
# but has different outcomes for btcnano-qt and btcnano-cli.
echo "[COPYRIGHT]" > footer.h2m
$BTCNANOD --version | sed -n '1!p' >> footer.h2m

for cmd in $BTCNANOD $BTCNANOCLI $BTCNANOTX $BTCNANOQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BTNVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${BTNVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
