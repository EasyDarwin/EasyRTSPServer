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

	__BASE_NODE_T	*pPrev;		//��һ����Ա
	__BASE_NODE_T	*pNext;		//��һ����Ա
}BASE_NODE_T;

class BaseList
{
public:
	BaseList(void);
	virtual ~BaseList(void);

	int		Lock();
	void	Unlock();

	//�������,���ⲿ����ռ�
	int		Add(void *userPtr);
	//�������, ���ڲ�����ռ�
	int		Add(void *userPtr, int userPtrSize);

	//ɾ������, �ú����ڲ�������ɾ���ڵ���Ϣ, �ٵ���DelNode,���������ɾ����Ա
	int		Del(void *userPtr);
	virtual int DelNode(void *userPtr);		//�����иú�������0, �Ż�ɾ����Ӧ�Ľڵ�



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
