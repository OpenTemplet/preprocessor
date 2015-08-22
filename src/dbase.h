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

#ifndef _DBASE_MODULE_
#define _DBASE_MODULE_

#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>

#define CLI 0
#define SRV 1
#define PROC 0
#define CHAN 1
#define ASM  2

struct Block{
	char* line;
	Block* next;
};

struct UserBlock{
	char*owner;
	char*section;
	int flag;
	Block* blocks;
	Block* blocks_end;
};

void  dbInitDataSet();
void  dbCleanupDataSet();
char* dbGetAtom(char*);
void  dbPrintXML(FILE*);
void  dbLoadFromXML(FILE*);

void dbSetModule(char*name,char*templet,char*rem);
bool dbGetModule(char**name,char**templet,char**rem);

void dbSetModParam(char*id,char*value);
bool dbGetModParam(int*id_num,char**id,char**value);

void dbSetObjParam(char*owner,char*id,char*value);
bool dbGetObjParam(int*id_num,char*owner,char**id,char**value);

void dbSetInclude(char*file,char*module);
bool dbGetInclude(int*id_num,char**file,char**module);

void dbSetProcess(char*id,char*entry,char*templet,char*rem);
bool dbGetProcess(int*id_num,char**id,char**entry,char**templet,char**rem);

void dbSetMethod(char*owner,char*id,int await,int x,int y,char*rem);
bool dbGetMethod(int*id_num,char*owner,char**id,int*await,int*x,int*y,char**rem);

void dbSetPort(char*owner,char*id,char*iface,char*module,int cli_srv,int x,int y,char*rem);
bool dbGetPort(int*id_num,char*owner,char**id,char**channel,char**module,int*cli_srv,int*x,int*y,char**rem);

void dbSetCondition(char*owner,char*method_from,int prior,char*id,char*method_to,int x,int y,char*rem);
bool dbGetCondition(char*owner,char*method_from,int prior,char**id,char**method_to,int*x,int*y,char**rem);

void dbSetActivate(char*owner,char*method_from,int prior,char*method_to,int x,int y,char*rem);
bool dbGetActivate(char*owner,char*method_from,int prior,char**method_to,int*x,int*y,char**rem);

void dbSetReceive(char*owner,char*port,int prior,char*id,char*method,int x,int y,char*rem);
bool dbGetReceive(char*owner,char*port,int prior,char**id,char**method,int*x,int*y,char**rem);

void dbSetSend(char*owner,char*method,int prior,char*id,char*port,int x,int y,char*rem);
bool dbGetSend(char*owner,char*method,int prior,char**id,char**port,int*x,int*y,char**rem);

void dbSetChannel(char*id,char*entry,char*templet,char*rem);
bool dbGetChannel(int*id_num,char**id,char**entry,char**templet,char**rem);

void dbSetState(char*owner,char*id,int cli_srv,int x,int y,char*rem);
bool dbGetState(int*id_num,char*owner,char**id,int*cli_srv,int*x,int*y,char**rem);

void dbSetMessage(char*owner,char*state_from,int prior,char*id,char*state_to,int x,int y,char*rem);
bool dbGetMessage(char*owner,char*state_from,int prior,char**id,char**state_to,int*x,int*y,char**rem);

void dbSetAssemble(char*id,char*templet,char*rem);
bool dbGetAssemble(int*id_num,char**id,char**templet,char**rem);

void dbSetAsmType(char*owner,int prc_int_asm,char*id,char*module);
bool dbGetAsmType(int*id_num,char*owner,int*prc_int_asm,char**id,char**module);

void dbPrintDefaultBlock(FILE*,char*owner,char*section,char*text);
bool dbPrintBlock(FILE*,char*owner,char*section);
bool dbPrintUnusedBlocks(FILE*);

UserBlock* dbGetTempet();

void dbReadBlocks(FILE*);
void dbClearBlocks();

void dbvUpdateViews();
bool dbvChannelMessage(int*id_num,char*channel,char**message);
bool dbvChannelMessageState(int*id_num,char*channel,char*message,char**state,int*cli_srv,char**rem);
bool dbvProcessMethodInMes(int*id_num,char*process,char*method,char**message,char**channel,char**module,char**port,int*cli_srv);
bool dbvProcessMethodOutMes(int*id_num,char*process,char*method,char**message,char**channel,char**module,char**port,int*cli_srv);
bool dbvProcessActivationMethod(int*id_num,char*process,char**method,int*prior);
bool dbvProcessCondition(int*id_num,char*process,char**condition,char**rem);

#endif