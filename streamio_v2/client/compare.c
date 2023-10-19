/*
 *	compare.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __CLIENT_COMPARE__C__
#define __CLIENT_COMPARE__C__

#define CMP_GT		1
#define CMP_GE		2
#define CMP_LT		3
#define CMP_LE		4
#define CMP_EQ		5
#define CMP_NE		6

static void set_compare (struct stio_v2_client_local_t *cl,
			volatile int *variable, const int op, const int exp) {
	cl->compare_variable = variable;
	cl->compare_operator = op;
	cl->compare_expect   = exp;
}

static int do_compare (struct stio_v2_client_local_t *cl) {
	switch (cl->compare_operator) {
	case CMP_GT:
		if (*cl->compare_variable > cl->compare_expect) return 1;
		break;
	case CMP_GE:
		if (*cl->compare_variable >= cl->compare_expect) return 1;
		break;
	case CMP_LT:
		if (*cl->compare_variable < cl->compare_expect) return 1;
		break;
	case CMP_LE:
		if (*cl->compare_variable <= cl->compare_expect) return 1;
		break;
	case CMP_EQ:
		if (*cl->compare_variable == cl->compare_expect) return 1;
		break;
	case CMP_NE:
		if (*cl->compare_variable != cl->compare_expect) return 1;
		break;
	}
	return 0;
}

#endif
