/*******************************************************************************************
*
*   raylib [textures] example - Image loading and texture creation
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   Example originally created with raylib 1.3, last time updated with raylib 1.3
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2023 Karim Salem (@kimo-s)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
void normalizeKernel(float *kernel, int size){
    float sum = 0.0f;
    for(int i = 0; i < size; i++)
    {
        sum += kernel[i]; 
    }

    if(sum != 0.0f)
    {
        for(int i = 0; i < size; i++)
        {
            kernel[i] /= sum; 
        }
    }
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    Image image = LoadImage("resources/cat.png");     // Loaded in CPU memory (RAM)

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - image convolution");

    float gaussiankernel[] = {1.0, 2.0, 1.0,
                    2.0, 4.0, 2.0,
                    1.0, 2.0, 1.0};

    float sobelkernel[] = {1.0, 0.0, -1.0,
                    2.0, 0.0, -2.0,
                    1.0, 0.0, -1.0};

    float sharpenkernel[] = {0.0, -1.0, 0.0,
                        -1.0, 5.0, -1.0,
                        0.0, -1.0, 0.0};

    normalizeKernel(gaussiankernel, 9);
    normalizeKernel(sharpenkernel, 9);
    normalizeKernel(sobelkernel, 9);

    Image catSharpend = ImageCopy(image);
    ImageKernelConvolution(&catSharpend, sharpenkernel, 9);
 
    Image catSobel = ImageCopy(image);
    ImageKernelConvolution(&catSobel, sobelkernel, 9);

    Image catGaussian = ImageCopy(image);
    for(int i = 0; i < 6; i++)
    {
        ImageKernelConvolution(&catGaussian, gaussiankernel, 9);
    }

    ImageCrop(&image, (Rectangle){ 0, 0, (float)200, (float)450 });
    ImageCrop(&catGaussian, (Rectangle){ 0, 0, (float)200, (float)450 });
    ImageCrop(&catSobel, (Rectangle){ 0, 0, (float)200, (float)450 });
    ImageCrop(&catSharpend, (Rectangle){ 0, 0, (float)200, (float)450 });
    Texture2D texture = LoadTextureFromImage(image);          // Image converted to texture, GPU memory (VRAM)
    Texture2D catSharpendTexture = LoadTextureFromImage(catSharpend);
    Texture2D catSobelTexture = LoadTextureFromImage(catSobel);
    Texture2D catGaussianTexture = LoadTextureFromImage(catGaussian);
    UnloadImage(image);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM
    UnloadImage(catGaussian);
    UnloadImage(catSobel);
    UnloadImage(catSharpend);

    SetTargetFPS(60);     // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexture(catSharpendTexture, 0, 0, WHITE);
            DrawTexture(catSobelTexture, 200, 0, WHITE);
            DrawTexture(catGaussianTexture, 400, 0, WHITE);
            DrawTexture(texture, 600, 0, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);       // Texture unloading
    UnloadTexture(catGaussianTexture);
    UnloadTexture(catSobelTexture);
    UnloadTexture(catSharpendTexture);

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
