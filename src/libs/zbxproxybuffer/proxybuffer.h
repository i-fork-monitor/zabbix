/*
** Zabbix
** Copyright (C) 2001-2023 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#ifndef ZABBIX_PROXYBUFFER_H
#define ZABBIX_PROXYBUFFER_H

#include "zbxproxybuffer.h"
#include "proxybuffer.h"
#include "zbxmutexs.h"
#include "zbxtime.h"
#include "zbxdbschema.h"

#define ZBX_MAX_HRECORDS	1000
#define ZBX_MAX_HRECORDS_TOTAL	10000

/* the space reserved in json buffer to hold at least one record plus service data */
#define ZBX_DATA_JSON_RESERVED		(ZBX_HISTORY_TEXT_VALUE_LEN * 4 + ZBX_KIBIBYTE * 4)

#define ZBX_DATA_JSON_RECORD_LIMIT	(ZBX_MAX_RECV_DATA_SIZE - ZBX_DATA_JSON_RESERVED)
#define ZBX_DATA_JSON_BATCH_LIMIT	((ZBX_MAX_RECV_DATA_SIZE - ZBX_DATA_JSON_RESERVED) / 2)

typedef enum
{
	PB_DATABASE,
	PB_DATABASE_MEMORY,
	PB_MEMORY,
	PB_MEMORY_DATABASE,
}
zbx_pb_state_t;

extern zbx_pb_state_t	pb_dst[];
extern zbx_pb_state_t	pb_src[];

typedef struct
{
	zbx_uint64_t	id;
	zbx_uint64_t	druleid;
	zbx_uint64_t	dcheckid;
	char		*dns;
	char		*ip;
	char		*value;
	int		port;
	int		clock;
	int		status;
}
zbx_pb_discovery_t;

ZBX_PTR_VECTOR_DECL(pb_discovery_ptr, zbx_pb_discovery_t *)

typedef struct
{
	zbx_uint64_t	id;
	zbx_uint64_t	itemid;
	zbx_uint64_t	lastlogsize;
	zbx_timespec_t	ts;		/* clock + ns */
	char		*value;
	char		*source;
	int		timestamp;
	int		severity;
	int		logeventid;
	int		state;
	int		mtime;
	int		flags;
	time_t		write_clock;
}
zbx_pb_history_t;

ZBX_PTR_VECTOR_DECL(pb_history_ptr, zbx_pb_history_t *)

typedef struct
{
	zbx_uint64_t	id;
	char		*host;
	char		*listen_ip;
	char		*listen_dns;
	char		*host_metadata;
	int		listen_port;
	int		tls_accepted;
	int		flags;
	int		clock;
}
zbx_pb_autoreg_t;

typedef struct
{
	zbx_list_t		history;
	zbx_list_t		discovery;
	zbx_list_t		autoreg;

	int			mode;
	zbx_pb_state_t		state;
	int			db_handles_num;		/* number of pending database inserts */
	int			max_age;
	int			offline_buffer;
	zbx_uint64_t		changes_num;
	zbx_mutex_t		mutex;

	/* ids of last records uploaded to server */
	zbx_uint64_t		history_lastid_sent;
	zbx_uint64_t		discovery_lastid_sent;
	zbx_uint64_t		autoreg_lastid_sent;

	/* ids of last records inserted into database */
	zbx_uint64_t		history_lastid_db;
	zbx_uint64_t		discovery_lastid_db;
	zbx_uint64_t		autoreg_lastid_db;

	zbx_uint64_t		history_lastid_mem;
}
zbx_pb_t;

extern zbx_pb_t	*pb_data;

zbx_uint64_t	pb_get_lastid(const char *table_name, const char *lastidfield);

typedef struct
{
	const char		*field;
	const char		*tag;
	zbx_json_type_t		jt;
	const char		*default_value;
}
zbx_history_field_t;

typedef struct
{
	const char		*table, *lastidfield;
	zbx_history_field_t	fields[ZBX_MAX_FIELDS];
}
zbx_history_table_t;

void	pb_lock(void);
void	pb_unlock(void);
void	*pb_malloc(size_t size);
void	pb_free(void *ptr);
char	*pb_strdup(const char *str);
size_t	pb_get_free_size(void);
int	pb_free_space(zbx_pb_t *pb, size_t size);

void	pb_set_state(zbx_pb_t *pb, zbx_pb_state_t state, const char *message);

void	pb_get_rows_db(struct zbx_json *j, const char *proto_tag, const zbx_history_table_t *ht,
		zbx_uint64_t *lastid, zbx_uint64_t *id, int *records_num, int *more);

void	pb_set_lastid(const char *table_name, const char *lastidfield, const zbx_uint64_t lastid);
void pd_fallback_to_database(zbx_pb_t *pb, const char *message);

#endif
