CC = clang
AS ?= clang -integrated-as -c
LTO = -flto=full
BINUTILS_PREFIX = llvm-

export CLANG_MAJOR_VERSION = $(shell clang --version | sed -n 1p | awk '{print $$3}' | sed -E 's/\..*//g')
