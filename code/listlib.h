/* -*- indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2017 Sony Global Education, Inc.
 */

#if !defined(LISTLIB_H)
#define LISTLIB_H

#define LE_OK 0
#define LE_NO_MEMORY 1
#define LE_RANGE 2

#define SET_ERROR(e, v) do { if (e) *(e) = (v); } while (0)

#define LIST_LENGTH(l)		(list_length(&(l)))
#define LIST_CONTAINS(l, v)	(list_contains(&(l), (v)))
#define LIST_REF(l, p)		(list_ref(&(l), (p), 0))
#define LIST_ADD(l, v)		(list_add(&(l), (v), 0))
#define LIST_DELETE(l, p)	(list_delete(&(l), (p), 0))
#define LIST_REPLACE(l, p, v)	(list_replace(&(l), (p), (v), 0))
#define LIST_INSERT(l, p, v)	(list_insert(&(l), (p), (v), 0))

struct elt {
  struct elt *e_next;
  float e_value;
};

static int
elt_equal(float x, float y)
{

  return x == y;
}

static struct elt *
elt_create(float v)
{
  struct elt *n = (struct elt *)malloc(sizeof(struct elt));

  if (n == 0)
    return 0;

  n->e_next = 0;
  n->e_value = v;
  return n;
}

static void
elt_link(struct elt **p, struct elt *n)
{

  n->e_next = *p;
  *p = n;
}

static void
elt_unlink(struct elt **p)
{
  struct elt *n = *p;

  *p = n->e_next;
  free(n);
}

static int
list_length(void *l)
{
  struct elt **p = (struct elt **)l;
  int length = 0;

  for (; *p != 0; p = &(*p)->e_next)
    length++;
  return length;
}

static int
list_contains(void *l, float v)
{
  struct elt **p = (struct elt **)l;

  for (; *p != 0; p = &(*p)->e_next)
    if (elt_equal(v, (*p)->e_value))
      return 1;
  return 0;
}

static float
list_ref(void *l, int pos, int *error)
{
  struct elt **p = (struct elt **)l;

  if (pos < 0) {
    SET_ERROR(error, LE_RANGE);
    return 0;
  }
  for (; *p != 0; p = &(*p)->e_next, pos--)
    if (pos == 0) {
      SET_ERROR(error, LE_OK);
      return (*p)->e_value;
    }
  SET_ERROR(error, LE_RANGE);
  return 0;
}

static void
list_add(void *l, float v, int *error)
{
  struct elt **p = (struct elt **)l;
  struct elt *n = elt_create(v);

  if (n == 0) {
    SET_ERROR(error, LE_NO_MEMORY);
    return;
  }
  for (; *p != 0; p = &(*p)->e_next)
    ;
  elt_link(p, n);
  SET_ERROR(error, LE_OK);
}

static void
list_delete(void *l, int pos, int *error)
{
  struct elt **p = (struct elt **)l;

  if (pos < 0) {
    SET_ERROR(error, LE_RANGE);
    return;
  }
  for (; *p != 0; p = &(*p)->e_next, pos--)
    if (pos == 0) {
      elt_unlink(p);
      SET_ERROR(error, LE_OK);
      return;
    }
  SET_ERROR(error, LE_RANGE);
}

static void
list_replace(void *l, int pos, float v, int *error)
{
  struct elt **p = (struct elt **)l;

  if (pos < 0) {
    SET_ERROR(error, LE_RANGE);
    return;
  }
  for (; *p != 0; p = &(*p)->e_next, pos--)
    if (pos == 0) {
      (*p)->e_value = v;
      SET_ERROR(error, LE_OK);
      return;
    }
  SET_ERROR(error, LE_RANGE);
}

static void
list_insert(void *l, int pos, float v, int *error)
{
  struct elt **p = (struct elt **)l;

  if (pos < 0) {
    SET_ERROR(error, LE_RANGE);
    return;
  }

  for (; *p != 0; p = &(*p)->e_next, pos--)
    if (pos == 0)
      break;

  if (pos > 0) {
    SET_ERROR(error, LE_RANGE);
    return;
  }

  struct elt *n = elt_create(v);
  if (n == 0) {
    SET_ERROR(error, LE_NO_MEMORY);
    return;
  }

  elt_link(p, n);
  SET_ERROR(error, LE_OK);
}

#endif /* !defined(LISTLIB_H) */
