project(tinymd5 CXX)
ADD_LIBRARY(tinymd5 STATIC EXCLUDE_FROM_ALL 
md5.cpp 
md5wrapper.cpp
)
