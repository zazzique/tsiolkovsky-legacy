
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/configuration.h>

#include "Common.h"
#include "GameDefines.h"
#include "Core.h"
#include "main.h"

AAssetManager *asset_manager;
char internal_path[PATH_MAX];

struct engine
{
    struct android_app* app;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    float pixel_scale;
};

AAssetManager *GetAssetManager()
{
	return asset_manager;
}

char *GetInternalPath()
{
	return internal_path;
}

static int OpenGL_Init(struct engine* engine)
{
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
    {
        LogPrint("Unable to eglMakeCurrent");
        return FALSE;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return TRUE;
}

static void OpenGL_Present(struct engine* engine)
{
    if (engine->display == NULL)
    {
        return;
    }

    eglSwapBuffers(engine->display, engine->surface);
}

static void OpenGL_Release(struct engine* engine)
{
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    struct engine* engine = (struct engine*)app->userData;

    if (AKeyEvent_getKeyCode(event) == AKEYCODE_BACK)
    {
    	return 1;
	};

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
    	int32_t action = AMotionEvent_getAction(event);
    	int32_t masked_action = action & AMOTION_EVENT_ACTION_MASK;
    	int32_t pointer_index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
    	size_t pointer_count;
    	float x, y;

    	switch (masked_action)
    	{
    		case AMOTION_EVENT_ACTION_DOWN:
    			x = AMotionEvent_getX(event, 0) / engine->pixel_scale;
    			y = ((float)engine->height - AMotionEvent_getY(event, 0)) / engine->pixel_scale;
   				Core_InputTouchBegan(x, y);
    			break;

    		case AMOTION_EVENT_ACTION_POINTER_DOWN:
    			x = AMotionEvent_getX(event, pointer_index) / engine->pixel_scale;
    			y = ((float)engine->height - AMotionEvent_getY(event, pointer_index)) / engine->pixel_scale;
    		   	Core_InputTouchBegan(x, y);
    		    break;

    		case AMOTION_EVENT_ACTION_UP:
    			x = AMotionEvent_getX(event, 0) / engine->pixel_scale;
    			y = ((float)engine->height - AMotionEvent_getY(event, 0)) / engine->pixel_scale;
    			Core_InputTouchEnded(x, y);
    			break;

    		case AMOTION_EVENT_ACTION_POINTER_UP:
    			x = AMotionEvent_getX(event, pointer_index) / engine->pixel_scale;
    			y = ((float)engine->height - AMotionEvent_getY(event, pointer_index)) / engine->pixel_scale;
    		    Core_InputTouchEnded(x, y);
    		    break;

    		case AMOTION_EVENT_ACTION_MOVE:
    			pointer_count = AMotionEvent_getPointerCount(event);

    			for (int i = 0; i < pointer_count; i ++)
    			{
    				x = AMotionEvent_getX(event, i) / engine->pixel_scale;
    				y = ((float)engine->height - AMotionEvent_getY(event, i)) / engine->pixel_scale;
    				Core_InputTouchMoved(x, y);
    			}
    			break;

    		case AMOTION_EVENT_ACTION_CANCEL:
    			Core_TouchesReset();
    			break;

    		case AMOTION_EVENT_ACTION_OUTSIDE:
    			Core_TouchesReset();
    			break;
    	}

        return 1;
    }

    return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    struct engine* engine = (struct engine*)app->userData;

    switch (cmd)
    {
    	case APP_CMD_INPUT_CHANGED:
    		LogPrint("APP_CMD_INPUT_CHANGED");
    		break;
    	case APP_CMD_INIT_WINDOW:
    		LogPrint("APP_CMD_INIT_WINDOW");
    		break;
    	case APP_CMD_TERM_WINDOW:
    		LogPrint("APP_CMD_TERM_WINDOW");
    		break;
    	case APP_CMD_WINDOW_RESIZED:
			LogPrint("APP_CMD_WINDOW_RESIZED");
			break;
    	case APP_CMD_WINDOW_REDRAW_NEEDED:
			LogPrint("APP_CMD_WINDOW_REDRAW_NEEDED");
			break;
    	case APP_CMD_CONTENT_RECT_CHANGED:
			LogPrint("APP_CMD_CONTENT_RECT_CHANGED");
			break;
    	case APP_CMD_GAINED_FOCUS:
			LogPrint("APP_CMD_GAINED_FOCUS");
			break;
    	case APP_CMD_LOST_FOCUS:
			LogPrint("APP_CMD_LOST_FOCUS");
			break;
    	case APP_CMD_CONFIG_CHANGED:
			LogPrint("APP_CMD_CONFIG_CHANGED");
			break;
    	case APP_CMD_LOW_MEMORY:
    		LogPrint("APP_CMD_LOW_MEMORY");
    		break;
    	case APP_CMD_START:
    		LogPrint("APP_CMD_START");
    		break;
    	case APP_CMD_RESUME:
    	   	LogPrint("APP_CMD_RESUME");
    	   	break;
    	case APP_CMD_SAVE_STATE:
    	   	LogPrint("APP_CMD_SAVE_STATE");
    	   	break;
    	case APP_CMD_PAUSE:
    	   	LogPrint("APP_CMD_PAUSE");
    	   	break;
    	case APP_CMD_STOP:
    	   	LogPrint("APP_CMD_STOP");
    	   	break;
    	case APP_CMD_DESTROY:
    	   	LogPrint("APP_CMD_DESTROY");
    	   	break;
    }

    static BOOL need_to_init = TRUE;

    switch (cmd)
    {
        case APP_CMD_LOW_MEMORY:
        	Core_MemoryWarning();
            break;

        case APP_CMD_GAINED_FOCUS:
            if (engine->app->window != NULL)
            {
            	if (OpenGL_Init(engine))
            	{
            		engine->animating = 1;
            		OpenGL_Present(engine);
            	}

            	if (need_to_init)
            	{
            		int32_t density = AConfiguration_getDensity(app->config);

            		engine->pixel_scale = 1.0f;

            		if (density <= 0)
            			engine->pixel_scale = 1.0f;
            		else
            			engine->pixel_scale = (float)density / 160.0f;

            		int32_t size = AConfiguration_getScreenSize(app->config);

            		U32 screen_size;

            		switch (size)
            		{
            			case ACONFIGURATION_SCREENSIZE_ANY:
            				screen_size = SCREEN_SIZE_SMALL;
            				break;

            			case ACONFIGURATION_SCREENSIZE_SMALL:
            			    screen_size = SCREEN_SIZE_SMALL;
            			    break;

            			case ACONFIGURATION_SCREENSIZE_NORMAL:
            				screen_size = SCREEN_SIZE_NORMAL;
            				break;

            			case ACONFIGURATION_SCREENSIZE_LARGE:
            				screen_size = SCREEN_SIZE_LARGE;
            				break;

            			case ACONFIGURATION_SCREENSIZE_XLARGE:
            				screen_size = SCREEN_SIZE_XLARGE;
            				break;
            		}

            		Core_Init(engine->width, engine->height, engine->pixel_scale, screen_size);
            		need_to_init = FALSE;
            	}
            	else
            	{
            		Core_RestoreResources();
            	}
            }
            break;

        case APP_CMD_LOST_FOCUS:

        	Core_Pause();
        	Core_UnloadResources();
        	OpenGL_Release(engine);

        	engine->animating = 0;
            break;

        case APP_CMD_DESTROY:
        	need_to_init = TRUE;
        	break;

        //case APP_CMD_GAINED_FOCUS:
       //      break;

        //case APP_CMD_LOST_FOCUS:
       //     engine->animating = 0;
        //    OpenGL_Present(engine);
            break;
    }
}

void android_main(struct android_app* state)
{
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    asset_manager = state->activity->assetManager;
    strcpy(internal_path, state->activity->internalDataPath);

    while (1)
    {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
        {
            if (source != NULL)
            {
                source->process(state, source);
            }

            if (state->destroyRequested != 0)
            {
            	Core_Release();
            	OpenGL_Release(&engine);
                return;
            }
        }

        if (engine.animating)
        {
        	Core_Process();
   			Core_Render();
   			OpenGL_Present(&engine);
        }
    }
}

