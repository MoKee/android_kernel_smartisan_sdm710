/* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/of_address.h>
#ifdef CONFIG_VENDOR_SMARTISAN
#include <soc/qcom/boot_stats.h>
#endif

struct boot_stats {
	uint32_t bootloader_start;
	uint32_t bootloader_end;
	uint32_t bootloader_display;
	uint32_t bootloader_load_kernel;
};

static void __iomem *mpm_counter_base;
static uint32_t mpm_counter_freq;
static struct boot_stats __iomem *boot_stats;

#ifdef CONFIG_VENDOR_SMARTISAN
struct boot_shared_imem_cookie_type __iomem *boot_imem = NULL;
extern char *log_first_idx_get(void);
extern char *log_next_idx_get(void);
#endif

static int mpm_parse_dt(void)
{
	struct device_node *np;
	u32 freq;

	np = of_find_compatible_node(NULL, NULL, "qcom,msm-imem-boot_stats");
	if (!np) {
		pr_err("can't find qcom,msm-imem node\n");
		return -ENODEV;
	}
	boot_stats = of_iomap(np, 0);
	if (!boot_stats) {
		pr_err("boot_stats: Can't map imem\n");
		return -ENODEV;
	}

	np = of_find_compatible_node(NULL, NULL, "qcom,mpm2-sleep-counter");
	if (!np) {
		pr_err("mpm_counter: can't find DT node\n");
		return -ENODEV;
	}

	if (!of_property_read_u32(np, "clock-frequency", &freq))
		mpm_counter_freq = freq;
	else
		return -ENODEV;

	if (of_get_address(np, 0, NULL, NULL)) {
		mpm_counter_base = of_iomap(np, 0);
		if (!mpm_counter_base) {
			pr_err("mpm_counter: cant map counter base\n");
			return -ENODEV;
		}
	}

	return 0;
}

#ifdef CONFIG_VENDOR_SMARTISAN
uint64_t get_boot_reason(void)
{
	if (boot_imem == NULL)
		boot_stats_init();

	return (uint64_t) boot_imem->pon_reason;
}

uint64_t get_poff_fault_reason(void)
{
	if (boot_imem == NULL)
		boot_stats_init();

	return (uint64_t) boot_imem->fault_reason;
}

uint32_t get_secure_boot_value(void)
{
	if (boot_imem == NULL)
		boot_stats_init();

	return (uint32_t) boot_imem->is_enable_secure_boot;
}

uint32_t get_lpddr_vendor_id(void)
{
	if (boot_imem == NULL)
		boot_stats_init();

	return (uint32_t) boot_imem->lpddr_vendor_id;
}
#endif

static void print_boot_stats(void)
{
	pr_info("KPI: Bootloader start count = %u\n",
		readl_relaxed(&boot_stats->bootloader_start));
	pr_info("KPI: Bootloader end count = %u\n",
		readl_relaxed(&boot_stats->bootloader_end));
	pr_info("KPI: Bootloader display count = %u\n",
		readl_relaxed(&boot_stats->bootloader_display));
	pr_info("KPI: Bootloader load kernel count = %u\n",
		readl_relaxed(&boot_stats->bootloader_load_kernel));
	pr_info("KPI: Kernel MPM timestamp = %u\n",
		readl_relaxed(mpm_counter_base));
	pr_info("KPI: Kernel MPM Clock frequency = %u\n",
		mpm_counter_freq);
}

int boot_stats_init(void)
{
	int ret;
#ifdef CONFIG_VENDOR_SMARTISAN
	struct device_node *np;
#endif

	ret = mpm_parse_dt();
	if (ret < 0)
		return -ENODEV;

#ifdef CONFIG_VENDOR_SMARTISAN
	np = of_find_compatible_node(NULL, NULL, "qcom,msm-imem");
	if (!np) {
		pr_err("can't find qcom,msm-imem node\n");
		return -ENODEV;
	}

	boot_imem = of_iomap(np, 0);

	boot_imem->kernel_log_buf_addr = virt_to_phys(log_buf_addr_get());
	boot_imem->log_first_idx_addr = virt_to_phys(log_first_idx_get());
	boot_imem->log_next_idx_addr = virt_to_phys(log_next_idx_get());
#endif

	print_boot_stats();

	iounmap(boot_stats);
	iounmap(mpm_counter_base);

	return 0;
}

