#!/bin/sh
echo "Initializing submodules"
git submodule init 
echo "Updating submodules"
git submodule update
echo "Installing submodule hooks"
install_hooks="$PWD/install-hooks.sh"
git submodule foreach '$install_hooks || :'
echo "Installing hooks"
$install_hooks
