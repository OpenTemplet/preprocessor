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

#include <stdlib.h>
#include <search.h>
#include <string.h>

#define TABLE_RESERVE_CHUNK 10

int atomTableSize=0;
int atomTableReserved=0;
char** atomTable=0;

char* newAtom(char*);

char* moduleTemplet=0;
char* moduleName=0;
char* moduleRem=0;

#define DB_INIT(Name)\
	db##Name##=(rec##Name##*)malloc(TABLE_RESERVE_CHUNK*sizeof(rec##Name##));\
	if(db##Name##==NULL)exit(-1);\
	db##Name##Reserved=TABLE_RESERVE_CHUNK;

#define DB_BEGIN_SET(Name)\
	if(db##Name##Size==db##Name##Reserved){\
		db##Name##Reserved+=TABLE_RESERVE_CHUNK;\
		db##Name##=(rec##Name##*)realloc(db##Name##,db##Name##Reserved*sizeof(rec##Name##));\
		if(!db##Name##)exit(-1);\
	}

#define DB_SET_FIELD(Name,field) db##Name##[db##Name##Size].##field##=newAtom(##field##);
#define DB_SET_IFIELD(Name,field) db##Name##[db##Name##Size].##field##=##field##;

#define DB_END_SET(Name) db##Name##Size++; 

#define DB_FREE(Name)\
	if(db##Name##Reserved)free(db##Name##);\
	db##Name##Size=0;\
	db##Name##Reserved=0;\
	db##Name##=0;

#define DB_BEGIN_GET(Name)  if(*id_num<0||*id_num>=db##Name##Size) return false;

#define DB_GET_FIELD(Name,field) *##field##=db##Name##[*id_num].##field##;
#define DB_GET_IFIELD(Name,field) *##field##=db##Name##[*id_num].##field##;

#define DB_END_GET() (*id_num)++;return true;

#define DB_DEFINE_BEGIN(Name) struct rec##Name##{

#define DB_DEFINE_END(Name) };\
	int db##Name##Size=0;\
	int db##Name##Reserved=0;\
	struct rec##Name##* db##Name##=0;

DB_DEFINE_BEGIN(ModParam)
char*id;char*value;
DB_DEFINE_END(ModParam)

DB_DEFINE_BEGIN(ObjParam)
char*owner;char*id;char*value;
DB_DEFINE_END(ObjParam)

DB_DEFINE_BEGIN(Include)
char*file;char*module;
DB_DEFINE_END(Include)

DB_DEFINE_BEGIN(Process)
char*id;char*entry;char*templet;char*rem;
DB_DEFINE_END(Process)

DB_DEFINE_BEGIN(Method)
char*owner;char*id;int await;int x;int y;char*rem;
DB_DEFINE_END(Method)

DB_DEFINE_BEGIN(Port)
char*owner;char*id;char*channel;char*module;int cli_srv;int x;int y;char*rem;
DB_DEFINE_END(Port)

DB_DEFINE_BEGIN(Condition)
char*owner;char*method_from;int prior;char*id;char*method_to;int x;int y;char*rem;
DB_DEFINE_END(Condition)

DB_DEFINE_BEGIN(Activate)
char*owner;char*method_from;int prior;char*method_to;int x;int y;char*rem;
DB_DEFINE_END(Activate)

DB_DEFINE_BEGIN(Receive)
char*owner;char*port;int prior;char*id;char*method;int x;int y;char*rem;
DB_DEFINE_END(Receive)

DB_DEFINE_BEGIN(Send)
char*owner;char*method;int prior;char*id;char*port;int x;int y;char*rem;
DB_DEFINE_END(Send)

DB_DEFINE_BEGIN(Channel)
char*id;char*entry;char*templet;char*rem;
DB_DEFINE_END(Channel)

DB_DEFINE_BEGIN(State)
char*owner;char*id;int cli_srv;int x;int y;char*rem;
DB_DEFINE_END(State)

DB_DEFINE_BEGIN(Message)
char*owner;char*state_from;int prior;char*id;char*state_to;int x;int y;char*rem;
DB_DEFINE_END(Message)

DB_DEFINE_BEGIN(Assemble)
char*id;char*templet;char*rem;
DB_DEFINE_END(Assemble)

DB_DEFINE_BEGIN(AsmType)
char*owner;int prc_int_asm;char*id;char*module;
DB_DEFINE_END(AsmType)

#define BLOCK_SIGNATURE "/*$TET$"
#define BLOCK_DELIM '$'
#define BLOCK_SUFFIX "*/"

#define MAX_LINE 1024
char linebuff[MAX_LINE];

int userBlockSize=0;
int userBlockReserved=0;
struct UserBlock* userBlocks=0;

struct ChannelMessageView{
	char*channel;char*message;
};

int sizeChannelMessageView=0;
int reservedChannelMessageView=0;
ChannelMessageView *arrChannelMessageView=0;

void _freeChannelMessageView()
{
	sizeChannelMessageView=0;
	reservedChannelMessageView=0;
	if(arrChannelMessageView)free(arrChannelMessageView);
	arrChannelMessageView=0;
}

struct ChannelMessageStateView{
	char*channel;char*message;char*state;int cli_srv;char*rem;
};

int sizeChannelMessageStateView=0;
int reservedChannelMessageStateView=0;
ChannelMessageStateView *arrChannelMessageStateView;

void _freeChannelMessageStateView()
{
	sizeChannelMessageStateView=0;
	reservedChannelMessageStateView=0;
	if(arrChannelMessageStateView)free(arrChannelMessageStateView);
	arrChannelMessageStateView=0;
}

struct ProcessMethodInMesView{
	char*process;char*method;char*port;int cli_srv;char*message;char*channel;char*module;
};

int sizeProcessMethodInMesView=0;
int reservedProcessMethodInMesView=0;
ProcessMethodInMesView *arrProcessMethodInMesView;

void _freeProcessMethodInMesView()
{
	sizeProcessMethodInMesView=0;
	reservedProcessMethodInMesView=0;
	if(arrProcessMethodInMesView)free(arrProcessMethodInMesView);
	arrProcessMethodInMesView=0;
}

struct ProcessMethodOutMesView{
	char*process;char*method;char*port;int cli_srv;char*message;char*channel;char*module;
};

int sizeProcessMethodOutMesView=0;
int reservedProcessMethodOutMesView=0;
ProcessMethodOutMesView *arrProcessMethodOutMesView;

void _freeProcessMethodOutMesView()
{
	sizeProcessMethodOutMesView=0;
	reservedProcessMethodOutMesView=0;
	if(arrProcessMethodOutMesView)free(arrProcessMethodOutMesView);
	arrProcessMethodOutMesView=0;
}

struct ProcessActivationMethodView{
	char*process;char*method;int prior;
};

int sizeProcessActivationMethodView=0;
int reservedProcessActivationMethodView=0;
ProcessActivationMethodView *arrProcessActivationMethodView;

void _freeProcessActivationMethodView()
{
	sizeProcessActivationMethodView=0;
	reservedProcessActivationMethodView=0;
	if(arrProcessActivationMethodView)free(arrProcessActivationMethodView);
	arrProcessActivationMethodView=0;
}

struct ProcessConditionView{
	char*process;char*condition;char*rem;
};

int sizeProcessConditionView=0;
int reservedProcessConditionView=0;
ProcessConditionView *arrProcessConditionView;

void _freeProcessConditionView()
{
	sizeProcessConditionView=0;
	reservedProcessConditionView=0;
	if(arrProcessConditionView)free(arrProcessConditionView);
	arrProcessConditionView=0;
}

bool dbGetNextBlock(int*id_num,UserBlock**block)
{
	if(*id_num<0||*id_num>=userBlockSize) return false;
	*block=&userBlocks[*id_num];
	(*id_num)++;return true;
}

UserBlock* dbGetBlock(char*owner,char*section)
{
	for(int i=0;i<userBlockSize;i++)
		if(userBlocks[i].owner==owner && userBlocks[i].section==section) return &userBlocks[i]; 
	return 0;
}

UserBlock* dbGetTempet()
{
	UserBlock* ret=dbGetBlock(dbGetAtom(moduleName),dbGetAtom("!templet!"));
	return ret;
}

void dbPrintDefaultBlock(FILE*f,char*owner,char*section,char*text)
{
	fprintf(f,"%s%s%c%s%s\n",BLOCK_SIGNATURE,owner,BLOCK_DELIM,section,BLOCK_SUFFIX);
	fprintf(f,"%s",text);
	fprintf(f,"%s%s\n",BLOCK_SIGNATURE,BLOCK_SUFFIX);
}

void _PrintBlock(FILE*f,UserBlock*blk)
{
	Block* next=blk->blocks;
	fprintf(f,"%s%s%c%s%s\n",BLOCK_SIGNATURE,blk->owner,BLOCK_DELIM,blk->section,BLOCK_SUFFIX);
	while(next){
		fprintf(f,"%s",next->line);
		next=next->next;
	}
	fprintf(f,"%s%s\n",BLOCK_SIGNATURE,BLOCK_SUFFIX);
}

bool dbPrintBlock(FILE*f,char*owner,char*section)
{
	UserBlock* block=dbGetBlock(owner,section);
	if(!block) return false;
	_PrintBlock(f,block);
	block->flag=1;
	return true;
}

bool dbPrintUnusedBlocks(FILE*f)
{
	int id_num=0;
	bool unused=false;
	UserBlock*block;
	while(dbGetNextBlock(&id_num,&block))
		if(!block->flag){_PrintBlock(f,block);unused=true;}
	return unused;
}

bool _findBlockBeg(char*str,char**owner,char**section,char**reminder)
{
	char* offset1,*offset2;

	offset1=strstr(str,BLOCK_SIGNATURE);
	if(!offset1)return false;

	offset1+=strlen(BLOCK_SIGNATURE);
	offset2=strchr(offset1,BLOCK_DELIM);
	if(!offset2)return false;

	*offset2='\0';
	offset2++;
	*owner=offset1;
	offset1=strstr(offset2,BLOCK_SUFFIX);
	if(!offset1) return false;
	
	*offset1='\0';
	*section=offset2;
	offset1+=strlen(BLOCK_SUFFIX);
    *reminder=offset1+strspn(offset1," \t\n");

	return true;
}

bool _findBlockEnd(char*str,char**reminder)
{
	char*offset1;

	offset1=strstr(str,BLOCK_SIGNATURE BLOCK_SUFFIX);
	if(!offset1)return false;

	*offset1='\0';
	*reminder=str+strspn(str," \t");
	if(**reminder!='\0'){*offset1='\n';*(offset1+1)='\0';}

	return true;
}

UserBlock* _newBlock(char*owner,char*section)
{
	UserBlock* block;
	if(userBlockSize==userBlockReserved){
		userBlockReserved+=TABLE_RESERVE_CHUNK;
		userBlocks=(UserBlock*)realloc(userBlocks,userBlockReserved*sizeof(UserBlock));
		if(!userBlocks)exit(-1);
	}
	block=&userBlocks[userBlockSize];
	userBlockSize++;

	memset(block,0,sizeof(UserBlock));
	block->owner=newAtom(owner);
	block->section=newAtom(section);
	
	return block;
}

void _addLine(UserBlock*blk,char*line)
{
	Block* block=(Block*)calloc(1,sizeof(Block));
	block->line=(char*)calloc(strlen(line)+1,sizeof(char));
	strcpy(block->line,line);
	
	if(blk->blocks_end)	blk->blocks_end->next=block;
	else blk->blocks=block;
	
	blk->blocks_end=block;
}

void dbReadBlocks(FILE*f)
{
	char*owner;
	char*section;
	char*reminder;
	UserBlock* block;

	while(fgets(linebuff,MAX_LINE,f)!=NULL){
		if(_findBlockBeg(linebuff,&owner,&section,&reminder)){

			block=_newBlock(owner,section);
			if(*reminder!='\0')_addLine(block,reminder);

			while(fgets(linebuff,MAX_LINE,f)!=NULL){
				if(_findBlockEnd(linebuff,&reminder)){
					if(*reminder!='\0')_addLine(block,reminder);
					break;
				}else
					_addLine(block,linebuff);				
			}
		}
	}
}

void dbClearBlocks()
{
	Block*next,*tmp;

	if(!userBlockReserved)return;
	
	for(int i=0;i<userBlockSize;i++){
		next=userBlocks[i].blocks;
		while(next){
			tmp=next;next=next->next;
			free(tmp->line);free(tmp);
		}
	}
	free(userBlocks);

	userBlockSize=0;
	userBlockReserved=0;
	userBlocks=0;
}

void dbInitDataSet()
{
	atomTable=(char**)malloc(TABLE_RESERVE_CHUNK*sizeof(char*));
	if(atomTable==NULL)exit(-1);
	atomTableReserved=TABLE_RESERVE_CHUNK;

	userBlocks=(UserBlock*)malloc(TABLE_RESERVE_CHUNK*sizeof(UserBlock));
	if(userBlocks==NULL)exit(-1);
	userBlockReserved=TABLE_RESERVE_CHUNK;

	DB_INIT(ModParam)
	DB_INIT(ObjParam)
	DB_INIT(Include)
	DB_INIT(Process)
	DB_INIT(Method)
	DB_INIT(Port)
	DB_INIT(Condition)
	DB_INIT(Activate)
	DB_INIT(Receive)
	DB_INIT(Send)
	DB_INIT(Channel)
	DB_INIT(State)
	DB_INIT(Message)
	DB_INIT(Assemble)
	DB_INIT(AsmType)
}

void dbCleanupDataSet()
{
	int i;

	if(atomTableReserved){
		for(i=0;i<atomTableSize;i++)free(atomTable[i]);
		free(atomTable);
		atomTableSize=0;
		atomTableReserved=0;
	}

	dbClearBlocks();
	
	moduleName=0;

	DB_FREE(ModParam)
	DB_FREE(ObjParam)
	DB_FREE(Include)
	DB_FREE(Process)
	DB_FREE(Method)
	DB_FREE(Port)
	DB_FREE(Condition)
	DB_FREE(Activate)
	DB_FREE(Receive)
	DB_FREE(Send)
	DB_FREE(Channel)
	DB_FREE(State)
	DB_FREE(Message)
	DB_FREE(Assemble)
	DB_FREE(AsmType)

	_freeChannelMessageView();
	_freeChannelMessageStateView();
	_freeProcessMethodInMesView();
	_freeProcessMethodOutMesView();
	_freeProcessActivationMethodView();
	_freeProcessConditionView();
}

int __cdecl atomCmp(const void *s1, const void *s2)
{
	return strcmp(*(const char**)s1,*(const char**)s2);
}

char* dbGetAtom(char*a)
{
	char** ret=(char**)_lfind(&a,atomTable,
		(unsigned int*)(&atomTableSize),(unsigned int)sizeof(char*),
		atomCmp);
	if(ret)return *ret;else return 0;
}

char* newAtom(char*a)
{
	char* new_atom;
	new_atom=dbGetAtom(a);
	if(new_atom)return new_atom;

	if(atomTableSize==atomTableReserved){
		atomTableReserved+=TABLE_RESERVE_CHUNK;
		atomTable=(char**)realloc(atomTable,atomTableReserved*sizeof(char*));
		if(!atomTable)exit(-1);
	}
	
	new_atom=(char*)calloc(strlen(a)+1,sizeof(char));
	if(!new_atom)exit(-1);
	
	strcpy(new_atom,a);
	
	atomTable[atomTableSize]=new_atom;
	atomTableSize++;
	
	return new_atom;
}

void dbSetModule(char*name,char*templet,char*rem)
{
	moduleTemplet=newAtom(templet);
	moduleName=newAtom(name);
	moduleRem=newAtom(rem);
}

bool dbGetModule(char**name,char**templet,char**rem)
{
	if(!moduleName)return false;
	*templet=moduleTemplet;
	*name=moduleName;
	*rem=moduleRem;
	return true;
}

void dbSetModParam(char*id,char*value)
{
	DB_BEGIN_SET(ModParam)
		DB_SET_FIELD(ModParam,id)
		DB_SET_FIELD(ModParam,value)
	DB_END_SET(ModParam)
}

bool dbGetModParam(int*id_num,char**id,char**value)
{
	DB_BEGIN_GET(ModParam)
		DB_GET_FIELD(ModParam,id)
		DB_GET_FIELD(ModParam,value)
	DB_END_GET()
}

void dbSetObjParam(char*owner,char*id,char*value)
{
	DB_BEGIN_SET(ObjParam)
		DB_SET_FIELD(ObjParam,id)
		DB_SET_FIELD(ObjParam,owner)
		DB_SET_FIELD(ObjParam,value)
	DB_END_SET(ObjParam)
}

bool _dbGetObjParam(int*id_num,char**owner,char**id,char**value)
{
	DB_BEGIN_GET(ObjParam)
		DB_GET_FIELD(ObjParam,owner)
		DB_GET_FIELD(ObjParam,id)
		DB_GET_FIELD(ObjParam,value)
	DB_END_GET()
}

bool dbGetObjParam(int*id_num,char*owner,char**id,char**value)
{
	bool found=false;
	char*owner_out;
	
	while(_dbGetObjParam(id_num,&owner_out,id,value)){
		if(owner==owner_out){found=true;break;}
	}
	return found;
}

void dbSetInclude(char*file,char*module)
{
	DB_BEGIN_SET(Include)
		DB_SET_FIELD(Include,file)
		DB_SET_FIELD(Include,module)
	DB_END_SET(Include);
}

bool dbGetInclude(int*id_num,char**file,char**module)
{
	DB_BEGIN_GET(Include)
		DB_GET_FIELD(Include,file)
		DB_GET_FIELD(Include,module)
	DB_END_GET()
}

void dbSetProcess(char*id,char*entry,char*templet,char*rem)
{
	DB_BEGIN_SET(Process)
		DB_SET_FIELD(Process,id)
		DB_SET_FIELD(Process,entry)
		DB_SET_FIELD(Process,templet)
		DB_SET_FIELD(Process,rem)
	DB_END_SET(Process)
}
bool dbGetProcess(int*id_num,char**id,char**entry,char**templet,char**rem)
{
	DB_BEGIN_GET(Process)
		DB_GET_FIELD(Process,id)
		DB_GET_FIELD(Process,entry)
		DB_GET_FIELD(Process,templet)
		DB_GET_FIELD(Process,rem)
	DB_END_GET()
}

void dbSetMethod(char*owner,char*id,int await,int x,int y,char*rem)
{
	DB_BEGIN_SET(Method)
		DB_SET_FIELD(Method,owner)
		DB_SET_FIELD(Method,id)
		DB_SET_IFIELD(Method,await)
		DB_SET_IFIELD(Method,x)
		DB_SET_IFIELD(Method,y)
		DB_SET_FIELD(Method,rem)
	DB_END_SET(Method)
}

bool _dbGetMethod(int*id_num,char**owner,char**id,int* await,int* x,int* y,char**rem)
{
	DB_BEGIN_GET(Method)
		DB_GET_FIELD(Method,owner)
		DB_GET_FIELD(Method,id)
		DB_GET_IFIELD(Method,await)
		DB_GET_IFIELD(Method,x)
		DB_GET_IFIELD(Method,y)
		DB_GET_FIELD(Method,rem)
	DB_END_GET()
}

bool dbGetMethod(int*id_num,char*owner,char**id,int* await,int* x,int* y,char**rem)
{
	bool found=false;
	char*owner_out;
	
	while(_dbGetMethod(id_num,&owner_out,id,await,x,y,rem)){
		if(owner==owner_out){found=true;break;}
	}
	return found;
}

void dbSetPort(char*owner,char*id,char*channel,char*module,int cli_srv,int x,int y,char*rem)
{
	DB_BEGIN_SET(Port)
		DB_SET_FIELD(Port,owner)
		DB_SET_FIELD(Port,id)
		DB_SET_FIELD(Port,channel)
		DB_SET_FIELD(Port,module)
		DB_SET_IFIELD(Port,cli_srv)
		DB_SET_IFIELD(Port,x)
		DB_SET_IFIELD(Port,y)
		DB_SET_FIELD(Port,rem)
	DB_END_SET(Port)
}

bool _dbGetPort(int*id_num,char**owner,char**id,char**channel,char**module,int*cli_srv,int*x,int*y,char**rem)
{
	DB_BEGIN_GET(Port)
		DB_GET_FIELD(Port,owner)
		DB_GET_FIELD(Port,id)
		DB_GET_FIELD(Port,channel)
		DB_GET_FIELD(Port,module)
		DB_GET_IFIELD(Port,cli_srv)
		DB_GET_IFIELD(Port,x)
		DB_GET_IFIELD(Port,y)
		DB_GET_FIELD(Port,rem)
	DB_END_GET()
}

bool dbGetPort(int*id_num,char*owner,char**id,char**channel,char**module,int*cli_srv,int*x,int*y,char**rem)
{
	bool found=false;
	char*owner_out;
	
	while(_dbGetPort(id_num,&owner_out,id,channel,module,cli_srv,x,y,rem)){
		if(owner==owner_out){found=true;break;}
	}
	return found;
}

void dbSetCondition(char*owner,char*method_from,int prior,char*id,char*method_to,int x,int y,char*rem)
{
	DB_BEGIN_SET(Condition)
		DB_SET_FIELD(Condition,owner)
		DB_SET_FIELD(Condition,method_from)
		DB_SET_IFIELD(Condition,prior)
		DB_SET_FIELD(Condition,id)
		DB_SET_FIELD(Condition,method_to)
		DB_SET_IFIELD(Condition,x)
		DB_SET_IFIELD(Condition,y)
		DB_SET_FIELD(Condition,rem)
	DB_END_SET(Condition)
}

bool _dbGetCondition(int*id_num,char**owner,char**method_from,int*prior,char**id,char**method_to,int*x,int*y,char**rem)
{
	DB_BEGIN_GET(Condition)
		DB_GET_FIELD(Condition,owner)
		DB_GET_FIELD(Condition,method_from)
		DB_GET_IFIELD(Condition,prior)
		DB_GET_FIELD(Condition,id)
		DB_GET_FIELD(Condition,method_to)
		DB_GET_IFIELD(Condition,x)
		DB_GET_IFIELD(Condition,y)
		DB_GET_FIELD(Condition,rem)
	DB_END_GET()
}

bool dbGetCondition(char*owner,char*method_from,int prior,char**id,char**method_to,int*x,int*y,char**rem)
{
	bool found=false;
	char*owner_out;char*method_from_out;int prior_out;

	int id_num=0;
	while(_dbGetCondition(&id_num,&owner_out,&method_from_out,&prior_out,id,method_to,x,y,rem)){
		if(owner_out==owner&&method_from_out==method_from&&prior_out==prior){found=true;break;}
	}
	return found;
}

void dbSetActivate(char*owner,char*method_from,int prior,char*method_to,int x,int y,char*rem)
{
	DB_BEGIN_SET(Activate)
		DB_SET_FIELD(Activate,owner)
		DB_SET_FIELD(Activate,method_from)
		DB_SET_IFIELD(Activate,prior)
		DB_SET_FIELD(Activate,method_to)
		DB_SET_IFIELD(Activate,x)
		DB_SET_IFIELD(Activate,y)
		DB_SET_FIELD(Activate,rem)
	DB_END_SET(Activate)
}
bool _dbGetActivate(int*id_num,char**owner,char**method_from,int*prior,char**method_to,int*x,int*y,char**rem)
{
	DB_BEGIN_GET(Activate)
		DB_GET_FIELD(Activate,owner)
		DB_GET_FIELD(Activate,method_from)
		DB_GET_IFIELD(Activate,prior)
		DB_GET_FIELD(Activate,method_to)
		DB_GET_IFIELD(Activate,x)
		DB_GET_IFIELD(Activate,y)
		DB_GET_FIELD(Activate,rem)
	DB_END_GET()
}
bool dbGetActivate(char*owner,char*method_from,int prior,char**method_to,int*x,int*y,char**rem)
{
	bool found=false;
	char*owner_out;char*method_from_out;int prior_out;
	
	int id_num=0;
	while(_dbGetActivate(&id_num,&owner_out,&method_from_out,&prior_out,method_to,x,y,rem)){
		if(owner_out==owner && method_from_out==method_from && prior_out==prior){found=true;break;}
	}
	return found;
}

void dbSetReceive(char*owner,char*port,int prior,char*id,char*method,int x,int y,char*rem)
{
	DB_BEGIN_SET(Receive)
		DB_SET_FIELD(Receive,owner)
		DB_SET_FIELD(Receive,port)
		DB_SET_IFIELD(Receive,prior)
		DB_SET_FIELD(Receive,id)
		DB_SET_FIELD(Receive,method)
		DB_SET_IFIELD(Receive,x)
		DB_SET_IFIELD(Receive,y)
		DB_SET_FIELD(Receive,rem)
	DB_END_SET(Receive)
}

bool _dbGetReceive(int*id_num,char**owner,char**port,int*prior,char**id,char**method,int*x,int*y,char**rem)
{
	DB_BEGIN_GET(Receive)
		DB_GET_FIELD(Receive,owner)
		DB_GET_FIELD(Receive,port)
		DB_GET_IFIELD(Receive,prior)
		DB_GET_FIELD(Receive,id)
		DB_GET_FIELD(Receive,method)
		DB_GET_IFIELD(Receive,x)
		DB_GET_IFIELD(Receive,y)
		DB_GET_FIELD(Receive,rem)
	DB_END_GET()
}

bool dbGetReceive(char*owner,char*port,int prior,char**id,char**method,int*x,int*y,char**rem)
{
	bool found=false;
	char*owner_out;char*port_out;int prior_out;
	
	int id_num=0;
	while(_dbGetReceive(&id_num,&owner_out,&port_out,&prior_out,id,method,x,y,rem)){
		if(owner_out==owner && port_out==port && prior_out==prior){found=true;break;}
	}
	return found;
}

void dbSetSend(char*owner,char*method,int prior,char*id,char*port,int x,int y,char*rem)
{
	DB_BEGIN_SET(Send)
		DB_SET_FIELD(Send,owner)
		DB_SET_FIELD(Send,method)
		DB_SET_IFIELD(Send,prior)
		DB_SET_FIELD(Send,id)
		DB_SET_FIELD(Send,port)
		DB_SET_IFIELD(Send,x)
		DB_SET_IFIELD(Send,y)
		DB_SET_FIELD(Send,rem)
	DB_END_SET(Send)
}

bool _dbGetSend(int*id_num,char**owner,char**method,int*prior,char**id,char**port,int*x,int*y,char**rem)
{
	DB_BEGIN_GET(Send)
		DB_GET_FIELD(Send,owner)
		DB_GET_FIELD(Send,method)
		DB_GET_IFIELD(Send,prior)
		DB_GET_FIELD(Send,id)
		DB_GET_FIELD(Send,port)
		DB_GET_IFIELD(Send,x)
		DB_GET_IFIELD(Send,y)
		DB_GET_FIELD(Send,rem)
	DB_END_GET()
}

bool dbGetSend(char*owner,char*method,int prior,char**id,char**port,int*x,int*y,char**rem)
{
	bool found=false;
	char*owner_out;char*method_out;int prior_out;
	
	int id_num=0;
	while(_dbGetSend(&id_num,&owner_out,&method_out,&prior_out,id,port,x,y,rem)){
		if(owner_out==owner && method_out==method && prior_out==prior){found=true;break;}
	}
	return found;
}

void dbSetChannel(char*id,char*entry,char*templet,char*rem)
{
	DB_BEGIN_SET(Channel)
		DB_SET_FIELD(Channel,id)
		DB_SET_FIELD(Channel,entry)
		DB_SET_FIELD(Channel,templet)
		DB_SET_FIELD(Channel,rem)
	DB_END_SET(Channel)
}

bool dbGetChannel(int*id_num,char**id,char**entry,char**templet,char**rem)
{
	DB_BEGIN_GET(Channel)
		DB_GET_FIELD(Channel,id)
		DB_GET_FIELD(Channel,entry)
		DB_GET_FIELD(Channel,templet)
		DB_GET_FIELD(Channel,rem)
	DB_END_GET()
}

void dbSetState(char*owner,char*id,int cli_srv,int x,int y,char*rem)
{
	DB_BEGIN_SET(State)
		DB_SET_FIELD(State,owner)
		DB_SET_FIELD(State,id)
		DB_SET_IFIELD(State,cli_srv)
		DB_SET_IFIELD(State,x)
		DB_SET_IFIELD(State,y)
		DB_SET_FIELD(State,rem)
	DB_END_SET(State)
}

bool _dbGetState(int*id_num,char**owner,char**id,int*cli_srv,int*x,int*y,char**rem)
{
	DB_BEGIN_GET(State)
		DB_GET_FIELD(State,owner)
		DB_GET_FIELD(State,id)
		DB_GET_IFIELD(State,cli_srv)
		DB_GET_IFIELD(State,x)
		DB_GET_IFIELD(State,y)
		DB_GET_FIELD(State,rem)
	DB_END_GET()
}

bool dbGetState(int*id_num,char*owner,char**id,int*cli_srv,int*x,int*y,char**rem)
{
	bool found=false;
	char*owner_out;
	
	while(_dbGetState(id_num,&owner_out,id,cli_srv,x,y,rem)){
		if(owner_out==owner){found=true;break;}
	}
	return found;
}

void dbSetMessage(char*owner,char*state_from,int prior,char*id,char*state_to,int x,int y,char*rem)
{
	DB_BEGIN_SET(Message)
		DB_SET_FIELD(Message,owner)
		DB_SET_FIELD(Message,state_from)
		DB_SET_IFIELD(Message,prior)
		DB_SET_FIELD(Message,id)
		DB_SET_FIELD(Message,state_to)
		DB_SET_IFIELD(Message,x)
		DB_SET_IFIELD(Message,y)
		DB_SET_FIELD(Message,rem)
	DB_END_SET(Message)
}

bool _dbGetMessage(int*id_num,char**owner,char**state_from,int*prior,char**id,char**state_to,int*x,int*y,char**rem)
{
	DB_BEGIN_GET(Message)
		DB_GET_FIELD(Message,owner)
		DB_GET_FIELD(Message,state_from)
		DB_GET_IFIELD(Message,prior)
		DB_GET_FIELD(Message,id)
		DB_GET_FIELD(Message,state_to)
		DB_GET_IFIELD(Message,x)
		DB_GET_IFIELD(Message,y)
		DB_GET_FIELD(Message,rem)
	DB_END_GET()
}

bool dbGetMessage(char*owner,char*state_from,int prior,char**id,char**state_to,int*x,int*y,char**rem)
{
	bool found=false;
	char*owner_out;char*state_from_out;int prior_out;
	
	int id_num=0;
	while(_dbGetMessage(&id_num,&owner_out,&state_from_out,&prior_out,id,state_to,x,y,rem)){
		if(owner_out==owner && state_from_out==state_from && prior_out==prior){found=true;break;}
	}
	return found;
}

void dbSetAssemble(char*id,char*templet,char*rem)
{
	DB_BEGIN_SET(Assemble)
		DB_SET_FIELD(Assemble,id)
		DB_SET_FIELD(Assemble,templet)
		DB_SET_FIELD(Assemble,rem)
	DB_END_SET(Assemble)
}

bool dbGetAssemble(int*id_num,char**id,char**templet,char**rem)
{
	DB_BEGIN_GET(Assemble)
		DB_GET_FIELD(Assemble,id)
		DB_GET_FIELD(Assemble,templet)
		DB_GET_FIELD(Assemble,rem)
	DB_END_GET()
}

void dbSetAsmType(char*owner,int prc_int_asm,char*id,char*module)
{
	DB_BEGIN_SET(AsmType)
		DB_SET_FIELD(AsmType,owner)
		DB_SET_IFIELD(AsmType,prc_int_asm)
		DB_SET_FIELD(AsmType,id)
		DB_SET_FIELD(AsmType,module)
	DB_END_SET(AsmType)
}

bool _dbGetAsmType(int*id_num,char**owner,int*prc_int_asm,char**id,char**module)
{
	DB_BEGIN_GET(AsmType)
		DB_GET_FIELD(AsmType,owner)
		DB_GET_IFIELD(AsmType,prc_int_asm)
		DB_GET_FIELD(AsmType,id)
		DB_GET_FIELD(AsmType,module)
	DB_END_GET()
}

bool dbGetAsmType(int*id_num,char*owner,int*prc_int_asm,char**id,char**module)
{
	bool found=false;
	char*owner_out;
	
	while(_dbGetAsmType(id_num,&owner_out,prc_int_asm,id,module)){
		if(owner_out==owner){found=true;break;}
	}
	return found;
}

void dbPrintXML(FILE*f)
{
	int id_num;
	char*id,*entry,*templet;
	char*name,*rem;int x,y;
	char*file,*module;
	char*value;
	
	if(!dbGetModule(&name,&templet,&rem))return;
	fprintf(f,"<module id=\"%s\" templet=\"%s\" rem=\"%s\">\n\n",name,templet,rem);
	
	id_num=0;
	while(dbGetInclude(&id_num,&file,&module)){
		fprintf(f,"<include file=\"%s\" module=\"%s\"/>\n",
			file,module);
	}
	fprintf(f,"\n");

	id_num=0;
	while(dbGetModParam(&id_num,&id,&value)){
		fprintf(f,"<param id=\"%s\" value=\"%s\"/>\n",
			id,value);
	}
	fprintf(f,"\n");

	id_num=0;
	while(dbGetChannel(&id_num,&id,&entry,&templet,&rem)){
		fprintf(f,"<channel id=\"%s\" entry=\"%s\" templet=\"%s\" rem=\"%s\">\n",
			id,entry,templet,rem);
		
		int _id_num=0;
		char*_id,*_value;
		while(dbGetObjParam(&_id_num,id,&_id,&_value)){
			fprintf(f,"\t<param id=\"%s\" value=\"%s\"/>\n",_id,_value);
		}
		fprintf(f,"\n");

		_id_num=0;
		int cli_srv;
		while(dbGetState(&_id_num,id,&_id,&cli_srv,&x,&y,&rem)){
			fprintf(f,"\t<state id=\"%s\" type=\"%s\" x=\"%d\" y=\"%d\" rem=\"%s\">\n",
				_id,cli_srv==CLI?"cli":"srv",x,y,rem);

			char*state_to,*__id;
			int prior=0;
			while(dbGetMessage(id,_id,prior,&__id,&state_to,&x,&y,&rem)){
				fprintf(f,"\t\t<message id=\"%s\" state=\"%s\" x=\"%d\" y=\"%d\" rem=\"%s\"/>\n",
					__id,state_to,x,y,rem);
				prior++;
			}

			fprintf(f,"\t</state>\n");
		}
		fprintf(f,"</channel>\n\n");
	}
	
	id_num=0;
	while(dbGetProcess(&id_num,&id,&entry,&templet,&rem)){
		fprintf(f,"<process id=\"%s\" entry=\"%s\" templet=\"%s\" rem=\"%s\">\n",
			id,entry,templet,rem);

		int _id_num=0;
		char*_id,*_value;
		while(dbGetObjParam(&_id_num,id,&_id,&_value)){
			fprintf(f,"\t<param id=\"%s\" value=\"%s\"/>\n",_id,_value);
		}
		fprintf(f,"\n");

		_id_num=0;
		char* port_id,*iface,*mes_id,*to;
		int cli_srv;
		while(dbGetPort(&_id_num,id,&port_id,&iface,&module,&cli_srv,&x,&y,&rem)){
			fprintf(f,"\t<port id=\"%s\" channel=\"%s\" module=\"%s\" type=\"%s\" x=\"%d\" y=\"%d\" rem=\"%s\">\n",
				port_id,iface,module,cli_srv==CLI?"cli":"srv",x,y,rem);

			int prior=0;
			while(dbGetReceive(id,port_id,prior,&mes_id,&to,&x,&y,&rem)){
				fprintf(f,"\t\t<receive id=\"%s\" method=\"%s\" x=\"%d\" y=\"%d\" rem=\"%s\"/>\n",
						mes_id,to,x,y,rem);
				prior++;
			}

			fprintf(f,"\t</port>\n");
		}
		
		_id_num=0;
		char* meth_id,*cond_id,*method_to;
		int await;
		while(dbGetMethod(&_id_num,id,&meth_id,&await,&x,&y,&rem)){
			fprintf(f,"\t<method id=\"%s\" await=\"%d\" x=\"%d\" y=\"%d\" rem=\"%s\">\n",
				meth_id,await,x,y,rem);
			
			for(int prior=0;;prior++){
				if(dbGetCondition(id,meth_id,prior,&cond_id,&method_to,&x,&y,&rem)){
					fprintf(f,"\t\t<condition id=\"%s\" method=\"%s\" x=\"%d\" y=\"%d\" rem=\"%s\"/>\n",
						cond_id,method_to,x,y,rem);
				}else if(dbGetActivate(id,meth_id,prior,&method_to,&x,&y,&rem)){
					fprintf(f,"\t\t<activate method=\"%s\" x=\"%d\" y=\"%d\" rem=\"%s\"/>\n",
						method_to,x,y,rem);
				}else if(dbGetSend(id,meth_id,prior,&mes_id,&to,&x,&y,&rem)){
					fprintf(f,"\t\t<send id=\"%s\" port=\"%s\" x=\"%d\" y=\"%d\" rem=\"%s\"/>\n",
						mes_id,to,x,y,rem);
				}else break;
			}

			fprintf(f,"\t</method>\n");
		}
		fprintf(f,"</process>\n\n");
	}
	
	id_num=0;
	while(dbGetAssemble(&id_num,&id,&templet,&rem)){
		fprintf(f,"<assemble id=\"%s\" templet=\"%s\" rem=\"%s\">\n",
			id,templet,rem);

		int _id_num=0;
		char*_id,*_value;
		while(dbGetObjParam(&_id_num,id,&_id,&_value)){
			fprintf(f,"\t<param id=\"%s\" value=\"%s\"/>\n",_id,_value);
		}
		fprintf(f,"\n");
		
		_id_num=0;
		int prc_int_asm;
		while(dbGetAsmType(&_id_num,id,&prc_int_asm,&_id,&module)){
			switch(prc_int_asm){
				case PROC:
					fprintf(f,"\t<process id=\"%s\" module=\"%s\"/>\n",_id,module);break;
				case CHAN:
					fprintf(f,"\t<channel id=\"%s\" module=\"%s\"/>\n",_id,module);break;
				case ASM:
					fprintf(f,"\t<assemble id=\"%s\" module=\"%s\"/>\n",_id,module);break;	
			}
		}
		
		fprintf(f,"</assemble>\n\n");
	}
	fprintf(f,"</module>");
}

void _addChannelMessage(char*channel,char*message)
{
	int i;
	for(i=0;i<sizeChannelMessageView;i++){
		if(arrChannelMessageView[i].channel==channel && 
			arrChannelMessageView[i].message==message) return;
	}
	if(sizeChannelMessageView==reservedChannelMessageView){
		reservedChannelMessageView+=TABLE_RESERVE_CHUNK;
		arrChannelMessageView=(ChannelMessageView*)realloc(arrChannelMessageView,
			reservedChannelMessageView*sizeof(ChannelMessageView));
		if(!arrChannelMessageView)exit(-1);
	}
	arrChannelMessageView[sizeChannelMessageView].channel=channel; 
	arrChannelMessageView[sizeChannelMessageView].message=message;
	sizeChannelMessageView++;
}

void _addChannelMessageState(char*channel,char*message,char*state,int cli_srv,char*rem)
{
	int i;
	for(i=0;i<sizeChannelMessageStateView;i++){
		if(arrChannelMessageStateView[i].channel==channel && 
			arrChannelMessageStateView[i].message==message &&
			arrChannelMessageStateView[i].state==state) return;
	}
	if(sizeChannelMessageStateView==reservedChannelMessageStateView){
		reservedChannelMessageStateView+=TABLE_RESERVE_CHUNK;
		arrChannelMessageStateView=(ChannelMessageStateView*)realloc(arrChannelMessageStateView,
			reservedChannelMessageStateView*sizeof(ChannelMessageStateView));
		if(!arrChannelMessageStateView)exit(-1);
	}
	arrChannelMessageStateView[sizeChannelMessageStateView].channel=channel;
	arrChannelMessageStateView[sizeChannelMessageStateView].message=message;
	arrChannelMessageStateView[sizeChannelMessageStateView].state=state;
	arrChannelMessageStateView[sizeChannelMessageStateView].cli_srv=cli_srv;
	arrChannelMessageStateView[sizeChannelMessageStateView].rem=rem;
	sizeChannelMessageStateView++;
}
void _updateChannelMessageView()
{
	if(arrChannelMessageView){ 
		free(arrChannelMessageView);
		sizeChannelMessageView=0;
		reservedChannelMessageView=0;
		arrChannelMessageView=0;
	}
	
	char*dummy,*channel,*message,*state;
	int idummy;

	int id_num1=0;
	while(dbGetChannel(&id_num1,&channel,&dummy,&dummy,&dummy)){
		int id_num2=0;
		while(dbGetState(&id_num2,channel,&state,&idummy,&idummy,&idummy,&dummy)){
			int prior=0;
			while(dbGetMessage(channel,state,prior,&message,&dummy,&idummy,&idummy,&dummy)){
				_addChannelMessage(channel,message);
				prior++;
			}
		}
	}
}

void _updateChannelMessageStateView()
{
	if(arrChannelMessageStateView){ 
		free(arrChannelMessageStateView);
		sizeChannelMessageStateView=0;
		reservedChannelMessageStateView=0;
		arrChannelMessageStateView=0;
	}
	
	char*dummy,*channel,*state,*state_rem,*message;
	int idummy,cli_srv;

	int id_num1=0;
	while(dbGetChannel(&id_num1,&channel,&dummy,&dummy,&dummy)){
		int id_num2=0;
		while(dbGetState(&id_num2,channel,&state,&cli_srv,&idummy,&idummy,&state_rem)){
			int prior=0;
			while(dbGetMessage(channel,state,prior,&message,&dummy,&idummy,&idummy,&dummy)){
				_addChannelMessageState(channel,message,state,cli_srv,state_rem);
				prior++;
			}
		}
	}
}

void _addProcessMethodInMes(char*process,char*method,char*port,int cli_srv,char*message,char*channel,char*module)
{
	int i;
	for(i=0;i<sizeProcessMethodInMesView;i++){
		if(arrProcessMethodInMesView[i].process==process && 
			arrProcessMethodInMesView[i].method==method &&
			arrProcessMethodInMesView[i].port==port) return;
	}
	if(sizeProcessMethodInMesView==reservedProcessMethodInMesView){
		reservedProcessMethodInMesView+=TABLE_RESERVE_CHUNK;
		arrProcessMethodInMesView=(ProcessMethodInMesView*)realloc(arrProcessMethodInMesView,
			reservedProcessMethodInMesView*sizeof(ProcessMethodInMesView));
		if(!arrProcessMethodInMesView)exit(-1);
	}
	arrProcessMethodInMesView[sizeProcessMethodInMesView].process=process;
	arrProcessMethodInMesView[sizeProcessMethodInMesView].method=method;
	arrProcessMethodInMesView[sizeProcessMethodInMesView].port=port;
	arrProcessMethodInMesView[sizeProcessMethodInMesView].cli_srv=cli_srv;
	arrProcessMethodInMesView[sizeProcessMethodInMesView].message=message;
	arrProcessMethodInMesView[sizeProcessMethodInMesView].channel=channel;
	arrProcessMethodInMesView[sizeProcessMethodInMesView].module=module;
	
	sizeProcessMethodInMesView++;
}

void _updateProcessMethodInMesView()
{
	if(arrProcessMethodInMesView){ 
		free(arrProcessMethodInMesView);
		sizeProcessMethodInMesView=0;
		reservedProcessMethodInMesView=0;
		arrProcessMethodInMesView=0;
	}

	char*dummy,*process,*method,*port,*channel,*module,*message,*method1;
	int idummy,cli_srv;
	int id_num1=0;
	while(dbGetProcess(&id_num1,&process,&dummy,&dummy,&dummy)){
		int id_num2=0;
		while(dbGetMethod(&id_num2,process,&method,&idummy,&idummy,&idummy,&dummy)){
			int id_num3=0;
			while(dbGetPort(&id_num3,process,&port,&channel,&module,&cli_srv,&idummy,&idummy,&dummy)){			
				int prior=0;
				while(dbGetReceive(process,port,prior,&message,&method1,&idummy,&idummy,&dummy)){
					if(method==method1 && *message!='\0')
						_addProcessMethodInMes(process,method,port,cli_srv,message,channel,module);
					prior++;
				}
			}
		}
	}
}

void _addProcessMethodOutMes(char*process,char*method,char*port,int cli_srv,char*message,char*channel,char*module)
{
	int i;
	for(i=0;i<sizeProcessMethodOutMesView;i++){
		if(arrProcessMethodOutMesView[i].process==process && 
			arrProcessMethodOutMesView[i].method==method &&
			arrProcessMethodOutMesView[i].port==port) return;
	}
	if(sizeProcessMethodOutMesView==reservedProcessMethodOutMesView){
		reservedProcessMethodOutMesView+=TABLE_RESERVE_CHUNK;
		arrProcessMethodOutMesView=(ProcessMethodOutMesView*)realloc(arrProcessMethodOutMesView,
			reservedProcessMethodOutMesView*sizeof(ProcessMethodOutMesView));
		if(!arrProcessMethodOutMesView)exit(-1);
	}
	arrProcessMethodOutMesView[sizeProcessMethodOutMesView].process=process;
	arrProcessMethodOutMesView[sizeProcessMethodOutMesView].method=method;
	arrProcessMethodOutMesView[sizeProcessMethodOutMesView].port=port;
	arrProcessMethodOutMesView[sizeProcessMethodOutMesView].cli_srv=cli_srv;
	arrProcessMethodOutMesView[sizeProcessMethodOutMesView].message=message;
	arrProcessMethodOutMesView[sizeProcessMethodOutMesView].channel=channel;
	arrProcessMethodOutMesView[sizeProcessMethodOutMesView].module=module;
	
	sizeProcessMethodOutMesView++;
}

void _updateProcessMethodOutMesView()
{
	if(arrProcessMethodOutMesView){ 
		free(arrProcessMethodOutMesView);
		sizeProcessMethodOutMesView=0;
		reservedProcessMethodOutMesView=0;
		arrProcessMethodOutMesView=0;
	}

	char*dummy,*process,*method,*channel,*module,*message,*port,*port1;
	int idummy,cli_srv;

	int id_num1=0;
	while(dbGetProcess(&id_num1,&process,&dummy,&dummy,&dummy)){
		int id_num2=0;
		while(dbGetMethod(&id_num2,process,&method,&idummy,&idummy,&idummy,&dummy)){
			int prior=0;
			bool send;
			while((send=dbGetSend(process,method,prior,&message,&port,&idummy,&idummy,&dummy)) ||
				dbGetActivate(process,method,prior,&dummy,&idummy,&idummy,&dummy) ||
				dbGetCondition(process,method,prior,&dummy,&dummy,&idummy,&idummy,&dummy)){
				if(send){	
					int id_num3=0;
					while(dbGetPort(&id_num3,process,&port1,&channel,&module,&cli_srv,&idummy,&idummy,&dummy)){
						if(port==port1)
							_addProcessMethodOutMes(process,method,port,cli_srv,message,channel,module);
					}
				}
				prior++;
			}
		}
	}
}

void _addProcessActivationMethod(char*process,char*method,int prior)
{
	int i;
	for(i=0;i<sizeProcessActivationMethodView;i++){
		if(arrProcessActivationMethodView[i].process==process && 
			arrProcessActivationMethodView[i].method==method &&
			arrProcessActivationMethodView[i].prior==prior) return;
	}
	if(sizeProcessActivationMethodView==reservedProcessActivationMethodView){
		reservedProcessActivationMethodView+=TABLE_RESERVE_CHUNK;
		arrProcessActivationMethodView=(ProcessActivationMethodView*)realloc(arrProcessActivationMethodView,
			reservedProcessActivationMethodView*sizeof(ProcessActivationMethodView));
		if(!arrProcessActivationMethodView)exit(-1);
	}
	arrProcessActivationMethodView[sizeProcessActivationMethodView].process=process;
	arrProcessActivationMethodView[sizeProcessActivationMethodView].method=method;
	arrProcessActivationMethodView[sizeProcessActivationMethodView].prior=prior;
	
	sizeProcessActivationMethodView++;
}

void _updateProcessActivationMethodView()
{
	if(arrProcessActivationMethodView){ 
		free(arrProcessActivationMethodView);
		sizeProcessActivationMethodView=0;
		reservedProcessActivationMethodView=0;
		arrProcessActivationMethodView=0;
	}
	
	char*dummy,*process,*method;
	int idummy;

	int id_num1=0;
	while(dbGetProcess(&id_num1,&process,&dummy,&dummy,&dummy)){
		int id_num2=0;
		while(dbGetMethod(&id_num2,process,&method,&idummy,&idummy,&idummy,&dummy)){
			int prior=0;
			bool act;
			while((act=dbGetActivate(process,method,prior,&dummy,&idummy,&idummy,&dummy)) || 
				dbGetSend(process,method,prior,&dummy,&dummy,&idummy,&idummy,&dummy) ||
				dbGetCondition(process,method,prior,&dummy,&dummy,&idummy,&idummy,&dummy)){
					if(act)_addProcessActivationMethod(process,method,prior);
					prior++;
			}
		}
	}
}

void _addProcessConditionView(char*process,char*condition,char*rem)
{
	int i;
	for(i=0;i<sizeProcessConditionView;i++){
		if(arrProcessConditionView[i].process==process && 
			arrProcessConditionView[i].condition==condition) return;
	}
	if(sizeProcessConditionView==reservedProcessConditionView){
		reservedProcessConditionView+=TABLE_RESERVE_CHUNK;
		arrProcessConditionView=(ProcessConditionView*)realloc(arrProcessConditionView,
			reservedProcessConditionView*sizeof(ProcessConditionView));
		if(!arrProcessConditionView)exit(-1);
	}
	arrProcessConditionView[sizeProcessConditionView].process=process;
	arrProcessConditionView[sizeProcessConditionView].condition=condition;
	arrProcessConditionView[sizeProcessConditionView].rem=rem;
	
	sizeProcessConditionView++;
}

void _updateProcessConditionView()
{
	if(arrProcessConditionView){ 
		free(arrProcessConditionView);
		sizeProcessConditionView=0;
		reservedProcessConditionView=0;
		arrProcessConditionView=0;
	}
	
	int id_num=0,id_num1,prior,idummy;
	char*process,*dummy,*method,*condition,*rem;
	while(dbGetProcess(&id_num,&process,&dummy,&dummy,&dummy)){
		id_num1=0;
		while(dbGetMethod(&id_num1,process,&method,&idummy,&idummy,&idummy,&rem)){
			prior=0;
			while(dbGetActivate(process,method,prior,&dummy,&idummy,&idummy,&dummy) || 
				dbGetSend(process,method,prior,&dummy,&dummy,&idummy,&idummy,&dummy) || 
				dbGetCondition(process,method,prior,&dummy,&dummy,&idummy,&idummy,&dummy)){
					if(dbGetCondition(process,method,prior,&condition,&dummy,&idummy,&idummy,&rem))
						_addProcessConditionView(process,condition,rem);
					prior++;
			}
		}
	}
}

void dbvUpdateViews()
{
	_updateChannelMessageView();
	_updateChannelMessageStateView();
	_updateProcessMethodInMesView();
	_updateProcessMethodOutMesView();
	_updateProcessActivationMethodView();
	_updateProcessConditionView();
}

bool dbvChannelMessage(int*id_num,char*channel,char**message)
{
	if(!(0<=*id_num && *id_num <sizeChannelMessageView))return false;
	while(*id_num<sizeChannelMessageView && arrChannelMessageView[*id_num].channel!=channel)(*id_num)++;
	if(arrChannelMessageView[*id_num].channel!=channel)return false;
	*message=arrChannelMessageView[*id_num].message;
	(*id_num)++;
	return true; 
}

bool dbvChannelMessageState(int*id_num,char*channel,char*message,char**state,int*cli_srv,char**rem)
{
	if(!(0<=*id_num && *id_num <sizeChannelMessageStateView))return false;
	
	while(*id_num<sizeChannelMessageStateView && 
		!(arrChannelMessageStateView[*id_num].channel==channel &&
		arrChannelMessageStateView[*id_num].message==message) )(*id_num)++;
	
	if(!(arrChannelMessageView[*id_num].channel==channel &&
		arrChannelMessageStateView[*id_num].message==message))return false;
	
	*state=arrChannelMessageStateView[*id_num].state;
	*cli_srv=arrChannelMessageStateView[*id_num].cli_srv;
	*rem=arrChannelMessageStateView[*id_num].rem;
	(*id_num)++;
	
	return true;
}

bool dbvProcessMethodInMes(int*id_num,char*process,char*method,char**message,char**channel,char**module,char**port,int*cli_srv)
{
	if(!(0<=*id_num && *id_num <sizeProcessMethodInMesView))return false;
	
	while(*id_num<sizeProcessMethodInMesView && 
		!(arrProcessMethodInMesView[*id_num].process==process &&
		arrProcessMethodInMesView[*id_num].method==method) )(*id_num)++;
	
	if(!(arrProcessMethodInMesView[*id_num].process==process &&
		arrProcessMethodInMesView[*id_num].method==method))return false;
	
	*message=arrProcessMethodInMesView[*id_num].message;
	*channel=arrProcessMethodInMesView[*id_num].channel;
	*module=arrProcessMethodInMesView[*id_num].module;
	*port=arrProcessMethodInMesView[*id_num].port;
	*cli_srv=arrProcessMethodInMesView[*id_num].cli_srv;
	(*id_num)++;

	return true;
}

bool dbvProcessMethodOutMes(int*id_num,char*process,char*method,char**message,char**channel,char**module,char**port,int*cli_srv)
{
	if(!(0<=*id_num && *id_num <sizeProcessMethodOutMesView))return false;
	
	while(*id_num<sizeProcessMethodOutMesView && 
		!(arrProcessMethodOutMesView[*id_num].process==process &&
		arrProcessMethodOutMesView[*id_num].method==method) )(*id_num)++;
	
	if(!(arrProcessMethodOutMesView[*id_num].process==process &&
		arrProcessMethodOutMesView[*id_num].method==method))return false;
	
	*message=arrProcessMethodOutMesView[*id_num].message;
	*channel=arrProcessMethodOutMesView[*id_num].channel;
	*module=arrProcessMethodOutMesView[*id_num].module;
	*port=arrProcessMethodOutMesView[*id_num].port;
	*cli_srv=arrProcessMethodOutMesView[*id_num].cli_srv;
	(*id_num)++;

	return true;
}

bool dbvProcessActivationMethod(int*id_num,char*process,char**method,int*prior)
{
	if(!(0<=*id_num && *id_num <sizeProcessActivationMethodView))return false;
	
	while(*id_num<sizeProcessActivationMethodView && 
		!(arrProcessActivationMethodView[*id_num].process==process) )(*id_num)++;
	
	if(!(arrProcessActivationMethodView[*id_num].process==process))return false;
	
	*method=arrProcessActivationMethodView[*id_num].method;
	*prior=arrProcessActivationMethodView[*id_num].prior;
	(*id_num)++;

	return true;
}

bool dbvProcessCondition(int*id_num,char*process,char**condition,char**rem)
{
	if(!(0<=*id_num && *id_num <sizeProcessConditionView))return false;
	
	while(*id_num<sizeProcessConditionView && 
		!(arrProcessConditionView[*id_num].process==process) )(*id_num)++;
	
	if(!(arrProcessConditionView[*id_num].process==process))return false;
	
	*condition=arrProcessConditionView[*id_num].condition;
	*rem=arrProcessConditionView[*id_num].rem;
	(*id_num)++;

	return true;
}