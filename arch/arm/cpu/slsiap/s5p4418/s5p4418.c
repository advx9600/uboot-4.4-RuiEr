/*
 * (C) Copyright 2009 Nexell Co.,
 * jung hyun kim<jhkim@nexell.co.kr>
 *
 * Configuation settings for the Nexell board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <platform.h>
#include <mach-api.h>
#include <pm.h>

#define DBGOUT(msg...)		do { printf("sys: " msg); } while (0)
#define printk(msg...)		do { printf(msg); } while (0)

static void cpu_base_init(void)
{
	U32 tie_reg, val;
	int i = 0;

	NX_RSTCON_Initialize();
	NX_RSTCON_SetBaseAddress((U32)IO_ADDRESS(NX_RSTCON_GetPhysicalAddress()));

	NX_TIEOFF_Initialize();
	NX_TIEOFF_SetBaseAddress((U32)IO_ADDRESS(NX_TIEOFF_GetPhysicalAddress()));

	NX_CLKGEN_Initialize();
	for (i = 0; NX_CLKGEN_GetNumberOfModule() > i; i++)
		NX_CLKGEN_SetBaseAddress(i, (U32)IO_ADDRESS(NX_CLKGEN_GetPhysicalAddress(i)));

	NX_GPIO_Initialize();
	for (i = 0; NX_GPIO_GetNumberOfModule() > i; i++) {
		NX_GPIO_SetBaseAddress(i, (U32)IO_ADDRESS(NX_GPIO_GetPhysicalAddress(i)));
		NX_GPIO_OpenModule(i);
	}

	NX_ALIVE_Initialize();
	NX_ALIVE_SetBaseAddress((U32)IO_ADDRESS(NX_ALIVE_GetPhysicalAddress()));
	NX_ALIVE_OpenModule();

	NX_CLKPWR_Initialize();
	NX_CLKPWR_SetBaseAddress((U32)IO_ADDRESS(NX_CLKPWR_GetPhysicalAddress()));
	NX_CLKPWR_OpenModule();

    /*
     * NOTE> ALIVE Power Gate must enable for RTC register access.
     * 		 must be clear wfi jump address
 	 */
	NX_ALIVE_SetWriteEnable(CTRUE);
	__raw_writel(0xFFFFFFFF, SCR_ARM_SECOND_BOOT);

	/*
	 * NOTE> Control for ACP register access.
	 */
	tie_reg = (U32)IO_ADDRESS(NX_TIEOFF_GetPhysicalAddress());

	val = __raw_readl(tie_reg + 0x70) & ~((3 << 30) | (3 << 10));
	writel(val,   (tie_reg + 0X70));

	val = __raw_readl(tie_reg + 0x80) & ~(3 << 3);
	writel(val,   (tie_reg + 0x80));
}

static void cpu_bus_init(void)
{
	/* MCUS for Static Memory. */
	NX_MCUS_Initialize();
	NX_MCUS_SetBaseAddress((U32)IO_ADDRESS(NX_MCUS_GetPhysicalAddress()));
	NX_MCUS_OpenModule();

	/*
	 * NAND Bus config
	 */
#if 0
	NX_MCUS_SetNANDBUSConfig
	(
		0, /* NF */
		CFG_SYS_NAND_TACS,		// tACS  ( 0 ~ 3 )
		CFG_SYS_NAND_TCAH,		// tCAH  ( 0 ~ 3 )
		CFG_SYS_NAND_TCOS,		// tCOS  ( 0 ~ 3 )
		CFG_SYS_NAND_TCOH,		// tCOH  ( 0 ~ 3 )
		CFG_SYS_NAND_TACC		// tACC  ( 1 ~ 16)
	);
#endif

	/*
	 * MCU-Static config: Static Bus #0 ~ #1
	 */
	#define STATIC_BUS_CONFIGUTATION( _n_ )								\
	NX_MCUS_SetStaticBUSConfig											\
	( 																	\
		NX_MCUS_SBUSID_STATIC ## _n_, 									\
		CFG_SYS_STATIC ## _n_ ## _BW, 									\
		CFG_SYS_STATIC ## _n_ ## _TACS, 								\
		CFG_SYS_STATIC ## _n_ ## _TCAH, 								\
		CFG_SYS_STATIC ## _n_ ## _TCOS, 								\
		CFG_SYS_STATIC ## _n_ ## _TCOH, 								\
		CFG_SYS_STATIC ## _n_ ## _TACC, 								\
		CFG_SYS_STATIC ## _n_ ## _TSACC,								\
		(NX_MCUS_WAITMODE ) CFG_SYS_STATIC ## _n_ ## _WAITMODE, 		\
		(NX_MCUS_BURSTMODE) CFG_SYS_STATIC ## _n_ ## _RBURST, 			\
		(NX_MCUS_BURSTMODE) CFG_SYS_STATIC ## _n_ ## _WBURST			\
	);

	STATIC_BUS_CONFIGUTATION( 0);
	STATIC_BUS_CONFIGUTATION( 1);
}

/*------------------------------------------------------------------------------
 *	CPU initialize
 */
void nxp_cpu_periph_init(void)
{
	#if		(CFG_UART_DEBUG_CH == 0)
	int id = RESET_ID_UART0;
	char *dev = "nxp-uart.0";
	#elif	(CFG_UART_DEBUG_CH == 1)
	int id = RESET_ID_UART1;
	char *dev = "nxp-uart.1";
	#elif	(CFG_UART_DEBUG_CH == 2)
	int id = RESET_ID_UART2;
	char *dev = "nxp-uart.2";
	#elif	(CFG_UART_DEBUG_CH == 3)
	int id = RESET_ID_UART3;
	char *dev = "nxp-uart.3";
	#elif	(CFG_UART_DEBUG_CH == 4)
	int id = RESET_ID_UART4;
	char *dev = "nxp-uart.4";
	#elif	(CFG_UART_DEBUG_CH == 5)
	int id = RESET_ID_UART5;
	char *dev = "nxp-uart.5";
	#endif

	struct clk *clk = clk_get(NULL, (const char*)dev);

	/* reset control: Low active ___|---   */
	NX_RSTCON_SetnRST(id, RSTCON_nDISABLE);
	NX_RSTCON_SetnRST(id, RSTCON_nENABLE);

	/* set clock   */
	clk_disable(clk);
	clk_set_rate(clk, CFG_UART_CLKGEN_CLOCK_HZ);
	clk_enable(clk);
}

void nxp_cpu_arch_init(void)
{
	cpu_base_init();
	cpu_bus_init();
}

unsigned int nxp_cpu_version(void)
{
	unsigned int revision = readl(0x0100);
	unsigned int version = 0;
	switch(revision) {
	case 0xe153000a: version = 1; break;
	default:		 version = 0; break;
	}
	return version;
}

void nxp_print_cpu_info(void)
{
	nxp_cpu_clock_print();
}

