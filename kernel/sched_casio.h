#ifndef __SCHED_CASIO_H_
#define __SCHED_CASIO_H_

#include <linux/sched.h>

#define ns_to_ktime(t) ktime_add_ns (ktime_set (0,0), t)

static inline unsigned long long casio_clock (void)
{
	return ktime_to_ns (ktime_get());
}

#endif
