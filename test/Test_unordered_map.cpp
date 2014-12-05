/*
 * =====================================================================================
 *
 *       Filename:  test_unordered_map.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/17/2013 04:06:08 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  plutoxiong (Xiong Yilin), plutoxiong@tencent.com
 *        Company:  Tencent Inc.
 *
 * =====================================================================================
 */
#include<iostream>
#include<tr1/unordered_map>

using namespace std;

int main(int argc, char** argv)
{

    tr1::unordered_map<int,int> mapData;
    
    mapData[1]= 100;
    cout<< mapData[1]<<endl;
    return 0;
}

