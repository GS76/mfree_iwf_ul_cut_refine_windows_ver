#ifndef MFREE_STUB_GLIBCXX_DEBUG_ASSERTIONS_H_
#define MFREE_STUB_GLIBCXX_DEBUG_ASSERTIONS_H_

#include <cassert>

#ifndef __glibcxx_assert
#define __glibcxx_assert(x) assert(x)
#endif

#ifndef _GLIBCXX_DEBUG_ASSERT
#define _GLIBCXX_DEBUG_ASSERT(x) __glibcxx_assert(x)
#endif

#ifndef _GLIBCXX_DEBUG_PEDASSERT
#define _GLIBCXX_DEBUG_PEDASSERT(x) __glibcxx_assert(x)
#endif

#ifndef __glibcxx_requires_nonempty
#define __glibcxx_requires_nonempty() __glibcxx_assert(!this->empty())
#endif

#ifndef __glibcxx_requires_subscript
#define __glibcxx_requires_subscript(n) __glibcxx_assert((n) < this->size())
#endif

#ifndef __glibcxx_requires_valid_range
#define __glibcxx_requires_valid_range(first, last) ((void)0)
#endif

#endif
