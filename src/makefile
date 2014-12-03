.SUFFIXES: .o .cpp
CXX        = g++
#CFLAGS     = -g -O -Wall #-DDEBUG]
#CFLAGS = -Wall  -O2 -DDEBUG
CFLAGS     = -g  -Wall -DNDEBUG
INCLUDEDIR = -I../include

OBJECT = CCommMgr.o CConfig.o CEvent.o CLog.o CAsyncLog.o CNetWorker.o CThread.o Utils.o CFileLog.o CHttpParser.o  CHttpResponse.o CTask.o
OUTPUT  := liblce.a 

all: $(OUTPUT)

liblce.a:$(OBJECT)
	ar -rs $@ $^


.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^ 
	
.c.o:
	gcc $(CFLAGS) $(INCLUDEDIR) -c $^ 



install:

	cp liblce.a ../lib/

uninstall:

clean:
	rm -f *.o $(OUTPUT)

