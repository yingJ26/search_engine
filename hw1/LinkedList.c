/*
 * Copyright ©2020 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Summer Quarter 2020 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>
#include <stdlib.h>

#include "CSE333.h"
#include "LinkedList.h"
#include "LinkedList_priv.h"


///////////////////////////////////////////////////////////////////////////////
// LinkedList implementation.

LinkedList* LinkedList_Allocate(void) {
    // Allocate the linked list record.
    LinkedList *ll = (LinkedList *) malloc(sizeof(LinkedList));
    Verify333(ll != NULL);

    // STEP 1: initialize the newly allocated record structure.
    ll->head = NULL;
    ll->tail = NULL;
    ll->num_elements = 0;
    // Return our newly minted linked list.
    return ll;
}

void LinkedList_Free(LinkedList *list,
                     LLPayloadFreeFnPtr payload_free_function) {
    Verify333(list != NULL);
    Verify333(payload_free_function != NULL);

    // STEP 2: sweep through the list and free all of the nodes' payloads as
    // well as the nodes themselves.
    while (list->head != NULL) {
        LinkedListNode *oldhead = list->head;
        payload_free_function(oldhead->payload);
        list->head = oldhead->next;
        free(oldhead);
    }
    free(list);
}

int LinkedList_NumElements(LinkedList *list) {
    Verify333(list != NULL);
    return list->num_elements;
}

void LinkedList_Push(LinkedList *list, LLPayload_t payload) {
    Verify333(list != NULL);

    // Allocate space for the new node.
    LinkedListNode *ln = (LinkedListNode *) malloc(sizeof(LinkedListNode));
    Verify333(ln != NULL);

    // Set the payload
    ln->payload = payload;

    if (list->num_elements == 0) {
        // Degenerate case; list is currently empty
        Verify333(list->head == NULL);
        Verify333(list->tail == NULL);
        ln->next = ln->prev = NULL;
        list->head = list->tail = ln;
        list->num_elements = 1;
    } else {
        // STEP 3: typical case; list has >=1 elements
        ln->prev = NULL;
        ln->next = list->head;
        list->num_elements += 1;
        list->head->prev = ln;
        list->head = ln;
    }
}

bool LinkedList_Pop(LinkedList *list, LLPayload_t *payload_ptr) {
    Verify333(payload_ptr != NULL);
    Verify333(list != NULL);

    // STEP 4: implement LinkedList_Pop.  Make sure you test for
    // and empty list and fail.  If the list is non-empty, there
    // are two cases to consider: (a) a list with a single element in it
    // and (b) the general case of a list with >=2 elements in it.
    // Be sure to call free() to deallocate the memory that was
    // previously allocated by LinkedList_Push().
    if (list->num_elements < 1) {
        return false;
    }
    LinkedListNode *ln = list->head;
    *payload_ptr = ln->payload;

    if (list->num_elements == 1) {
        list->head = list->tail = NULL;
    } else {
        list->head = ln->next;
        list->head->prev = NULL;
    }
    free(ln);
    list->num_elements -= 1;
    return true;
}

void LinkedList_Append(LinkedList *list, LLPayload_t payload) {
    Verify333(list != NULL);

    // STEP 5: implement LinkedList_Append.  It's kind of like
    // LinkedList_Push, but obviously you need to add to the end
    // instead of the beginning.
    LinkedListNode *ln = (LinkedListNode*)
            malloc(sizeof(LinkedListNode));
    ln->prev = ln->next = NULL;
    if (ln == NULL) {
        return;
    }
    ln->payload = payload;
    if (list->num_elements == 0) {
        list->head = list->tail = ln;
    } else {
        ln->prev = list->tail;
        list->tail->next = ln;
        list->tail = ln;
    }
    list->num_elements += 1;
}

void LinkedList_Sort(LinkedList *list, bool ascending,
                     LLPayloadComparatorFnPtr comparator_function) {
    Verify333(list != NULL);
    if (list->num_elements < 2) {
        // No sorting needed.
        return;
    }

    // We'll implement bubblesort! Nnice and easy, and nice and slow :)
    int swapped;
    do {
        LinkedListNode *curnode;

        swapped = 0;
        curnode = list->head;
        while (curnode->next != NULL) {
            int compare_result = comparator_function(curnode->payload,
                                                     curnode->next->payload);
            if (ascending) {
                compare_result *= -1;
            }
            if (compare_result < 0) {
                // Bubble-swap the payloads.
                LLPayload_t tmp;
                tmp = curnode->payload;
                curnode->payload = curnode->next->payload;
                curnode->next->payload = tmp;
                swapped = 1;
            }
            curnode = curnode->next;
        }
    } while (swapped);
}


///////////////////////////////////////////////////////////////////////////////
// LLIterator implementation.

LLIterator* LLIterator_Allocate(LinkedList *list) {
    Verify333(list != NULL);

    // OK, let's manufacture an iterator.
    LLIterator *li = (LLIterator *) malloc(sizeof(LLIterator));
    Verify333(li != NULL);

    // Set up the iterator.
    li->list = list;
    li->node = list->head;

    return li;
}

void LLIterator_Free(LLIterator *iter) {
    Verify333(iter != NULL);
    free(iter);
}

bool LLIterator_IsValid(LLIterator *iter) {
    Verify333(iter != NULL);
    Verify333(iter->list != NULL);

    return (iter->node != NULL);
}

bool LLIterator_Next(LLIterator *iter) {
    Verify333(iter != NULL);
    Verify333(iter->list != NULL);
    Verify333(iter->node != NULL);

    // STEP 6: advance to the node beyond the iterator and return true if
    // it was valid.
    iter->node = iter->node->next;
    return LLIterator_IsValid(iter);
}

void LLIterator_Get(LLIterator *iter, LLPayload_t *payload) {
    Verify333(iter != NULL);
    Verify333(iter->list != NULL);
    Verify333(iter->node != NULL);

    *payload = iter->node->payload;
}

bool LLIterator_Remove(LLIterator *iter,
                       LLPayloadFreeFnPtr payload_free_function) {
    Verify333(iter != NULL);
    Verify333(iter->list != NULL);
    Verify333(iter->node != NULL);

    // STEP 7: implement LLIterator_Remove.  This is the most
    // complex function you'll build.  There are several cases
    // to consider:
    // - degenerate case: the list becomes empty after deleting.
    // - degenerate case: iter points at head
    // - degenerate case: iter points at tail
    // - fully general case: iter points in the middle of a list,
    //                       and you have to "splice".
    //
    // Be sure to call the payload_free_function to free the payload
    // the iterator is pointing to, and also free any LinkedList
    // data structure element as appropriate.
    LLPayload_t payload = iter->node->payload;
    // Three degnerate cases
    if (iter->list->num_elements == 1 || iter->list->head == iter->node) {
        LinkedList_Pop(iter->list, &payload);
        iter->node = iter->list->head;
    } else if (iter->list->tail == iter->node) {
        LinkedList_Slice(iter->list, &payload);
        iter->node = iter->list->head;
    } else {
        // node is in the middle of the list
        LinkedListNode *temp = iter->node;
        temp->next->prev = temp->prev;
        temp->prev->next = temp->next;
        iter->list->num_elements -= 1;
        payload = iter->node->payload;
        iter->node = temp->next;
        free(temp);
    }
    (*payload_free_function)(payload);
    return iter->list->num_elements != 0;
}


///////////////////////////////////////////////////////////////////////////////
// Helper functions

bool LinkedList_Slice(LinkedList *list, LLPayload_t *payload_ptr) {
    Verify333(payload_ptr != NULL);
    Verify333(list != NULL);

    // STEP 8: implement LinkedList_Slice.
    if (list->num_elements < 1) {
        return false;
    }
    LinkedListNode *ln = list->tail;
    *payload_ptr = ln->payload;

    if (list->num_elements == 1) {
        list->head = list->tail = NULL;
    } else {
        list->tail = ln->prev;
        ln->prev->next = NULL;
    }
    free(ln);
    list->num_elements -= 1;
    return true;
}

void LLIterator_Rewind(LLIterator *iter) {
    iter->node = iter->list->head;
}
