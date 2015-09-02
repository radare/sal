/*  
 *          File: stack.h
 *        Author: Robert I. Pitts <rip@cs.bu.edu>
 * Last Modified: March 7, 2000
 *         Topic: Stack - Array Implementation
 * ----------------------------------------------------------------
 *
 * This is the interface for a stack of characters.
 */

#ifndef _STACK_H
#define _STACK_H


/*
 * Type: stackElementT
 * -------------------
 * This is the type of the objects entered in the stack.
 * Edit it to change the type of things to be placed in
 * the stack.
 */

//typedef char sal_stack_element_t;

/*
 * Type: sal_stack_t
 * --------------
 * This is the type for a stack, i.e., it is a type that
 * holds the information necessary to keep track of a stack.
 * It has a pointer `contents' to a dynamically-allocated
 * array (used to hold the contents of the stack), an integer
 * `maxSize' that holds the size of this array (i.e., the
 * maximum number of things that can be held in the stack),
 * and another integer `top,' which stores the array index of
 * the element at the top of the stack.
 */

typedef struct {
  void **contents;
  int maxSize;
  int top;
} sal_stack_t;

// pancake - hack fget = failsafe get
void *sal_stack_fget(sal_stack_t *stack);

/*
 * Function: StackInit
 * Usage: StackInit(&stack, maxSize);
 * -------------------------
 * A new stack variable is initialized.  The initialized
 * stack is made empty.  MaxSize is used to determine the
 * maximum number of character that can be held in the
 * stack.
 */

sal_stack_t *sal_stack_new(int maxSize);

/* Function: StackDestroy
 * Usage: StackDestroy(&stack);
 * -----------------------
 * This function frees all memory associated with the stack.
 * The `stack' variable may not be used again unless
 * StackInit(&stack, maxSize) is first called on the stack.
 */

void sal_stack_free(sal_stack_t *stackP);

/*
 * Functions: StackPush, StackPop
 * Usage: StackPush(&stack, element); element = StackPop(&stack);
 * --------------------------------------------
 * These are the fundamental stack operations that add an element to
 * the top of the stack and remove an element from the top of the stack.
 * A call to StackPop on an empty stack or to StackPush on a full stack
 * is an error.  Make use of StackIsEmpty()/StackIsFull() (see below)
 * to avoid these errors.
 */

void sal_stack_push(sal_stack_t *stackP, void *element);
void *sal_stack_pop(sal_stack_t *stackP);

/*
 * Functions: StackIsEmpty, StackIsFull
 * Usage: if (StackIsEmpty(&stack)) ...
 * -----------------------------------
 * These return a true value if the stack is empty
 * or full (respectively).
 */

int sal_stack_is_empty(sal_stack_t *stackP);
int sal_stack_is_full(sal_stack_t *stackP);

#endif  /* not defined _STACK_H */
