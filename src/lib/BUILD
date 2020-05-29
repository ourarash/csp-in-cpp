# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
cc_library(
    name = "SolutionLib",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "csp_lib",
    srcs = ["csp/csp.cc"],
    hdrs = ["csp/csp.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "dining_philosophors_lib",
    srcs = ["csp/dining_philosophors.cc"],
    hdrs = ["csp/dining_philosophors.h"],
    visibility = ["//visibility:public"],
    deps = [":csp_lib"],
)
