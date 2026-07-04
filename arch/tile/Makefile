# SPDX-License-Identifier: GPL-2.0
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies.

# The tile compiler may emit .eh_frame information for backtracing.
# In kernel modules, this causes load failures due to unsupported relocations.
KBUILD_CFLAGS   += -fno-asynchronous-unwind-tables

LIBGCC_PATH     := \
  $(shell $(CC) $(KBUILD_CFLAGS) $(KCFLAGS) -print-libgcc-file-name)

KBUILD_DEFCONFIG := tilegx_defconfig

# The MDE userland (initramfs /etc/rc) gates its tilegx device setup on
# `uname -m` = "tilegx"; the top-level default UTS_MACHINE would be "tile".
UTS_MACHINE := tilegx

# Used as a file extension when useful, e.g. head_$(BITS).o
export BITS := 64

CHECKFLAGS	+= -m$(BITS)

libs-y		+= arch/tile/lib/
libs-y		+= $(LIBGCC_PATH)

define archhelp
	echo '  vmlinux         - the ELF image the Tilera hypervisor loads directly'
endef
