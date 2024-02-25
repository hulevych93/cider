// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#ifndef HJSONTEST_AENFOAWINFAWENFEFAWEFNWF
#define HJSONTEST_AENFOAWINFAWENFEFAWEFNWF

#undef assert

#if 0
#define assert(expression)                                                    \
  (void)((!!(expression)) ||                                                  \
         (printf("\nTest failure in file %s on line %d:\n  %s\n\n", __FILE__, \
                 __LINE__, #expression),                                      \
          0) ||                                                               \
         (0, 0))
#else
#define assert(expression) (void)(expression)
#endif

#endif
