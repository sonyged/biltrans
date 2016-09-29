#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "listlib.h"

static void
print(const char *tag, void *l)
{
  struct elt **p = (struct elt **)l;

  printf("%s: [", tag);
  for (; *p != 0; p = &(*p)->e_next)
    printf("%f, ", (*p)->e_value);
  printf("]\n");
}

#define OP(exp) do { exp; print(#exp, &l); } while (0)

int
main()
{
  void *l = 0;

  OP();
  assert(LIST_LENGTH(l) == 0);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == false);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 0);
  assert(LIST_REF(l, 1) == 0);
  assert(LIST_REF(l, 2) == 0);

  OP(LIST_ADD(l, 3));
  assert(LIST_LENGTH(l) == 1);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == true);
  assert(LIST_CONTAINS(l, 4) == false);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 3);
  assert(LIST_REF(l, 1) == 0);
  assert(LIST_REF(l, 2) == 0);
  assert(LIST_REF(l, 3) == 0);

  OP(LIST_ADD(l, 2));
  assert(LIST_LENGTH(l) == 2);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == true);
  assert(LIST_CONTAINS(l, 3) == true);
  assert(LIST_CONTAINS(l, 4) == false);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 3);
  assert(LIST_REF(l, 1) == 2);
  assert(LIST_REF(l, 2) == 0);
  assert(LIST_REF(l, 3) == 0);

  OP(LIST_ADD(l, 2));
  assert(LIST_LENGTH(l) == 3);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == true);
  assert(LIST_CONTAINS(l, 3) == true);
  assert(LIST_CONTAINS(l, 4) == false);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 3);
  assert(LIST_REF(l, 1) == 2);
  assert(LIST_REF(l, 2) == 2);
  assert(LIST_REF(l, 3) == 0);

  OP(LIST_REPLACE(l, 1, 4));
  assert(LIST_LENGTH(l) == 3);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == true);
  assert(LIST_CONTAINS(l, 3) == true);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 3);
  assert(LIST_REF(l, 1) == 4);
  assert(LIST_REF(l, 2) == 2);
  assert(LIST_REF(l, 3) == 0);

  OP(LIST_DELETE(l, 0));
  assert(LIST_LENGTH(l) == 2);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == true);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 4);
  assert(LIST_REF(l, 1) == 2);
  assert(LIST_REF(l, 2) == 0);
  assert(LIST_REF(l, 3) == 0);

  OP(LIST_INSERT(l, 0, 1));
  assert(LIST_LENGTH(l) == 3);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == true);
  assert(LIST_CONTAINS(l, 2) == true);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 1);
  assert(LIST_REF(l, 1) == 4);
  assert(LIST_REF(l, 2) == 2);
  assert(LIST_REF(l, 3) == 0);

  OP(LIST_INSERT(l, 1, 3));
  assert(LIST_LENGTH(l) == 4);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == true);
  assert(LIST_CONTAINS(l, 2) == true);
  assert(LIST_CONTAINS(l, 3) == true);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 1);
  assert(LIST_REF(l, 1) == 3);
  assert(LIST_REF(l, 2) == 4);
  assert(LIST_REF(l, 3) == 2);
  assert(LIST_REF(l, 4) == 0);

  OP(LIST_DELETE(l, 4));
  assert(LIST_LENGTH(l) == 4);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == true);
  assert(LIST_CONTAINS(l, 2) == true);
  assert(LIST_CONTAINS(l, 3) == true);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 1);
  assert(LIST_REF(l, 1) == 3);
  assert(LIST_REF(l, 2) == 4);
  assert(LIST_REF(l, 3) == 2);
  assert(LIST_REF(l, 4) == 0);

  OP(LIST_DELETE(l, 3));
  assert(LIST_LENGTH(l) == 3);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == true);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == true);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 1);
  assert(LIST_REF(l, 1) == 3);
  assert(LIST_REF(l, 2) == 4);
  assert(LIST_REF(l, 3) == 0);
  assert(LIST_REF(l, 4) == 0);

  OP(LIST_DELETE(l, 1));
  assert(LIST_LENGTH(l) == 2);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == true);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 1);
  assert(LIST_REF(l, 1) == 4);
  assert(LIST_REF(l, 2) == 0);
  assert(LIST_REF(l, 3) == 0);
  assert(LIST_REF(l, 4) == 0);

  OP(LIST_INSERT(l, 2, 1));
  assert(LIST_LENGTH(l) == 3);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == true);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 1);
  assert(LIST_REF(l, 1) == 4);
  assert(LIST_REF(l, 2) == 1);
  assert(LIST_REF(l, 3) == 0);
  assert(LIST_REF(l, 4) == 0);

  OP(LIST_DELETE(l, 2));
  assert(LIST_LENGTH(l) == 2);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == true);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 1);
  assert(LIST_REF(l, 1) == 4);
  assert(LIST_REF(l, 2) == 0);
  assert(LIST_REF(l, 3) == 0);
  assert(LIST_REF(l, 4) == 0);

  OP(LIST_DELETE(l, 0));
  assert(LIST_LENGTH(l) == 1);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == true);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 4);
  assert(LIST_REF(l, 1) == 0);
  assert(LIST_REF(l, 2) == 0);
  assert(LIST_REF(l, 3) == 0);
  assert(LIST_REF(l, 4) == 0);

  OP(LIST_DELETE(l, 0));
  assert(LIST_LENGTH(l) == 0);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == false);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 0);
  assert(LIST_REF(l, 1) == 0);
  assert(LIST_REF(l, 2) == 0);
  assert(LIST_REF(l, 3) == 0);
  assert(LIST_REF(l, 4) == 0);

  OP(LIST_DELETE(l, 0));
  assert(LIST_LENGTH(l) == 0);
  assert(LIST_CONTAINS(l, 0) == false);
  assert(LIST_CONTAINS(l, 1) == false);
  assert(LIST_CONTAINS(l, 2) == false);
  assert(LIST_CONTAINS(l, 3) == false);
  assert(LIST_CONTAINS(l, 4) == false);
  assert(LIST_CONTAINS(l, 5) == false);

  assert(LIST_REF(l, 0) == 0);
  assert(LIST_REF(l, 1) == 0);
  assert(LIST_REF(l, 2) == 0);
  assert(LIST_REF(l, 3) == 0);
  assert(LIST_REF(l, 4) == 0);

  exit(0);
}
