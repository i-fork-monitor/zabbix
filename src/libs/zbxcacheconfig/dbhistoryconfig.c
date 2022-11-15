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

#include "zbxcacheconfig.h"
#include "dbconfig.h"
#include "zbxserver.h"
#include "actions.h"

static void	DCget_history_data_host(DC_HISTORY_DATA_HOST *dst_host, const ZBX_DC_HOST *src_host, unsigned int mode)
{
	const ZBX_DC_IPMIHOST	*ipmihost;

	dst_host->hostid = src_host->hostid;
	dst_host->proxy_hostid = src_host->proxy_hostid;
	dst_host->status = src_host->status;

	if (ZBX_ITEM_GET_HOST & mode)
		zbx_strscpy(dst_host->host, src_host->host);

	if (ZBX_ITEM_GET_HOSTNAME & mode)
		zbx_strlcpy_utf8(dst_host->name, src_host->name, sizeof(dst_host->name));

	if (ZBX_ITEM_GET_MAINTENANCE & mode)
	{
		dst_host->maintenance_status = src_host->maintenance_status;
		dst_host->maintenance_type = src_host->maintenance_type;
		dst_host->maintenance_from = src_host->maintenance_from;
	}

	if (ZBX_ITEM_GET_HOSTINFO & mode)
	{
		dst_host->tls_accept = src_host->tls_accept;
#if defined(HAVE_GNUTLS) || defined(HAVE_OPENSSL)
		zbx_strscpy(dst_host->tls_issuer, src_host->tls_issuer);
		zbx_strscpy(dst_host->tls_subject, src_host->tls_subject);

		if (NULL == src_host->tls_dc_psk)
		{
			*dst_host->tls_psk_identity = '\0';
			*dst_host->tls_psk = '\0';
		}
		else
		{
			zbx_strscpy(dst_host->tls_psk_identity, src_host->tls_dc_psk->tls_psk_identity);
			zbx_strscpy(dst_host->tls_psk, src_host->tls_dc_psk->tls_psk);
		}
#endif
	}
}

static void	DCget_history_data_item(DC_HISTORY_DATA_ITEM *dst_item, const ZBX_DC_ITEM *src_item, unsigned int mode)
{
	const ZBX_DC_LOGITEM	*logitem;
	const ZBX_DC_TRAPITEM	*trapitem;

	dst_item->type = src_item->type;
	dst_item->value_type = src_item->value_type;
	dst_item->state = ITEM_STATE_NORMAL;
	dst_item->status = src_item->status;

	zbx_strscpy(dst_item->key_orig, src_item->key);

	dst_item->itemid = src_item->itemid;
	dst_item->flags = src_item->flags;
	dst_item->key = NULL;

	switch (src_item->value_type)
	{
		case ITEM_VALUE_TYPE_LOG:
			if (NULL != (logitem = (ZBX_DC_LOGITEM *)zbx_hashset_search(&config->logitems,
					&src_item->itemid)))
			{
				zbx_strscpy(dst_item->logtimefmt, logitem->logtimefmt);
			}
			else
				*dst_item->logtimefmt = '\0';
			break;
	}

	if (ZBX_ITEM_GET_INTERFACE & mode)
	{
		const ZBX_DC_INTERFACE		*dc_interface;

		dc_interface = (ZBX_DC_INTERFACE *)zbx_hashset_search(&config->interfaces, &src_item->interfaceid);

		DCget_interface(&dst_item->interface, dc_interface);
	}

	if (0 == (ZBX_ITEM_GET_TRAPPER & mode))
		return;

	switch (src_item->type)
	{
		case ITEM_TYPE_TRAPPER:
			if (NULL != (trapitem = (ZBX_DC_TRAPITEM *)zbx_hashset_search(&config->trapitems,
					&src_item->itemid)))
			{
				zbx_strscpy(dst_item->trapper_hosts, trapitem->trapper_hosts);
			}
			else
				*dst_item->trapper_hosts = '\0';
			break;
		default:
			/* nothing to do */;
	}
}

/******************************************************************************
 *                                                                            *
 * Purpose: retrieve item maintenances information from configuration cache   *
 *                                                                            *
 * Parameters: items    - [OUT] pointer to array of DC_ITEM structures        *
 *             errcodes - [IN] SUCCEED if record located and FAIL otherwise   *
 *             num      - [IN] number of elements in items, keys, errcodes    *
 *                                                                            *
 * NOTE: Maintenances can be dynamically updated by timer processes that      *
 *       currently only lock configuration cache.                             *
 *                                                                            *
 ******************************************************************************/
static	void	get_items_maintenances(DC_HISTORY_DATA_ITEM *items, const int *errcodes, int num)
{
	int			i;
	const ZBX_DC_HOST	*dc_host = NULL;

	RDLOCK_CACHE;

	for (i = 0; i < num; i++)
	{
		if (FAIL == errcodes[i])
			continue;

		if (NULL == dc_host || dc_host->hostid != items[i].host.hostid)
		{
			if (NULL == (dc_host = (ZBX_DC_HOST *)zbx_hashset_search(&config->hosts,
					&items[i].host.hostid)))
			{
				continue;
			}
		}

		DCget_history_data_host(&items[i].host, dc_host, ZBX_ITEM_GET_MAINTENANCE);
	}

	UNLOCK_CACHE;
}

/******************************************************************************
 *                                                                            *
 * Purpose: locate item in configuration cache by host and key                *
 *                                                                            *
 * Parameters: items    - [OUT] pointer to array of DC_ITEM structures        *
 *             keys     - [IN] list of item keys with host names              *
 *             errcodes - [OUT] SUCCEED if record located and FAIL otherwise  *
 *             num      - [IN] number of elements in items, keys, errcodes    *
 *                                                                            *
 * NOTE: Item and host is retrieved using history read lock that must be      *
 *       write locked only when configuration sync occurs to avoid processes  *
 *       blocking each other.                                                 *
 *                                                                            *
 ******************************************************************************/
void	DCconfig_history_data_get_items_by_keys(DC_HISTORY_DATA_ITEM *items, zbx_host_key_t *keys, int *errcodes,
		size_t num)
{
	size_t			i;
	const ZBX_DC_ITEM	*dc_item;
	const ZBX_DC_HOST	*dc_host;

	memset(errcodes, 0, sizeof(int) * num);

	RDLOCK_CACHE_CONFIG_HISTORY;

	for (i = 0; i < num; i++)
	{
		if (NULL == (dc_host = DCfind_host(keys[i].host)) ||
				NULL == (dc_item = DCfind_item(dc_host->hostid, keys[i].key)))
		{
			errcodes[i] = FAIL;
			continue;
		}

		DCget_history_data_host(&items[i].host, dc_host, ZBX_ITEM_GET_DEFAULT & (~ZBX_ITEM_GET_MAINTENANCE));
		DCget_history_data_item(&items[i], dc_item, ZBX_ITEM_GET_DEFAULT);
	}

	UNLOCK_CACHE_CONFIG_HISTORY;

	get_items_maintenances(items, errcodes, num);
}

/******************************************************************************
 *                                                                            *
 * Purpose: convert item history/trends housekeeping period to numeric values *
 *          expanding user macros and applying global housekeeping settings   *
 *                                                                            *
 ******************************************************************************/
static void	dc_items_convert_hk_periods(const zbx_config_hk_t *config_hk, DC_HISTORY_ITEM *item)
{
	if (NULL != item->trends_period)
	{
		zbx_substitute_simple_macros(NULL, NULL, NULL, NULL, &item->host.hostid, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, &item->trends_period, MACRO_TYPE_COMMON, NULL, 0);

		if (SUCCEED != zbx_is_time_suffix(item->trends_period, &item->trends_sec, ZBX_LENGTH_UNLIMITED))
			item->trends_sec = ZBX_HK_PERIOD_MAX;

		if (0 != item->trends_sec && ZBX_HK_OPTION_ENABLED == config_hk->history_global)
			item->trends_sec = config_hk->trends;

		item->trends = (0 != item->trends_sec);
	}

	if (NULL != item->history_period)
	{
		zbx_substitute_simple_macros(NULL, NULL, NULL, NULL, &item->host.hostid, NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, &item->history_period, MACRO_TYPE_COMMON, NULL, 0);

		if (SUCCEED != zbx_is_time_suffix(item->history_period, &item->history_sec, ZBX_LENGTH_UNLIMITED))
			item->history_sec = ZBX_HK_PERIOD_MAX;

		if (0 != item->history_sec && ZBX_HK_OPTION_ENABLED == config_hk->history_global)
			item->history_sec = config_hk->history;

		item->history = (0 != item->history_sec);
	}
}

/******************************************************************************
 *                                                                            *
 * Purpose: Get item with specified ID                                        *
 *                                                                            *
 * Parameters: items    - [OUT] pointer to DC_ITEM structures                 *
 *             itemids  - [IN] array of item IDs                              *
 *             errcodes - [OUT] SUCCEED if item found, otherwise FAIL         *
 *             num      - [IN] number of elements                             *
 *                                                                            *
 * NOTE: Item and host is retrieved using history read lock that must be      *
 *       write locked only when configuration sync occurs to avoid processes  *
 *       blocking each other.                                                 *
 *                                                                            *
 ******************************************************************************/
void	DCconfig_history_data_get_items_by_itemids(DC_HISTORY_DATA_ITEM *items, const zbx_uint64_t *itemids,
		int *errcodes, int num, unsigned int mode)
{
	int			i;
	const ZBX_DC_ITEM	*dc_item;
	const ZBX_DC_HOST	*dc_host = NULL;

	memset(errcodes, 0, sizeof(int) * num);

	RDLOCK_CACHE_CONFIG_HISTORY;

	for (i = 0; i < num; i++)
	{
		if (NULL == (dc_item = (ZBX_DC_ITEM *)zbx_hashset_search(&config->items, &itemids[i])))
		{
			errcodes[i] = FAIL;
			continue;
		}

		if (NULL == dc_host || dc_host->hostid != dc_item->hostid)
		{
			if (NULL == (dc_host = (ZBX_DC_HOST *)zbx_hashset_search(&config->hosts, &dc_item->hostid)))
			{
				errcodes[i] = FAIL;
				continue;
			}
		}

		DCget_history_data_host(&items[i].host, dc_host, mode & (~ZBX_ITEM_GET_MAINTENANCE));
		DCget_history_data_item(&items[i], dc_item, mode);
	}

	UNLOCK_CACHE_CONFIG_HISTORY;

	get_items_maintenances(items, errcodes, num);
}

static void	DCget_history_host(DC_HISTORY_HOST *dst_host, const ZBX_DC_HOST *src_host, unsigned int mode)
{
	const ZBX_DC_HOST_INVENTORY	*host_inventory;

	dst_host->hostid = src_host->hostid;
	dst_host->proxy_hostid = src_host->proxy_hostid;
	dst_host->status = src_host->status;

	zbx_strscpy(dst_host->host, src_host->host);

	if (ZBX_ITEM_GET_HOSTNAME & mode)
		zbx_strlcpy_utf8(dst_host->name, src_host->name, sizeof(dst_host->name));

	if (NULL != (host_inventory = (ZBX_DC_HOST_INVENTORY *)zbx_hashset_search(&config->host_inventories,
			&src_host->hostid)))
	{
		dst_host->inventory_mode = (signed char)host_inventory->inventory_mode;
	}
	else
		dst_host->inventory_mode = HOST_INVENTORY_DISABLED;
}

static void	DCget_history_item(DC_HISTORY_ITEM *dst_item, const ZBX_DC_ITEM *src_item)
{
	const ZBX_DC_NUMITEM	*numitem;

	dst_item->type = src_item->type;
	dst_item->value_type = src_item->value_type;

	dst_item->state = src_item->state;
	dst_item->lastlogsize = src_item->lastlogsize;
	dst_item->mtime = src_item->mtime;

	if ('\0' != *src_item->error)
		dst_item->error = zbx_strdup(NULL, src_item->error);
	else
		dst_item->error = NULL;

	dst_item->inventory_link = src_item->inventory_link;
	dst_item->valuemapid = src_item->valuemapid;
	dst_item->status = src_item->status;

	dst_item->history_period = zbx_strdup(NULL, src_item->history_period);
	dst_item->flags = src_item->flags;

	zbx_strscpy(dst_item->key_orig, src_item->key);

	switch (src_item->value_type)
	{
		case ITEM_VALUE_TYPE_FLOAT:
		case ITEM_VALUE_TYPE_UINT64:
			numitem = (ZBX_DC_NUMITEM *)zbx_hashset_search(&config->numitems, &src_item->itemid);

			dst_item->trends_period = zbx_strdup(NULL, numitem->trends_period);

			if ('\0' != *numitem->units)
				dst_item->units = zbx_strdup(NULL, numitem->units);
			else
				dst_item->units = NULL;
			break;
		default:
			dst_item->trends_period = NULL;
			dst_item->units = NULL;
	}
}

/******************************************************************************
 *                                                                            *
 * Purpose: Get item with specified ID                                        *
 *                                                                            *
 * Parameters: items    - [OUT] pointer to DC_ITEM structures                 *
 *             itemids  - [IN] array of item IDs                              *
 *             errcodes - [OUT] SUCCEED if item found, otherwise FAIL         *
 *             num      - [IN] number of elements                             *
 *                                                                            *
 * NOTE: Item and host is retrieved using history read lock that must be      *
 *       write locked only when configuration sync occurs to avoid processes  *
 *       blocking each other. Item can only be processed by one history       *
 *       syncer at a time, thus it is safe to read dynamic data such as error *
 *       as no other process will update it.                                  *
 *                                                                            *
 ******************************************************************************/
void	DCconfig_history_get_items_by_itemids(DC_HISTORY_ITEM *items, const zbx_uint64_t *itemids, int *errcodes, int num,
		unsigned int mode)
{
	int			i;
	const ZBX_DC_ITEM	*dc_item;
	const ZBX_DC_HOST	*dc_host = NULL;
	zbx_config_hk_t		config_hk;
	zbx_dc_um_handle_t	*um_handle;

	memset(errcodes, 0, sizeof(int) * num);

	RDLOCK_CACHE_CONFIG_HISTORY;

	for (i = 0; i < num; i++)
	{
		if (NULL == (dc_item = (ZBX_DC_ITEM *)zbx_hashset_search(&config->items, &itemids[i])))
		{
			errcodes[i] = FAIL;
			continue;
		}

		if (NULL == dc_host || dc_host->hostid != dc_item->hostid)
		{
			if (NULL == (dc_host = (ZBX_DC_HOST *)zbx_hashset_search(&config->hosts, &dc_item->hostid)))
			{
				errcodes[i] = FAIL;
				continue;
			}
		}

		DCget_history_host(&items[i].host, dc_host, mode);
		DCget_history_item(&items[i], dc_item);

		config_hk = config->config->hk;
	}

	UNLOCK_CACHE_CONFIG_HISTORY;

	um_handle = zbx_dc_open_user_macros();

	/* avoid unnecessary allocations inside lock if there are no error or units */
	for (i = 0; i < num; i++)
	{
		if (FAIL == errcodes[i])
			continue;

		items[i].itemid = itemids[i];

		dc_items_convert_hk_periods(&config_hk, &items[i]);
	}

	zbx_dc_close_user_macros(um_handle);
}

void	DCconfig_clean_history_items(DC_HISTORY_ITEM *items, int *errcodes, size_t num)
{
	size_t	i;

	for (i = 0; i < num; i++)
	{
		if (NULL != errcodes && SUCCEED != errcodes[i])
			continue;

		if (ITEM_VALUE_TYPE_FLOAT == items[i].value_type || ITEM_VALUE_TYPE_UINT64 == items[i].value_type)
			zbx_free(items[i].units);

		zbx_free(items[i].error);
		zbx_free(items[i].history_period);
		zbx_free(items[i].trends_period);
	}
}

/******************************************************************************
 *                                                                            *
 * Purpose: Get functions by IDs                                              *
 *                                                                            *
 * Parameters: functions   - [OUT] pointer to DC_FUNCTION structures          *
 *             functionids - [IN] array of function IDs                       *
 *             errcodes    - [OUT] SUCCEED if item found, otherwise FAIL      *
 *             num         - [IN] number of elements                          *
 *                                                                            *
 * NOTE: Data is retrieved using history read lock that must be write locked  *
 *       only when configuration sync occurs to avoid processes blocking      *
 *       each other.                                                          *
 *                                                                            *
 ******************************************************************************/
void	DCconfig_history_get_functions_by_functionids(DC_FUNCTION *functions, zbx_uint64_t *functionids, int *errcodes,
		size_t num)
{
	size_t			i;
	const ZBX_DC_FUNCTION	*dc_function;

	RDLOCK_CACHE_CONFIG_HISTORY;

	for (i = 0; i < num; i++)
	{
		if (NULL == (dc_function = (ZBX_DC_FUNCTION *)zbx_hashset_search(&config->functions, &functionids[i])))
		{
			errcodes[i] = FAIL;
			continue;
		}

		DCget_function(&functions[i], dc_function);
		errcodes[i] = SUCCEED;
	}

	UNLOCK_CACHE_CONFIG_HISTORY;
}

/******************************************************************************
 *                                                                            *
 * Purpose: get enabled triggers for specified items                          *
 *                                                                            *
 * NOTE: Trigger is retrieved using history read lock that must be            *
 *       write locked only when configuration sync occurs to avoid processes  *
 *       blocking each other. Trigger can only be processed by one process at *
 *       a time, thus it is safe to read dynamic data such as error as no     *
 *       other process will update it.                                        *
 *                                                                            *
 ******************************************************************************/
void	DCconfig_history_get_triggers_by_itemids(zbx_hashset_t *trigger_info, zbx_vector_ptr_t *trigger_order,
		const zbx_uint64_t *itemids, const zbx_timespec_t *timespecs, int itemids_num)
{
	int			i, j, found;
	const ZBX_DC_ITEM	*dc_item;
	const ZBX_DC_TRIGGER	*dc_trigger;
	DC_TRIGGER		*trigger;

	RDLOCK_CACHE_CONFIG_HISTORY;

	for (i = 0; i < itemids_num; i++)
	{
		/* skip items which are not in configuration cache and items without triggers */

		if (NULL == (dc_item = (ZBX_DC_ITEM *)zbx_hashset_search(&config->items, &itemids[i])) ||
				NULL == dc_item->triggers)
		{
			continue;
		}

		/* process all triggers for the specified item */

		for (j = 0; NULL != (dc_trigger = dc_item->triggers[j]); j++)
		{
			if (TRIGGER_STATUS_ENABLED != dc_trigger->status)
				continue;

			/* find trigger by id or create a new record in hashset if not found */
			trigger = (DC_TRIGGER *)DCfind_id(trigger_info, dc_trigger->triggerid, sizeof(DC_TRIGGER),
					&found);

			if (0 == found)
			{
				DCget_trigger(trigger, dc_trigger, ZBX_TRIGGER_GET_ALL);
				zbx_vector_ptr_append(trigger_order, trigger);
			}

			/* copy latest change timestamp */

			if (trigger->timespec.sec < timespecs[i].sec ||
					(trigger->timespec.sec == timespecs[i].sec &&
					trigger->timespec.ns < timespecs[i].ns))
			{
				/* DCconfig_get_triggers_by_itemids() function is called during trigger processing */
				/* when syncing history cache. A trigger cannot be processed by two syncers at the */
				/* same time, so it is safe to update trigger timespec within read lock.           */
				trigger->timespec = timespecs[i];
			}
		}
	}

	UNLOCK_CACHE_CONFIG_HISTORY;
}

/******************************************************************************
 *                                                                            *
 * Purpose: copies configuration cache action conditions to the specified     *
 *          vector                                                            *
 *                                                                            *
 * Parameters: dc_action  - [IN] the source action                            *
 *             conditions - [OUT] the conditions vector                       *
 *                                                                            *
 ******************************************************************************/
static void	dc_action_copy_conditions(const zbx_dc_action_t *dc_action, zbx_vector_ptr_t *conditions)
{
	int				i;
	zbx_condition_t			*condition;
	zbx_dc_action_condition_t	*dc_condition;

	zbx_vector_ptr_reserve(conditions, dc_action->conditions.values_num);

	for (i = 0; i < dc_action->conditions.values_num; i++)
	{
		dc_condition = (zbx_dc_action_condition_t *)dc_action->conditions.values[i];

		condition = (zbx_condition_t *)zbx_malloc(NULL, sizeof(zbx_condition_t));

		condition->conditionid = dc_condition->conditionid;
		condition->actionid = dc_action->actionid;
		condition->conditiontype = dc_condition->conditiontype;
		condition->op = dc_condition->op;
		condition->value = zbx_strdup(NULL, dc_condition->value);
		condition->value2 = zbx_strdup(NULL, dc_condition->value2);
		zbx_vector_uint64_create(&condition->eventids);

		zbx_vector_ptr_append(conditions, condition);
	}
}

/******************************************************************************
 *                                                                            *
 * Purpose: creates action evaluation data from configuration cache action    *
 *                                                                            *
 * Parameters: dc_action - [IN] the source action                             *
 *                                                                            *
 * Return value: the action evaluation data                                   *
 *                                                                            *
 * Comments: The returned value must be freed with zbx_action_eval_free()     *
 *           function later.                                                  *
 *                                                                            *
 ******************************************************************************/
static zbx_action_eval_t	*dc_action_eval_create(const zbx_dc_action_t *dc_action)
{
	zbx_action_eval_t		*action;

	action = (zbx_action_eval_t *)zbx_malloc(NULL, sizeof(zbx_action_eval_t));

	action->actionid = dc_action->actionid;
	action->eventsource = dc_action->eventsource;
	action->evaltype = dc_action->evaltype;
	action->opflags = dc_action->opflags;
	action->formula = zbx_strdup(NULL, dc_action->formula);
	zbx_vector_ptr_create(&action->conditions);

	dc_action_copy_conditions(dc_action, &action->conditions);

	return action;
}

/******************************************************************************
 *                                                                            *
 * Purpose: gets action evaluation data                                       *
 *                                                                            *
 * Parameters: actions         - [OUT] the action evaluation data             *
 *             uniq_conditions - [OUT] unique conditions that actions         *
 *                                     point to (several sources)             *
 *             opflags         - [IN] flags specifying which actions to get   *
 *                                    based on their operation classes        *
 *                                    (see ZBX_ACTION_OPCLASS_* defines)      *
 *                                                                            *
 * Comments: The returned actions and conditions must be freed with           *
 *           zbx_action_eval_free() function later.                           *
 *                                                                            *
 ******************************************************************************/
void	zbx_dc_config_history_get_actions_eval(zbx_vector_ptr_t *actions, unsigned char opflags)
{
	const zbx_dc_action_t		*dc_action;
	zbx_hashset_iter_t		iter;

	zabbix_log(LOG_LEVEL_DEBUG, "In %s()", __func__);

	RDLOCK_CACHE_CONFIG_HISTORY;

	zbx_hashset_iter_reset(&config->actions, &iter);

	while (NULL != (dc_action = (const zbx_dc_action_t *)zbx_hashset_iter_next(&iter)))
	{
		if (0 != (opflags & dc_action->opflags))
			zbx_vector_ptr_append(actions, dc_action_eval_create(dc_action));
	}

	UNLOCK_CACHE_CONFIG_HISTORY;

	zabbix_log(LOG_LEVEL_DEBUG, "End of %s() actions:%d", __func__, actions->values_num);
}

/******************************************************************************
 *                                                                            *
 * Purpose: updates item nextcheck values in configuration cache              *
 *                                                                            *
 * Parameters: items      - [IN] the items to update                          *
 *             values     - [IN] the items values containing new properties   *
 *             errcodes   - [IN] item error codes. Update only items with     *
 *                               SUCCEED code                                 *
 *             values_num - [IN] the number of elements in items,values and   *
 *                               errcodes arrays                              *
 *                                                                            *
 ******************************************************************************/
void	zbx_dc_items_update_nextcheck(DC_HISTORY_DATA_ITEM *items, zbx_agent_value_t *values, int *errcodes, size_t values_num)
{
	size_t			i;
	ZBX_DC_ITEM		*dc_item;
	ZBX_DC_HOST		*dc_host;
	ZBX_DC_INTERFACE	*dc_interface;

	RDLOCK_CACHE;

	for (i = 0; i < values_num; i++)
	{
		if (FAIL == errcodes[i])
			continue;

		/* update nextcheck for items that are counted in queue for monitoring purposes */
		if (FAIL == zbx_is_counted_in_item_queue(items[i].type, items[i].key_orig))
			continue;

		if (NULL == (dc_item = (ZBX_DC_ITEM *)zbx_hashset_search(&config->items, &items[i].itemid)))
			continue;

		if (ITEM_STATUS_ACTIVE != dc_item->status)
			continue;

		if (NULL == (dc_host = (ZBX_DC_HOST *)zbx_hashset_search(&config->hosts, &dc_item->hostid)))
			continue;

		if (HOST_STATUS_MONITORED != dc_host->status)
			continue;

		if (ZBX_LOC_NOWHERE != dc_item->location)
			continue;

		dc_interface = (ZBX_DC_INTERFACE *)zbx_hashset_search(&config->interfaces, &dc_item->interfaceid);

		/* update nextcheck for items that are counted in queue for monitoring purposes */
		DCitem_nextcheck_update(dc_item, dc_interface, ZBX_ITEM_COLLECTED, values[i].ts.sec,
				NULL);
	}

	UNLOCK_CACHE;
}
