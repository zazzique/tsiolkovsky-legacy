
#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

void ApplyColorModificationsF(float *r, float *g, float *b);
void ApplyColorModifications(U8 *r, U8 *g, U8 *b);

void TexManager_Init();
int	 TexManager_AddTexture(char *filename, U32 user_flags);
BOOL TexManager_LoadTexture(int index, char *filename);
BOOL TexManager_UnloadTexture(int index);
void TexManager_LoadAll();
void TexManager_UnloadAll();
int  TexManager_GetTextureIndexByName(char *name);
void TexManager_SetTextureByIndex(int index);
void TexManager_SetTextureByName(char *name);
void TexManager_GetTextureResolutionByIndex(int index, I32 *width, I32 *height);
void TexManager_GetTextureResolutionByName(char *name, I32 *width, I32 *height);
void TexManager_RemoveTextureByIndex(int index);
void TexManager_RemoveTextureByName(char *name);
void TexManager_RemoveTexturesByFlag(U32 user_flag);
void TexManager_RemoveAllTextures();
void TexManager_Release();

#endif /* _TEXTURE_MANAGER_H_ */
