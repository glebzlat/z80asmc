#ifndef INTERFACES_H
#define INTERFACES_H

#include <assert.h>

#define CALL(OBJ, METHOD) do {\
  assert(OBJ); \
  assert((OBJ)->METHOD); \
  assert((OBJ)->_m_impl); \
  return (OBJ)->METHOD((OBJ)->_m_impl); \
} while (0)

#define CALL_VA(OBJ, METHOD, ...) do {\
  assert(OBJ); \
  assert((OBJ)->METHOD); \
  assert((OBJ)->_m_impl); \
  return (OBJ)->METHOD((OBJ)->_m_impl, __VA_ARGS__); \
} while (0)

#define CALL_VOID(OBJ, METHOD) do {\
  assert(OBJ); \
  assert((OBJ)->METHOD); \
  assert((OBJ)->_m_impl); \
  (OBJ)->METHOD((OBJ)->_m_impl); \
} while (0)


#endif // INTERFACES_H
