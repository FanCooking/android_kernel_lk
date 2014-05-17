/* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <debug.h>
#include <platform.h>
#include <qgic.h>
#include <qtimer.h>
#include <board.h>
#include <mmu.h>
#include <arch/arm/mmu.h>
#include <platform/iomap.h>
#include <smem.h>
#include <reg.h>
#include <board.h>
#include <qpic_nand.h>
#include <target.h>

extern struct smem_ram_ptable* target_smem_ram_ptable_init();

#define MB                                  (1024*1024)

#define MSM_IOMAP_SIZE                      ((MSM_IOMAP_END - MSM_IOMAP_BASE)/MB)

/* LK memory - Strongly ordered, executable */
#define LK_MEMORY                             (MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH | \
                                              MMU_MEMORY_AP_READ_WRITE)
/* Scratch memory - Strongly ordered, non-executable */
#define SCRATCH_MEMORY                        (MMU_MEMORY_TYPE_NORMAL | \
                                              MMU_MEMORY_AP_READ_WRITE | MMU_MEMORY_XN)
/* Peripherals - shared device */
#define IOMAP_MEMORY                          (MMU_MEMORY_TYPE_DEVICE_SHARED | \
                                              MMU_MEMORY_AP_READ_WRITE | MMU_MEMORY_XN)

#define SCRATCH_REGION1_VIRT_START            SCRATCH_REGION1
#define SCRATCH_REGION2_VIRT_START            SCRATCH_REGION2

#define SDRAM_BANK0_LAST_FIXED_ADDR           (SCRATCH_REGION2 + SCRATCH_REGION2_SIZE)

/* Map all the accesssible memory according to the following rules:
 * 1. Map 1MB from MSM_SHARED_BASE with 1 -1 mapping.
 * 2. Map MEMBASE - MEMSIZE with 1 -1 mapping.
 * 3. Map all the scratch regions immediately after Appsbl memory.
 *     Virtual addresses start right after Appsbl Virtual address.
 * 4. Map all the IOMAP space with 1 - 1 mapping.
 * 5. Map all the rest of the SDRAM/ IMEM regions as 1 -1.
 */
mmu_section_t mmu_section_table[] = {
/*   Physical addr,         Virtual addr,               Size (in MB),              Flags   */
	{MSM_SHARED_BASE,       MSM_SHARED_BASE,            1,                         SCRATCH_MEMORY},
	{MEMBASE,               MEMBASE,                    MEMSIZE / MB,              LK_MEMORY},
	{MSM_IOMAP_BASE,        MSM_IOMAP_BASE,             MSM_IOMAP_SIZE,            IOMAP_MEMORY},
	{SCRATCH_REGION1,       SCRATCH_REGION1_VIRT_START, SCRATCH_REGION1_SIZE / MB, SCRATCH_MEMORY},
	{SCRATCH_REGION2,       SCRATCH_REGION2_VIRT_START, SCRATCH_REGION2_SIZE / MB, SCRATCH_MEMORY},
};

void platform_early_init(void)
{
	/* Initialize board identifier data */
	board_init();

	/* Initialize clock driver */
	platform_clock_init();

	/* Initialize interrupt controller */
	qgic_init();

	/* timer */
	qtimer_init();
}

void platform_init(void)
{
	dprintf(INFO, "platform_init()\n");
}

void platform_uninit(void)
{
	qtimer_uninit();
	if (!platform_boot_dev_isemmc())
		qpic_nand_uninit();
}

/* Do not use default identitiy mappings. */
int platform_use_identity_mmu_mappings(void)
{
	return 0;
}

uint32_t platform_get_sclk_count(void)
{
	return readl(MPM2_MPM_SLEEP_TIMETICK_COUNT_VAL);
}

addr_t get_bs_info_addr()
{
	return ((addr_t)BS_INFO_ADDR);
}

/* Setup memory for this platform */
void platform_init_mmu_mappings(void)
{
	uint32_t i;
	uint32_t sections;
	ram_partition ptn_entry;
	uint32_t table_size = ARRAY_SIZE(mmu_section_table);
	uint32_t len = 0;

	ASSERT(smem_ram_ptable_init_v1());

	len = smem_get_ram_ptable_len();

	/* Configure the MMU page entries for SDRAM and IMEM memory read
	   from the smem ram table*/
	for(i = 0; i < len; i++)
	{
		smem_get_ram_ptable_entry(&ptn_entry, i);
		if(ptn_entry.type == SYS_MEMORY)
		{
			if((ptn_entry.category == SDRAM) ||
			   (ptn_entry.category == IMEM))
			{
				/* Check to ensure that start address is 1MB aligned */
				ASSERT((ptn_entry.start & (MB-1)) == 0);

				sections = (ptn_entry.size) / MB;
				while(sections--)
				{
					arm_mmu_map_section(ptn_entry.start +
										sections * MB,
										ptn_entry.start +
										sections * MB,
										(MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH |
										 MMU_MEMORY_AP_READ_WRITE |
										 MMU_MEMORY_XN));
				}
			}
		}
	}

	/* Configure the MMU page entries for memory read from the
	   mmu_section_table */
	for (i = 0; i < table_size; i++)
	{
		sections = mmu_section_table[i].num_of_sections;

		while (sections--)
		{
			arm_mmu_map_section(mmu_section_table[i].paddress +
								sections * MB,
								mmu_section_table[i].vaddress +
								sections * MB,
								mmu_section_table[i].flags);
		}
	}
}

addr_t platform_get_virt_to_phys_mapping(addr_t virt_addr)
{
	uint32_t paddr;
	uint32_t table_size = ARRAY_SIZE(mmu_section_table);
	uint32_t limit;

	for (uint32_t i = 0; i < table_size; i++)
	{
		limit = (mmu_section_table[i].num_of_sections * MB) - 0x1;

		if (virt_addr >= mmu_section_table[i].vaddress &&
			virt_addr <= (mmu_section_table[i].vaddress + limit))
		{
				paddr = mmu_section_table[i].paddress + (virt_addr - mmu_section_table[i].vaddress);
				return paddr;
		}
	}
	/* No special mapping found.
	 * Assume 1-1 mapping.
	 */
	 paddr = virt_addr;
	return paddr;
}

addr_t platform_get_phys_to_virt_mapping(addr_t phys_addr)
{
	uint32_t vaddr;
	uint32_t table_size = ARRAY_SIZE(mmu_section_table);
	uint32_t limit;

	for (uint32_t i = 0; i < table_size; i++)
	{
		limit = (mmu_section_table[i].num_of_sections * MB) - 0x1;

		if (phys_addr >= mmu_section_table[i].paddress &&
			phys_addr <= (mmu_section_table[i].paddress + limit))
		{
				vaddr = mmu_section_table[i].vaddress + (phys_addr - mmu_section_table[i].paddress);
				return vaddr;
		}
	}

	/* No special mapping found.
	 * Assume 1-1 mapping.
	 */
	 vaddr = phys_addr;

	return vaddr;
}
