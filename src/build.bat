cl /c decode.c
cl /c encode.c
cl /c compress.c
cl /c psb.cpp
cl /I"C:\Users\yuanrui\Documents\krkr_psbfile\jsoncpp\include" /EHsc /c decompiler.cpp
link decode.obj encode.obj compress.obj decompiler.obj psb.obj json_reader.obj json_value.obj json_writer.obj /OUT:decompiler.exe
