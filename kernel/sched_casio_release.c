/* RELEASE */

void init_casio_task_release(struct casio_task_release *r)
{
	r->tasks = RB_ROOT;
	r->erf = NULL;
}
