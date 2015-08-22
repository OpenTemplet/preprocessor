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

#ifndef _PARSER_MODULE_
#define _PARSER_MODULE_

/*
<templet> ::= {<channel> | <process>}.
<channel> ::= '~' <ident> ['=' <state> {';' <state>}] '.'.
<state> ::= ['+'] <ident> [ ('?'|'!')  [<rules>] ].
<rules> ::= <ident> '->' <ident> {'|' <ident> '->' <ident>}.
<process> ::= '*' <ident> ['=' ((<ports> [';' <actions>]) |  <actions>) ] '.'.
<ports> ::= <port> {';' <port>}.
<port> ::= <ident>':'<ident> ('?'|'!') [(<rules> ['|' '->' <ident>]) | ( '->' <ident>)].
<actions> ::= <action> {';' <action>}.
<action> ::= ['+'] <ident> '(' [<args>] ')' [('->' [<ident>] '|' <ident>) | ('->' <ident>)] .
<args> ::= <ident> ('?'|'!') <ident> {',' <ident> ('?'|'!') <ident>}.
*/

void prParseModule(char*hfile,char*cppfile,char*modname);

#endif