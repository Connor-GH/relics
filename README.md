## relics
 Exokernel OS in C99. Made more of a proof of concept and hobby than anything.
<img src="pictures/screen4.png" align="left">
<img src="pictures/screen1.png" align="left">
<img src="pictures/screen2.png" align="right">
<img src="pictures/screen3.png" align="right">

- builds in 0.1 sec with a 16-thread Ryzen 5700G
- can run on as little as 1033K of ram as of commit b8a7ca9f212b2dfad0eadf5f07625d7e21414d5f

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
