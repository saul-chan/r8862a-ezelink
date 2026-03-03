#ifndef __SYS_HAL_H__
#define __SYS_HAL_H__
#include <stdint.h>
extern int g_debug;
#define BIT(n)  (1 << (n))
#define LOG(fmt, ...)   if(g_debug) printf(fmt, ##__VA_ARGS__)
#define typecheck(type,x) \
({	type __dummy; \
	typeof(x) __dummy2; \
	(void)(&__dummy == &__dummy2); \
	1; \
})

#define time_after(a,b)		\
	(typecheck(uint64_t, a) && \
	 typecheck(uint64_t, b) && \
	 ((long)((b) - (a)) < 0))

#define reg_poll_timeout(addr, field, val, cond, timeout_us)	\
({ \
	uint64_t timeout = timer_get_us() + timeout_us; \
	for (;;) { \
		(val) = REG_R(addr, field); \
		if (cond) \
			break; \
		if (timeout_us && time_after(timer_get_us(), timeout)) { \
			(val) = REG_R(addr, field); \
			break; \
		} \
	} \
	(cond) ? 0 : -1; \
})

#define readl_poll_timeout(addr, val, cond, timeout_us)	\
({ \
	uint64_t timeout = timer_get_us() + timeout_us; \
	for (;;) { \
		(val) = readl(addr); \
		if (cond) \
			break; \
		if (timeout_us && time_after(timer_get_us(), timeout)) { \
			(val) = readl(addr); \
			break; \
		} \
	} \
	(cond) ? 0 : -1; \
})

#define pcie_reg_read_poll_timeout(addr, val, cond, timeout_us)	\
({ \
	uint64_t timeout = timer_get_us() + timeout_us; \
	for (;;) { \
		(val) = PCIE_serdes_reg_read(addr); \
		if (cond) \
			break; \
		if (timeout_us && time_after(timer_get_us(), timeout)) { \
			(val) = PCIE_serdes_reg_read(addr); \
			break; \
		} \
	} \
	(cond) ? 0 : -1; \
})

#define eth_reg_read_poll_timeout(index, addr, val, cond, timeout_us)	\
({ \
	uint64_t timeout = timer_get_us() + timeout_us; \
	for (;;) { \
		(val) = ETH_serdes_reg_read(index, addr); \
		if (cond) \
			break; \
		if (timeout_us && time_after(timer_get_us(), timeout)) { \
			(val) = ETH_serdes_reg_read(index, addr); \
			break; \
		} \
	} \
	(cond) ? 0 : -1; \
})
//#define hal_soc_write(addr, value)             printf("WR ADDR[%#x] value[%#x]\n", addr, value)
//#define hal_soc_read(addr)                     ({ 		\
											     printf("RD ADDR[%#x]\n", addr); \
	
#define hal_soc_write(addr, value)             ({\
												if(g_debug) \
												printf("WR ADDR[%#x] value[%#x]\n", addr, value); \
												hal_soc_write(addr, value);})

#define hal_soc_read(addr)                 	  ({ uint32_t __val = 0; \
												if (g_debug) \
											    printf("RD ADDR[%#x] ", addr); \
												__val = hal_soc_read(addr); \
												if (g_debug) \
											    printf("val[%#x]\n", addr); \
												__val;})

#define BITS_PER_LONG 32
#define GENMASK(h, l) \
	(((~((uint32_t)0)) - (((uint32_t)1) << (l)) + 1) & \
	 (~((uint32_t)0) >> (BITS_PER_LONG - 1 - (h))))
#define REG_W(addr, field, value) 		({ uint32_t __val =  hal_soc_read(addr); \
     									   __val &= ~GENMASK(field##_END, field##_BEGIN); \
     									   __val |= (value << (field##_BEGIN)) & GENMASK(field##_END, field##_BEGIN); \
	 									   hal_soc_write(addr, __val); \
										   if (g_debug) \
									   		printf("WR [%s][%#x] [%s][%d_%d] __val[%#x] val[%#x]\n", \
									   		#addr, addr, #field,field##_BEGIN,field##_END,__val, value); \
										 })

#define REG_R(addr, field)         		({ uint32_t __val = hal_soc_read(addr); \
										   if (g_debug) \
							printf("RD [%s][%#x] [%s][%d_%d] val[%#x]\n", \
									   #addr, addr, #field,field##_BEGIN,field##_END, __val); \
						 __val &= GENMASK(field##_END, field##_BEGIN); \
     									  (__val >> (field##_BEGIN)); \
										 })      
#define readl(addr)  	hal_soc_read( addr)
#define writel(value, addr)    hal_soc_write( addr, value)
#define dubhe1000_eth_print(args...)	printf(args)
#define dubhe1000_eth_r32(base_addr, reg)		(readl(base_addr + reg))
#define dubhe1000_eth_w32(base_addr, reg, value)	(writel((value), base_addr + reg))


void sleep_nano(uint32_t nsec); 
uint64_t  timer_get_us(void);
#endif /*__SYS_HAL_H__*/


