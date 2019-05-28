px4_add_module(
    MODULE examples__px4_james_app
    MAIN px4_james_app
    STACK_MAIN 2000
    SRCS
        px4_james_app.c
    DEPENDS
    )
