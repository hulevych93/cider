# Copyright (C) 2022-2025 Hulevych Mykhailo
# SPDX-License-Identifier: MIT

cider_project(monte-carlo)

cider_collect_src(LIST_OUT "tests")

add_library(monte-carlo ${LIST_OUT})

add_dependencies(monte-carlo ${CIDER_THIRDPARTY_TARGET_NAME})
