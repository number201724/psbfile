g++ -c common/psb.cpp -o psb.o
g++ -c	scenes/scene_parse/parse.cpp -o parse.o
g++ psb.o parse.o -o parse