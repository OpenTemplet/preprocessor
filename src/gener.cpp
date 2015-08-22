/*--------------------------------------------------------------------------*/
/*  Copyright 2010-2015 Sergey Vostokin                                     */
/*                                                                          */
/*  Licensed under the Apache License, Version 2.0 (the "License");         */
/*  you may not use this file except in compliance with the License.        */
/*  You may obtain a copy of the License at                                 */
/*                                                                          */
/*  http://www.apache.org/licenses/LICENSE-2.0                              */
/*                                                                          */
/*  Unless required by applicable law or agreed to in writing, software     */
/*  distributed under the License is distributed on an "AS IS" BASIS,       */
/*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*/
/*  See the License for the specific language governing permissions and     */
/*  limitations under the License.                                          */
/*--------------------------------------------------------------------------*/

#include "dbase.h"
#include "gener.h"

#include <ctype.h>
#include <stdlib.h>

extern bool sample;
extern bool demo;

char* _rtModule()
{
	static char defrt[]="TEMPLET";
	char*rtl_atom=dbGetAtom("rtl");
	char*id,*value;
	int id_num=0;
	while(dbGetModParam(&id_num,&id,&value))
		if(id==rtl_atom)return value;
	return defrt;
}

void rgPrintHeader(FILE*f)
{
	char*module;
	char*module_templet;
	char*module_rem;

	int id_num;
	char*id,*entry,*templet,*rem,*dummy;
	char*_standard=dbGetAtom("");

	if(!dbGetModule(&module,&module_templet,&module_rem))
	{printf("Module description not found\n");exit(-1);};

	if (!dbPrintBlock(f, module, dbGetAtom("!h-copyright!")))
		dbPrintDefaultBlock(f, module, "!h-copyright!", "// put copyright info here\n");

	fprintf(f, "\n");

	fprintf(f,
		"#ifndef _TEMPLET_MODULE_%s\n"
		"#define _TEMPLET_MODULE_%s\n"
		"\n"
		"#include <string.h>\n"
		"#include <assert.h>\n"
		"\n",module,module);

	id_num=0;char*file;
	while(dbGetInclude(&id_num,&file,&dummy)){
		fprintf(f,"#include \"%s\"\n",file);
	}
	fprintf(f,"\n");

	if(!dbPrintBlock(f,module,dbGetAtom("!h-prologue!")))
		dbPrintDefaultBlock(f,module,"!h-prologue!","// reference additional headers here\n");

	fprintf(f, "\n");

	id_num=0;
	while(dbGetChannel(&id_num,&id,&entry,&templet,&rem)){
		if(templet==_standard)rgChannelHeader(f,id,entry,rem);
	}
	
	id_num=0;
	while(dbGetProcess(&id_num,&id,&entry,&templet,&rem)){
		if(templet==_standard)rgProcessHeader(f,id,entry,rem);
	}
	
	id_num=0;
	if(dbGetAssemble(&id_num,&id,&templet,&rem)){
		if(templet==_standard)rgAssembleHeader(f,id,module,rem);
	}

	if (!dbPrintBlock(f, module, dbGetAtom("!h-epilogue!")))
		dbPrintDefaultBlock(f, module, "!h-epilogue!", "\n");

	fprintf(f,"#endif\n");
}

void rgPrintImplementation(FILE*f,char*incfile)
{
	int id_num;
	char*module_templet;
	char*id,*entry,*templet,*module,*rem;
	char*_standard=dbGetAtom("");

	dbGetModule(&module,&module_templet,&rem);

	if (!dbPrintBlock(f, module, dbGetAtom("!cpp-copyright!")))
		dbPrintDefaultBlock(f, module, "!cpp-copyright!", "// put copyright info here\n");

	fprintf(f, "\n");

	if(!dbPrintBlock(f,module,dbGetAtom("!templet!"))){
	
		if(sample)dbPrintDefaultBlock(f,module,"!templet!",
			"/*\n"
			"~EmptyChannel.\n"
			"\n"
			"~Channel=\n"
			"	+InitialClientState ? Message -> ServerState | AlternativeMessage -> ServerState;\n"
			"	 ServerState  ! Message  -> ClientState;\n"
			"	 ClientState  ?.\n"
			"\n"
			"*EmptyProcess.\n"
			"\n"
			"*Process=\n"
			"	 ClientPort : ChannelType ! Message -> Method | AnotherMessage -> AnotherMethod | -> DefaultMethod;\n"
			"	 ServerPort : ChannelType ? -> DefaultMethod;\n"
			"\n"
			"	+InitialMethod()->Method;\n"
			"	 Method(ClientPort?Message)->|AnotherMethod;\n"
			"	 AnotherMethod(ClientPort?AnotherMessage,ServerPort!SendingMessage)->Method|AnotherMethod;\n"
			"	 DefaultMethod().\n"
			"\n"
				
		);else dbPrintDefaultBlock(f,module,"!templet!","// put templet spec here\n");
	}
	
	fprintf(f,
		"\n#include \"%s.h\"\n\n",incfile);

	if(!dbPrintBlock(f,module,dbGetAtom("!cpp-prologue!")))
		dbPrintDefaultBlock(f,module,"!cpp-prologue!","// put global variables and funcs here\n");

	id_num=0;
	while(dbGetChannel(&id_num,&id,&entry,&templet,&rem)){
		if(templet==_standard)rgChannelImpl(f,id,entry,rem);
	}
	
	id_num=0;
	while(dbGetProcess(&id_num,&id,&entry,&templet,&rem)){
		if(templet==_standard)rgProcessImpl(f,id,entry,rem);
	}
	
	id_num=0;
	if(dbGetAssemble(&id_num,&id,&templet,&rem)){
		if(templet==_standard)rgAssembleImpl(f,id,module,rem);
	}
	
	if(!dbPrintBlock(f,id,dbGetAtom("!cpp-epilogue!")))
	dbPrintDefaultBlock(f,id,"!cpp-epilogue!","// place your code here\n");
	fprintf(f,"\n");
}

void rgChannelHeader(FILE*f, char*id, char*entry, char*rem)
{
	fprintf(f,
		"class %s:public %s::Channel{\n"
		"public:\n"
		"\t%s(%s::Assemble*a);\n"
		"\t~%s();\n",
		id, _rtModule(), id, _rtModule(), id);

	if (!demo){
		fprintf(f,
			"public:\n"
			"\tstruct cli{%s*c;}_cli;\n"
			"\tstruct srv{%s*c;}_srv;\n", id, id);
	}
	
	fprintf(f,"public:\n");

	int id_num=0;
	char *message;int id_num1;char*state;int cs;

	while(dbvChannelMessage(&id_num,id,&message)){
		fprintf(f,"\tstruct %s{",message);
		
		fprintf(f,"//");
		id_num1=0;
		while(dbvChannelMessageState(&id_num1,id,message,&state,&cs,&rem))
			fprintf(f,"<%s> ",state);
		fprintf(f,"\n");

		if(!dbPrintBlock(f,id,message))
			dbPrintDefaultBlock(f,id,message,"// place your code here\n");

		fprintf(f,"\t};\n");
	}

	if (!demo){

		fprintf(f, "public:\n");
		id_num = 0;
		while (dbvChannelMessage(&id_num, id, &message))
			fprintf(f, "\tvoid _send_%s();\n", message);

		fprintf(f, "public:\n");
		id_num = 0;
		while (dbvChannelMessage(&id_num, id, &message))
			fprintf(f, "\t%s* _get_%s(){return &_mes_%s;}\n", message, message, message);

		fprintf(f, "private:\n");
		id_num = 0;
		while (dbvChannelMessage(&id_num, id, &message))
			fprintf(f, "\t%s::%s  _mes_%s;\n", id, message, message);

		fprintf(f, "private:\n");
		fprintf(f, "\tenum _state_Iface{\n");
		id_num = 0; int idummy;
		char*rem1, *rem2;

		if (dbGetState(&id_num, id, &state, &idummy, &idummy, &idummy, &rem1)){
			fprintf(f, "\t\t_st_%s", state);
			while (dbGetState(&id_num, id, &state, &idummy, &idummy, &idummy, &rem2)){
				fprintf(f, ",\n\t\t_st_%s", state);
				rem1 = rem2;
			}
			fprintf(f, "\n");
		}

		fprintf(f, "\t};\n");

		fprintf(f, "\tenum _sent_Iface{_no_snt");
		id_num = 0;

		while (dbvChannelMessage(&id_num, id, &message))
			fprintf(f, ",\n\t\t_snt_%s", message);

		fprintf(f, "\n\t};\n");

		fprintf(f,
			"private:\n"
			"\tenum _state_Iface _state;\n"
			"\tenum _sent_Iface _sent;\n");

		fprintf(f, "public:\n");
		id_num = 0;

		while (dbvChannelMessage(&id_num, id, &message)){
			fprintf(f, "\t// access tests for message '%s'\n", message);

			fprintf(f, "\tbool _s_in_%s(){return _active==SRV && _sent==_snt_%s;}\n", message, message);
			fprintf(f, "\tbool _c_in_%s(){return _active==CLI && _sent==_snt_%s;}\n", message, message);

			id_num1 = 0;
			fprintf(f, "\tbool _s_out_%s(){return _active==SRV && (", message);
			while (dbvChannelMessageState(&id_num1, id, message, &state, &cs, &rem))
				if (cs == SRV)fprintf(f, "_state==_st_%s||", state);
			fprintf(f, "false);}\n");

			id_num1 = 0;
			fprintf(f, "\tbool _c_out_%s(){return _active==CLI && (", message);
			while (dbvChannelMessageState(&id_num1, id, message, &state, &cs, &rem))
				if (cs == CLI)fprintf(f, "_state==_st_%s||", state);
			fprintf(f, "false);}\n");
		}
	}
	fprintf(f,"};\n\n");
}

void _printMethodPar(FILE*f,char*id,char*method)
{
	char*message,*channel,*module,*port;
	int cli_srv;
	int c=1;
	int id_num=0;
	bool comma=false;
	
	fprintf(f,"(");

	if(dbvProcessMethodInMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
		if(*module=='\0')fprintf(f,"/*in*/%s::%s*p%d",channel,message,c);
		else fprintf(f,"/*in*/%s::%s::%s*p%d",module,channel,message,c);
		c++;comma=true;
		while(dbvProcessMethodInMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
			if(*module=='\0')fprintf(f,",/*in*/%s::%s*p%d",channel,message,c);
			else fprintf(f,",/*in*/%s::%s::%s*p%d",module,channel,message,c);
			c++;
		}
	}
	id_num=0;
	while(dbvProcessMethodOutMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
		if(comma)fprintf(f,",");else comma=true;
		if(*module=='\0')fprintf(f,"/*out*/%s::%s*p%d",channel,message,c);
		else fprintf(f,"/*out*/%s::%s::%s*p%d",module,channel,message,c);
		c++;
	}

	fprintf(f,")");
}

void rgProcessHeader(FILE*f,char*id,char*entry,char*rem)
{
	fprintf(f,
		"class %s:public %s::Process{\n"
		"public:\n"
		"\t%s(%s::Assemble*a);\n"
		"\t~%s();\n",
		id,_rtModule(),id,_rtModule(),id);

	fprintf(f,"private:\n");

	fprintf(f,"\t//methods\n");

	int id_num=0,idummy;
	char*method;
	while(dbGetMethod(&id_num,id,&method,&idummy,&idummy,&idummy,&rem)){
		if(!isdigit(method[0])){
			fprintf(f,"\tbool %s",method);
			_printMethodPar(f,id,method);
			fprintf(f,";\n");
		}
	}
		
	id_num=0;char*condition;
	while(dbvProcessCondition(&id_num,id,&condition,&rem)){
		if(condition[0]!='\0' && !isdigit(condition[0])){
			fprintf(f,"\tbool %s();\n",condition);
		}
	}

	fprintf(f,"\n");
	if(!dbPrintBlock(f,id,dbGetAtom("!userdata!")))
		dbPrintDefaultBlock(f,id,"!userdata!","// place your code here\n");
	fprintf(f,"\n");
	
	id_num = 0;
	char*port, *dummy, *rem1;
	bool empty = true, comma = false;
	
	if (!demo){
		fprintf(f, "public:\n\tenum{\n");
		
		if (dbGetPort(&id_num, id, &port, &dummy, &dummy, &idummy, &idummy, &idummy, &rem)){
			fprintf(f, "\t\t_port_%s", port); comma = true;
			empty = false;
			while (dbGetPort(&id_num, id, &port, &dummy, &dummy, &idummy, &idummy, &idummy, &rem1)){
				fprintf(f, ",\n\t\t_port_%s", port);
				rem = rem1;
			}
		}

		id_num = 0;
		if (dbGetMethod(&id_num, id, &method, &idummy, &idummy, &idummy, &rem1)){
			if (comma)fprintf(f, ",\n\t\t_method_%s", method);
			else fprintf(f, "\t\t_method_%s", method);
			rem = rem1;
			empty = false;

			while (dbGetMethod(&id_num, id, &method, &idummy, &idummy, &idummy, &rem1)){
				fprintf(f, ",\n\t\t_method_%s", method);
				rem = rem1;
			}
		}
		if (!empty)fprintf(f, "\n");

		fprintf(f, "\t};\n");
	}

	id_num=0;int cli_srv;
	char *channel,*module;

	fprintf(f, "public:\n");
	
	if (!demo){

		while (dbGetPort(&id_num, id, &port, &channel, &module, &cli_srv, &idummy, &idummy, &rem)){
			if (*module == '\0'){
				fprintf(f, "\tvoid p_%s(%s::%s*p){_%s=(%s*)p->c;_%s->_%sPort=this;_%s->_%s_selector=_port_%s;}\n",
					port, channel, cli_srv == CLI ? "cli" : "srv", port, channel, port,
					cli_srv == CLI ? "cli" : "srv", port, cli_srv == CLI ? "cli" : "srv", port);

				if (cli_srv == CLI){
					fprintf(f, "\t%s* p_%s(){ _%s = !_%s ? _assemble->_regChan(_%s), new %s(_assemble) : _%s; _%s->_cliPort = this; _%s->_cli_selector = _port_%s; return _%s; }\n",
						channel, port, port, port, port, channel, port, port, port, port, port);
				}
				else{//cli_srv == SRV
					fprintf(f, "\tvoid p_%s(%s*p){ _%s = p; _%s->_srvPort = this; _%s->_srv_selector = _port_%s; }\n",
						port, channel, port, port, port, port);
				}
			}
			else{
				fprintf(f, "\tvoid p_%s(%s::%s::%s*p){_%s=(%s::%s*)p->c;_%s->_%sPort=this;_%s->_%s_selector=_port_%s;}\n",
					port, module, channel, cli_srv == CLI ? "cli" : "srv", port, module, channel, port,
					cli_srv == CLI ? "cli" : "srv", port, cli_srv == CLI ? "cli" : "srv", port);
			}
		}
	}
	else{
		while (dbGetPort(&id_num, id, &port, &channel, &module, &cli_srv, &idummy, &idummy, &rem)){
			if (*module == '\0'){
				if (cli_srv == CLI){
					fprintf(f, "\t%s* p_%s(){return 0; }\n",channel, port);
				}
				else{//cli_srv == SRV
					fprintf(f, "\tvoid p_%s(%s*p){}\n",port, channel);
				}
			}
		}
	}

	fprintf(f,
		"protected:\n"
		"\tvirtual void _run(int _selector,%s::Channel*_channel);\n",_rtModule());

	if (!demo){

		fprintf(f,
			"private:\n"
			"\t// ports\n");

		id_num = 0;

		while (dbGetPort(&id_num, id, &port, &channel, &module, &idummy, &idummy, &idummy, &rem)){
			if (*module == '\0')fprintf(f, "\t%s* _%s;\n", channel, port);
			else fprintf(f, "\t%s::%s* _%s;\n", module, channel, port);
		}

		fprintf(f, "\t// initial activator\n");
		if (*entry != '\0')fprintf(f, "\t%s::Activator* _entry;\n", _rtModule());

		id_num = 0; int prior;
		while (dbvProcessActivationMethod(&id_num, id, &method, &prior)){
			fprintf(f, "\t%s::Activator* _activate_%s_%d;\n", _rtModule(), method, prior);
		}

		id_num = 0; int await;
		while (dbGetMethod(&id_num, id, &method, &await, &idummy, &idummy, &rem1)){
			if (await > 1)fprintf(f, "\tint _await_%s;\n", method);
		}
	}
	fprintf(f,"};\n\n");
}

void rgAssembleHeader(FILE*f,char*id,char*module,char*rem)
{
	int id_num;
	int prc_int_asm;
	char*_id,*_module;
	char*_this=dbGetAtom("");

	fprintf(f,
		"class %s:public %s::Assemble{\n"
		"public:\n"
		"\t%s(int NT);\n"
		"\t~%s();\n\n",
		id,_rtModule(),id,id);

	if(!dbPrintBlock(f,id,dbGetAtom("!userdata!")))
		dbPrintDefaultBlock(f,id,"!userdata!","// place your code here\n");
	fprintf(f,"public:\n");

	id_num=0;
	while(dbGetAsmType(&id_num,id,&prc_int_asm,&_id,&_module)){
		if (!demo){
			if (prc_int_asm == PROC){
				if (*_module == '\0')
					fprintf(f, "\t%s*new_%s(){%s*p=new %s(this);_regProc(p);return p;}\n", _id, _id, _id, _id);
				else
					fprintf(f, "\t%s::%s*new_%s_%s(){%s::%s*p=new %s::%s(this);_regProc(p);return p;}\n",
					_module, _id, _module, _id, _module, _id, _module, _id);
			}
		}else{
			if (prc_int_asm == PROC){
				if (*_module == '\0')
					fprintf(f, "\t%s*new_%s(){return 0;}\n", _id, _id, _id, _id);
			}
		}
	}

	id_num=0;
	while(dbGetAsmType(&id_num,id,&prc_int_asm,&_id,&_module)){
		if (!demo){
			if (prc_int_asm == CHAN){
				if (*_module == '\0')
					fprintf(f, "\tvoid new_%s(%s::cli*&c,%s::srv*&s){%s*ch=new %s(this);_regChan(ch);\n"
					"\t\tch->_cli.c=ch->_srv.c=ch;c=&ch->_cli;s=&ch->_srv;}\n",
					_id, _id, _id, _id, _id);
				else
					fprintf(f, "\tvoid new_%s_%s(%s::%s::cli*&c,%s::%s::srv*&s){%s::%s*ch=new %s::%s(this);_regChan(ch);\n"
					"\t\tch->_cli.c=ch->_srv.c=ch;c=&ch->_cli;s=&ch->_srv;}\n",
					_module, _id, _module, _id, _module, _id, _module, _id, _module, _id);
			}
		}
	}

	fprintf(f,"};\n\n");
}

void _sendMesImpl(FILE*f,char*channel,char*message)
{
	fprintf(f,"void %s::_send_%s()\n{\n",channel,message);
	
	fprintf(f,"\tassert(");
	
	int id_num=0;char*state;int cs;char*rem;
	while(dbvChannelMessageState(&id_num,channel,message,&state,&cs,&rem)){
		fprintf(f," _state==_st_%s ||",state);
	}
	
	fprintf(f," true );\n");
	
	fprintf(f,"\t_sent=_snt_%s;\n",message);
	
	id_num=0;char*state1,*s_cs,*message1;
	int prior,idummy;
	while(dbvChannelMessageState(&id_num,channel,message,&state,&cs,&rem)){
		s_cs = (cs==CLI)? "SRV" : "CLI";
		prior=0;
		while(dbGetMessage(channel,state,prior,&message1,&state1,&idummy,&idummy,&rem)){
			if(message==message1){
				if(state!=state1)
					fprintf(f,"\tif(_state==_st_%s){_state=_st_%s;_active=RTL_%s;}\n",state,state1,s_cs);
				else
					fprintf(f,"\tif(_state==_st_%s)return;\n",state);
			}
			prior++;
		}
	}

	fprintf(f,"\t_send();\n");
	fprintf(f,"}\n\n");
}

void rgChannelImpl(FILE*f,char*id,char*entry,char*rem)
{
	fprintf(f,"//////////////////////class %s////////////////////\n",id);
	
	fprintf(f,"%s::%s(%s::Assemble*a):%s::Channel(a)\n{\n",id,id,_rtModule(),_rtModule());
	
	if (!demo){
		fprintf(f, "\t_state=_st_%s;\n", entry);
		fprintf(f, "\t_sent=_no_snt;\n");
	}

	if(!dbPrintBlock(f,id,dbGetAtom("!constructor!")))
		dbPrintDefaultBlock(f,id,"!constructor!","// place your code here\n");

	fprintf(f,"}\n\n");

	fprintf(f,"%s::~%s()\n{\n",id,id);

	if(!dbPrintBlock(f,id,dbGetAtom("!destructor!")))
		dbPrintDefaultBlock(f,id,"!destructor!","// place your code here\n");
	fprintf(f,"}\n\n");

	char*message;int id_num=0;
	
	if (!demo)
	while(dbvChannelMessage(&id_num,id,&message))
		_sendMesImpl(f,id,message);
}

void _printMethodAssert(FILE*f,char*id,char*method)
{
	char*message,*channel,*module,*port;
	int cli_srv;
	int c=1;
	int id_num=0;
	bool comma=false;
	
	fprintf(f,"\t\t\t\tres=(");
	
	if(dbvProcessMethodInMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
		fprintf(f,"_%s->_%c_in_%s()",port,(cli_srv==CLI ? 'c' : 's'),message);
		comma=true;
		while(dbvProcessMethodInMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
			fprintf(f,"&&_%s->_%c_in_%s()",port,(cli_srv==CLI ? 'c' : 's'),message);
		}
	}
	id_num=0;
	while(dbvProcessMethodOutMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
		if(comma)fprintf(f,"&&");else comma=true;
		fprintf(f,"_%s->_%c_out_%s()",port,(cli_srv==CLI ? 'c' : 's'),message);
	}

	if(!comma)fprintf(f,"true");

	fprintf(f,");\n");
}

void _printMethodActParam(FILE*f,char*id,char*method)
{
	char*message,*channel,*module,*port;
	int cli_srv;
	int c=1;
	int id_num=0;
	bool comma=false;
	
	if(dbvProcessMethodInMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
		fprintf(f,"_%s->_get_%s()",port,message);
		comma=true;
		while(dbvProcessMethodInMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
			fprintf(f,",_%s->_get_%s()",port,message);
		}
	}
	id_num=0;
	while(dbvProcessMethodOutMes(&id_num,id,method,&message,&channel,&module,&port,&cli_srv)){
		if(comma)fprintf(f,",");else comma=true;
		fprintf(f,"_%s->_get_%s()",port,message);
	}
}

void rgProcessImpl(FILE*f,char*id,char*entry,char*rem)
{
	fprintf(f,"//////////////////////class %s////////////////////\n",id);
	
	fprintf(f,"%s::%s(%s::Assemble*a):%s::Process(a)\n{\n",id,id,_rtModule(),_rtModule());

	int id_num=0,idummy;
	char*port,*dummy;

	if (!demo){
		while (dbGetPort(&id_num, id, &port, &dummy, &dummy, &idummy, &idummy, &idummy, &dummy)){
			fprintf(f, "\t_%s=0;\n", port);
		}
		if (*entry != '\0')fprintf(f, "\t_entry=_createActivator();_entry->_send(_method_%s);\n", entry);
	}

	id_num=0;int prior;char*method;int await;
	
	if (!demo){
		while (dbvProcessActivationMethod(&id_num, id, &method, &prior)){
			fprintf(f, "\t_activate_%s_%d=_createActivator();\n", method, prior);
		}

		id_num = 0; 
		while (dbGetMethod(&id_num, id, &method, &await, &idummy, &idummy, &dummy)){
			if (await > 1)fprintf(f, "\t_await_%s=0;\n", method);
		}
	}

	if(!dbPrintBlock(f,id,dbGetAtom("!constructor!")))
		dbPrintDefaultBlock(f,id,"!constructor!","// place your code here\n");

	fprintf(f,"}\n\n");

	fprintf(f,"%s::~%s()\n{\n",id,id);
	
	if (!demo){
		if (*entry != '\0')fprintf(f, "\tdelete _entry;\n");

		id_num = 0;
		while (dbvProcessActivationMethod(&id_num, id, &method, &prior)){
			fprintf(f, "\tdelete _activate_%s_%d;\n", method, prior);
		}
	}
	if(!dbPrintBlock(f,id,dbGetAtom("!destructor!")))
		dbPrintDefaultBlock(f,id,"!destructor!","// place your code here\n");

	fprintf(f,"}\n\n");

	if(!dbPrintBlock(f,id,dbGetAtom("!usercode!")))
		dbPrintDefaultBlock(f,id,"!usercode!","// place your code here\n");
	fprintf(f,"\n");

	id_num=0;
	while(dbGetMethod(&id_num,id,&method,&idummy,&idummy,&idummy,&rem)){
		if(!isdigit(method[0])){
			fprintf(f,"bool %s::%s",id,method);_printMethodPar(f,id,method);
			fprintf(f,"\n{\n");
			if(!dbPrintBlock(f,id,method))
				dbPrintDefaultBlock(f,id,method,"// place your code here\n\treturn true;\n");
			fprintf(f,"}\n\n");
		}
	}

	id_num=0;char*condition;
	while(dbvProcessCondition(&id_num,id,&condition,&rem)){
		if(condition[0]!='\0' && !isdigit(condition[0])){
			fprintf(f,"bool %s::%s()\n{\n",id,condition);
			if(!dbPrintBlock(f,id,condition))
				dbPrintDefaultBlock(f,id,condition,"// place your code here\n\treturn true;\n");
			fprintf(f,"}\n\n");
		}
	}

	id_num=0;int cli_srv;
	char *channel,*module;

	fprintf(f,"void %s::_run(int _selector,%s::Channel*_channel)\n{\n",id,_rtModule());
	fprintf(f,"\tbool res;\n");

	if(!dbPrintBlock(f,id,dbGetAtom("!run!")))
			dbPrintDefaultBlock(f,id,"!run!","// place run method interception code here\n");

	id_num=0;char*message;
	
	if (!demo){
		fprintf(f, "\tfor(;;){\n\t\tswitch(_selector){\n");

		while (dbGetPort(&id_num, id, &port, &channel, &module, &cli_srv, &idummy, &idummy, &rem)){
			fprintf(f, "\t\t\tcase _port_%s://%s\n", port, rem);

			prior = 0; bool stop_selection = false;
			while (dbGetReceive(id, port, prior, &message, &method, &idummy, &idummy, &rem)){

				if (*message == '\0'){
					fprintf(f, "\t\t\t\t_selector=_method_%s;break;\n", method);
					stop_selection = true;
				}
				else{
					if (!stop_selection)fprintf(f, "\t\t\t\tif(_%s->_%c_in_%s()){_selector=_method_%s;break;};\n",
						port, cli_srv == CLI ? 'c' : 's', message, method);
				}
				prior++;
			}
			fprintf(f, "\t\t\t\tassert(0);\n\t\t\t\treturn;\n");
		}

		id_num = 0;
		char*method_to, *port_to, *cond_id, *mes_id;

		while (dbGetMethod(&id_num, id, &method, &await, &idummy, &idummy, &rem)){
			fprintf(f, "\t\t\tcase _method_%s:\n", method);

			if (await > 1){
				fprintf(f, "\t\t\t\tif(++_await_%s!=%d)return;\n", method, await);
				fprintf(f, "\t\t\t\t_await_%s=0;\n", method);
			}

			_printMethodAssert(f, id, method);

			if (!isdigit(method[0])){
				fprintf(f, "\t\t\t\tif(res)res=%s(", method);
				_printMethodActParam(f, id, method);
				fprintf(f, ");\n");
			}

			for (prior = 0;; prior++){
				if (dbGetCondition(id, method, prior, &cond_id, &method_to, &idummy, &idummy, &rem)){
					if (cond_id[0] != '\0' && !isdigit(cond_id[0]))fprintf(f, "\t\t\t\tif(%s()){_selector=_method_%s;break;}\n", cond_id, method_to);
					else if (cond_id[0] == '1')fprintf(f, "\t\t\t\tif(res){_selector=_method_%s;break;}\n", method_to);
					else if (cond_id[0] == '0')fprintf(f, "\t\t\t\tif(!res){_selector=_method_%s;break;}\n", method_to);
					else fprintf(f, "\t\t\t\t_selector=_method_%s;break;\n", method_to);
				}
				else if (dbGetActivate(id, method, prior, &method_to, &idummy, &idummy, &rem)){
					fprintf(f, "\t\t\t\t_activate_%s_%d->_send(%s::_method_%s);\n", method, prior, id, method_to);
				}
				else if (dbGetSend(id, method, prior, &mes_id, &port_to, &idummy, &idummy, &rem)){
					fprintf(f, "\t\t\t\tif(res)_%s->_send_%s();\n", port_to, mes_id);
				}
				else break;
			}
			fprintf(f, "\t\t\t\treturn;\n");
		}

		fprintf(f, "\t\t\tdefault: assert(0); return;\n");
		fprintf(f, "\t\t}\n\t}\n}\n\n");
	}
	else{
		fprintf(f, "\n}\n\n");
	}
}

void rgAssembleImpl(FILE*f,char*id,char*module,char*rem)
{
	fprintf(f,"//////////////////////class %s////////////////////\n",id);

	fprintf(f,"%s::%s(int NT): %s::Assemble(NT)\n{\n",id,id,_rtModule());

	if(!dbPrintBlock(f,id,dbGetAtom("!constructor!")))
		dbPrintDefaultBlock(f,id,"!constructor!","// place your code here\n");
	
	fprintf(f,"}\n\n");

	fprintf(f,"%s::~%s()\n{\n",id,id);

	if(!dbPrintBlock(f,id,dbGetAtom("!destructor!")))
		dbPrintDefaultBlock(f,id,"!destructor!","// place your code here\n");
	
	fprintf(f,"}\n\n");
}