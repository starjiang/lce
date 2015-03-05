.SUFFIXES: .o .cpp
CXX        = g++

#CFLAGS = -Wall  -O2 -DDEBUG
#CFLAGS = -g -O2 -Wall -DNDEBUG

CFLAGS = -g -O2 -Wall -DNDEBUG -DHAVE_MYSQL_H
INCLUDEDIR = -I../include -I/opt/lampp/include

OBJECT = CCommMgr.o CConfig.o CEvent.o CLog.o CAsyncLog.o CNetWorker.o CThread.o Utils.o CFileLog.o CHttpParser.o  CHttpResponse.o CTask.o CTcpConnector.o CUdpConnector.o CMysql.o
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

