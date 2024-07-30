# IOSurfaceTest

Perform cross-process rendering on the Mac platform based on IOSurface

---

### Step 1:
Create an `IOSurface` at either end and obtain the `IOSurfaceID` (which is actually a uint32)

1. Set `kIOSurfaceIsGlobal` to `kCFBooleanTrue`
2. The pixel format can only be BGRA

### Step 2:
Send the above `IOSurfaceID` to other processes by any means. After receiving the `IOSurfaceID`, other processes can find the corresponding `IOSurface` through `IOSurfaceLookup`

### Step 3:
In each process, create a CVPixelBuffer / CVOpenGLTextureCache / CVOpenGLTexture through the same IOSurface, and then obtain an OpenGL Texture id

> Note that the type of this OpenGL texture is `GL_TEXTURE_RECTANGLE`, not the more commonly used `GL_TEXTURE_2D`. `sampler2DRect` instead of `sampler2D` needs to be used in the shader