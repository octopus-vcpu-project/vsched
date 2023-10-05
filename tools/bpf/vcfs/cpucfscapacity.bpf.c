// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2023 Jianchen Shan 
#include <vmlinux.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
//#include "compat.bpf.h"

struct event {
	int cpu;
	int capacity;
};

SEC("raw_tracepoint/sched_cpu_capacity_tp")
int BPF_PROG(cpu_capacity, struct rq *rq)
{
	bpf_printk("capacity is %d\n", BPF_CORE_READ(rq, cpu_capacity));
	struct event *eventp;

	/*
	 * send data from kernel to user space
	 *
	 */
	/*
	eventp = reserve_buf(sizeof(*eventp));
	if (!eventp)
			return 0;

 		eventp->capacity = BPF_CORE_READ(rq, cpu_capacity);
		eventp->cpu = bpf_get_smp_processor_id();
	eventp->capacity = 10;
	eventp->cpu = 20;
	if (argp->src)
		bpf_probe_read_user_str(eventp->src, sizeof(eventp->src), argp->src);
	else
		eventp->src[0] = '\0';

	submit_buf(ctx, eventp, sizeof(*eventp));
	*/

	return 0;
}

char LICENSE[] SEC("license") = "GPL";
