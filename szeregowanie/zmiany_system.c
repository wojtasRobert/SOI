/*
 *
 * 
 *	W FORKU INICJUJEMY WARTOSCI STWORZONYCH POL Z STRUKTURZE PROC.H
 *	PONIZEJ ZNAJDUJA SIE FUNKCJE OBSLUGUJACE WYWOLANIA KERNELA 
 *
 */

PRIVATE int do_fork(m_ptr)
register message *m_ptr;	/* pointer to request message */
{
/* Handle sys_fork().  m_ptr->PROC1 has forked.  The child is m_ptr->PROC2. */

#if (CHIP == INTEL)
  reg_t old_ldt_sel;
#endif
  register struct proc *rpc;
  struct proc *rpp;

  rpp = proc_addr(m_ptr->PROC1);
  assert(isuserp(rpp));
  rpc = proc_addr(m_ptr->PROC2);
  assert(isemptyp(rpc));

  /* Copy parent 'proc' struct to child. */
#if (CHIP == INTEL)
  old_ldt_sel = rpc->p_ldt_sel;	/* stop this being obliterated by copy */
#endif

  *rpc = *rpp;			/* copy 'proc' struct */

#if (CHIP == INTEL)
  rpc->p_ldt_sel = old_ldt_sel;
#endif
  rpc->p_nr = m_ptr->PROC2;	/* this was obliterated by copy */

  rpc->p_flags |= NO_MAP;	/* inhibit the process from running */

  rpc->p_flags &= ~(PENDING | SIG_PENDING | P_STOP);

  /* Only 1 in group should have PENDING, child does not inherit trace status*/
  sigemptyset(&rpc->p_pending);
  rpc->p_pendcount = 0;
  rpc->p_pid = m_ptr->PID;	/* install child's pid */
  rpc->p_reg.retreg = 0;	/* child sees pid = 0 to know it is child */

  rpc->user_time = 0;		/* set all the accounting times to 0 */
  rpc->sys_time = 0;
  rpc->child_utime = 0;
  rpc->child_stime = 0;

/*===========================================================================*
 *				dodane					     *
 *===========================================================================*/

  if(m_ptr->PID % 2 == 0)	/* ustawienie grupy i dozwolonego przydzialu procesora */
  {
      rpc->p_group = GROUP_A;
      rpc->p_ticks = PROC_TIME[0];	/* przydzielenie wartosci kwantu czasu dostepnego dla grupy procesu */
  }
  else
  {  
      rpc->p_group = GROUP_B;
      rpc->p_ticks = PROC_TIME[1];
  }
  rpc->p_first = 1;		/* nie byl jeszcze w sched() */

  return(OK);
}

PRIVATE int do_getgroup(m_ptr)			/* zwracanie grupy */
message *m_ptr;
{
      struct proc * rp;
      rp = proc_addr(m_ptr->m1_i1);
      return ret_gr(rp);
}

PRIVATE int do_setsched(m_ptr)			/* ustawienie proporcji */
message *m_ptr;
{
      struct proc * rp;
      int x;
      rp = proc_addr(m_ptr->m1_i1);
      x = m_ptr->m2_l2;
      return set_scheduler(rp,x);
}

PRIVATE int do_chgroup(m_ptr)			/* zmiana grupy */
message *m_ptr;
{
	struct proc *rp;
	rp = proc_addr(m_ptr->m1_i1);
	return change_group(rp);
}

PRIVATE int do_getutime(m_ptr)			/* zwracanie czasu */
message *m_ptr;
{
	struct proc *rp;
	rp = proc_addr(m_ptr->m1_i1);
	return ret_utime(rp);
}	

PRIVATE int do_retprop(m_ptr)			/* zwracanie proporcji */
message *m_ptr;
{
	struct proc *rp;
	rp = proc_addr(m_ptr->m1_i1);
	return ret_prop(rp);
}
