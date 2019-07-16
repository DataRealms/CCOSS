#ifndef AUCAT_H
#define AUCAT_H

#include "amsg.h"

struct aucat {
	int fd;				/* socket */
	struct amsg rmsg, wmsg;		/* temporary messages */
	size_t wtodo, rtodo;		/* bytes to complete the packet */
#define RSTATE_MSG	0		/* message being received */
#define RSTATE_DATA	1		/* data being received */
	unsigned rstate;		/* one of above */
#define WSTATE_IDLE	2		/* nothing to do */
#define WSTATE_MSG	3		/* message being transferred */
#define WSTATE_DATA	4		/* data being transferred */
	unsigned wstate;		/* one of above */
	unsigned maxwrite;		/* bytes we're allowed to write */
};

int _aucat_rmsg(struct aucat *, int *);
int _aucat_wmsg(struct aucat *, int *);
size_t _aucat_rdata(struct aucat *, void *, size_t, int *);
size_t _aucat_wdata(struct aucat *, const void *, size_t, unsigned, int *);
int _aucat_open(struct aucat *, const char *, unsigned);
void _aucat_close(struct aucat *, int);
int _aucat_pollfd(struct aucat *, struct pollfd *, int);
int _aucat_revents(struct aucat *, struct pollfd *);
int _aucat_setfl(struct aucat *, int, int *);

#endif /* !defined(AUCAT_H) */
