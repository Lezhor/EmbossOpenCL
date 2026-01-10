#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "opencltest.h"

int main() {
    cl_int status;

    // 1. Get the number of platforms
    cl_uint num_platforms;
    status = clGetPlatformIDs(0, NULL, &num_platforms);

    // 2. Prepare an array and get the platform IDs
    cl_platform_id* all_platforms = new cl_platform_id[num_platforms];
    status = clGetPlatformIDs(num_platforms, all_platforms, NULL);

    if (status != CL_SUCCESS || num_platforms == 0) {
        std::cout << "No OpenCL platforms found." << std::endl;
        return 1;
    }

    // 3. Get the devices for the first platform [0]
    cl_uint num_devices;
    status = clGetDeviceIDs(all_platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    
    std::cout << "The platform #0 has " << num_devices << " devices in it" << std::endl;

    // 4. Get the device ids
    cl_device_id* all_devices = new cl_device_id[num_devices];
    status = clGetDeviceIDs(all_platforms[0], CL_DEVICE_TYPE_ALL, num_devices, all_devices, NULL);

    // 5. Print the name of the first device just to be sure
    for (int i = 0; i < sizeof(all_devices); i++) {
		char deviceName[1024];
		clGetDeviceInfo(all_devices[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
		std::cout << "Device #" << i << " name: " << deviceName << std::endl;
    }

	//std::cout << "These are the devices available: " << std::endl;
	//for (int i = 0; i < num_devices; i++)
	//{
	//	// get device name
	//	char device_name[128];
	//	status = clGetDeviceInfo(all_devices[i], CL_DEVICE_NAME, sizeof(char) * 128, device_name, NULL);
	//	std::cout << "Device #" << i << ", name: " << device_name << std::endl;
	//}

	std::cout << "These are the devices available: " << std::endl;
	for (int i = 0; i < all_devices.size(); i++) {
		std::cout << "Device #" << i << ", name: "
			<< all_devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
	}

    // Cleanup
    delete[] all_platforms;
    delete[] all_devices;

    return 0;
}
