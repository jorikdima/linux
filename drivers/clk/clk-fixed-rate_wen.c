/*
 * Copyright (C) 2010-2011 Canonical Ltd <jeremy.kerr@canonical.com>
 * Copyright (C) 2011-2012 Mike Turquette, Linaro Ltd <mturquette@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Fixed rate clock implementation
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/of.h>

/*
 * DOC: basic fixed-rate clock that have special procedure to be enabled
 *
 * Traits of this clock:
 * prepare - clk_(un)prepare only ensures parents are prepared
 * enable - clk_enable ensures parents are enabled and make procedure to enable clk
 * rate - rate is always a fixed value.  No clk_set_rate support
 * parent - fixed parent.  No clk_set_parent support
 * 
 */

struct platform_device;

static int clk_fixed_rate_wen_enable(struct clk_hw *hw)
{
  struct clk_fixed_rate_wen *fixed = to_clk_fixed_rate_wen(hw);
    
  if (fixed->enable)
    return fixed->enable(fixed->dev);
  return 0;
}

static void clk_fixed_rate_wen_disable(struct clk_hw *hw)
{
  struct clk_fixed_rate_wen *fixed = to_clk_fixed_rate_wen(hw);
  
  if (fixed->disable)
    fixed->disable();
}


static unsigned long clk_fixed_rate_wen_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	return to_clk_fixed_rate_wen(hw)->fixed_rate;
}

static unsigned long clk_fixed_rate_wen_recalc_accuracy(struct clk_hw *hw,
		unsigned long parent_accuracy)
{
	return to_clk_fixed_rate_wen(hw)->fixed_accuracy;
}

const struct clk_ops clk_fixed_rate_wen_ops = {
	.recalc_rate = clk_fixed_rate_wen_recalc_rate,
	.recalc_accuracy = clk_fixed_rate_wen_recalc_accuracy,
	.enable = clk_fixed_rate_wen_enable,
	.disable = clk_fixed_rate_wen_disable,
};
EXPORT_SYMBOL_GPL(clk_fixed_rate_wen_ops);

/**
 * clk_hw_register_fixed_rate_wen_with_accuracy - register fixed-rate clock with
 * the clock framework
 * @dev: device that is registering this clock
 * @name: name of this clock
 * @parent_name: name of clock's parent
 * @flags: framework-specific flags
 * @fixed_rate: non-adjustable clock rate
 * @fixed_accuracy: non-adjustable clock rate
 */
struct clk_hw *clk_hw_register_fixed_rate_wen_with_accuracy(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		unsigned long fixed_rate, unsigned long fixed_accuracy, 
		int (*en_ptr)(struct device *dev), int (*dis_ptr)(void))
{
	struct clk_fixed_rate_wen *fixed;
	struct clk_hw *hw;
	struct clk_init_data init;
	int ret;

	/* allocate fixed-rate clock */
	fixed = kzalloc(sizeof(*fixed), GFP_KERNEL);
	if (!fixed)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &clk_fixed_rate_wen_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = (parent_name ? &parent_name: NULL);
	init.num_parents = (parent_name ? 1 : 0);

	/* struct clk_fixed_rate assignments */
	fixed->dev = dev;
	fixed->enable = en_ptr;
	fixed->disable = dis_ptr;
	fixed->fixed_rate = fixed_rate;
	fixed->fixed_accuracy = fixed_accuracy;
	fixed->hw.init = &init;
	

	/* register the clock */
	hw = &fixed->hw;
	ret = clk_hw_register(dev, hw);
	if (ret) {
		kfree(fixed);
		hw = ERR_PTR(ret);
	}

	return hw;
}
EXPORT_SYMBOL_GPL(clk_hw_register_fixed_rate_wen_with_accuracy);

struct clk *clk_register_fixed_rate_wen_with_accuracy(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		unsigned long fixed_rate, unsigned long fixed_accuracy, 
		int (*en_ptr)(struct device *dev), int (*dis_ptr)(void))
{
	struct clk_hw *hw;

	hw = clk_hw_register_fixed_rate_wen_with_accuracy(dev, name, parent_name,
			flags, fixed_rate, fixed_accuracy, en_ptr, dis_ptr);
	if (IS_ERR(hw))
		return ERR_CAST(hw);
	return hw->clk;
}
EXPORT_SYMBOL_GPL(clk_register_fixed_rate_wen_with_accuracy);

/**
 * clk_hw_register_fixed_wen_rate - register fixed-rate clock with the clock
 * framework
 * @dev: device that is registering this clock
 * @name: name of this clock
 * @parent_name: name of clock's parent
 * @flags: framework-specific flags
 * @fixed_rate: non-adjustable clock rate
 */
struct clk_hw *clk_hw_register_fixed_rate_wen(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned long fixed_rate, int (*en_ptr)(struct device *dev), int (*dis_ptr)(void))
{
	return clk_hw_register_fixed_rate_wen_with_accuracy(dev, name, parent_name,
						     flags, fixed_rate, 0, en_ptr, dis_ptr);
}
EXPORT_SYMBOL_GPL(clk_hw_register_fixed_rate_wen);

struct clk *clk_register_fixed_rate_wen(struct device *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned long fixed_rate, int (*en_ptr)(struct device *dev), int (*dis_ptr)(void))
{
	return clk_register_fixed_rate_wen_with_accuracy(dev, name, parent_name,
						     flags, fixed_rate, 0, en_ptr, dis_ptr);
}
EXPORT_SYMBOL_GPL(clk_register_fixed_rate_wen);

void clk_unregister_fixed_rate_wen(struct clk *clk)
{
	struct clk_hw *hw;

	hw = __clk_get_hw(clk);
	if (!hw)
		return;

	clk_unregister(clk);
	kfree(to_clk_fixed_rate_wen(hw));
}
EXPORT_SYMBOL_GPL(clk_unregister_fixed_rate_wen);

void clk_hw_unregister_fixed_rate_wen(struct clk_hw *hw)
{
	struct clk_fixed_rate_wen *fixed;

	fixed = to_clk_fixed_rate_wen(hw);

	clk_hw_unregister(hw);
	kfree(fixed);
}
EXPORT_SYMBOL_GPL(clk_hw_unregister_fixed_rate_wen);

#ifdef CONFIG_OF
/**
 * of_fixed_clk_setup() - Setup function for simple fixed rate clock
 */
void of_fixed_clk_setup_wen(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	u32 rate;
	u32 accuracy = 0;

	if (of_property_read_u32(node, "clock-frequency", &rate))
		return;

	of_property_read_u32(node, "clock-accuracy", &accuracy);

	of_property_read_string(node, "clock-output-names", &clk_name);

	clk = clk_register_fixed_rate_wen_with_accuracy(NULL, clk_name, NULL,
						    0, rate, accuracy, NULL, NULL);
	if (!IS_ERR(clk))
		of_clk_add_provider(node, of_clk_src_simple_get, clk);
}
EXPORT_SYMBOL_GPL(of_fixed_clk_setup_wen);
CLK_OF_DECLARE(fixed_clk_wen, "fixed-clock_wen", of_fixed_clk_setup_wen);
#endif
