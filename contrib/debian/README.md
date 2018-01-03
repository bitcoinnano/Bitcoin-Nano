
Debian
====================
This directory contains files used to package btcnanod/btcnano-qt
for Debian-based Linux systems. If you compile btcnanod/btcnano-qt yourself, there are some useful files here.

## btcnano: URI support ##


btcnano-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install btcnano-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your btcnano-qt binary to `/usr/bin`
and the `../../share/pixmaps/btcnano128.png` to `/usr/share/pixmaps`

btcnano-qt.protocol (KDE)

