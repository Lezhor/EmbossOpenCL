// OpenCL2cpp.cpp : Defines the entry point for the console application.
//

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <cstdlib>

#include <CL/cl2.hpp>

/*
This is a helper function that reads the kernel function from a file
and returns it as a char array
*/
std::string read_kernel(const std::string& filename) {
	std::string kernel_text;

	std::ifstream kernel_reader;
	kernel_reader.open(filename, std::ios::in);

	std::string line;
	while (std::getline(kernel_reader, line)) {
		kernel_text.append(line);
		kernel_text.append("\n");
	}
	kernel_reader.close();

	return kernel_text;
}


int main()
{
	const int size = 1024;

	/*
	  We also want to fill the array with actual data and then copy it to the device
	*/
	int* a = new int[size];
	int* b = new int[size];
	int* c = new int[size];

	for (size_t i = 0; i < size; i++)
	{
		a[i] = 1;
		b[i] = 2;
	}

	/*
	Everything else will happen inside main function.
	At start we need to get one of the OpenCL platforms.
	This is basically the drivers of our graphics card,
	so platform can be from Nvidia, Intel, AMD.
	*/

	//get all platforms (drivers)
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);

	if (all_platforms.size() == 0) {
		std::cout << " No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Platform default_platform = all_platforms[0];
	std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

	/*
	Once we selected the first platform (default_platform) we will use it
	in the next steps. Now we need to get device of our platform.
	For example AMD's platform has support for multiple devices
	(CPU's and GPU's). We will now select the first device (default_device):
	*/
	//get default device of the default platform
	std::vector<cl::Device> all_devices;
	default_platform.getDevices(CL_DEVICE_TYPE_GPU, &all_devices);

	if (all_devices.size() == 0) {
		std::cout << " No devices found. Check OpenCL installation!\n";
		exit(1);
	}

	cl::Device default_device = all_devices[0];
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

	/*
	Now we need to create a Context. Imagine the Context
	as the runtime link to the our device and platform:
	*/
	cl::Context context({ default_device });

	//create queue to which we will push commands for the device.
	cl::CommandQueue queue(context, default_device);

	/*
	Next we need to create the program which we want to execute on our device:
	*/
	cl::Program::Sources sources;

	/*
	Actual source of our program(kernel) is there:
	*/

	/*
	// We can use C++11 raw string literals for kernel source code
	std::string kernel1{ R"CLC(
		kernel void vectorAdd(global const int* a, global const int* b, global int* c, const int size)
		{
			int i = get_local_size( 0 ) * get_group_id( 0 ) + get_local_id( 0 );

			if (i < size)
			{
				c[i] = a[i] + b[i];
			}
		})CLC" };
  */

  // or we can use our function to read the kernel code from file:
	std::string kernel1 = read_kernel("vector_add_kernel.cl");


	/*
	Next we need our kernel sources to build. We also check for the errors at building.
	Sources is a list of pairs <kernel-code, string length>
	*/
	sources.push_back({ kernel1.c_str(), kernel1.length() });

	cl::Program program(context, sources);
	if (program.build({ default_device }) != CL_SUCCESS) {
		std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
		exit(1);
	}

	// Create a Kernel and specify its name
	cl::Kernel vectorAddKernel(program, "vectorAdd");



	/*
	For arrays A, B, C we need to allocate the space on the device:
	*/

	size_t memSize = sizeof(int) * size;

	// create buffers on the device
	cl::Buffer dev_a(context, CL_MEM_READ_ONLY, memSize);
	cl::Buffer dev_b(context, CL_MEM_READ_ONLY, memSize);
	cl::Buffer dev_c(context, CL_MEM_READ_WRITE, memSize);


	/*
	We need to copy arrays from A and B to the device.
	This means that we will copy arrays from the host to the device.
	Host represents our main. At first we need to create a queue
	which is the queue to the commands we will send to the our device:
	*/

	/*
	Now we can copy data from arrays A and B to dev_A and dev_B
	which represent memory on the device:
	*/
	//write arrays A and B to the device
	queue.enqueueWriteBuffer(dev_a, CL_TRUE, 0, memSize, a);
	queue.enqueueWriteBuffer(dev_b, CL_TRUE, 0, memSize, b);


	/*
	Now we can run the kernel which in parallel sums A and B and writes to C.
	We do this with KernelFunctor which runs the kernel on the device.
	In quotes we specify the name of the Kernel function (same as in code above)
	*/

	// Set the arguments of the kernel-function
	vectorAddKernel.setArg(0, dev_a);
	vectorAddKernel.setArg(1, dev_b);
	vectorAddKernel.setArg(2, dev_c);
	vectorAddKernel.setArg(3, size);

	// finally, run the kernel:
	int localSize = default_device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
	int globalSize = std::ceil((double)size / (double)localSize) * localSize;

	queue.enqueueNDRangeKernel(vectorAddKernel, cl::NullRange, cl::NDRange(globalSize), cl::NDRange(localSize));

	/*
	At the end we want to read the results in buffer C from device memory
	back to host memory:
	*/
	queue.enqueueReadBuffer(dev_c, CL_TRUE, 0, memSize, c);

	double avg = 0.0;

	for (int i = 0; i < size; i++)
	{
		avg += (c[i] - avg) / (i + 1); // running average
	}

	std::cout << "Average is: " << avg << std::endl;

	// close the command queue
	// all other objects have distructors and will close themselves
	queue.finish();

	return 0;
}