/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/


#ifndef __CO_ROUTINE_INNER_H__

#include "co_routine.h"
#include "coctx.h"
struct stCoRoutineEnv_t;
struct stCoSpec_t
{
	void *value;
};

struct stStackMem_t
{
	stCoRoutine_t* occupy_co;	// 正使用该共享栈的协程
	int stack_size;		// 该共享栈的大小
	char* stack_bp; //stack_buffer + stack_size (共享栈基址)
	char* stack_buffer;		// 共享栈的内存

};

struct stShareStack_t
{
	unsigned int alloc_idx;		// 用于分配共享栈(RoundRobin)
	int stack_size;		// 共享栈的大小
	int count;		// 共享栈的数目
	stStackMem_t** stack_array;		// 协程组
};



struct stCoRoutine_t
{
	stCoRoutineEnv_t *env;	// 协程运行的环境
	pfn_co_routine_t pfn;	// 协程所封装的回调函数
	void *arg;				// 回调函数的参数
	coctx_t ctx;			// 协程上下文，保存当前协程让出cpu时 寄存器的状态

	char cStart;			// 标志：协程上下文是否被初始化，若否，则使用coctx_make初始化ctx
	char cEnd;				// 标志： 协程回调函数是否执行完毕
	char cIsMain;			// 标志：是否是主协程
	char cEnableSysHook;	// 标志：是否使用系统函数hook
	char cIsShareStack;		// 标志：是否使用了共享栈

	void *pvEnv;			// 协程当前的系统环境变量(hook开启时使用)

	//char sRunStack[ 1024 * 128 ];
	stStackMem_t* stack_mem;	// 协程当前使用的栈内存空间 ,当使用共享栈的时候，指向共享的栈内存


	//save satck buffer while confilct on same stack_buffer;
	char* stack_sp;				// 协程切换时, 保存当前的rsp地址，即当前栈顶地址
	unsigned int save_size;		// 协程切换时, 保存的栈内存大小
	char* save_buffer;			// 协程切换时，将stack_mem拷贝到此，保存当时的栈数据

	stCoSpec_t aSpec[1024];

};



//1.env
void 				co_init_curr_thread_env();
stCoRoutineEnv_t *	co_get_curr_thread_env();

//2.coroutine
void    co_free( stCoRoutine_t * co );
void    co_yield_env(  stCoRoutineEnv_t *env );

//3.func



//-----------------------------------------------------------------------------------------------

struct stTimeout_t;
struct stTimeoutItem_t ;

stTimeout_t *AllocTimeout( int iSize );
void 	FreeTimeout( stTimeout_t *apTimeout );
int  	AddTimeout( stTimeout_t *apTimeout,stTimeoutItem_t *apItem ,uint64_t allNow );

struct stCoEpoll_t;
stCoEpoll_t * AllocEpoll();
void 		FreeEpoll( stCoEpoll_t *ctx );

stCoRoutine_t *		GetCurrThreadCo();
void 				SetEpoll( stCoRoutineEnv_t *env,stCoEpoll_t *ev );

typedef void (*pfnCoRoutineFunc_t)();

#endif

#define __CO_ROUTINE_INNER_H__
