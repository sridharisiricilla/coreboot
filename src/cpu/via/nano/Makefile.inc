##
## This file is part of the coreboot project.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##

subdirs-y += ../../x86/tsc
subdirs-y += ../../x86/mtrr
subdirs-y += ../../x86/lapic
subdirs-y += ../../x86/cache
subdirs-y += ../../x86/smm

ramstage-y += nano_init.c
ramstage-y += update_ucode.c

cpu_microcode_bins += 3rdparty/blobs/cpu/via/nano/microcode.bin

cpu_incs-y += $(src)/cpu/via/car/cache_as_ram.inc
