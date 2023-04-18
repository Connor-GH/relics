## relics
 Exokernel OS in C99. Made more of a proof of concept and hobby than anything.
<img src="pictures/screen1.png" align="left">
<img src="pictures/screen2.png" align="right">
<img src="pictures/screen3.png" align="right">

download with:

``git clone https://github.com/Connor-GH/relics``

``cd relics``

compile with:

``make``


# Mandatory Dependencies:
- clang/gcc
- gmake
- QEMU-x86_64

Fair warning: use either the lld, gold, or mold linker.
The BFD linker works, but is not ideal.
