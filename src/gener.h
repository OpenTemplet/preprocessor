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

#ifndef _REFERENCE_IMPLEMENTATION_MODULE_
#define _REFERENCE_IMPLEMENTATION_MODULE_

#include <stdio.h>

void rgPrintImplementation(FILE*,char*incfile);
void rgPrintHeader(FILE*);

void rgChannelHeader(FILE*f,char*id,char*entry,char*rem);
void rgProcessHeader(FILE*f,char*id,char*entry,char*rem);
void rgAssembleHeader(FILE*f,char*id,char*module,char*rem);

void rgChannelImpl(FILE*f,char*id,char*entry,char*rem);
void rgProcessImpl(FILE*f,char*id,char*entry,char*rem);
void rgAssembleImpl(FILE*f,char*id,char*module,char*rem);

#endif