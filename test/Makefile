.SUFFIXES: .o .cpp
vpath %.h ../include
vpath %.cpp ../src
CFLAGS     = -g -O2
INCLUDEDIR = -I../include
LIBS = -lpthread
SRC_OBJECT =CCommMgr.o CEvent.o CThread.o  CHttpReader.o CHttpWriter.o CTask.o CNetWorker.o CAsyncLog.o CFileLog.o CTcpConnector.o CUdpConnector.o Utils.o CLog.o

all:test_httpserver test_blockqueue test_workerpool test_any_workerpool test_http_workerpool test_anyvalue test_networker test_echoserver test_event test_convert test_connector test_log test_cache

test_httpserver:Test_HttpServer.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_workerpool:Test_CWorkerPool.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_http_workerpool:Test_CHttpWorkerPool.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_any_workerpool:Test_CAnyWorkerPool.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_anyvalue:Test_CAnyValue.o
	$(CXX) -o $@ $^ $(LIBS)

test_blockqueue:Test_BlockQueue.o
	$(CXX) -o $@ $^ $(LIBS)

test_networker:Test_CNetWorker.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_echoserver:Test_EchoServer.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_event:Test_CEvent.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_convert:Test_JsonToAnyValue.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_connector:Test_Connector.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)

test_log:Test_AsyncLog.o $(SRC_OBJECT)
	$(CXX) -o $@ $^ $(LIBS)
test_cache:Test_LruCache.o
	$(CXX) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDEDIR) -c $^
.c.o:
	gcc $(CFLAGS) $(INCLUDEDIR) -c $^
clean:
	rm -f *.o $(SRC_OBJECT) test_*
