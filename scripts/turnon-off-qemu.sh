#!/bin/bash
# set username of user
USERNAME="`whoami`"
printf "Welcome to the QEMU Enabler/Disabler!\n"
printf  \
"Pick an option:\n \
1 - Turn QEMU on.   Only works if it is off.\n \
2 - Turn QEMU off.  Only works if it is on.\n"
read response
[[ "$response" == "1" ]] &&
    sudo usermod -a -G libvirt,libvirt-qemu $USERNAME &&
    sudo systemctl start libvirtd &&
    printf "libvirtd is now on\n" &&
    id $USERNAME &&
    printf "Don't forget!\n You get out of QEMU with ctrl+alt+g\n"
[[ "$response" == "2" ]] &&
    sudo gpasswd -d $USERNAME libvirt &&
    sudo gpasswd -d $USERNAME libvirt-qemu &&
    sudo systemctl stop libvirtd &&
    printf "libvirtd is now off\n" &&
    id $USERNAME
