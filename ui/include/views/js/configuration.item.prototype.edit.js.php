<?php
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


/**
 * @var CView $this
 */

include dirname(__FILE__).'/common.item.edit.js.php';
include dirname(__FILE__).'/item.preprocessing.js.php';
include dirname(__FILE__).'/editabletable.js.php';
include dirname(__FILE__).'/itemtest.js.php';
?>
<script>
	const view = {
		form_name: null,

		init({form_name, trends_default}) {
			this.form_name = form_name;

			// Field switchers.
			new CViewSwitcher('value_type', 'change', item_form.field_switches.for_value_type);

			$('#value_type')
				.change(this.valueTypeChangeHandler)
				.data('old-value', $('#value_type').val());

			$('#type')
				.change(this.typeChangeHandler)
				.trigger('change');

			$('#history_mode')
				.change(function() {
					if ($('[name="history_mode"][value=' + <?= ITEM_STORAGE_OFF ?> + ']').is(':checked')) {
						$('#history').prop('disabled', true).hide();
					}
					else {
						$('#history').prop('disabled', false).show();
					}
				})
				.trigger('change');

			$('#trends_mode')
				.change(function() {
					if ($('[name="trends_mode"][value=' + <?= ITEM_STORAGE_OFF ?> + ']').is(':checked')) {
						$('#trends').prop('disabled', true).hide();
					}
					else {
						$('#trends').prop('disabled', false).show();
					}
				})
				.trigger('change');
		},

		typeChangeHandler() {
			// Selected item type.
			const type = parseInt(jQuery('#type').val(), 10);
			const has_key_button = [ <?= ITEM_TYPE_ZABBIX ?>, <?= ITEM_TYPE_ZABBIX_ACTIVE ?>, <?= ITEM_TYPE_SIMPLE ?>,
				<?= ITEM_TYPE_INTERNAL ?>, <?= ITEM_TYPE_DB_MONITOR ?>, <?= ITEM_TYPE_SNMPTRAP ?>, <?= ITEM_TYPE_JMX ?>,
				<?= ITEM_TYPE_IPMI ?>
			];

			jQuery('#keyButton').prop('disabled', !has_key_button.includes(type));

			if (type == <?= ITEM_TYPE_SSH ?> || type == <?= ITEM_TYPE_TELNET ?>) {
				jQuery('label[for=username]').addClass('<?= ZBX_STYLE_FIELD_LABEL_ASTERISK ?>');
				jQuery('input[name=username]').attr('aria-required', 'true');
			}
			else {
				jQuery('label[for=username]').removeClass('<?= ZBX_STYLE_FIELD_LABEL_ASTERISK ?>');
				jQuery('input[name=username]').removeAttr('aria-required');
			}

			jQuery('z-select[name="value_type"]').trigger('change');
		},

		valueTypeChangeHandler() {
			// If non-numeric type is changed to numeric, set the default value for trends.
			if ((this.value == <?= ITEM_VALUE_TYPE_FLOAT ?> || this.value == <?= ITEM_VALUE_TYPE_UINT64 ?>)
					&& <?= json_encode([
						ITEM_VALUE_TYPE_STR, ITEM_VALUE_TYPE_LOG, ITEM_VALUE_TYPE_TEXT, ITEM_VALUE_TYPE_BINARY
					]) ?>.includes($(this).data('old-value'))) {
				const trends = $('#trends');

				if (trends.val() == 0) {
					trends.val(trends_default);
				}

				$('#trends_mode_1').prop('checked', true);
			}

			$('#trends_mode').trigger('change');
			$(this).data('old-value', this.value);
		},

		editHost(e, hostid) {
			e.preventDefault();
			const host_data = {hostid};

			this.openHostPopup(host_data);
		},

		editTemplate(e, templateid) {
			e.preventDefault();
			const template_data = {templateid};

			this.openTemplatePopup(template_data);
		},

		openHostPopup(host_data) {
			const original_url = location.href;
			const overlay = PopUp('popup.host.edit', host_data, {
				dialogueid: 'host_edit',
				dialogue_class: 'modal-popup-large',
				prevent_navigation: true
			});

			overlay.$dialogue[0].addEventListener('dialogue.create', this.events.elementSuccess, {once: true});
			overlay.$dialogue[0].addEventListener('dialogue.update', this.events.elementSuccess, {once: true});
			overlay.$dialogue[0].addEventListener('dialogue.delete',
				this.events.elementDelete.bind(this, 'host.list'), {once: true}
			);
			overlay.$dialogue[0].addEventListener('overlay.close', () => {
				history.replaceState({}, '', original_url);
			}, {once: true});
			overlay.$dialogue[0].addEventListener('edit.linked', (e) =>
				this.openTemplatePopup({templateid: e.detail.templateid})
			);
		},

		openTemplatePopup(template_data) {
			const original_url = location.href;
			const overlay = PopUp('template.edit', template_data, {
				dialogueid: 'templates-form',
				dialogue_class: 'modal-popup-large',
				prevent_navigation: true
			});

			overlay.$dialogue[0].addEventListener('dialogue.submit', this.events.elementSuccess, {once: true});
			overlay.$dialogue[0].addEventListener('dialogue.delete',
				this.events.elementDelete.bind(this, 'template.list'), {once: true}
			);
			overlay.$dialogue[0].addEventListener('overlay.close', () => {
				history.replaceState({}, '', original_url);
			}, {once: true});
			overlay.$dialogue[0].addEventListener('edit.linked', (e) =>
				this.openTemplatePopup({templateid: e.detail.templateid})
			);
		},

		refresh() {
			const url = new Curl('');
			const form = document.getElementsByName(this.form_name)[0];
			const fields = getFormFields(form);

			post(url.getUrl(), fields);
		},

		events: {
			elementSuccess(e) {
				const data = e.detail;

				if ('success' in data) {
					postMessageOk(data.success.title);

					if ('messages' in data.success) {
						postMessageDetails('success', data.success.messages);
					}
				}

				view.refresh();
			},

			elementDelete(action, e) {
				const data = e.detail;

				if ('success' in data) {
					postMessageOk(data.success.title);

					if ('messages' in data.success) {
						postMessageDetails('success', data.success.messages);
					}

					const curl = new Curl('zabbix.php');
					curl.setArgument('action', action);

					location.href = curl.getUrl();
				}
			}
		}
	};
</script>
