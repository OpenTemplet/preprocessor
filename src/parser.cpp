/*--------------------------------------------------------------------------*/
/*  Copyright 2013-2015 Sergey Vostokin                                     */
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

#include "parser.h"
#include "dbase.h"

#include <list>
#include <iostream>
#include <string>

using namespace std;

struct Rule{
	string hyp;
	string con;
};
//----
struct Port{
	string name;
	string type;
	bool is_server;
	list<Rule>  rules;
	string def_rule;
};

struct Arg{
	string port;
	bool is_input;
	string message;
};

struct Oper{
	string name;
	bool is_init;
	list<Arg> args;
	string on_true;
	string on_false;
};

struct Proc{
	string name;
	list<Port>  ports;
	list<Oper>  opers;
	//------------
	string entry;
};
//----
struct State{
	string name;
	bool is_init;
	bool is_server;
	list<Rule> rules;
};

struct Chan{
	string name;
	list<State> states;
	//----------------
	string entry;
};
//----
struct SyntTree{
	list<Proc> prcs;
	list<Chan> chs;
};
//--------------------------------------
class Lexer{
public:
	Lexer(UserBlock*);
	Lexer();
	~Lexer();
	void init(UserBlock*);

	class Pos{
		friend class Lexer;
		Block*block;char*ch;
	};

	Pos getPos();
	void setPos(Pos);

	bool is_ident();
	bool is_delim();
	bool is_rule();
	bool is_end();

	bool lex(string&);
	bool next();
	bool rewind();

private:
	bool skip_blank();
private:
	Block* cur_block;
	char*  cur_char;
	UserBlock* user_block;
	string lexeme;
	bool valid;
};

Lexer::Lexer(UserBlock*blk)
{
	user_block=blk;
	cur_block=0;
	cur_char=0;
	valid=false;
}

Lexer::Lexer()
{
	user_block=0;
	cur_block=0;
	cur_char=0;
	valid=false;
}

Lexer::~Lexer()
{
}

void Lexer::init(UserBlock*blk)
{
	user_block=blk;
	cur_block=0;
	cur_char=0;
	valid=false;
}

Lexer::Pos Lexer::getPos()
{
	Pos pos; 
	pos.block=cur_block; pos.ch=cur_char;
	return pos;
}

void Lexer::setPos(Pos p)
{
	cur_block=p.block;
	cur_char=p.ch;
	valid=false;
}

bool Lexer::is_ident()
{
	return valid && !is_delim() && !is_rule();
}

bool Lexer::is_delim()
{
	return valid && lexeme.length()==1 && strchr("*=:!;.~+?|(,)",lexeme[0]);
}

bool Lexer::is_rule()
{
	return valid && lexeme=="->";
}

bool Lexer::is_end()
{
	return cur_char==0;
}

 bool Lexer::lex(string&l)
{
	if(!valid)return false;
	l=lexeme;
	return true;
}

bool Lexer::next()
{
	if(!skip_blank()){valid=false;return false;}
	
	bool ident=false;
	lexeme.clear();

	while(*cur_char!='\0'){
		if(!ident && strchr("*=:!;.~+?|(,)",*cur_char)){lexeme=(*cur_char);cur_char++;valid=true;return true;}
		else if(!ident && *cur_char=='-' && *(cur_char+1)=='>'){lexeme="->";cur_char+=2;valid=true;return true;}
		else if(!ident && (*cur_char=='_'||isalpha(*cur_char))){ident=true;lexeme+=(*cur_char);cur_char++;}
		else if(ident && (*cur_char=='_'||isalpha(*cur_char)||isdigit(*cur_char))){lexeme+=(*cur_char);cur_char++;}
		else if(ident){valid=true;return true;}
		else {cur_char=0;valid=false;return false;}
	}

	valid=false;
	return false;
}

bool Lexer::rewind()
{
	if(!user_block || !user_block->blocks)return false;
	cur_block=user_block->blocks;
	cur_char=user_block->blocks->line;
	
	return true;
}

bool Lexer::skip_blank()
{
	if(cur_char==0)return false;
	
	while(cur_block){
		while(*cur_char!='\0'){
			if(isspace(*cur_char))cur_char++;
			else if(*cur_char=='/' && *(cur_char+1)=='*'){cur_char+=2;}
			else if(*cur_char=='*' && *(cur_char+1)=='/'){cur_char+=2;}
			else return true;
		}
		cur_block=cur_block->next;
		if(cur_block)cur_char=cur_block->line;else cur_char=0;
	}
	
	return false;
}

//--------------------------------------

void print_synt_tree(SyntTree&st)
{
	for(list<Chan>::const_iterator it=st.chs.begin();it!=st.chs.end();it++){
		Chan const & chan=*it;

		cout<<"~"<<chan.name; 
		if(chan.states.empty()) cout<<".\n";
		else{
			cout<<"=\n";

			for(list<State>::const_iterator it=chan.states.begin();it!=chan.states.end();it++){
				State const & state=*it;
				cout<<'\t'<<(state.is_init?'+':' ')<<state.name<<(state.is_server?" ! ":" ? ");

				for(list<Rule>::const_iterator itr=state.rules.begin();itr!=state.rules.end();itr++){
					Rule const & rule=*itr;
					cout<<rule.hyp<<" -> "<<rule.con;

					list<Rule>::const_iterator tmp=itr;tmp++;
					if(tmp!=state.rules.end())cout<<" | ";
				}

				list<State>::const_iterator tmp=it;tmp++;
				if(tmp==chan.states.end())cout<<".\n";else cout<<";\n";
			}
		}
	}

	for(list<Proc>::const_iterator it=st.prcs.begin();it!=st.prcs.end();it++){
		Proc const & proc=*it;

		cout<<"*"<<proc.name; 
		if(proc.ports.empty() && proc.opers.empty()) cout<<".\n";
		else{
			cout<<"=\n";

			for(list<Port>::const_iterator it=proc.ports.begin();it!=proc.ports.end();it++){
				Port const & port=(*it);
				cout<<"\t "<<port.name<<" : "<<port.type<<(port.is_server?" ? ":" ! ");

				for(list<Rule>::const_iterator itr=port.rules.begin();itr!=port.rules.end();itr++){
					Rule const & rule=*itr;
					cout<<rule.hyp<<" -> "<<rule.con;

					list<Rule>::const_iterator tmp=itr;tmp++;
					if(tmp!=port.rules.end()||(tmp==port.rules.end()&&!port.def_rule.empty()))cout<<" | ";
				}

				if(!port.def_rule.empty())cout<<" -> "<<port.def_rule;

				list<Port>::const_iterator tmp=it;tmp++;
				if(tmp==proc.ports.end()&&proc.opers.empty())cout<<".";else cout<<";\n";
			}
			cout<<'\n';

			for(list<Oper>::const_iterator it=proc.opers.begin();it!=proc.opers.end();it++){
				Oper const & oper=(*it);
				cout<<"\t"<<(oper.is_init?'+':' ')<<oper.name<<"(";

				for(list<Arg>::const_iterator ita=oper.args.begin();ita!=oper.args.end();ita++){
					Arg const & arg=*ita;
					cout<<arg.port<<(arg.is_input?'?':'!')<<arg.message;

					list<Arg>::const_iterator tmp=ita;tmp++;
					if(tmp!=oper.args.end())cout<<",";
				}

				cout<<")";
				
				cout<<(!oper.on_true.empty()||!oper.on_false.empty()?"->":"")<<oper.on_true<<(!oper.on_false.empty()?"|":"")<<oper.on_false;

				list<Oper>::const_iterator tmp=it;tmp++;
				if(tmp==proc.opers.end())cout<<".\n"; else cout<<";\n";
			}
			
		}
	}
}

bool parse_rules(Lexer& lex, list<Rule>& rules)
{
	string l;
	Lexer::Pos pos[2];
	Rule rule;

	//<rules> ::= <ident> '->' <ident> {'|' <ident> '->' <ident>}.

	pos[0]=lex.getPos();

	if(lex.next() && lex.lex(rule.hyp) && lex.is_ident());else{lex.setPos(pos[0]);return false;}
	if(lex.next() && lex.lex(l) && l=="->");else{lex.setPos(pos[0]);return false;}
	if(lex.next() && lex.lex(rule.con) && lex.is_ident());else{lex.setPos(pos[0]);return false;}

	rules.push_back(rule);

	while(pos[1]=lex.getPos(),lex.next() &&
		lex.lex(l) && l=="|" &&
		lex.next() && lex.lex(rule.hyp) && lex.is_ident() && 
		lex.next() && lex.lex(l) && l=="->" &&
		lex.next() && lex.lex(rule.con) && lex.is_ident())	{	
		
		rules.push_back(rule);
	}
	lex.setPos(pos[1]);
	return true;
}

bool parse_state(Lexer& lex,State& state)
{
	string l;
	Lexer::Pos pos[3];

	state.name.clear();
	state.rules.clear();
	state.is_init=state.is_server=false;

	//<state> ::= ['+'] <ident> [ ('?'|'!')  [<rules>] ].

	pos[0]=lex.getPos();
	if(lex.next() && lex.lex(l) && l=="+")state.is_init=true;
	else {lex.setPos(pos[0]);state.is_init=false;}

	if(lex.next() && lex.lex(state.name) && lex.is_ident());
	else{lex.setPos(pos[0]);return false;}
	
	pos[1]=lex.getPos();
	if(lex.next() && lex.lex(l) && (l=="?" || l=="!")){
		state.is_server=(l=="?"?false:true);
		
		pos[2]=lex.getPos();
		if(parse_rules(lex,state.rules));
		else lex.setPos(pos[2]);
	}
	else lex.setPos(pos[1]);

	return true;
}

bool parse_channel(Lexer& lex,Chan& chan)
{
	string l;
	Lexer::Pos pos[3];
	State state;

	chan.name.clear();
	chan.states.clear();

	//<channel> ::= '~' <ident> ['=' <state> {';' <state>}] '.'.

	pos[0]=lex.getPos();
	if(lex.next() && lex.lex(l) && l=="~" && lex.next() && lex.lex(chan.name) && lex.is_ident()){
		pos[1]=lex.getPos();
		if(lex.next() && lex.lex(l) && l=="=" && parse_state(lex,state)){
			chan.states.push_back(state);

			while(pos[2]=lex.getPos(),lex.next() && lex.lex(l) && l==";" && parse_state(lex,state))
				chan.states.push_back(state);
			lex.setPos(pos[2]);

		}
		else lex.setPos(pos[1]);

		if(lex.next() && lex.lex(l) && l==".")
			return true;
		else{
			lex.setPos(pos[0]);return false;
		}
	}
	else{
		lex.setPos(pos[0]);return false;
	};
}

bool parse_port(Lexer& lex,Port& port)
{
	string l;
	Lexer::Pos pos[3];

	port.name.clear();
	port.type.clear();
	port.is_server=false;
	port.def_rule.clear();
	port.rules.clear();

	//<port> ::= <ident>':'<ident> ('?'|'!') [(<rules> ['|' '->' <ident>]) | ( '->' <ident>)].
	
	pos[0]=lex.getPos();

	if( lex.next() && lex.lex(port.name) && lex.is_ident() &&
		lex.next() && lex.lex(l) && l==":" &&
		lex.next() && lex.lex(port.type) && lex.is_ident()
		);
	else {
		lex.setPos(pos[0]);return false;
	}

	if(lex.next() && lex.lex(l) && (l=="?" || l=="!"))
		port.is_server=(l=="?");
	else{lex.setPos(pos[0]);return false;}

	pos[1]=lex.getPos();
	if(parse_rules(lex,port.rules)){
		
		pos[2]=lex.getPos();
		if( lex.next() && lex.lex(l) && l=="|" && 
			lex.next() && lex.lex(l) && l=="->" && 
			lex.next() && lex.lex(port.def_rule));
		else lex.setPos(pos[2]);
	}
	else{
		lex.setPos(pos[1]);
		if( lex.next() && lex.lex(l) && l=="->" &&
			lex.next() && lex.lex(port.def_rule) && lex.is_ident());
		else lex.setPos(pos[1]);
	}

	return true;
}

bool parse_ports(Lexer& lex,list<Port>& ports)
{
	string l;
	Lexer::Pos pos[2];
	Port port;

	ports.clear();
	//<ports> ::= <port> {';' <port>}.
	
	pos[0]=lex.getPos();
	if(parse_port(lex,port))ports.push_back(port);
	else{lex.setPos(pos[0]);return false;}

	while(pos[1]=lex.getPos(),lex.next() && lex.lex(l) && l==";" && parse_port(lex,port))ports.push_back(port);
	lex.setPos(pos[1]);

	return true;
}

bool parse_args(Lexer& lex,list<Arg>& args)
{
	string l;
	Lexer::Pos pos[3];
	Arg arg;

	args.clear();
	//<args> ::= <ident> ('?'|'!') <ident> {',' <ident> ('?'|'!') <ident>}.
	
	pos[0]=lex.getPos();
	if( lex.next() && lex.lex(arg.port) && lex.is_ident() &&
		lex.next() && lex.lex(l) && (l=="?" || l=="!") &&
		lex.next() && lex.lex(arg.message) && lex.is_ident()) {
		
		arg.is_input=l=="?";
		args.push_back(arg);
	}
	else {
		lex.setPos(pos[0]);return false;
	}

	while(pos[0]=lex.getPos(),lex.next() && lex.lex(l) && l=="," &&
			lex.next() && lex.lex(arg.port) && lex.is_ident() &&
			lex.next() && lex.lex(l) && (l=="?" || l=="!") &&
			lex.next() && lex.lex(arg.message) && lex.is_ident()) {arg.is_input=l=="?"; args.push_back(arg);}
	lex.setPos(pos[0]);

	return true;
}

bool parse_action(Lexer& lex,Oper& oper)
{
	string l;
	Lexer::Pos pos[4];

	oper.is_init=false;
	oper.name.clear();
	oper.args.clear();
	oper.on_true.clear();
	oper.on_false.clear();

	//<action> ::= ['+'] <ident> '(' [<args>] ')' ['->'( [<ident>] '|' <ident>) | <ident>)] .

	pos[0]=lex.getPos();
	if(lex.next() && lex.lex(l) && l=="+")oper.is_init=true;
	else {oper.is_init=false;lex.setPos(pos[0]);}

	pos[3]=lex.getPos();
	if( lex.next() && lex.lex(oper.name) && lex.is_ident() &&
		lex.next() && lex.lex(l) && l=="(" &&
		parse_args(lex,oper.args) &&
		lex.next() && lex.lex(l) && l==")");
	else {
		lex.setPos(pos[3]);
		if(lex.next() && lex.lex(oper.name) && lex.is_ident() &&
			lex.next() && lex.lex(l) && l=="(" && 
			lex.next() && lex.lex(l) && l==")");
		else{lex.setPos(pos[0]);return false;}
	}

	pos[1]=lex.getPos();
	if(lex.next() && lex.lex(l) && l=="->"){
		
		pos[2]=lex.getPos();
		
		if( lex.next() && lex.lex(oper.on_true) && lex.is_ident() &&
			lex.next() && lex.lex(l) && l=="|" &&
			lex.next() && lex.lex(oper.on_false) && lex.is_ident());
		else{
			lex.setPos(pos[2]);
			if( lex.next() && lex.lex(l) && l=="|" &&
				lex.next() && lex.lex(oper.on_false) && lex.is_ident())oper.on_true.clear();
			else{
				lex.setPos(pos[2]);
				if(lex.next() && lex.lex(oper.on_true) && lex.is_ident())oper.on_false.clear();
				else{ lex.setPos(pos[0]);return false;}
			}
		}
	}
	else{
		lex.setPos(pos[1]);
	}

	return true;
}

bool parse_actions(Lexer& lex,list<Oper>& opers)
{
	string l;
	Lexer::Pos pos[2];
	Oper oper;

	opers.clear();
	//<actions> ::= <action> {';' <action>}.

	pos[0]=lex.getPos();
	if(parse_action(lex,oper))opers.push_back(oper);
	else{lex.setPos(pos[0]);return false;}

	while(pos[1]=lex.getPos(),lex.next() && lex.lex(l) && l==";" && parse_action(lex,oper))opers.push_back(oper);
	lex.setPos(pos[1]);

	return true;
}

bool parse_process(Lexer& lex,Proc& proc)
{
	string l;
	Lexer::Pos pos[4];

	proc.name.clear();
	proc.ports.clear();
	proc.opers.clear();

	//<process> ::= '*' <ident> ['=' ((<ports> [';' <actions>]) |  <actions>) ] '.'.
	
	pos[0]=lex.getPos();
	if(lex.next() && lex.lex(l) && l=="*" && lex.next() && lex.lex(proc.name) && lex.is_ident()){
		
		pos[1]=lex.getPos();
		if(lex.next() && lex.lex(l) && l=="="){
			
			pos[2]=lex.getPos();
			if(parse_ports(lex,proc.ports)){
				pos[3]=lex.getPos();
				if(lex.next() && lex.lex(l) && l==";" && parse_actions(lex,proc.opers));
				else lex.setPos(pos[3]);
			}
			else{
				if(parse_actions(lex,proc.opers));
				else{lex.setPos(pos[0]);return false;}
			}
		}
		else lex.setPos(pos[1]);

		if(lex.next() && lex.lex(l) && l==".")return true;
		else {lex.setPos(pos[0]);return false;}
	}
	else{
		lex.setPos(pos[0]);return false;
	};
}

bool parse_templet(Lexer& lex,SyntTree& tree)
{
	Lexer::Pos pos[1];
	Chan chan;
	Proc proc;

	lex.rewind();
	
	tree.chs.clear();
	tree.prcs.clear();

	//<templet> ::= {<channel> | <process>}.
	
	bool goon=true;
	while(goon){
		pos[0]=lex.getPos();
		goon=false;
		if(parse_process(lex,proc)){
			tree.prcs.push_back(proc);
			goon=true;
		}
		else{
			lex.setPos(pos[0]);
			if(parse_channel(lex,chan)){
				tree.chs.push_back(chan);
				goon=true;
			}
		}
	}
	lex.setPos(pos[0]);

	lex.next();
	return	lex.is_end();
}

bool check_chen_entry(SyntTree& st,ostream& o)
{
	bool res=true;
	for(list<Chan>::iterator it=st.chs.begin();it!=st.chs.end();it++){
		Chan & chan=*it;
		bool no_entry=true;
		for(list<State>::const_iterator it=chan.states.begin();it!=chan.states.end();it++){
			State const & state=*it;
			if(no_entry && state.is_init){ no_entry=false; chan.entry=state.name;}
			else if(!no_entry && state.is_init){o<<"semantic error: channel '"<<chan.name<<"' has more then one '"<<state.name<<"' initial state\n";res=false;}
		}
		if(no_entry && !chan.states.empty()){o<<"semantic error: channel '"<<chan.name<<"' has no entry state\n";res=false;}
	}
	return res;
}

bool check_proc_entry(SyntTree& st,ostream& o)
{
	bool res=true;

	for(list<Proc>::iterator it=st.prcs.begin();it!=st.prcs.end();it++){
		Proc & proc=*it;
		bool no_entry=true;
		for(list<Oper>::const_iterator it=proc.opers.begin();it!=proc.opers.end();it++){
			Oper const & oper=(*it);
			if(no_entry && oper.is_init){no_entry=false; proc.entry=oper.name;}
			else if(!no_entry && oper.is_init){res=false;o<<"semantic error: process'"<<proc.name<<"' has more then one initial operation '"<<oper.name<<"'\n";}
		}
	}

	return res;
}

bool check_proc_message(SyntTree& st,ostream& o)
{
	bool res=true;

	for(list<Proc>::const_iterator it=st.prcs.begin();it!=st.prcs.end();it++){
		Proc const & proc=*it;
		for(list<Oper>::const_iterator it=proc.opers.begin();it!=proc.opers.end();it++){
			Oper const & oper=(*it);
			for(list<Arg>::const_iterator it=oper.args.begin();it!=oper.args.end();it++){
				Arg const & arg=*it;
				if(arg.is_input){
					for(list<Port>::const_iterator it=proc.ports.begin();it!=proc.ports.end();it++){
						Port const & port=(*it);
						if(port.name==arg.port){
							for(list<Rule>::const_iterator it=port.rules.begin();it!=port.rules.end();it++){
								Rule const & rule=*it;
								if(rule.hyp==arg.message && rule.con==oper.name)goto NEXT_ARG;
							}
						}
					}
					res=false;
					o<<"semantic error: in process '"<<proc.name<<"' no input message for operation "<<oper.name<<"( "<<arg.port<<"?"<<arg.message<<" )\n";
				}
NEXT_ARG:		;
			}
		}
	}
	return res;
}

void check_port_message(SyntTree& st,ostream& o)
{
	for(list<Proc>::const_iterator it=st.prcs.begin();it!=st.prcs.end();it++){
		Proc const & proc=*it;
		for(list<Port>::const_iterator it=proc.ports.begin();it!=proc.ports.end();it++){
			Port const & port=(*it);
			for(list<Rule>::const_iterator it=port.rules.begin();it!=port.rules.end();it++){
				Rule const & rule=*it;
				for(list<Oper>::const_iterator it=proc.opers.begin();it!=proc.opers.end();it++){
					Oper const & oper=(*it);
					if(oper.name==rule.con){
						bool no_input=true;
						for(list<Arg>::const_iterator it=oper.args.begin();it!=oper.args.end();it++){
							Arg const & arg=*it;
							if(arg.is_input && arg.message==rule.hyp)no_input=false;
						}
						if(no_input){cout<<"semantic warning: message '"<<rule.hyp<<"' is send to method '"<<oper.name<<"' form port '"<<port.name<<"' but not read\n";}
					}
				}
			}
		}
	}
}

bool check_all(SyntTree& t,ostream& o)
{
	bool res=true;
	if(!check_chen_entry(t,o))res=false;
	if(!check_proc_entry(t,o))res=false;
	if(!check_proc_message(t,o))res=false;
	check_port_message(t,o);
	return res;
}

#define MAXSTRLEN 1024
extern char rtl_code[MAXSTRLEN];
extern char include_file[MAXSTRLEN];

void save_to_db(SyntTree& st,char*modname)
{
	if(rtl_code[0]!='\0')dbSetModParam("rtl",rtl_code);
	if(include_file[0]!='\0')dbSetInclude(include_file,rtl_code);

	for(list<Chan>::const_iterator it=st.chs.begin();it!=st.chs.end();it++){
		Chan const & chan=*it;
		dbSetChannel((char*)chan.name.c_str(),(char*)chan.entry.c_str(),"","");

		for(list<State>::const_iterator it=chan.states.begin();it!=chan.states.end();it++){
			State const & state=*it;
			dbSetState((char*)chan.name.c_str(),(char*)state.name.c_str(),state.is_server?SRV:CLI,0,0,"");

			int i=0;
			for(list<Rule>::const_iterator it=state.rules.begin();it!=state.rules.end();it++,i++){
				Rule const & rule=*it;
				dbSetMessage((char*)chan.name.c_str(),(char*)state.name.c_str(),i,(char*)rule.hyp.c_str(),(char*)rule.con.c_str(),0,0,"");
			}
		}
	}
	for(list<Proc>::const_iterator it=st.prcs.begin();it!=st.prcs.end();it++){
		Proc const & proc=*it;
		dbSetProcess((char*)proc.name.c_str(),(char*)proc.entry.c_str(),"","");

		for(list<Port>::const_iterator it=proc.ports.begin();it!=proc.ports.end();it++){
				Port const & port=(*it);
				dbSetPort((char*)proc.name.c_str(),(char*)port.name.c_str(),(char*)port.type.c_str(),"",port.is_server?SRV:CLI,0,0,"");

				int i=0;
				for(list<Rule>::const_iterator it=port.rules.begin();it!=port.rules.end();it++){
					Rule const & rule=*it;
					dbSetReceive((char*)proc.name.c_str(),(char*)port.name.c_str(),i,(char*)rule.hyp.c_str(),(char*)rule.con.c_str(),0,0,"");
					i++;
				}
				if(!port.def_rule.empty())
					dbSetReceive((char*)proc.name.c_str(),(char*)port.name.c_str(),i,"",(char*)port.def_rule.c_str(),0,0,"");
		}

		for(list<Oper>::const_iterator it=proc.opers.begin();it!=proc.opers.end();it++){
			Oper const & oper=(*it);
			dbSetMethod((char*)proc.name.c_str(),(char*)oper.name.c_str(),1,0,0,"");

			int i=0;
			for(list<Arg>::const_iterator it=oper.args.begin();it!=oper.args.end();it++){
				Arg const & arg=*it;
				if(!arg.is_input){
					dbSetSend((char*)proc.name.c_str(),(char*)oper.name.c_str(),i,(char*)arg.message.c_str(),(char*)arg.port.c_str(),0,0,"");
					i++;
				}
			}
			if(!oper.on_true.empty()){
				dbSetCondition((char*)proc.name.c_str(),(char*)oper.name.c_str(),i,"1",(char*)oper.on_true.c_str(),0,0,"");
				i++;
			}
			if(!oper.on_false.empty()){
				dbSetCondition((char*)proc.name.c_str(),(char*)oper.name.c_str(),i,"0",(char*)oper.on_false.c_str(),0,0,"");
			}
		}
	}

	dbSetAssemble(modname,"","");
	for(list<Chan>::const_iterator it=st.chs.begin();it!=st.chs.end();it++){
		Chan const & chan=*it;
		dbSetAsmType(modname,CHAN,(char*)chan.name.c_str(),"");
	}
	for(list<Proc>::const_iterator it=st.prcs.begin();it!=st.prcs.end();it++){
		Proc const & proc=*it;
		dbSetAsmType(modname,PROC,(char*)proc.name.c_str(),"");
	}
}

extern bool finemod;

void prParseModule(char*hfile,char*cppfile,char*modname)
{
	FILE*f;
	UserBlock* tet;

	f=fopen(hfile,"r");
	if(f){dbReadBlocks(f);fclose(f);}
	else{cout<<"warning: can't open h-file '"<<hfile<<"'\n";}

	f=fopen(cppfile,"r");
	if(f){dbReadBlocks(f);fclose(f);}
	else{
		cout<<"warning: can't open cpp-file '"<<cppfile<<"'\n";
	}

	dbSetModule(modname,"","");
	
	tet=dbGetTempet();
	
	if(tet){
//		bool res;
		Lexer lex(tet);
		SyntTree tree;

		if(!parse_templet(lex,tree)){
			cout<<"syntax error in templet block\n";
			exit(-1);
		}

		if(finemod)print_synt_tree(tree);

		if(!check_all(tree,cout))exit(-1);

		save_to_db(tree,modname);
	}

	dbvUpdateViews();
}