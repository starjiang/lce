.SUFFIXES: .o .cpp
vpath %.h ../include
vpath %.cpp ../src
CFLAGS     = -g -O2
INCLUDEDIR = -I../include
LIBS = -lpthread
SRC_OBJECT =CCommMgr.o CEvent.o CThread.o  CHttpParser.o CHttpResponse.o CTask.o CNetWorker.o CAsyncLog.o CFileLog.o Utils.o

all:test_httpserver test_workerpool test_anyvalue test_networker test_echoserver test_event test_convert

test_httpserver:Test_HttpServer.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_workerpool:Test_CWorkerPool.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_anyvalue:Test_CAnyValue.o
	$(CXX) -o $@ $^ $(LIBS)

test_networker:Test_CNetWorker.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_echoserver:Test_EchoServer.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_event:Test_CEvent.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_convert:Test_JsonToAnyValue.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^
.c.o:
	gcc $(CFLAGS) $(INCLUDEDIR) -c $^
clean:
	rm -f *.o $(SRC_OBJECT) test_*
