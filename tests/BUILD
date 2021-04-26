load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "tests",
    srcs = glob(["**/*.cc"]),
    deps = [
        "//src/lib:SolutionLib",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "dining_philosophors_test",
    srcs = ["dining_philosophors_test.cc"],
    deps = [
        "//src/lib:csp_lib",
        "//src/lib:dining_philosophors_lib",
        "@googletest//:gtest_main",
        "@glog",
    ],
)
