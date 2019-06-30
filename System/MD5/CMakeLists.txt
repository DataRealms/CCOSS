set(SOURCES md5.c
md5.h)

complete_path(${SOURCES})
set(md5_SRC ${RESULT})
source_group(MD5 FILES ${RESULT})

add_library(md5 ${md5_SRC})
