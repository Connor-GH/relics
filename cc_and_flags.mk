_WFLAGS = -Wformat-security -Warray-bounds -Wstack-protector \
		  -Wall -Wextra -Wpedantic -Wshadow -Wvla -Wpointer-arith -Wwrite-strings \
		  -Wfloat-equal -Wcast-align -Wcast-qual \
		  -Wunreachable-code -Wundef -Werror=format-security -Werror=array-bounds \
		  -Werror=uninitialized
WGCC = -Wlogical-op -Wcast-align=strict \
	   -Wsuggest-attribute=format -Wsuggest-attribute=malloc \
	   -Wsuggest-attribute=pure -Wsuggest-attribute=const \
	   -Wsuggest-attribute=noreturn -Wsuggest-attribute=cold

WNOFLAGS=
VISIBILITY ?= -fvisibility=hidden

DCC ?= gdc
DCC_BASIC_O ?= -o
DCC_BASIC_C ?= -c
_DFLAGS =


ifeq ($(RELEASE),true)
	_COMMON_CFLAGS = -march=$(MARCH)
	D_MCPU_DMD = baseline
else
	_COMMON_CFLAGS = -march=x86-64
	D_MCPU_DMD = native
	MARCH = native
endif
_LFLAGS  =

# detect if the user chose GCC or Clang

ifeq ($(shell $(CC) -v 2>&1 | grep -c "gcc version"), 1)
	include gcc_chosen.mk
	ifeq ($(DEBUG),true)
		# gcc-specific security/debug flags
		WGCC   += -fanalyzer
	endif #debug
	_COMMON_CFLAGS += $(_WFLAGS) $(WGCC)

else ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
	include clang_chosen.mk
	_WFLAGS += -Weverything
	WNOFLAGS += -Wno-unsafe-buffer-usage -Wno-reserved-macro-identifier \
				-Wno-unused-macros
	ifeq ($(DEBUG),true)
	 # clang-specific security/debug flags
endif #debug

	_COMMON_CFLAGS += $(_WFLAGS)
	WNOFLAGS += -Wno-disabled-macro-expansion
endif #compiler

ifeq ($(DEBUG),true)
	# generic security/debug flags
endif # DEBUG


# Flags every compile will need
_COMMON_CFLAGS += $(COMMON_FLAGS) $(COMMON_FLAGS_C) -D_PACKAGE_NAME=\"$(PACKAGE_NAME)\" \
				 -D_PACKAGE_VERSION=\"$(PACKAGE_VERSION)\" \
		  $(VISIBILITY) $(WNOFLAGS)
_CFLAGS = $(_COMMON_CFLAGS) -std=$(C_STD) $(CFLAGS)
_LFLAGS += $(LDFLAGS)
_LD_DFLAGS =
# LTO will be turned on later
ifneq ($(DCC), gdc)
	DCC_BASIC_O = -of=
	_DFLAGS += -release -betterC
  ifeq ($(DCC),dmd)
	_DFLAGS += -O -mcpu=$(D_MCPU_DMD) -defaultlib=none # -mattr=-avx,-sse,64bit
  else # is ldc
	_DFLAGS += -O3 -mcpu=$(MARCH) -nodefaultlib -mattr=-avx,-sse,64bit
	# _LD_DFLAGS += -L-lstdc++ -release
    ifeq ($(CC), clang)
    ifeq ($(DEBUG),true)
    endif
	  #_DFLAGS += $(LTO)
	  #_CFLAGS += $(LTO)
	  #_CXXFLAGS += $(LTO)
	  #_LD_DFLAGS += $(LTO) -Xcc="$(LTO)"
    endif

  endif # if dmd/ldc
else
	_DFLAGS += $(COMMON_FLAGS) -march=$(MARCH) -fno-druntime -defaultlib=none -fno-stack-protector $(DFLAGS)
	_LD_DFLAGS +=
	GDC_XD = -xd
endif # if gdc
_DFLAGS += $(DFLAGS)
_LD_DFLAGS += -L-O2 $(LDFLAGS) $(LFLAGS)
