.SUFFIXES: .o .cpp
CXX        = g++
#CFLAGS     = -g -O -Wall #-DDEBUG]
#CFLAGS = -Wall  -O2 -DDEBUG
CFLAGS     = -g  -Wall -DNDEBUG


OBJECT = CCommMgr.o CConfig.o CEvent.o CLog.o CFIFOBuffer.o CThread.o Utils.o CFileLog.o CHttpParser.o  CHttpResponse.o CTask.o CAnyValue.o
OUTPUT  := liblce.a 

all: $(OUTPUT)

liblce.a:$(OBJECT)
	ar -rs $@ $^


.cpp.o:
	$(CXX) $(CFLAGS) -c $^ 
	
.c.o:
	gcc $(CFLAGS) $(INCLUDEDIR) -c $^ 



install:

	cp *.h include
	cp liblce.a lib

uninstall:

clean:
	rm -f *.o $(OUTPUT)

