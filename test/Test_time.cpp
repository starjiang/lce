#include <sys/time.h>
#include <stdint.h>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{

    timeval tv;
    gettimeofday(&tv, 0);
    uint64_t ddwTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    time_t time = ddwTime;
    cout << ddwTime << endl
         << time << endl;

    return 0;
}
