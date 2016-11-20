//
//  ModelManager.h
//  Carnivores
//
//  Created by Dmitry Nechay on 16.07.09.
//  Copyright 2009 Action Forms. All rights reserved.
//

#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_

#define MODELS_DEFAULT_SCALE 1.0f
#define ANIMATIONS_DEFAULT_SCALE 0.125f

#define MAX_MODELS_COUNT 128
#define MAX_MODELS_IN_LIST 1024
#define MAX_MODELS_OF_ONE_TYPE_IN_LIST 128


#define FF_DOUBLE_SIDE 0x0001
#define FF_DARK_BACK   0x0002
#define FF_TRANSPARENT 0x0004
#define FF_HIDDEN      0x0008
#define FF_MORTAL      0x0010
#define FF_PHONG       0x0020
#define FF_ENV_MAP     0x0040
#define FF_NEED_VC     0x0080
#define FF_DARK        0x8000

#define MF_STATIC      0x0001
#define MF_ANIMATED    0x0002
#define MF_ACCEPT_WIND 0x0004

typedef struct _ModelPosition3D
{
	float x;
	float y;
	float z;
	float alpha;
	float beta;
	float gamma;
}
ModelPosition3D;

typedef struct _ModelTriangle
{
	I32 point_indices[3];
	U16 flags;
	U16 dmask;
}
ModelTriangle;

typedef struct _RenderModelAniState
{
	int valid;
	
	float cross_interpolation_k;
	
	I16 *ani_data_1;
	I16 *ani_data_2;
	int frame_1;
	int frame_2;
	float interpolation_k;
	
	I16 *blend_ani_data_1;
	I16 *blend_ani_data_2;
	int blend_frame_1;
	int blend_frame_2;
	float blend_interpolation_k;
}
RenderModelAniState;

typedef struct _Model
{
	char filename[32];
	char texture[32];
	int texture_index;
	float bound_top;
	float bound_left;
	float bound_right;
	float bound_radius_sqr;
	float sqr;
	U32 flags;
	U32 user_flags;
	I32 num_points;
	I32 num_triangles;
	I32 num_indices;
	ModelTriangle *triangles;
	Vector3D *points;
	I16 *tex_coords_array;
	float *vertex_array;
	U16 *index_array;
	BOOL valid;
}
Model;

extern Model models[MAX_MODELS_COUNT];

inline float ModelManager_GetModelSQR(int index)
{
	return models[index].sqr;
}

inline float ModelManager_GetModelBoundHeight(int index)
{
	return models[index].bound_top;
}

inline float ModelManager_GetModelBoundLeft(int index)
{
	return models[index].bound_left;
}

inline float ModelManager_GetModelBoundRight(int index)
{
	return models[index].bound_right;
}

inline float ModelManager_GetModelBoundRadiusSQR(int index)
{
	return models[index].bound_radius_sqr;
}

void ModelManager_Init();
void ModelManager_RenderModelsInList();
void ModelManager_RenderModelByIndex(int index, RenderModelAniState* ani_state);
void ModelManager_RenderModelByName(char *name, RenderModelAniState* ani_state);
void ModelManager_RenderModelByIndexWithCustomTexture(int index, RenderModelAniState* ani_state, int texture_index);
int	 ModelManager_AddModel(char *filename, U32 flags, U32 user_flags, float scale);
int  ModelManager_GetModelIndexByName(char *name);
void ModelManager_PushToRenderListByIndex(int index, ModelPosition3D *pos, float *color, float scale, RenderModelAniState* ani_state);
void ModelManager_PushToRenderListByName(char *name, ModelPosition3D *pos, float *color, float scale, RenderModelAniState* ani_state);
void ModelManager_Animate(Model *model, RenderModelAniState* ani_state);
void ModelManager_AnimateWind(Model *model, const ModelPosition3D *pos);
void ModelManager_RemoveModelByIndex(int index);
void ModelManager_RemoveModelByName(char *name);
void ModelManager_RemoveModelsByFlag(U32 user_flag);
void ModelManager_RemoveAllModels();
void ModelManager_Release();

void ModelManager_SetMatrix(float x, float y, float z, float alpha, float beta, float gamma, float scale);
void ModelManager_SetMatrixQ(float x, float y, float z, float alpha, float beta, float gamma, float scale);
void ModelManager_SetCustomMatrix(float *m);

BOOL ModelManager_GetVertexPos(int index, ModelPosition3D *pos, float scale, RenderModelAniState* ani_state, int vertex_index, Vector3D *result);
BOOL ModelManager_GetVertexPosAndNormal(int index, ModelPosition3D *pos, float scale, RenderModelAniState* ani_state, int vertex_index, Vector3D *result_pos, Vector3D *result_normal);
int  ModelManager_TraceModel(int index, ModelPosition3D *pos, float scale, RenderModelAniState* ani_state, Vector3D *trace_a, Vector3D *trace_b, Vector3D *trace_nv, U16 *flags, Vector3D *triangle_normal);
int  ModelManager_FindClosestPoint(int index, ModelPosition3D * pos, float scale, Vector3D *distpoint, float spheredist, Vector3D *closest, Vector3D *closest_triangle_normal);

#endif /* _MODEL_MANAGER_H_ */
