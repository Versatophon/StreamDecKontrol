
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>

#include <thread>
#include "StreamDeckRawDevice.h"

#include "StreamDeckWindow.h"

int32_t main(int32_t pArgC, char** pArgV)
{
    StreamDeckWindow* lStreamDeckWindow = new StreamDeckWindow();

    int32_t lReturnValue = lStreamDeckWindow->Execute();

    delete lStreamDeckWindow;

    return lReturnValue;
}
