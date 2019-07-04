#!/usr/bin/env bash

set -e

desktop_dir=~/.local/share/applications

if [[ ! -d "$desktop_dir/open-in-mpv.desktop" ]]; then
	pushd $desktop_dir
	cat << 'EOF' >> open-in-mpv.desktop
[Desktop Entry]
Name=open-in-mpv
Exec=open-in-mpv %u
Type=Application
Terminal=false
MimeType=x-scheme-handler/mpv
EOF
	update-desktop-database .
	popd
fi

xdg-mime default open-in-mpv.desktop x-scheme-handler/mpv