#########################################################################
# File Name: build_comm.sh
# Author: starjiang
# mail: 82776315@qq.com
# Created Time: Thu 04 Dec 2014 05:10:22 PM
#########################################################################
#!/bin/bash
g++ -g -otest_comm Test_CCommMgr.cpp -I../include  ../src/CEvent.cpp ../src/CCommMgr.cpp ../src/CTask.cpp ../src/CThread.cpp ../src/Utils.cpp ../src/CHttpResponse.cpp ../src/CHttpParser.cpp  -lpthread
