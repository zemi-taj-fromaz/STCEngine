@echo off
setlocal

rem Relative path to the Vulkan SDK glslc executable
set "glslcPath=..\..\libs\VulkanSDK\1.3.261.0\Bin\glslc.exe"

echo "...compiling shaders"

rem Compile each shader with relative paths
"%glslcPath%" IlluminateShader.vert -o IlluminateShadervert.spv
"%glslcPath%" IlluminateShader.frag -o IlluminateShaderfrag.spv
"%glslcPath%" PlainShader.vert -o PlainShadervert.spv
"%glslcPath%" PlainShader.frag -o PlainShaderfrag.spv
"%glslcPath%" ParticleShader.vert -o ParticleShadervert.spv
"%glslcPath%" ParticleShader.frag -o ParticleShaderfrag.spv
"%glslcPath%" skyboxshader.vert -o skyboxshadervert.spv
"%glslcPath%" skyboxshader.frag -o skyboxshaderfrag.spv
"%glslcPath%" TextureShader.vert -o TextureShadervert.spv
"%glslcPath%" TextureShader.frag -o TextureShaderfrag.spv
"%glslcPath%" GridShader.vert -o GridShadervert.spv
"%glslcPath%" GridShader.frag -o GridShaderfrag.spv
"%glslcPath%" CubemapShader.vert -o CubemapShadervert.spv
"%glslcPath%" CubemapShader.frag -o CubemapShaderfrag.spv
"%glslcPath%" ComputeShader.comp -o ComputeShadercomp.spv 
"%glslcPath%" ComputeShader.vert -o ComputeShadervert.spv
"%glslcPath%" ComputeShader.frag -o ComputeShaderfrag.spv
"%glslcPath%" MandelbulbShader.vert -o MandelbulbShadervert.spv
"%glslcPath%" MandelbulbShader.frag -o MandelbulbShaderfrag.spv
"%glslcPath%" PBRShader.vert -o PBRShadervert.spv
"%glslcPath%" PBRShader.frag -o PBRShaderfrag.spv
"%glslcPath%" AimShader.vert -o AimShadervert.spv
"%glslcPath%" AimShader.frag -o AimShaderfrag.spv
"%glslcPath%" ReloadShader.vert -o ReloadShadervert.spv
"%glslcPath%" ReloadShader.frag -o ReloadShaderfrag.spv
"%glslcPath%" TextShader.vert -o TextShadervert.spv
"%glslcPath%" TextShader.frag -o TextShaderfrag.spv
"%glslcPath%" ButtonShader.vert -o ButtonShadervert.spv
"%glslcPath%" ButtonShader.frag -o ButtonShaderfrag.spv
"%glslcPath%" DeerShader.vert -o DeerShadervert.spv
"%glslcPath%" DeerShader.frag -o DeerShaderfrag.spv
"%glslcPath%" OceanShader.vert -o OceanShadervert.spv
"%glslcPath%" OceanShader.frag -o OceanShaderfrag.spv

echo "Compilation Done"

pause