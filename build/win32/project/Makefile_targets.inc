all: $(PREBUILD) $(PROGNAME) $(POSTBUILD)

clean: $(POSTCLEAN)
# break this down because del has limit for command-line args length (NMAKE : fatal error U1077: 'del' : return code '0x1')
	@<<zbx_del_obj.bat
	del $(OBJS) 2>NUL
<<
	del $(RESOURCE_RES) $(MESSAGES_H) $(PROGNAME) messages.rc 2>NUL
	del MSG00001.bin $(TARGETNAME).$(TARGETEXT).manifest $(PDBS) $(TARGETNAME).$(TARGETEXT).idb 2>NUL

$(PROGNAME): $(MESSAGES_H) $(OBJS) $(RESOURCE_RES)
	$(LINKER) $(OBJS) $(RESOURCE_RES) $(LFLAGS) /OUT:$(PROGNAME) $(LIBS)
	$(MT) /outputresource:"$(PROGNAME);#1" /manifest $(TARGETNAME).$(TARGETEXT).manifest /nologo

$(MESSAGES_H): messages.mc
	$(MC) -U -h ".\\" -r ".\\" messages.mc

$(RESOURCE_RES): $(RESOURCE_RC) $(RESOURCE_H) $(DESC_H)
	$(RC) $(RFLAGS) /d "NDEBUG" /d "_VC80_UPGRADE=0x0600" /l 0x419 /fo"$(RESOURCE_RES)" $(RESOURCE_RC)

{..\..\..\src\libs\zbxalgo}.c{..\..\..\src\libs\zbxalgo}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxcommon}.c{..\..\..\src\libs\zbxcommon}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxbincommon}.c{..\..\..\src\libs\zbxbincommon}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxnum}.c{..\..\..\src\libs\zbxnum}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxstr}.c{..\..\..\src\libs\zbxstr}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxfile}.c{..\..\..\src\libs\zbxfile}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxfile\win32}.c{..\..\..\src\libs\zbxfile\win32}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxparam}.c{..\..\..\src\libs\zbxparam}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxexpr}.c{..\..\..\src\libs\zbxexpr}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxip}.c{..\..\..\src\libs\zbxip}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxtime}.c{..\..\..\src\libs\zbxtime}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxcommshigh}.c{..\..\..\src\libs\zbxcommshigh}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxcomms}.c{..\..\..\src\libs\zbxcomms}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxcompress}.c{..\..\..\src\libs\zbxcompress}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxcfg}.c{..\..\..\src\libs\zbxcfg}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxcrypto}.c{..\..\..\src\libs\zbxcrypto}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxgetopt}.c{..\..\..\src\libs\zbxgetopt}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxhash}.c{..\..\..\src\libs\zbxhash}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxjson}.c{..\..\..\src\libs\zbxjson}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxlog}.c{..\..\..\src\libs\zbxlog}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxxml}.c{..\..\..\src\libs\zbxxml}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxregexp}.c{..\..\..\src\libs\zbxregexp}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxthreads}.c{..\..\..\src\libs\zbxthreads}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxmutexs}.c{..\..\..\src\libs\zbxmutexs}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxexec}.c{..\..\..\src\libs\zbxexec}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxhttp}.c{..\..\..\src\libs\zbxhttp}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxcurl}.c{..\..\..\src\libs\zbxcurl}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxsysinfo}.c{..\..\..\src\libs\zbxsysinfo}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxsysinfo\agent}.c{..\..\..\src\libs\zbxsysinfo\agent}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxsysinfo\simple}.c{..\..\..\src\libs\zbxsysinfo\simple}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxsysinfo\common}.c{..\..\..\src\libs\zbxsysinfo\common}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxsysinfo\win32}.c{..\..\..\src\libs\zbxsysinfo\win32}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxsysinfo\win32\perfstat}.c{..\..\..\src\libs\zbxsysinfo\win32\perfstat}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxsysinfo\win32}.cpp{..\..\..\src\libs\zbxsysinfo\win32}.o:
	$(CC) $? /Fo"$@" $(CPPFLAGS)

{..\..\..\src\libs\zbxsysinfo\alias}.c{..\..\..\src\libs\zbxsysinfo\alias}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxvariant}.c{..\..\..\src\libs\zbxvariant}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxversion}.c{..\..\..\src\libs\zbxversion}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxwin32}.c{..\..\..\src\libs\zbxwin32}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxwinservice}.c{..\..\..\src\libs\zbxwinservice}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\libs\zbxagentget}.c{..\..\..\src\libs\zbxagentget}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\zabbix_agent\active_checks}.c{..\..\..\src\zabbix_agent\active_checks}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\zabbix_agent\metrics}.c{..\..\..\src\zabbix_agent\metrics}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\zabbix_agent\active_checks\eventlog_win32}.c{..\..\..\src\zabbix_agent\active_checks\eventlog_win32}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\zabbix_agent\listener}.c{..\..\..\src\zabbix_agent\listener}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\zabbix_agent\agent_conf}.c{..\..\..\src\zabbix_agent\agent_conf}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{..\..\..\src\zabbix_agent\logfiles}.c{..\..\..\src\zabbix_agent\logfiles}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)

{$(PROJECTDIR)}.c{$(PROJECTDIR)}.o:
	$(CC) $? /Fo"$@" $(CFLAGS)
