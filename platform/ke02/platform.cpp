extern "C" {
#include <sysinit.h>
}

extern "C"
void platform_init()
{

}

extern "C"
void SystemInit()
{
    sysinit();
}