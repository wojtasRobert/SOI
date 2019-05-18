/* DODANE WYWOLANIA SYSTEMOWE */
PUBLIC int get_group()
{
	message m;
	m.m1_i1 = mm_in.m_source;
	return _taskcall(SYSTASK, SYS_GETGROUP, &m);
}

PUBLIC int set_scheduler()
{
	message m;
	m.m1_i1 = mm_in.m_source;
	m.m2_l2 = mm_in.m1_i1;
	return _taskcall(SYSTASK, SYS_SETSCHED, &m);
}

PUBLIC int change_group()
{
	message m;
	m.m1_i1 = mm_in.m_source;
	return _taskcall(SYSTASK, SYS_CHGROUP, &m);
}
PUBLIC int get_utime()
{
	message m;
	m.m1_i1 = mm_in.m_source;
	return _taskcall(SYSTASK, SYS_GETUTIME, &m);
}

PUBLIC int ret_prop()
{
	message m;
	m.m1_i1 = mm_in.m_source;
	return _taskcall(SYSTASK, SYS_RETPROP, &m);
}

