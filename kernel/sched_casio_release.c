/* RELEASE */

void init_casio_task_release(struct casio_task_release *r)
{
	r->tasks = RB_ROOT;
	r->erf = NULL;
}

void casio_setup_release_timer(void)
{
	struct rq * rq = cpu_rq(smp_processor_id());
	hrtimer_init(&rq->casio_rq.release.timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
	rq->casio_rq.release.timer.function = &wake_up_job;
	hrtimer_start(&rq->casio_rq.release.timer, ktime_set(KTIME_SEC_MAX, 0), HRTIMER_MODE_ABS_PINNED);
}
