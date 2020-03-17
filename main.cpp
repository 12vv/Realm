#include "common/RayTracer.h"

#define APP 1

#if APP == 1
#define APPLICATION App1
#include "test/app1.h"
#elif APP == 0
#define APPLICATION TestApp
#include "test/test.h"
#endif

#ifdef _WIN32
#define WAIT_ON_EXIT 1
#else
#define WAIT_ON_EXIT 0
#endif

int main(int argc, char** argv)  
{
    std::unique_ptr<APPLICATION> currentApplication = make_unique<APPLICATION>();
    RayTracer rayTracer(std::move(currentApplication));

    DIAGNOSTICS_TIMER(timer, "Ray Tracer");
    rayTracer.Run();
    DIAGNOSTICS_END_TIMER(timer);

    DIAGNOSTICS_PRINT();

#if defined(_WIN32) && WAIT_ON_EXIT
    int exit = 0;
    std::cin >> exit;
#endif

    return 0;
}
