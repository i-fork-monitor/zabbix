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

package mysql

import (
	"fmt"
	"os"
	"testing"

	"github.com/google/go-cmp/cmp"
)

func Test_PluginOptions_setCustomQueriesDirDefault(t *testing.T) {
	t.Parallel()

	type fields struct {
		CustomQueriesPath    string
		CustomQueriesEnabled bool
	}

	tests := []struct {
		name   string
		fields fields
		want   *PluginOptions
	}{
		{
			"+valid",
			fields{
				CustomQueriesPath:    "C:\\valid\\abs\\path",
				CustomQueriesEnabled: true,
			},
			&PluginOptions{
				CustomQueriesPath:    "C:\\valid\\abs\\path",
				CustomQueriesEnabled: true,
			},
		},
		{
			"+default",
			fields{
				CustomQueriesEnabled: true,
			},
			&PluginOptions{
				CustomQueriesPath: fmt.Sprintf(
					"%s\\Zabbix Agent 2\\Custom Queries\\Mysql", os.Getenv("ProgramFiles"),
				),
				CustomQueriesEnabled: true,
			},
		},
		{
			"-empty",
			fields{},
			&PluginOptions{},
		},
	}
	for _, tt := range tests {
		tt := tt
		t.Run(tt.name, func(t *testing.T) {
			t.Parallel()

			pc := &PluginOptions{
				CustomQueriesPath:    tt.fields.CustomQueriesPath,
				CustomQueriesEnabled: tt.fields.CustomQueriesEnabled,
			}

			pc.setCustomQueriesPathDefault()
			if diff := cmp.Diff(tt.want, pc); diff != "" {
				t.Fatalf("PluginOptions.setCustomQueriesPathDefault() = %s", diff)
			}
		})
	}
}
