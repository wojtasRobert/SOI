/* 
 *
 * PROCEDURA SCHED ORAZ FUNKCJE WOLANE Z SYSTEM.C PODCZAS WYWOLAN KERNELA
 *
 */

PRIVATE void sched()
{
/* The current process has run too long.  If another low priority (user)
 * process is runnable, put the current process on the end of the user queue,
 * possibly promoting another user to head of the queue.
 */
  if (rdy_head[USER_Q] == NIL_PROC) return;

  if (rdy_head[USER_Q]->p_first == 1)
  {   
	  /* One or more user processes queued. */
	rdy_head[USER_Q]->p_first = 0;
	rdy_tail[USER_Q]->p_nextready = rdy_head[USER_Q];
	rdy_tail[USER_Q] = rdy_head[USER_Q];
	rdy_head[USER_Q] = rdy_head[USER_Q]->p_nextready;
	rdy_tail[USER_Q]->p_nextready = NIL_PROC;
	pick_proc();
  } 
  else if (--rdy_head[USER_Q]->p_ticks == 0 || rdy_head[USER_Q]->p_ticks == 0)
  {
	if (rdy_head[USER_Q]->p_group == GROUP_A) rdy_head[USER_Q]->p_ticks = PROC_TIME[0];
	else rdy_head[USER_Q]->p_ticks = PROC_TIME[1];

	rdy_tail[USER_Q]->p_nextready = rdy_head[USER_Q];
	rdy_tail[USER_Q] = rdy_head[USER_Q];
	rdy_head[USER_Q] = rdy_head[USER_Q]->p_nextready;
	rdy_tail[USER_Q]->p_nextready = NIL_PROC;
  	pick_proc();
  }  
}

/*********************************************************************************/

PUBLIC int ret_gr(rp)
struct proc *rp;
{
	return rp->p_group;
}

PUBLIC int set_scheduler(rp, x)
struct proc *rp;
{
	if( rp->p_group == GROUP_A)
	{
	 	PROC_TIME[0] = x;
		PROC_TIME[1] = 100 - x;
		return 0;
	}
	else
	{
	 return -1;
	}
}

PUBLIC int change_group(rp)
struct proc *rp;
{
	if(rp->p_group == GROUP_A)
	{
		rp->p_group = GROUP_B;
		return 0;
	}
	else
	{
		rp->p_group = GROUP_A;
		return 0;
	}
}

PUBLIC int ret_utime(rp)
struct proc *rp;
{
	return rp->user_time;
}

PUBLIC int ret_prop(rp)
struct proc *rp;
{
	return PROC_TIME[0];
}
