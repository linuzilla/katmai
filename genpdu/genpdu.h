/*
 *	genpdu.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __GENPDU_H__
#define __GENPDU_H__

struct genpdu_t;

struct generic_pdu_callback_data_t {
	void		*data;
	int		len;
	void		*self;
	struct udplib_t	*udp;
};

struct generic_pdu_t {
	unsigned int	cksum;
	int		cmd;
	int		len;
	unsigned int	flag;
};

struct generic_full_pdu_t {
	struct generic_pdu_t	h;
	char			start;
};

struct gp_cmd_handler_t {
	int	cmd;
	int	(*func)(struct generic_pdu_callback_data_t *ptr);
};

struct genpdu_private_data_t {
	int				num;
	int				real_num;
	struct gp_cmd_handler_t		*handler;
	unsigned int			(*flag_callback)(struct genpdu_t *);
};

struct genpdu_t {
	struct genpdu_private_data_t	pd;
	unsigned int	(*cksum)(const void *pdu, const int len);
	void		(*mkcksum)(void *pdu);
	int		(*add)(struct genpdu_t *self, const int cmd,
				int (*f)(struct generic_pdu_callback_data_t *));
	int		(*addlist)(struct genpdu_t *self,
					struct gp_cmd_handler_t *list);
	void		(*clear)(struct genpdu_t *self);
	int		(*do_cmd)(struct genpdu_t *self, void *pdu,
					const int len, void *parm, int *cmd);
	void		(*pack)(struct genpdu_t *self,
				void *pdu, const int cmd, const int len);
	// void		(*pack)(void *pdu, const int cmd, const int len);
	void		(*set_flag_callback)(struct genpdu_t *self,
				unsigned int (*cbk)(struct genpdu_t *));
};

struct genpdu_t *	new_genpdu (const int num);

#endif
