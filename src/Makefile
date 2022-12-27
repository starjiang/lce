.SUFFIXES: .o .cpp
CXX        = g++

#CFLAGS = -Wall  -O2
#CFLAGS = -g -O2 -Wall
CFLAGS = $(CFLAG)
CFLAGS += -g -O2 -Wall -std=c++11 -Wno-format-truncation
INCLUDEDIR = -I../include

OBJECT = CCommMgr.o CConfig.o CEvent.o CLog.o CAsyncLog.o CNetWorker.o CThread.o Utils.o CFileLog.o CHttpReader.o  CHttpWriter.o CTask.o CTcpConnector.o CUdpConnector.o CMysql.o
OUTPUT  := liblce.a 

all: $(OUTPUT)

liblce.a:$(OBJECT)
	ar -rs $@ $^


.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^ 
	
.c.o:
	gcc $(CFLAGS) $(INCLUDEDIR) -c $^ 



install:
	rm ../lib -rf
	mkdir ../lib
	cp liblce.a ../lib/

uninstall:

clean:
	rm -f *.o $(OUTPUT)
	rm ../lib -r
