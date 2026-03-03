#include "dubhe2000_fwd.h"

void dubhe1000_soft_reset_fwd(struct dubhe1000_adapter *adapter)
{
	void __iomem *fwd_reset_regs = NULL;

	fwd_reset_regs = adapter->rst_para_regs + FWD_APB_SUB_RST_REG_OFFSET;

	writel(FWD_ASSERT_VAL, fwd_reset_regs);
	//Note: This Soft Reset will be completed in 1 us.
	udelay(FWD_HOLD_TIME);
	pr_info("####Soft Reset FWD: DONE!\n");
}
