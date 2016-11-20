
LOCAL_PATH := $(call my-dir)

#
# Game
#
include $(CLEAR_VARS)

LOCAL_MODULE           := ancient-wings
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/Andr01d/FMOD/inc \
                          $(LOCAL_PATH)/Core \
                          $(LOCAL_PATH)/Game \
                          $(LOCAL_PATH)/Andr01d
LOCAL_CFLAGS           += -std=c99
LOCAL_SRC_FILES        := Core\Core.c \
                          Core\CoreVariables.c \
                          Core\FastMath.c \
                          Core\Font.c \
                          Core\GUIControls.c \
                          Core\ModelManager.c \
                          Core\Sound.c \
                          Core\Sprites.c \
                          Core\TextureManager.c \
                          Core\Vector.c \
                          Game\Game.c \
                          Game\GameVariables.c \
                          Game\Menu.c \
                          Andr01d\Log.c \
                          Andr01d\Files.c \
                          Andr01d\GameConfig.c \
                          Andr01d\main.c \
                          Andr01d\Render.c \
                          Andr01d\Timer.c

LOCAL_LDLIBS           := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
