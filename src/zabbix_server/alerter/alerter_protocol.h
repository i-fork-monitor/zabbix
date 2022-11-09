/*
** Zabbix
** Copyright (C) 2001-2022 Zabbix SIA
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

#ifndef ZABBIX_ALERTER_PROTOCOL_H
#define ZABBIX_ALERTER_PROTOCOL_H

#include "alert.h"

#define ZBX_IPC_SERVICE_ALERTER	"alerter"

/* alerter -> manager */
#define ZBX_IPC_ALERTER_REGISTER	1000
#define ZBX_IPC_ALERTER_RESULT		1001
#define ZBX_IPC_ALERTER_MEDIATYPES	1002
#define ZBX_IPC_ALERTER_ALERTS		1003
#define ZBX_IPC_ALERTER_WATCHDOG	1004
#define ZBX_IPC_ALERTER_RESULTS		1005
#define ZBX_IPC_ALERTER_DROP_MEDIATYPES	1006

/* manager -> alerter */
#define ZBX_IPC_ALERTER_EMAIL		1100
#define ZBX_IPC_ALERTER_SMS		1102
#define ZBX_IPC_ALERTER_EXEC		1104
#define ZBX_IPC_ALERTER_WEBHOOK		1105

/* process -> manager */
#define ZBX_IPC_ALERTER_DIAG_STATS		1200
#define ZBX_IPC_ALERTER_DIAG_TOP_MEDIATYPES	1201
#define ZBX_IPC_ALERTER_DIAG_TOP_SOURCES	1202
#define ZBX_IPC_ALERTER_SEND_ALERT		1203
#define ZBX_IPC_ALERTER_BEGIN_DISPATCH		1204
#define ZBX_IPC_ALERTER_SEND_DISPATCH		1205
#define ZBX_IPC_ALERTER_END_DISPATCH		1206

/* manager -> process */
#define ZBX_IPC_ALERTER_DIAG_STATS_RESULT		1300
#define ZBX_IPC_ALERTER_DIAG_TOP_MEDIATYPES_RESULT	1301
#define ZBX_IPC_ALERTER_DIAG_TOP_SOURCES_RESULT		1302
#define ZBX_IPC_ALERTER_ABORT_DISPATCH			1303

#define ZBX_WATCHDOG_ALERT_FREQUENCY	(15 * SEC_PER_MIN)
#define ZBX_ALERT_NO_DEBUG		0
#define ZBX_ALERT_DEBUG			1

/* media type data */
typedef struct
{
	zbx_uint64_t		mediatypeid;

	int			location;

	/* the number of currently processing alerts */
	int			alerts_num;

	/* the number of alert objects for this media type */
	int			refcount;

	/* alert pool queue */
	zbx_binary_heap_t	queue;

	/* media type data */
	int			type;
	char			*smtp_server;
	char			*smtp_helo;
	char			*smtp_email;
	char			*exec_path;
	char			*gsm_modem;
	char			*username;
	char			*passwd;
	char			*exec_params;
	char			*script;
	char			*script_bin;
	char			*error;
	unsigned short		smtp_port;
	unsigned char		smtp_security;
	unsigned char		smtp_verify_peer;
	unsigned char		smtp_verify_host;
	unsigned char		smtp_authentication;

	int			maxsessions;
	int			maxattempts;
	int			attempt_interval;
	int			timeout;
	int			script_bin_sz;
	unsigned char		content_type;
	unsigned char		flags;
}
zbx_am_mediatype_t;

typedef struct
{
	zbx_uint64_t	mediaid;
	zbx_uint64_t	mediatypeid;
	char		*sendto;
}
zbx_am_media_t;

/* media type data */
typedef struct
{
	zbx_uint64_t		mediatypeid;

	/* media type data */
	unsigned char		type;
	char			*smtp_server;
	char			*smtp_helo;
	char			*smtp_email;
	char			*exec_path;
	char			*gsm_modem;
	char			*username;
	char			*passwd;
	char			*exec_params;
	char			*timeout;
	char			*script;
	char			*attempt_interval;
	unsigned short		smtp_port;
	unsigned char		smtp_security;
	unsigned char		smtp_verify_peer;
	unsigned char		smtp_verify_host;
	unsigned char		smtp_authentication;

	int			maxsessions;
	int			maxattempts;
	unsigned char		content_type;
	unsigned char		process_tags;
	time_t			last_access;
}
zbx_am_db_mediatype_t;

/* alert data */
typedef struct
{
	zbx_uint64_t	alertid;
	zbx_uint64_t	mediatypeid;
	zbx_uint64_t	eventid;
	zbx_uint64_t	objectid;
	zbx_uint64_t	p_eventid;

	char		*sendto;
	char		*subject;
	char		*message;
	char		*params;
	int		status;
	int		retries;
	int		source;
	int		object;
}
zbx_am_db_alert_t;

/* alert status update data */
typedef struct
{
	zbx_uint64_t	alertid;
	zbx_uint64_t	eventid;
	zbx_uint64_t	mediatypeid;
	int		retries;
	int		status;
	int		source;
	char		*value;
	char		*error;
}
zbx_am_result_t;

void	zbx_am_db_mediatype_clear(zbx_am_db_mediatype_t *mediatype);
void	zbx_am_db_alert_free(zbx_am_db_alert_t *alert);
void	zbx_am_media_clear(zbx_am_media_t *media);
void	zbx_am_media_free(zbx_am_media_t *media);

zbx_uint32_t	zbx_alerter_serialize_result(unsigned char **data, const char *value, int errcode, const char *error,
		const char *debug);
void	zbx_alerter_deserialize_result(const unsigned char *data, char **value, int *errcode, char **error,
		char **debug);

zbx_uint32_t	zbx_alerter_serialize_result_ext(unsigned char **data, const char *recipient, const char *value,
		int errcode, const char *error, const char *debug);
void	zbx_alerter_deserialize_result_ext(const unsigned char *data, char **recipient, char **value, int *errcode,
		char **error, char **debug);

zbx_uint32_t	zbx_alerter_serialize_email(unsigned char **data, zbx_uint64_t alertid, zbx_uint64_t mediatypeid,
		zbx_uint64_t eventid, const char *sendto, const char *subject, const char *message,
		const char *smtp_server, unsigned short smtp_port, const char *smtp_helo, const char *smtp_email,
		unsigned char smtp_security, unsigned char smtp_verify_peer, unsigned char smtp_verify_host,
		unsigned char smtp_authentication, const char *username, const char *password,
		unsigned char content_type);

void	zbx_alerter_deserialize_email(const unsigned char *data, zbx_uint64_t *alertid, zbx_uint64_t *mediatypeid,
		zbx_uint64_t *eventid, char **sendto, char **subject, char **message, char **smtp_server,
		unsigned short *smtp_port, char **smtp_helo, char **smtp_email, unsigned char *smtp_security,
		unsigned char *smtp_verify_peer, unsigned char *smtp_verify_host, unsigned char *smtp_authentication,
		char **username, char **password, unsigned char *content_type);

zbx_uint32_t	zbx_alerter_serialize_sms(unsigned char **data, zbx_uint64_t alertid,  const char *sendto,
		const char *message, const char *gsm_modem);

void	zbx_alerter_deserialize_sms(const unsigned char *data, zbx_uint64_t *alertid, char **sendto, char **message,
		char **gsm_modem);

zbx_uint32_t	zbx_alerter_serialize_exec(unsigned char **data, zbx_uint64_t alertid, const char *command);

void	zbx_alerter_deserialize_exec(const unsigned char *data, zbx_uint64_t *alertid, char **command);

zbx_uint32_t	zbx_alerter_serialize_alert_send(unsigned char **data, zbx_uint64_t mediatypeid, unsigned char type,
		const char *smtp_server, const char *smtp_helo, const char *smtp_email, const char *exec_path,
		const char *gsm_modem, const char *username, const char *passwd, unsigned short smtp_port,
		unsigned char smtp_security, unsigned char smtp_verify_peer, unsigned char smtp_verify_host,
		unsigned char smtp_authentication, const char *exec_params, int maxsessions, int maxattempts,
		const char *attempt_interval, unsigned char content_type, const char *script, const char *timeout,
		const char *sendto, const char *subject, const char *message, const char *params);

void	zbx_alerter_deserialize_alert_send(const unsigned char *data, zbx_uint64_t *mediatypeid,
		unsigned char *type, char **smtp_server, char **smtp_helo, char **smtp_email, char **exec_path,
		char **gsm_modem, char **username, char **passwd, unsigned short *smtp_port,
		unsigned char *smtp_security, unsigned char *smtp_verify_peer, unsigned char *smtp_verify_host,
		unsigned char *smtp_authentication, char **exec_params, int *maxsessions, int *maxattempts,
		char **attempt_interval, unsigned char *content_type, char **script, char **timeout,
		char **sendto, char **subject, char **message, char **params);

zbx_uint32_t	zbx_alerter_serialize_webhook(unsigned char **data, const char *script_bin, int script_sz,
		int timeout, const char *params, unsigned char debug);

void	zbx_alerter_deserialize_webhook(const unsigned char *data, char **script_bin, int *script_sz, int *timeout,
		char **params, unsigned char *debug);

zbx_uint32_t	zbx_alerter_serialize_mediatypes(unsigned char **data, zbx_am_db_mediatype_t **mediatypes,
		int mediatypes_num);

void	zbx_alerter_deserialize_mediatypes(const unsigned char *data, zbx_am_db_mediatype_t ***mediatypes,
		int *mediatypes_num);

zbx_uint32_t	zbx_alerter_serialize_alerts(unsigned char **data, zbx_am_db_alert_t **alerts, int alerts_num);

void	zbx_alerter_deserialize_alerts(const unsigned char *data, zbx_am_db_alert_t ***alerts, int *alerts_num);

zbx_uint32_t	zbx_alerter_serialize_medias(unsigned char **data, zbx_am_media_t **medias, int medias_num);

void	zbx_alerter_deserialize_medias(const unsigned char *data, zbx_am_media_t ***medias, int *medias_num);

zbx_uint32_t	zbx_alerter_serialize_results(unsigned char **data, zbx_am_result_t **results, int results_num);

void	zbx_alerter_deserialize_results(const unsigned char *data, zbx_am_result_t ***results, int *results_num);

zbx_uint32_t	zbx_alerter_serialize_ids(unsigned char **data, zbx_uint64_t *ids, int ids_num);

void	zbx_alerter_deserialize_ids(const unsigned char *data, zbx_uint64_t **ids, int *ids_num);

void	zbx_alerter_deserialize_top_request(const unsigned char *data, int *limit);

zbx_uint32_t	zbx_alerter_serialize_diag_stats(unsigned char **data, zbx_uint64_t alerts_num);

zbx_uint32_t	zbx_alerter_serialize_top_mediatypes_result(unsigned char **data, zbx_am_mediatype_t **mediatypes,
		int mediatypes_num);

zbx_uint32_t	zbx_alerter_serialize_top_sources_result(unsigned char **data, zbx_am_source_stats_t **sources,
		int sources_num);

zbx_uint32_t	zbx_alerter_serialize_begin_dispatch(unsigned char **data, const char *subject, const char *message,
		const char *content_name, const char *content_type, const char *content, zbx_uint32_t content_size);
void	zbx_alerter_deserialize_begin_dispatch(const unsigned char *data, char **subject, char **message,
		char **content_name, char **content_type, char **content, zbx_uint32_t *content_size);

zbx_uint32_t	zbx_alerter_serialize_send_dispatch(unsigned char **data, const ZBX_DB_MEDIATYPE *mt,
		const zbx_vector_str_t *recipients);
void	zbx_alerter_deserialize_send_dispatch(const unsigned char *data, ZBX_DB_MEDIATYPE *mt, zbx_vector_str_t
		*recipients);

#endif
