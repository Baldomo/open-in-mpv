#!/usr/bin/env bash

set -e

force_update=true
desktop_dir=~/.local/share/applications

if [[ ! -f "${desktop_dir}/open-in-mpv.desktop" || "${force_update}" = true ]]; then
	pushd "${desktop_dir}"
	cat << 'EOF' >> open-in-mpv.desktop
[Desktop Entry]
Name=open-in-mpv
Exec=env LD_PRELOAD='' open-in-mpv %u
Type=Application
Terminal=false
NoDisplay=true
MimeType=x-scheme-handler/mpv
EOF
	update-desktop-database .
	popd
fi

xdg-mime default open-in-mpv.desktop x-scheme-handler/mpv
