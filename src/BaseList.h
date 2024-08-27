#ifndef __BASE_LIST_H__
#define __BASE_LIST_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <pthread.h>
#endif

#ifdef _WIN32
typedef	CRITICAL_SECTION	BaseMutex;
#else
typedef	pthread_mutex_t		BaseMutex;
#endif

typedef struct __BASE_NODE_T
{
	void			*userPtr;

	__BASE_NODE_T	*pPrev;		//上一个成员
	__BASE_NODE_T	*pNext;		//下一个成员
}BASE_NODE_T;

class BaseList
{
public:
	BaseList(void);
	virtual ~BaseList(void);

	int		Lock();
	void	Unlock();

	//添加数据,在外部申请空间
	int		Add(void *userPtr);
	//添加数据, 在内部申请空间
	int		Add(void *userPtr, int userPtrSize);

	//删除数据, 该函数内部，仅会删除节点信息, 再调用DelNode,由子类进行删除成员
	int		Del(void *userPtr);
	virtual int DelNode(void *userPtr);		//子类中该函数返回0, 才会删除对应的节点



	void	DeleteAll();


	BASE_NODE_T	*GetBaseNodeList()	{return pBaseNodeList;}
protected:
	BASE_NODE_T		*pBaseNodeList;
	BASE_NODE_T		*pBaseNodeLast;
	BaseMutex		baseMutex;
	int				baseNodeNum;

	int		Del(BASE_NODE_T *pNode);

	void	PrintList();
};



#endif
