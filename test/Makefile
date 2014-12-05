.SUFFIXES: .o .cpp .h
CFLAGS     = -g -O2 -DNDEBUG
INCLUDEDIR = -I../include
LIBS = -lpthread
SRC_OBJECT =../src/CCommMgr.o ../src/CEvent.o ../src/CThread.o  ../src/CHttpParser.o ../src/CHttpResponse.o ../src/Utils.o ../src/CTask.o ../src/CNetWorker.o

all:test_httpserver test_workerpool test_anyvalue test_networker test_echoserver test_event test_convert

test_httpserver:Test_HttpServer.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_HttpServer.o:Test_HttpServer.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^

test_workerpool:Test_CWorkerPool.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_CWorkerPool.o:Test_CWorkerPool.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^

test_anyvalue:Test_CAnyValue.o
	$(CXX) -o $@ $^ $(LIBS)
Test_CAnyValue.o:Test_CAnyValue.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^

test_networker:Test_CNetWorker.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_CNetWorker.o:Test_CNetWorker.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^

test_echoserver:Test_EchoServer.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_EchoServer.o:Test_EchoServer.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^

test_event:Test_CEvent.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_CEvent.o:Test_CEvent.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^

test_convert:Test_JsonToAnyValue.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
Test_JsonToAnyValue.o:Test_JsonToAnyValue.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^ -o../src/$@
.c.o:
	gcc $(CFLAGS) $(INCLUDEDIR) -c $^
clean:
	rm -f *.o $(SRC_OBJECT) test_*
