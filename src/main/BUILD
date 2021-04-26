# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = ["//src/lib:SolutionLib"],
)

cc_binary(
    name = "conditional_var_main",
    srcs = ["conditional_var_main.cc"],
    deps = ["//src/lib:SolutionLib"],
)

cc_binary(
    name = "producer_consumer_lock_main",
    srcs = ["producer_consumer_lock_main.cc"],
    deps = ["//src/lib:SolutionLib"],
)

cc_binary(
    name = "conditional_var_csp_main",
    srcs = ["conditional_var_csp_main.cc"],
    deps = [
        "//src/lib:SolutionLib",
        "//src/lib:csp_lib",
    ],
)

cc_binary(
    name = "conditional_var_csp_sync_rcv_main",
    srcs = ["conditional_var_csp_sync_rcv_main.cc"],
    deps = [
        "//src/lib:SolutionLib",
        "//src/lib:csp_lib",
    ],
)

cc_binary(
    name = "conditional_var_csp_adder_main",
    srcs = ["conditional_var_csp_adder_main.cc"],
    deps = [
        "//src/lib:SolutionLib",
        "//src/lib:csp_lib",
    ],
)

cc_binary(
    name = "conditional_var_csp_adder_fork_join_main",
    srcs = ["conditional_var_csp_adder_fork_join_main.cc"],
    deps = [
        "//src/lib:SolutionLib",
        "//src/lib:csp_lib",
    ],
)

cc_binary(
    name = "dining_philosophore_main",
    srcs = ["dining_philosophore_main.cc"],
    deps = [
        "//src/lib:SolutionLib",
        "//src/lib:csp_lib",
        "//src/lib:dining_philosophors_lib",
    ],
)

cc_binary(
    name = "map_reduce_main",
    srcs = ["map_reduce_main.cc"],
    deps = [
        "//src/lib:csp_lib",
        "//src/lib:map_reduce_lib",
    ],
)

cc_binary(
    name = "map_reduce_file",
    srcs = ["map_reduce_file.cc"],
    deps = [
        "//src/lib:csp_lib",
        "//src/lib:map_reduce_lib",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
    ],
)
