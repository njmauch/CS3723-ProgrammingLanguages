/*****************************************************************************
cs3723p1.c by Nathan Mauch
Purpose:
	This program is given a heap of memory and will allocate memory for customers
	and items to be added to the heap.  It will then associate items with customers 
	and customers with other customers.  When called upone by the driver, it will 
	work through the heap and mark all of the nodes for collection, then will follow
	all in use nodes as 'I'.  Once mark and follow is done program will then work through
	heap garbage collecting any non in use node and adding to the free list to have 
	new nodes added.
Command Paramaters:
	p1 -i inputFile > outputFile
		The input file contains commands which tells the driver what to
		use when testing the hexDump function.
Input:
	Standard input file with customer records and inventory records
	Sample data:
		ALLOC C111 Customer 111,Sal A Mander,NULL,NULL,100.00
		ASSOC C111 pFirstItem PPF001
		ALLOC SBB001 LineItem SBB001,2,19.95,NULL
Results:
	A heap with in use nodes and free nodes
Returns:
	#define RC_NOT_AVAIL 901            // There isn't any free memory to handle alloc request
	#define RC_INVALID_ADDR 903         // Invalid address which isn't within heap
	#define RC_ASSOC_ATTR_NOT_PTR 801   // Attribute name for ASSOC not a pointer attribute
	#define RC_ASSOC_ATTR_NOT_FOUND 802 // Attribute name for ASSOC not found for the from node
	0									Okay
Notes:
******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cs3723p1.h"

FreeNode *search(GCManager *pMgr, int shSize, FreeNode **ppPrec);
void addToFreeList(GCManager *pMgr, FreeNode *pF, int shRemaining);
void removeFreeNode(GCManager *pMgr, FreeNode *p, FreeNode **ppPrec);
/***************************** gcInit ***************************************
void gcInit(GCManager *pMgr)
Purpose:
	Initializes the heap to be set to all 0 values, points the free head
	to the beginning of the storage, sets cGC to 'F' for free node.
Parameters:
	O GCManager *pMgr  Provides metadata about the user data and
					   information for storage management.
Notes:
	Sets node size of pFreeHead and sets pFreeNext to NULL
*******************************************************************************/
void gcInit(GCManager *pMgr)
{
	pMgr->pFreeHead = (FreeNode*)pMgr->pBeginStorage;
	memset(pMgr->pFreeHead, 0, pMgr->iHeapSize);
	pMgr->pFreeHead->cGC = 'F';
	pMgr->pFreeHead->shNodeSize = pMgr->iHeapSize;
	pMgr->pFreeHead->pFreeNext = NULL;
}
/***************************** gcAllocate ***************************************
void * gcAllocate(GCManager *pMgr, short shDataSize, short shNodeType, char sbData[], GCResult *pGCResult)
Purpose:
	Allocates memory for nodes to be added to the heap from the free list.
Parameters:
	O GCManager *pMgr      Provides metadata about the user data and
					       information for storage management.
	I short     shDataSize The size of the data to be stored into the InUseNode in bytes.
	I short     shNodeType The type of node, 0 for customer or 1 for item
	I char      sbData	   The data to be stored in the node
	O GCResult  *pGCResult the result of the function and stores error message if necessary.
						   0 for okay.
Notes:
	Returns pointer to the start of the users data without the overhead space.
*******************************************************************************/
void * gcAllocate(GCManager *pMgr, short shDataSize, short shNodeType, char sbData[], GCResult *pGCResult)
{
	int shSize;
	int shRemaining;
	FreeNode *p, *pPrec;
	InUseNode *pInUseNode;
	void *pUserData = NULL;

	//Size of the node plus the over head 
	shSize = shDataSize + NODE_OVERHEAD_SZ;
	//Finds the next pointer in the free list with enough space to fit the node
	p = search(pMgr, shSize, &pPrec);
	//If not able to find a big enough free node
	if (p == NULL)
	{
		sprintf(pGCResult->szErrorMessage, "Could not allocate %hi bytes", shDataSize);
		pGCResult->rc = RC_NOT_AVAIL;
		return NULL;
	}
	//Remove that free node from the free list since it is now in use
	removeFreeNode(pMgr, p, &pPrec);

	//Set node to InUse and changed status to 'I', initialize node type and insert user data
	pInUseNode = (InUseNode*)p;
	pInUseNode->cGC = 'I';
	pInUseNode->shNodeType = shNodeType;
	memcpy(pInUseNode->sbData, sbData, shDataSize);

	//Determine the remaining space on free node after InUseNode is allocated to that space.
	shRemaining = p->shNodeSize - shSize;

	//If remaining space is larger than minimum node size add remaining space to the free list
	if (shRemaining >= pMgr->iMinimumNodeSize)
	{
		FreeNode *pF = (FreeNode *)(((char*)p) + shSize);
		addToFreeList(pMgr, pF, shRemaining);
		p->shNodeSize = shSize;
	}
	//Set result to 0 for okay and then set pointer to beginning of the user data
	pGCResult->rc = 0;
	pUserData = (void*)((char*)pInUseNode + NODE_OVERHEAD_SZ);
	return pUserData;
}
/***************************** gcMark ***************************************
void gcMark(GCManager *pMgr, GCResult *pGCResult)
Purpose:
	Goes through each node in the heap and marks them all for collection
Parameters:
	O GCManager *pMgr      Provides metadata about the user data and
						   information for storage management.
	O GCResult  *pGCResult the result of the function and stores error message if necessary.
						   0 for okay.
Notes:
*******************************************************************************/
void gcMark(GCManager *pMgr, GCResult *pGCResult)
{
	char *pCh;
	short shTempSize;
	InUseNode *pAlloc;
	FreeNode *pFree;
	//Traverse through heap and marking every node to C for collection
	for (pCh = pMgr->pBeginStorage; pCh < pMgr->pEndStorage; pCh += shTempSize)
	{
		pAlloc = (InUseNode *)pCh;
		shTempSize = pAlloc->shNodeSize;
		switch (pAlloc->cGC)
		{
		case 'F':
			pFree = (FreeNode *)pAlloc;
			pFree->cGC = 'C';
			break;
		case 'I':
			// It is in use
			pAlloc->cGC = 'C';
			break;
		default:
			// It is unknown 
			printf("\t%08lX %-5s %4d *** unknown cGC\n"
				, ULAddr(pAlloc), "Unk", shTempSize);
		}
	}
	pGCResult->rc = 0;
}
/***************************** gcFollow ***************************************
void gcFollow(GCManager *pMgr, void *pUserData, GCResult *pGCResult)
Purpose:
	Traverses through InUseNodes and marks them to 'I' and then traverses to any 
	associated nodes and marks them to 'I' as well to not be collected in collect phase
Parameters:
	O GCManager *pMgr      Provides metadata about the user data and
						   information for storage management.
	I void      *pUserData Pointer to the user's portion of the data.
	O GCResult  *pGCResult the result of the function and stores error message if necessary.
						   0 for okay.
Notes:
	Recursively calls itself to traverse associated nodes 
*******************************************************************************/
void gcFollow(GCManager *pMgr, void *pUserData, GCResult *pGCResult)
{
	int iAt;                        // control variable representing subscript in metaAttrM
	MetaAttr *pAttr;                // slightly simplifies referencing item in metaAttrM
	void **ppNode;                  // pointer into user data if this attribute is a pointer
	InUseNode *pIn;

	pMgr->pFreeHead = NULL;
	//Back up the over head to be able to get cGC flag and size of node
	pIn = (InUseNode*)((char*)pUserData - NODE_OVERHEAD_SZ);
	//If node is already marked InUse then break out of routine
	if (pIn->cGC == 'I')
	{
		pGCResult->rc = 0;
		return;
	}
	// Loop through each of the user data's attributes.  The subscripts start with
	// shBeginMetaAttr from nodeTypeM and end when the corresponding metaAttr's node type is
	// different.
	for (iAt = pMgr->nodeTypeM[pIn->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pIn->shNodeType; iAt++)
	{
		pAttr = &(pMgr->metaAttrM[iAt]);   // slightly simplify the reference in the metaAttrM array

		// set the data based on the attribute data type
		switch (pAttr->cDataType)
		{
		//Looking for pointer attributes in user data to traverse any associated nodes
		case 'P':  // pointer
			ppNode = (void **)&(pIn->sbData[pAttr->shOffset]);
			if (*ppNode != NULL)
			{
				gcFollow(pMgr, *ppNode, pGCResult);

			}
			pIn->cGC = 'I';
			break;
			// set the data based on the attribute data type
		}
	}
	pGCResult->rc = 0;
}
/***************************** gcCollect ***************************************
void gcMark(GCManager *pMgr, GCResult *pGCResult)
Purpose:
	Collects and garbage nodes and adds them to the free list.  If nodes are adjacent
	combines the nodes.
Parameters:
	O GCManager *pMgr      Provides metadata about the user data and
						   information for storage management.
	O GCResult  *pGCResult the result of the function and stores error message if necessary.
						   0 for okay.
Notes:
*******************************************************************************/
void gcCollect(GCManager *pMgr, GCResult *pGCResult)
{
	char *pCh, *pChNext;
	short shTempSize;
	InUseNode *pAlloc;
	FreeNode *pFree, *pNext;
	//Traverse through every node in the heap.
	for (pCh = pMgr->pBeginStorage; pCh < pMgr->pEndStorage; pCh += shTempSize)
	{
		pAlloc = (InUseNode *)pCh;
		shTempSize = pAlloc->shNodeSize;
		switch (pAlloc->cGC)
		{
		//Looking for any node that is marked for collection
		case 'C':
			pFree = (FreeNode *)pAlloc;
			pChNext = shTempSize + pCh;
			pNext = (FreeNode*)pChNext;
			if (pNext->cGC == 'C')
			{
				//combine adjacent nodes
				pFree->shNodeSize += pNext->shNodeSize;
				drPrintCombining(pMgr, pFree, pNext);
			}
			else
				drPrintCollecting(pMgr, pFree);
			if (shTempSize >= pMgr->iMinimumNodeSize)
			{
				pFree->cGC = 'F';
				addToFreeList(pMgr, pFree, pFree->shNodeSize);
			}
			break;
		default:
			// It is unknown 
			break;
		}
	}
	pGCResult->rc = 0;
}
/***************************** gcAssociate ***************************************
void gcAssoc(GCManager *pMgr, void *pUserDataFrom, char szAttrName[], void *pUserDataTo, GCResult *pGCResult)
Purpose:
	Associates nodes to other nodes and stores the address of the to user in the data of the from user
Parameters:
	O GCManager *pMgr         Provides metadata about the user data and
						      information for storage management.
	I void     *pUserDataFrom Pointer to the user's portion of the data.
	I char     szAttrName[]   Name of the attribute that is being assoicated to the user from node
	I void     *pUserDataTo   Pointer to the data that is getting assoicated with the user from
	O GCResult *pGCResult     The result of the function and stores error message if necessary.
						      0 for okay.
Notes:
*******************************************************************************/
void gcAssoc(GCManager *pMgr, void *pUserDataFrom, char szAttrName[], void *pUserDataTo, GCResult *pGCResult)
{
	int iAt;                        // control variable representing subscript in metaAttrM
	MetaAttr *pAttr;                // slightly simplifies referencing item in metaAttrM
	void **ppNode;                  // pointer into user data if this attribute is a pointer
	InUseNode *pIn;

	pIn = (InUseNode*)((char*)pUserDataFrom - NODE_OVERHEAD_SZ);

	// Loop through each of the user data's attributes.  The subscripts start with
	// shBeginMetaAttr from nodeTypeM and end when the corresponding metaAttr's node type is
	// different.
	for (iAt = pMgr->nodeTypeM[pIn->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pIn->shNodeType; iAt++)
	{
		pAttr = &(pMgr->metaAttrM[iAt]);   // slightly simplify the reference in the metaAttrM array

		// set the data based on the attribute data type
		switch (pAttr->cDataType)
		{
		case 'P':  // pointer
			if (strcmp(pAttr->szAttrName, szAttrName) == 0)
			{
				//sets the address of the to node in the from node
				ppNode = (void **)&(pIn->sbData[pAttr->shOffset]);
				*ppNode = pUserDataTo;
				pGCResult->rc = 0;
				return;
			}
			break;
		default:
			pGCResult->rc = RC_ASSOC_ATTR_NOT_PTR;
			sprintf(pGCResult->szErrorMessage, "Specificed attribute %s is not a pointer", szAttrName);
		}
	}
	pGCResult->rc = RC_ASSOC_ATTR_NOT_FOUND;
	sprintf(pGCResult->szErrorMessage, "Could not match %s attr", szAttrName);
	return;
}
/******************************* search *************************
FreeNode *search(GCManager *pMgr, int shSize, FreeNode **ppPrec)
Purpose:
	Searches for free node in the heap. if found returns pointer to node
	if not found returns null.
Paramaters:
	O GCManager *pMgr         Provides metadata about the user data and
						      information for storage management.
	I int       shSize		  Size of the node that needs to be added to the heap
	O FreeNode  **ppPrecedes  Double pointer to return preceding node value to be
							  inserted into ordered linked list
Returns:
	p - Node of value to be inserted
	NULL - if no match is found
******************************************************************/
FreeNode *search(GCManager *pMgr, int shSize, FreeNode **ppPrec)
{
	FreeNode *p;
	// NULL used when the list is empty 
	// or when we need to insert at the beginning
	*ppPrec = NULL;
	for (p = pMgr->pFreeHead; p != NULL; p = p->pFreeNext)
	{
		if (shSize <= p->shNodeSize)
		{
			return p;
		}
		*ppPrec = p;
	}
	// Not found return NULL
	return NULL;
}
/******************************* addToFreeList *************************
void addToFreeList(GCManager *pMgr, FreeNode *pF, int shRemaining)
Purpose:
	Adds node to the free list and puts at the front of the free list
Paramaters:
	O GCManager *pMgr         Provides metadata about the user data and
							  information for storage management.
	I FreeNode  *pF           Free node to be added to the free list
	I int       shRemaining   Size of the free node
******************************************************************/
void addToFreeList(GCManager *pMgr, FreeNode *pF, int shRemaining)
{
	pF->shNodeSize = shRemaining;
	pF->cGC = 'F';
	pF->pFreeNext = pMgr->pFreeHead;
	pMgr->pFreeHead = pF;
}
/**************** removeFreeNode *************************************
void removeFreeNode(GCManager *pMgr, FreeNode *p, FreeNode **ppPrec)
Purpose:
	Removes the free node from the free list 
Paramaters:
	O GCManager *pMgr         Provides metadata about the user data and
						      information for storage management.
	I FreeNode  *p  		  Pointer to the free node to be remove
	O FreeNode  **ppPrecedes  Double pointer to return preceding node value to be
							  inserted into ordered linked list
***************************************************************/
void removeFreeNode(GCManager *pMgr, FreeNode *p, FreeNode **ppPrec)
{
	FreeNode *pF;
	*ppPrec = NULL;
	if (pMgr->pFreeHead == p)
	{
		pMgr->pFreeHead = p->pFreeNext;
		return;
	}
	for (pF = pMgr->pFreeHead; pF != p; pF = pF->pFreeNext)
	{
		*ppPrec = pF;
	}
	*ppPrec = pF->pFreeNext;
}