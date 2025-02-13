# STCEngine

This is a small rendering engine developed for personal and academic purposes (Windows only).

Used libraries:
    GLM (mathematics)
    GLFW (windowing and input library)
    STB_Image ( Image loading library)
    tiny_obj_loader (for loading objects in .obj format)
    fftw ( Fourirer transform implementation on CPU side)

Setup to run the project:

1) Download VulkanSDK https://vulkan.lunarg.com/sdk/home#windows
Select these components during installation:
    ![image](https://github.com/zemi-taj-fromaz/DZGEngine/assets/99961022/433e455f-aa3c-4731-899d-607608e973b4)

        To confirm the installation -> open the Bin folder in the installation directory and run vkcube.exe
        If you see an image like this it means it went well:
    ![image](https://github.com/zemi-taj-fromaz/DZGEngine/assets/99961022/715b30e3-2b17-41a5-b0b9-a9e68ad9abfb)

3) Add appropriate directories to "<AdditionalLibraryDirectories>" in the property sheet
    ![Snimka zaslona 2024-06-22 175139](https://github.com/zemi-taj-fromaz/STCEngine/assets/99961022/5c8f3b8d-a492-4db1-a627-8d6d38be876e)



4) Add appropriate directories to "<AdditionalIncludeDirectories>"  in the property sheet
![Snimka zaslona 2024-06-22 175426](https://github.com/zemi-taj-fromaz/STCEngine/assets/99961022/eb75fe87-a005-4e9a-b3fe-a0189d9e1a5e)


6) Set proper path
        
        In "resources/compile.bat" set a proper to glslc.exe for shader compilation. Run this script initially and after that -  in the case of initial shader adding

        example: set "glslcPath=C:\VulkanSDK\1.3.283.0\Bin\glslc.exe"

If you wish to run any of the code, I advize you to open the project files in Visual Studio and run it from there, or move the executable files (x64 folder) to the project directory and run the executable.
Most of the project was written in c++, using VulkanAPI as a graphics API.

//-----------------------------------------------------------------------------------

Currently developed scenes using this engine:

# Disney BRDF

Implementation of disney's physically based lighting model, published in the following paper:
    https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
By adjusting given parameters allows for rendering broad spectrum of surfaces.
![image](https://github.com/zemi-taj-fromaz/STCEngine/assets/99961022/4761f1ad-588a-4206-8704-c6d7a58e7052)


# Ocean waves
Uses FFT implementation on CPU ( instead of the GPU to remain interactive), renders a body of water in real time.
![image](https://github.com/zemi-taj-fromaz/VulkanEngine/assets/99961022/fcb958dc-88e8-496a-b5e2-9116cdc1f624)
![image](https://github.com/user-attachments/assets/5410c430-9316-4fab-af6b-a9388d84179b)


# Mandelbulb 3D

3D variation of the famous Mandelbrot fractal -> Typing keys from 0 to 9 will change the look of the fractal (recomputing it with different parameters)
to produce stunning shapes.

![image](https://github.com/zemi-taj-fromaz/VulkanEngine/assets/99961022/fb309db8-31b2-45a5-8d1c-933f0868132c)

![image](https://github.com/zemi-taj-fromaz/VulkanEngine/assets/99961022/6fe9a1ef-2043-4264-b205-b2a68a0bd5f9)



# Deer Factor

FPS where you're being chased by violent deer.

Map is generated by applying perlin noise to a  field.

![image](https://github.com/zemi-taj-fromaz/VulkanEngine/assets/99961022/59c9da95-3c64-4b3f-9ef7-53dea5e48dbe)




