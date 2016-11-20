
#ifndef _TREMOR_RENDER_H_
#define _TREMOR_RENDER_H_

enum TRVariableType
{
	TR_UNSIGNED_BYTE,
	TR_BYTE,
	TR_UNSIGNED_SHORT,
	TR_SHORT,
	TR_FLOAT
};

enum TRPrimitiveType
{
	TR_POINTS,
	TR_LINES,
	TR_LINE_LOOP,
	TR_LINE_STRIP,
	TR_TRIANGLES,
	TR_TRIANGLE_STRIP,
	TR_TRIANGLE_FAN
};

enum TRBlendType
{
    TR_ZERO,
    TR_ONE,
    TR_SRC_COLOR,
    TR_SRC_ALPHA,
    TR_ONE_MINUS_SRC_ALPHA,
    TR_DST_COLOR
};

enum TRMatrixMode
{
	TR_PROJECTION,
	TR_MODEL,
	TR_TEXTURE
};

BOOL Render_Init(I32 width, I32 height);
void Render_Release();

void Render_CreateTexture(U32 *texture_id_ptr, U8 *image_data, I32 width, I32 height, I32 bpp, BOOL compressed, I32 compressed_size);
void Render_BindTexture(U32 texture_id);
void Render_DeleteTexture(U32 *texture_id_ptr);

void Render_ActiveTextureLayer(int layer);

void Render_Clear(float r, float g, float b, float a);

void Render_SetMatrixMode(enum TRMatrixMode matrix_mode);
void Render_ResetMatrix();
void Render_SetMatrix(float *m);
void Render_MatrixTranslate(float x, float y, float z);
void Render_MatrixRotate(float angle, float x, float y, float z);
void Render_MatrixScale(float x, float y, float z);
void Render_PushMatrix();
void Render_PopMatrix();
void Render_SetProjectionOrtho(float z_near, float z_far, float scale);
void Render_SetProjectionFrustum(float z_near, float z_far, float fov_x, float fov_y);

void Render_SetColor(float r, float g, float b, float a);
void Render_SetBlendFunc(enum TRBlendType sfactor, enum TRBlendType dfactor);

void Render_EnableFaceCulling();
void Render_DisableFaceCulling();
void Render_EnableTextures();
void Render_DisableTextures();
void Render_EnableBlend();
void Render_DisableBlend();
void Render_EnableDepthMask();
void Render_DisableDepthMask();
void Render_EnableDepthTest();
void Render_DisableDepthTest();
void Render_EnableAlphaTest();
void Render_DisableAlphaTest();
void Render_EnableVertexArray();
void Render_DisableVertexArray();
void Render_EnableColorArray();
void Render_DisableColorArray();
void Render_EnableTexCoordArray();
void Render_DisableTexCoordArray();
void Render_EnableIndexArray();
void Render_DisableIndexArray();

void Render_SetVertexArray(void *pointer, int size, enum TRVariableType type, int stride);
void Render_SetColorArray(void *pointer, int size, enum TRVariableType type, int stride);
void Render_SetTexCoordArray(void *pointer, int size, enum TRVariableType type, int stride);
void Render_SetIndexArray(void *pointer, enum TRVariableType type);

void Render_DrawArrays(enum TRPrimitiveType primitive_type, int vertex_count);

#endif /* _TREMOR_RENDER_H_ */
