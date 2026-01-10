#include <CL/cl.h>
#include <stdio.h>

int main() {
    cl_platform_id platform_id = NULL;
    cl_uint num_platforms;

    // Ask OpenCL for the number of available platforms
    cl_int ret = clGetPlatformIDs(1, &platform_id, &num_platforms);

    if (ret == CL_SUCCESS) {
        printf("Success! Found %u OpenCL platform(s).\n", num_platforms);
    }
    else {
        printf("Failed to find OpenCL platforms.\n");
    }
    return 0;
}