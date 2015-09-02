/*
 *          File: stack.c
 *        Author: Robert I. Pitts <rip@cs.bu.edu>
 * Last Modified: March 7, 2000
 *         Topic: Stack - Array Implementation
 * ----------------------------------------------------------------
 *
 * This is an array implementation of a character stack.
 */

#include <stdio.h>
#include <stdlib.h>  /* for dynamic allocation */
#include "alloc.h"
#include "stack.h"       

/************************ Function Definitions **********************/

sal_stack_t *sal_stack_new(int maxSize)
{
  sal_stack_t *stackP = (sal_stack_t*)sal_malloc(sizeof(sal_stack_t));
  void *newContents;

  /* Allocate a new array to hold the contents. */

  newContents = (void *)sal_malloc(sizeof(void) * maxSize);

  if (newContents == NULL) {
    fprintf(stderr, "Insufficient memory to initialize stack.\n");
    exit(1);  /* Exit, returning error code. */
  }

  stackP->contents = newContents;
  stackP->maxSize = maxSize;
  stackP->top = -1;  /* I.e., empty */

  return stackP;
}

void sal_stack_free(sal_stack_t *stackP)
{
  /* Get rid of array. */
  free(stackP->contents);

  stackP->contents = NULL;
  stackP->maxSize = 0;
  stackP->top = -1;  /* I.e., empty */
}

void sal_stack_push(sal_stack_t *stackP, void *element)
{
  if (sal_stack_is_full(stackP)) {
    fprintf(stderr, "Can't push element on stack: stack is full.\n");
    exit(1);  /* Exit, returning error code. */
  }

  /* Put information in array; update top. */
  stackP->contents[++stackP->top] = element;
}

void *sal_stack_pop(sal_stack_t *stackP)
{
  if (sal_stack_is_empty(stackP)) {
    fprintf(stderr, "Can't pop element from stack: stack is empty.\n");
    exit(1);  /* Exit, returning error code. */
  }

  return stackP->contents[stackP->top--];
}

void *sal_stack_get(sal_stack_t *stackP)
{
  if (sal_stack_is_empty(stackP)) {
    fprintf(stderr, "Can't pop element from stack: stack is empty.\n");
    exit(1);  /* Exit, returning error code. */
  }

  return stackP->contents[stackP->top];
}

void *sal_stack_fget(sal_stack_t *stack) {
	if (stack->top>-1)
		return sal_stack_get(stack);
	return NULL;
}

int sal_stack_is_empty(sal_stack_t *stackP)
{
  return stackP->top < 0;
}

int sal_stack_is_full(sal_stack_t *stackP)
{
  return stackP->top >= stackP->maxSize;
}
