void init_casio_task_ready(struct casio_task_ready *r)
{
	r->tasks = RB_ROOT;
	r->edf = NULL;
}
void init_casio_rq(struct casio_rq *casio_rq)
{
	printk(KERN_INFO "SCHED_CASIO: init_casio_rq\n");
	init_casio_task_ready(&casio_rq->ready);
	init_casio_task_release(&casio_rq->release);
}

const struct sched_class casio_sched_class = {
	.next		= &rt_sched_class,
	.enqueue_task	= enqueue_task_casio,
	.dequeue_task	= dequeue_task_casio,
	.check_preempt_curr = check_preempt_curr_casio,
	pick_next_task	= pick_next_task_casio,
};

static void enqueue_task_casio(struct rq *rq, struct task_struct *p, int wakeup)
{
	unsigned long long now = casio_clock();
	p->casio_task.job.deadline = now + p-> casio_task.deadline;
	_enqueue_task_casio(rq, p);
}

/*
enqueue a task into the ready queue (a binary tree)
edf points to the task with earliest deadline
*/
void _enqueue_task_casio(struct rq *rq, struct task_struct *p)
{
	struct rb_node **node;
	struct rb_node *parent = NULL;
	struct casio_task *entry;
	int edf = 1;
	node = &rq->casio_rq.ready.tasks.rb_node;
	while(*node!=NULL){
		parent = *node;
		entry = rb_entry(parent, struct casio_task, ready_node);
		if(p->casio_task.job.deadline < entry-<job.deadline)
		{
			node = &parent->rb_left;
		}
		else
		{
			node = &parent->rb_right;
			edf=0;
		}
	}
	if(edf)
	{
		rq->casio_rq.ready.edf = p;
	}

	rb_link_node(&p->casio_task.ready_node, parent, node);
	rb_insert_color(&p->casio_task.ready_node, &rq->casio_rq.ready.tasks);
}

static void dequeue_task_casio(struct rq *rq, struct task_struct *p, int sleep)
{
	_dequeue_task_casio(rq, p);
}

/*dequeue a task from the ready queue (a binary tree)
edf points to the task with earliest deadline; if there is no task it is NULL
*/
void _dequeue_task_casio(struct rq *rq, struct task_struct *p)
{
	struct rb_node *node;
	struct casio_task *entry;
	if(rq->casio_rq.ready.edf == p)
	{
		rq->casio_rq.ready.edf = NULL;
		node = rb_next(&p->casio_task.ready_node);
		if(node)
		{
			entry = rb_entry(node, struct casio_task, ready_node);
			rq->casio_rq.ready.edf = (struct task_struct*) containter_of(entry, struct task_struct, casio_task;
		}
	}
	rb_erase(&p->casio_task.ready_node, &rq->casio_rq.ready.tasks);
	p->casio_task.ready_node.rb_left = p->casio_task.ready_node.rb_right = NULL;
}

static struct task_struct * pick_next_task_casio(struct rq *rq)
{
	return rq->casio_rq.ready.edf;
}
