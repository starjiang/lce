.SUFFIXES: .o .cpp
CFLAGS     = -g -O2 -DNDEBUG
INCLUDEDIR = -I../include
LIBS = -lpthread
SRC_OBJECT =../src/CCommMgr.o ../src/CEvent.o ../src/CThread.o  ../src/CHttpParser.o ../src/CHttpResponse.o ../src/Utils.o ../src/CTask.o ../src/CNetWorker.o

all:test_httpserver test_workerpool test_anyvalue test_networker test_echoserver test_event

test_httpserver:Test_HttpServer.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_HttpServer.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c Test_HttpServer.cpp

test_workerpool:Test_CWorkerPool.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_CWorkerPool.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c Test_CWorkerPool.cpp

test_anyvalue:Test_CAnyValue.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_CAnyValue.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c Test_CAnyValue.cpp

test_networker:Test_CNetWorker.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_CNetWorker.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c Test_CNetWorker.cpp

test_echoserver:Test_EchoServer.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_EchoServer.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c Test_EchoServer.cpp

test_event:Test_CEvent.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_CEvent.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c Test_CEvent.cpp

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^ -o../src/$@
.c.o:
	gcc $(CFLAGS) $(INCLUDEDIR) -c $^
clean:
	rm -f *.o $(SRC_OBJECT) test_*
