/*
 *	db_mssql.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __DB_MSSQL_H__
#define __DB_MSSQL_H__

struct db_mssql_pd_t;
struct x_object_t;

struct db_mssql_t {
	struct db_mssql_pd_t	*pd;

	int	(*verbose)(struct db_mssql_t *self, const int level);
	int	(*query)(struct db_mssql_t *self, const char *query, ...);
	char **	(*fetch)(struct db_mssql_t *self);
	int	(*findex)(struct db_mssql_t *self, const char *str);
	int	(*store_result)(struct db_mssql_t *self);
	struct x_object_t *	(*fetch_array)(struct db_mssql_t *self);
	unsigned int	(*num_rows)(struct db_mssql_t *self);
	unsigned int	(*num_fields)(struct db_mssql_t *self);
	int	(*free_result)(struct db_mssql_t *self);
	int	(*perror)(struct db_mssql_t *self, const char *str);
	int	(*connect)(struct db_mssql_t *self,
				char *server, const char *account,
				const char *passwd, char *database);
	int	(*select_db)(struct db_mssql_t *self, char *str);
	void	(*disconnect)(struct db_mssql_t *self);
	void	(*dispose)(struct db_mssql_t *);
};

struct db_mssql_t * new_dbmssql (void);

#endif
