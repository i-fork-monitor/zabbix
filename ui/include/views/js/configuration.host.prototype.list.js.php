<?php
/*
** Copyright (C) 2001-2025 Zabbix SIA
**
** This program is free software: you can redistribute it and/or modify it under the terms of
** the GNU Affero General Public License as published by the Free Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
** without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License along with this program.
** If not, see <https://www.gnu.org/licenses/>.
**/


/**
 * @var CView $this
 */
?>

<script>
	const view = {
		init({context, checkbox_hash}) {
			this.context = context;
			this.checkbox_hash = checkbox_hash;

			this.registerSubscribers();
		},

		registerSubscribers() {
			ZABBIX.EventHub.subscribe({
				require: {
					context: CPopupManager.EVENT_CONTEXT,
					event: CPopupManagerEvent.EVENT_SUBMIT
				},
				callback: ({data, event}) => {
					uncheckTableRows('host_prototypes_' + this.checkbox_hash, [], false);

					if (data.submit.success.action === 'delete') {
						const url = new URL('host_discovery.php', location.origin);

						url.searchParams.set('context', this.context);

						event.setRedirectUrl(url.href);
					}
				}
			});
		}
	};
</script>
