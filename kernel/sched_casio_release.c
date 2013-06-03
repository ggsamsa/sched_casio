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

/*
enqueue a task into the release queue order by the release time
erf points to the task with earliest release
*/

void _enqueue_task_release(struct rq *rq, struct task_struct *p)
{
	struct rb_node **node;
	struct rb_node *parent = NULL;
	struct casio_task *entry;
	int erf = 1;
	node = &rq->casio_rq.release.task.rb_node;
	while(*node != NULL)
	{
		parent = *node;
		entry = rb_entry/parent, struct casio_task, release_node);
		if(p->casio_task.job.release < entry->job.release)
		{
			node = &parent->rb_left;
		}
		else
		{
			node = &parent->rb_right;
			erf = 0;
		}
	}
	if(erf)
	{
		rq->casio_rq.release.erf = p;
	}
	rb_link_node(&p->casio_task.release_node, parent, node);
	rb_insert_color(&p->casio_task.release_node, &rq->casio_rq.release.tasks);
}

/*
dequeue a task from the release queue order by the release time
erf points to the task with earliest release or NULL (if there is no task into the release queue)
*/
void _dequeue_task_release(struct rq *rq)
{
	struct rb_node *node;
	struct casio_task *entry;
	struct task_struct *p = rq->casio_rq.release.erf;
	rq->casio_rq.release.erf = NULL;
	node = rb_next(&p->casio_task.release_node);
	if(node)
	{
		entry = rb_entry(node, struct casio_task, release_node);
		rq->casio_rq.release.erf=(struct tas_struct*) container_of(entry, struct task_struct, casio_task);
	}
	rb_erase(&p->casio_task.release_node, &rq->casio_rq.release.tasks);
	p->casio_task.release_node.rb_left = p->casio_task.release_node.rb_right = NULL;
}

/*
This system call puts a task into the waiting state until the releas time.
FOr that it set ups a timer to expire at task release.
*/

asmlinkage long sys_casio_delay_until(unsigned long h, unsigned long l)
{
	struct rq * rq = NULL;
	struct task_struct *p = NULL;
	unsigned long long release;
	unsigned long flags;
	//get the curret processor runqueue
	rq = cpu_rq(smp_processor_id());
	if(rq)
	{
	//the next two instructions assure that this code is not preempted neither by an interrupt nor by a highest priority task
		//disable all local (cpu) interrupts
		local_irq_save(flags);
		//disable preemptions
		preempt_disable();
		//convert two 32-bit variables into one 64-bits variables
		release = 0; release = h; release <<= 32; release |= 1;
		//update the job release
		rq->curr->casio_task.job.release = release;
		if(release > casio_clock())
		{
			//rq-<curr point to the currently executing task
			//change the currently executing task state to waiting
			rq->curr->state = TASK_INTERRUPTIBLE;
			//enqueue task into the release queue
			_enqueue_task_release(rq, rq->curr);
			//update timer expiration
			p = rq->casio_rq.release.erf;
			if(p->casio_task.job.release < ktime_to_ns(rq->casio_rq.release.timer._expires) + EPSILON)
			{
				hrtimer_start_range_ns(&rq->casio_rq.release.timer, ns_to_ktime(p->casio_task.job.release), 0, HRTIMER_MODE_ABS_PINNED);
			}
		}
		else //miss deadline
		{
			//this is only for logging information
			dequeue_task(rq, rq->curr, 0)
			atomic_inc(&rq->curr->casio_task.job.nr);
			enqueue_task(rq, rq->curr, 0, false);
		}
		//set the currently executing task to be preempted
		set tsk_need_resched(rq->curr);
		//enable interrupts and preemptions
		local_irq_restore(flags);
		preempt_enable_no_resched();
		//invoke the scheduler core to remove the curretly executing task from the processor and also from the ready queue
		schedule();
		return 0;
	}
	return -1;
}
