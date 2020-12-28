![](https://github.com/LBCrion/sfwbar/blob/main/.github/sfwbar.png)

### SFWBar

SFWBar (Sway Floating Window Bar) is a flexible taskbar application for 
[Sway](https://github.com/swaywm/sway) wayland
compositor, designed with a stacking layout in mind. 

## SFWBar implements the following features:
1. Taskbar - to control floating windows
1. Window placement engine - to open new windows in more logical locations
1. A simple widget set to display information from system files

## Compiling from Source

Install dependencies:
glib
gtk3
gtk-layer-shell
json-c

Compile instructions:
meson build
ninja -C build
sudo ninja -C build install

## Configuration
Copy sfwbar.config and sfwbar.css from /usr/share/sfwbar/ to ~/.config/sfwbar/

you may want to add the following line to your sway config file to open windows
as floating by default:

for_window [app_id="[.]*"] floating enable

See man page for config file details. 
