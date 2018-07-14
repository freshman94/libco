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

#include "coctx.h"
#include <string.h>


#define ESP 0
#define EIP 1
#define EAX 2
#define ECX 3
// -----------
#define RSP 0
#define RIP 1
#define RBX 2
#define RDI 3
#define RSI 4

#define RBP 5
#define R12 6
#define R13 7
#define R14 8
#define R15 9
#define RDX 10
#define RCX 11
#define R8 12
#define R9 13


//----- --------
// 32 bit
// | regs[0]: ret |
// | regs[1]: ebx |
// | regs[2]: ecx |
// | regs[3]: edx |
// | regs[4]: edi |
// | regs[5]: esi |
// | regs[6]: ebp |
// | regs[7]: eax |  = esp
enum
{
	kEIP = 0,
	kESP = 7,
};

//-------------
// 64 bit
//low | regs[0]: r15 |
//    | regs[1]: r14 |
//    | regs[2]: r13 |
//    | regs[3]: r12 |
//    | regs[4]: r9  |
//    | regs[5]: r8  | 
//    | regs[6]: rbp |
//    | regs[7]: rdi |
//    | regs[8]: rsi |
//    | regs[9]: ret |  //ret func addr
//    | regs[10]: rdx |
//    | regs[11]: rcx | 
//    | regs[12]: rbx |
//hig | regs[13]: rsp |
enum
{
	kRDI = 7,
	kRSI = 8,
	kRETAddr = 9,
	kRSP = 13,
};

//64 bit
extern "C"
{
	extern void coctx_swap( coctx_t *,coctx_t* ) asm("coctx_swap");	//后半句asm("co..")表示在汇编代码中
																	//使用的函数名为coctx_swap
};
#if defined(__i386__)
int coctx_init( coctx_t *ctx )
{
	memset( ctx,0,sizeof(*ctx));
	return 0;
}
int coctx_make( coctx_t *ctx,coctx_pfn_t pfn,const void *s,const void *s1 )
{
	//make room for coctx_param
	char *sp = ctx->ss_sp + ctx->ss_size - sizeof(coctx_param_t);
	sp = (char*)((unsigned long)sp & -16L);

	
	coctx_param_t* param = (coctx_param_t*)sp ;
	param->s1 = s;
	param->s2 = s1;

	memset(ctx->regs, 0, sizeof(ctx->regs));

	ctx->regs[ kESP ] = (char*)(sp) - sizeof(void*);
	ctx->regs[ kEIP ] = (char*)pfn;

	return 0;
}
#elif defined(__x86_64__)
int coctx_make( coctx_t *ctx,coctx_pfn_t pfn,const void *s,const void *s1 )
{
	char *sp = ctx->ss_sp + ctx->ss_size;		// make room for coctx_param(s,s1)
	sp = (char*) ((unsigned long)sp & -16LL  );	// 16字节对齐

	memset(ctx->regs, 0, sizeof(ctx->regs));

	ctx->regs[ kRSP ] = sp - 8;

	ctx->regs[ kRETAddr] = (char*)pfn;

	/*
	ss_sp 是在堆上分配的，地址从低到高增长，而栈是从高到低增长，这里要转下

	高地址  ------  <- ss_sp + ss_size
	|pading|
	|s2    |
	|s1    |
	------  <- sp
	|void* | 这个返回地址只是预留空间，不需要填。因为 CoRoutineFunc 函数执行完了表示该协程已经跑完，将其 end 标记位置1（co->cEnd = 1）并调用 co_yield_env 切出。不需要再回到该协程来所以也不需要记录调用 CoRoutineFunc 后的返回地址了
	------  <- ctx->regs[ kRETAddr ] 这里为返回地址预留空间的目的在于：参照前言中函数调用的 stack frame layout 图。函数调用压入参数后还需要压入返回地址，这样才能按照约定 ebp + 4 读取参数1，ebp + 8 读取参数2
	|      |
	低地址  ------  <- ss_sp

	*/


	ctx->regs[ kRDI ] = (char*)s;
	ctx->regs[ kRSI ] = (char*)s1;
	return 0;
}

int coctx_init( coctx_t *ctx )
{
	memset( ctx,0,sizeof(*ctx));
	return 0;
}

#endif

