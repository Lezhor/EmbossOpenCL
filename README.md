# EmbossOpenCL

This repository contains all files for the Emboss with OpenCL Task for the Lecture "Programmierkonzepte und Algorithmen" by Nikita Kovalenko 2025/26.

The relevant Files are explained here:
- `EmbossOpenCL.cpp` - Entrypoint for the Project
- `EmbossCPUReference.cpp` - Currently disableed. Same convolution-logic but single-threaded.
- `utils.h` - helper function for reading kernel flie
- `gray_and_emboss_2_kernel.cl` - kernel file

Also there are some images in the root:
- `input.jpg` - The input image used throughout the project (2048x1536)
- `output_small_kernel.jpg`, `output.jpg`, `output_git_kernel.jpg` - Outputs based on specific kernels $w_1$, $w_2$, $w_3$ (see chapter 2.2 in Documentation)
