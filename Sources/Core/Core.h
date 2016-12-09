
#ifndef _TREMOR_CORE_H_
#define _TREMOR_CORE_H_

void Core_Init(int init_screen_width, int init_screen_height, float init_pixel_scale, U32 init_screen_size);
void Core_Process();
void Core_Render();
void Core_Release();

void Core_Pause();

void Core_RestoreResources();
void Core_UnloadResources();

void Core_MemoryWarning();

void Core_InputTouchBegan(float x, float y);
void Core_InputTouchMoved(float x, float y);
void Core_InputTouchEnded(float x, float y);
void Core_TouchesReset();

#endif /* _TREMOR_CORE_H_ */