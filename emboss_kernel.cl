__kernel void processImage(__global const unsigned char* inputImage, 
                           __global unsigned char* outputImage, 
                           int width, 
                           int height, 
                           int channels) 
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    // 1. Boundary Check
    // We cannot process the very edge pixels because they don't have neighbors
    // on all sides. We skip them (or leave them black).
    if (x < 1 || y < 1 || x >= width - 1 || y >= height - 1) {
        return; 
    }

    // 2. The Convolution Matrix (Emboss)
    //    -2  -1   0
    //    -1   1   1
    //     0   1   2
    const int filter[3][3] = {
        {-2, -1,  0},
        {-1,  1,  1},
        { 0,  1,  2}
    };

    // 3. Apply the filter to each channel (R, G, B) independently
    for (int c = 0; c < channels; c++) 
    {
        // If it's the Alpha channel (4th channel), just copy it directly and skip math
        if (c == 3) {
            int currentPixel = (y * width * channels) + (x * channels) + c;
            outputImage[currentPixel] = inputImage[currentPixel];
            continue;
        }

        float sum = 0.0f;

        // Loop through the 3x3 neighbors
        // ky runs from -1 (left/top) to +1 (right/bottom)
        for (int ky = -1; ky <= 1; ky++) {
            for (int kx = -1; kx <= 1; kx++) {
                
                // Calculate neighbor coordinate
                int neighborX = x + kx;
                int neighborY = y + ky;

                // Calculate the exact memory address of that neighbor's specific color channel
                int index = (neighborY * width * channels) + (neighborX * channels) + c;

                // Get value and multiply by the filter weight
                // We add 1 to kx/ky to convert range [-1, 1] to array index [0, 2]
                sum += inputImage[index] * filter[ky + 1][kx + 1];
            }
        }

        // 4. Clamping
        // Convolution can produce negative numbers or numbers > 255.
        // We usually add 128 to Emboss to make it gray-ish instead of mostly black.
        int finalVal = (int)(sum) + 128; 

        if (finalVal < 0) finalVal = 0;
        if (finalVal > 255) finalVal = 255;

        // Write to output
        int outputIndex = (y * width * channels) + (x * channels) + c;
        outputImage[outputIndex] = (unsigned char)finalVal;
    }
}