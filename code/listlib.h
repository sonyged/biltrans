#define LIST_LENGTH(l)		(list_length(&(l)))
#define LIST_CONTAINS(l, v)	(list_contains(&(l), (v)))
#define LIST_REF(l, p)		(list_ref(&(l), (p)))
#define LIST_ADD(l, v)		(list_add(&(l), (v)))
#define LIST_DELETE(l, p)	(list_delete(&(l), (p)))
#define LIST_REPLACE(l, p, v)	(list_replace(&(l), (p), (v)))
#define LIST_INSERT(l, p, v)	(list_insert(&(l), (p), (v)))

struct elt {
  struct elt *e_next;
  float e_value;
};

static bool
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

static bool
list_contains(void *l, float v)
{
  struct elt **p = (struct elt **)l;

  for (; *p != 0; p = &(*p)->e_next)
    if (elt_equal(v, (*p)->e_value))
      return true;
  return false;
}

static float
list_ref(void *l, int pos)
{
  struct elt **p = (struct elt **)l;

  if (pos < 0)
    return 0;
  for (; *p != 0; p = &(*p)->e_next, pos--)
    if (pos == 0)
      return (*p)->e_value;
  return 0;
}

static void
list_add(void *l, float v)
{
  struct elt **p = (struct elt **)l;
  struct elt *n = elt_create(v);

  if (n == 0)
    return;
  for (; *p != 0; p = &(*p)->e_next)
    ;
  elt_link(p, n);
}

static void
list_delete(void *l, int pos)
{
  struct elt **p = (struct elt **)l;

  if (pos < 0)
    return;
  for (; *p != 0; p = &(*p)->e_next, pos--)
    if (pos == 0) {
      elt_unlink(p);
      return;
    }
}

static void
list_replace(void *l, int pos, float v)
{
  struct elt **p = (struct elt **)l;

  if (pos < 0)
    return;
  for (; *p != 0; p = &(*p)->e_next, pos--)
    if (pos == 0) {
      (*p)->e_value = v;
      return;
    }
}

static void
list_insert(void *l, int pos, float v)
{
  struct elt **p = (struct elt **)l;

  if (pos < 0)
    return;

  struct elt *n = elt_create(v);
  if (n == 0)
    return;

  for (; *p != 0; p = &(*p)->e_next, pos--)
    if (pos == 0)
      break;

  elt_link(p, n);
}
