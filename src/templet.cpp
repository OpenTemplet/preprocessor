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

#define _CRT_SECURE_NO_WARNINGS

#include <tchar.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include "dbase.h"
#include "gener.h"
#include "parser.h"

#define MAXSTRLEN 1024

char modname[MAXSTRLEN]="";
bool finemod=false;
bool sample=false;
bool demo = false;

char hfile[MAXSTRLEN]="";
char cppfile[MAXSTRLEN]="";

char rtl_code[MAXSTRLEN]="";
char include_file[MAXSTRLEN]="";

void _printHeader()
{
	printf("\nUsage: tet.exe <Templet cpp file> {<option>}\n"
		"\nOptions list:\n"
		" -f -- output Templet specification for given module\n"
		" -s -- use sample specification if no Templet spec found\n"
		" -d -- generate module in compilable but unexecutable form\n"
		" -r <rtl namespace> -- override default runtime namespace\n"
		" -i <file> -- set runtime include file\n"
	);
}

bool _parseComLine(int argc, char* argv[])
{
	int c;

	if(argc==1)return false; 

	for(c=1;c<argc;c++){
		if(argv[c][0]=='-' || argv[c][0]=='/'){
			if(argv[c][1]=='f' || argv[c][1]=='F'){
				if(!finemod)finemod=true;
				else goto error;
			}else if(argv[c][1]=='r' || argv[c][1]=='R'){
				++c; if(c<argc && rtl_code[0]=='\0'){
					strcpy(rtl_code,argv[c]);}
				else goto error;
			}else if(argv[c][1]=='i' || argv[c][1]=='I'){
				++c; if(c<argc && include_file[0]=='\0'){
					strcpy(include_file,argv[c]);}
				else goto error;
			}else if(argv[c][1]=='s' || argv[c][1]=='S'){
				sample=true;
			}else if (argv[c][1] == 'd' || argv[c][1] == 'D'){
				demo = true;
			}else{
				printf("Bad flag '%c'",argv[c][1]);exit(-1);
			}
		}
		else {
				if(cppfile[0]=='\0'){
					strcpy(cppfile,argv[c]);continue;}
error:			printf("Bad parameter '%s'",argv[c]);exit(-1);
		}
	}

	char name[_MAX_FNAME+1];
	char absPath[_MAX_DIR];
	char drive[_MAX_DIR];
	char dir[_MAX_DIR];

	_fullpath(absPath, cppfile,_MAX_DIR-1);
	_splitpath(absPath,drive,dir,name,0);

	sprintf(hfile,"%s%s%s.h",drive,dir,name);
	sprintf(cppfile,"%s%s%s.cpp",drive,dir,name);
	strcpy(modname,name);
	
	return true;
}

void _processModule()
{
	FILE*f;
	char incfile[_MAX_FNAME+1];
	char absPath[_MAX_DIR];
	
	dbInitDataSet();
	
	prParseModule(hfile,cppfile,modname);

	f=fopen(hfile,"w");
	if(!f){printf("Cannot open file '%s' for writing",hfile);exit(-1);}
	rgPrintHeader(f);
	fclose(f);

	f=fopen(cppfile,"w");
	if(!f){printf("Cannot open file '%s' for writing",hfile);exit(-1);}
	
	_fullpath(absPath,cppfile,_MAX_DIR-1);
	_splitpath(absPath,0,0,incfile,0);

	rgPrintImplementation(f,incfile);
	
	if(dbPrintUnusedBlocks(f))
		printf("Warning: unused blocks found, see the end of '%s' file\n",cppfile);
	
	fclose(f);

	dbCleanupDataSet();
}

int main(int argc, char* argv[])
{
	printf("  +-----+\n");
	printf("  |  \\  |\n");
	printf("  |   \\ |    TEMPLET Preprocessor for C++ ver 1.4.1\n");
	printf("  |   / |\n");
	printf("  |  /  |     Copyright (c) 2010-2015 by Sergey Vostokin\n");
	printf("  +-----+ \n");
	printf("  |  \\  |    Visit project web site at http://templet.ssau.ru\n");
	printf("  |   \\ |\n");
	printf("  |   / |     TEMPLET is a trademark of\n");
	printf("  |  /  |     Samara State Aerospace University (www.ssau.ru)\n");
	printf("  +-----+\n");
	printf("  TEMPLET\n\n");
		
	if(_parseComLine(argc,argv)){
		_processModule();
		printf("Ok\n");
	}else
		_printHeader();
	
	return 0;
}