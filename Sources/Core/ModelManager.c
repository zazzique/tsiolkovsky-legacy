//
//  ModelManager.m
//  Carnivores
//
//  Created by Dmitry Nechay on 16.07.09.
//  Copyright 2009 Action Forms. All rights reserved.
//

#include "Common.h"

#include "Files.h"
#include "FastMath.h"
#include "Vector.h"
#include "TextureManager.h"
#include "Render.h"
#include "ModelManager.h"


#define POINTS_IN_ANI_CACHE 4096

typedef struct _RenderModel
{
	int index;
	ModelPosition3D pos;
	RenderModelAniState ani_state;
	float color[4];
	float scale;
}
RenderModel;

typedef struct _ModelCache
{
	int count;
	U16 indices[MAX_MODELS_OF_ONE_TYPE_IN_LIST];
}
ModelCache;


Model models[MAX_MODELS_COUNT];

RenderModel models_render_list[MAX_MODELS_IN_LIST];
int models_in_list_count;

ModelCache models_cache[MAX_MODELS_COUNT];
Vector3D points_cache[POINTS_IN_ANI_CACHE];


void ModelManager_Init()
{
	for (int i = 0; i < MAX_MODELS_COUNT; i++)
		models[i].valid = FALSE;
	
	for (int i = 0; i < MAX_MODELS_COUNT; i++)
	{
		models_cache[i].count = 0;
	}
	
	models_in_list_count = 0;
}


void ModelManager_RenderModelsInList()
{
	RenderModel *rmptr;
	
	int using_alpha = -1;
    
    Render_SetMatrixMode(TR_TEXTURE);
    Render_ResetMatrix();
    Render_MatrixScale(1.0f / 255.0f, 1.0f / 255.0f, 1.0f);
	
	Render_SetMatrixMode(TR_MODEL);
    
    Render_EnableTextures();
	Render_EnableVertexArray();
    Render_DisableColorArray();
    Render_EnableTexCoordArray();
    Render_EnableIndexArray();
    
	int i, j;
	
	for (i = 0; i < MAX_MODELS_COUNT; i++)
	{
		if (models_cache[i].count == 0)
			continue;
		
		TexManager_SetTextureByIndex(models[i].texture_index);
        
	
        Render_SetVertexArray(models[i].vertex_array, 3, TR_FLOAT, 0);
        Render_SetTexCoordArray(models[i].tex_coords_array, 2, TR_SHORT, 0);
        Render_SetIndexArray(models[i].index_array, TR_UNSIGNED_SHORT);
				
		
		for (j = 0; j < models_cache[i].count; j++)
		{
			rmptr = &models_render_list[models_cache[i].indices[j]];
			
			if (rmptr->ani_state.valid)
			{
				ModelManager_Animate(&models[i], &rmptr->ani_state);
			}
			if (models[i].flags & MF_ACCEPT_WIND)
			{
				ModelManager_AnimateWind(&models[i], &rmptr->pos);
			}
			
			if (rmptr->color[3] == 1.0f)
			{
				if (using_alpha != 0)
				{
                    Render_EnableAlphaTest();
				}
				using_alpha = 0;
			}
			else
			{
				if (using_alpha != 1)
				{
					Render_DisableAlphaTest();
				}
				using_alpha = 1;
			}
			
			ModelManager_SetMatrix(rmptr->pos.x, rmptr->pos.y, rmptr->pos.z, -rmptr->pos.alpha, -rmptr->pos.beta, -rmptr->pos.gamma, rmptr->scale);
						
			Render_SetColor(rmptr->color[0], rmptr->color[1], rmptr->color[2], rmptr->color[3]);
			
            Render_DrawArrays(TR_TRIANGLES, models[i].num_indices);
		}
		
		models_cache[i].count = 0;
	}
	
	Render_SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	models_in_list_count = 0;
}


void ModelManager_RenderModelByIndex(int index, RenderModelAniState* ani_state)
{
	if (index >= 0 && index < MAX_MODELS_COUNT)
	{
		ModelManager_RenderModelByIndexWithCustomTexture(index, ani_state, models[index].texture_index);
	}
}


void ModelManager_RenderModelByName(char *name, RenderModelAniState* ani_state)
{
	int index = ModelManager_GetModelIndexByName(name);
	
	ModelManager_RenderModelByIndex(index, ani_state);
}


void ModelManager_RenderModelByIndexWithCustomTexture(int index, RenderModelAniState* ani_state, int texture_index)
{
	if (index >= 0 && index < MAX_MODELS_COUNT)
	{
		if (models[index].valid)
		{
			if (ani_state != NULL)
			{
				if (ani_state->valid)
				{
					ModelManager_Animate(&models[index], ani_state);
				}
			}
            
			if (texture_index >= 0)
			{
				Render_EnableTextures();
				Render_EnableTexCoordArray();
			}
			else
			{
				Render_DisableTextures();
				Render_DisableTexCoordArray();
			}
            Render_EnableVertexArray();
            Render_DisableColorArray();
            Render_EnableIndexArray();
			
			if (texture_index >= 0)
				TexManager_SetTextureByIndex(texture_index);
			
            Render_SetVertexArray(models[index].vertex_array, 3, TR_FLOAT, 0);

			if (texture_index >= 0)
				Render_SetTexCoordArray(models[index].tex_coords_array, 2, TR_SHORT, 0);

            Render_SetIndexArray(models[index].index_array, TR_UNSIGNED_SHORT);
			
			Render_DrawArrays(TR_TRIANGLES, models[index].num_indices);
		}
	}
}


void ModelManager_SetMatrix(float x, float y, float z, float alpha, float beta, float gamma, float scale)
{
	float mm[4][4];
	float ca, sa, cb, sb, cg, sg;
	
    FastSinCos(alpha, &sa, &ca);

	mm[3][0] = x;
	mm[3][1] = y;
	mm[3][2] = z;
	
	mm[0][3] = 0.0f; mm[1][3] = 0.0f; mm[2][3] = 0.0f; mm[3][3] = 1.0f;
	
	if (beta == 0.0f && gamma == 0.0f)
	{
		mm[0][0] = ca * scale;   mm[1][0] = 0.0f;   mm[2][0] = sa * scale;
		mm[0][1] = 0.0f;         mm[1][1] = scale;  mm[2][1] = 0.0f;
		mm[0][2] = -sa * scale;  mm[1][2] = 0.0f;   mm[2][2] = ca * scale;
	}
	else
	{
		FastSinCos(beta, &sb, &cb);
		FastSinCos(gamma, &sg, &cg);
		
		mm[0][0] = (cg * ca + sg * sb * sa) * scale;  mm[1][0] = (cg * sb * sa - sg * ca) * scale;  mm[2][0] = (cb * sa) * scale;
		mm[0][1] = (sg * cb) * scale;                 mm[1][1] = (cg * cb) * scale;                 mm[2][1] = (-sb) * scale;
		mm[0][2] = (sg * sb * ca - cg * sa) * scale;  mm[1][2] = (sg * sa + cg * sb * ca) * scale;  mm[2][2] = (cb * ca) * scale;
	}
	
	Render_SetMatrixMode(TR_MODEL);
	Render_SetMatrix(&mm[0][0]);
}


void ModelManager_SetMatrixQ(float x, float y, float z, float alpha, float beta, float gamma, float scale)
{
	float mm[4][4];
	float ca = cosf(alpha);
	float sa = sinf(alpha);      
	float cb = cosf(beta);
	float sb = sinf(beta);   
	float cg = cosf(gamma);
	float sg = sinf(gamma);
	
	mm[3][0] = x;
	mm[3][1] = y;
	mm[3][2] = z;
	
	mm[0][3] = 0.0f; mm[1][3] = 0.0f; mm[2][3] = 0.0f; mm[3][3] = 1.0f;
	
	mm[0][0] = (cg * ca + sg * sb * sa) * scale;  mm[1][0] = (cg * sb * sa - sg * ca) * scale;  mm[2][0] = (cb * sa) * scale;
	mm[0][1] = (sg * cb) * scale;                 mm[1][1] = (cg * cb) * scale;                 mm[2][1] = (-sb) * scale;
	mm[0][2] = (sg * sb * ca - cg * sa) * scale;  mm[1][2] = (sg * sa + cg * sb * ca) * scale;  mm[2][2] = (cb * ca) * scale;
	
	Render_SetMatrixMode(TR_MODEL);
	Render_SetMatrix(&mm[0][0]);
}


void ModelManager_SetCustomMatrix(float *m)
{
	Render_SetMatrixMode(TR_MODEL);
	Render_SetMatrix(m);
}


void ModelManager_AnimateCache(Model *model, RenderModelAniState* ani_state)
{
	int i;
	
	if (model->num_points > POINTS_IN_ANI_CACHE)
	{
		LogPrint("Error: too many points in model!\n");
		return;
	}
	
	if (ani_state == NULL)
	{
		for (i = 0; i < model->num_points; i++)
		{
			points_cache[i] = model->points[i];
		}
		
		return;
	}
	
	if (!ani_state->valid)
	{
		for (i = 0; i < model->num_points; i++)
		{
			points_cache[i] = model->points[i];
		}
		
		return;
	}
	
	Vector3D v1, v2;
	
	float ik = ani_state->interpolation_k;
	float one_minus_ik = 1.0f - ani_state->interpolation_k;
	
	float blend_ik = ani_state->blend_interpolation_k;
	float blend_one_minus_ik = 1.0f - ani_state->blend_interpolation_k;
	
	float cross_ik = ani_state->cross_interpolation_k;
	float cross_one_minus_ik = 1.0f - ani_state->cross_interpolation_k;
	
	I16 *frame_points_1 = &ani_state->ani_data_1[model->num_points * 3 * ani_state->frame_1];
	I16 *frame_points_2 = &ani_state->ani_data_2[model->num_points * 3 * ani_state->frame_2];
	I16 *blend_frame_points_1;
	I16 *blend_frame_points_2;
	
	if (cross_ik < 1.0f)
	{
		blend_frame_points_1 = &ani_state->blend_ani_data_1[model->num_points * 3 * ani_state->blend_frame_1];
		blend_frame_points_2 = &ani_state->blend_ani_data_2[model->num_points * 3 * ani_state->blend_frame_2];
		
		for (i = 0; i < model->num_points; i++)
		{
			
			v1.x = -(float)*frame_points_1 * one_minus_ik - (float)*frame_points_2 * ik;
			v2.x = -(float)*blend_frame_points_1 * blend_one_minus_ik - (float)*blend_frame_points_2 * blend_ik;
			
			frame_points_1 ++;
			frame_points_2 ++;
			blend_frame_points_1 ++;
			blend_frame_points_2 ++;
			
			v1.y = (float)*frame_points_1 * one_minus_ik + (float)*frame_points_2 * ik;
			v2.y = (float)*blend_frame_points_1 * blend_one_minus_ik + (float)*blend_frame_points_2 * blend_ik;
			
			frame_points_1 ++;
			frame_points_2 ++;
			blend_frame_points_1 ++;
			blend_frame_points_2 ++;
			
			v1.z = (float)*frame_points_1 * one_minus_ik + (float)*frame_points_2 * ik;
			v2.z = (float)*blend_frame_points_1 * blend_one_minus_ik + (float)*blend_frame_points_2 * blend_ik;
			
			frame_points_1 ++;
			frame_points_2 ++;
			blend_frame_points_1 ++;
			blend_frame_points_2 ++;
			

			points_cache[i].x = (v1.x * cross_ik + v2.x * cross_one_minus_ik) * ANIMATIONS_DEFAULT_SCALE;
			points_cache[i].y = (v1.y * cross_ik + v2.y * cross_one_minus_ik) * ANIMATIONS_DEFAULT_SCALE;
			points_cache[i].z = (v1.z * cross_ik + v2.z * cross_one_minus_ik) * ANIMATIONS_DEFAULT_SCALE;
		}
	}
	else
	{
		for (i = 0; i < model->num_points; i++)
		{
			
			v1.x = -(float)*frame_points_1 * one_minus_ik - (float)*frame_points_2 * ik;
			
			frame_points_1 ++;
			frame_points_2 ++;
						
			v1.y = (float)*frame_points_1 * one_minus_ik + (float)*frame_points_2 * ik;
			
			frame_points_1 ++;
			frame_points_2 ++;
						
			v1.z = (float)*frame_points_1 * one_minus_ik + (float)*frame_points_2 * ik;
	
			frame_points_1 ++;
			frame_points_2 ++;
			
			
			points_cache[i].x = v1.x * ANIMATIONS_DEFAULT_SCALE;
			points_cache[i].y = v1.y * ANIMATIONS_DEFAULT_SCALE;
			points_cache[i].z = v1.z * ANIMATIONS_DEFAULT_SCALE;
		}
	}
}


void ModelManager_Animate(Model *model, RenderModelAniState* ani_state)
{
	int i, j;
	int point_index;
	
	ModelManager_AnimateCache(model, ani_state);
	
	float *ac_p = &model->vertex_array[0];
	
	for (i = 0; i < model->num_triangles; i++)
	{
		for (j = 0; j < 3; j++)
		{
			point_index = model->triangles[i].point_indices[j];
			
			*ac_p = points_cache[point_index].x; ac_p ++;
			*ac_p = points_cache[point_index].y; ac_p ++;
			*ac_p = points_cache[point_index].z; ac_p ++;
		}
	}
}

void ModelManager_AnimateWind(Model *model, const ModelPosition3D *pos)
{
	int i, j;
	int point_index;
	
	float *ac_p = &model->vertex_array[0];
	
	const float amplitude = 16.0f;
	const float amplitude2 = 4.0f;
	
	const float grass_w = 2.0f;
	const float tree_w = 1.0f;
	
	const float grass_size_middle = 256.0f;
	const float grass_size_interval = 512.0f;

	float w = tree_w;
	if (fabsf(model->bound_top - grass_size_middle) < ( grass_size_interval / 2.0f))
		w += sinf(fabsf(model->bound_top - grass_size_middle) * pi / grass_size_interval) * grass_w;	

	float phi = pos->x - pos->z;
	float xkoef = sinf(w * level_time + phi) * amplitude;
	float ykoef = cosf(w * level_time + phi) * amplitude2;
	
	for (i = 0; i < model->num_triangles; i++)
	{
		for (j = 0; j < 3; j++)
		{
			point_index = model->triangles[i].point_indices[j];
			
			float x = model->points[point_index].x;
			float y = model->points[point_index].y;
			float z = model->points[point_index].z;
			
			float heightkoef = y / model->bound_top;
			
			x += xkoef * heightkoef;
			y += ykoef * heightkoef;
			
			*(ac_p++) = x; 
			*(ac_p++) = y;
			*(ac_p++) = z;
		}
	}
}

int	 ModelManager_AddModel(char *filename, U32 flags, U32 user_flags, float scale)
{
	FileHandler model_file;
	
	int i, j, k;
	
	int index = ModelManager_GetModelIndexByName(filename);
	if (index >= 0)
		return index;
	
	U32 face_info;
	
	Vector3D c_pos;
	Vector3D v_pos;
	
	float dist_sqr;
	float bound_x;
	
	U32 sprite_enabled;
	
	index = -1;
		
	for (i = 0; i < MAX_MODELS_COUNT; i++)
	{
		if (!models[i].valid)
		{
			index = i;
			break;
		}
	}
	
	if (index < 0)
	{
		LogPrint("Error: too many models!\n");
		return -1;
	}
	
	strncpy(models[index].filename, filename, 31);
	models[index].flags = flags;
	models[index].user_flags = user_flags;
	
	if (!Files_OpenFile(&model_file, filename))
	{
		LogPrint("Error: model '%s' not found!\n", filename);
		return -1;
	}
	
	if (strcmp(Files_GetFileExtension(&model_file), "3dn") != 0)
	{
		LogPrint("Error: unsupported model format ('%s')!\n", filename);
		return -1;
	}
	
	
	Files_Read(&model_file, &models[index].num_points, sizeof(I32));
	
	Files_Read(&model_file, &models[index].num_triangles, sizeof(I32));
	
	Files_Skip(&model_file, sizeof(I32));

	
	Files_Read(&model_file, &models[index].texture, sizeof(models[index].texture));
	
	strcat(models[index].texture, STR_DEFAULT_TEXTURE_EXTENTION);
	models[index].texture_index = TexManager_AddTexture(models[index].texture, FLAG_MODEL_TEXTURE);
	
	Files_Read(&model_file, &sprite_enabled, sizeof(U32));
	
	if (sprite_enabled != 0)
	{
		Files_Skip(&model_file, 32);
	}
	
	models[index].points = (Vector3D *)malloc(sizeof(Vector3D) * models[index].num_points);
	models[index].tex_coords_array = (I16 *)malloc(sizeof(I16) * models[index].num_triangles * 6);
	models[index].vertex_array = (float *)malloc(sizeof(float) * models[index].num_triangles * 9);
	models[index].index_array = (U16 *)malloc(sizeof(U16) * models[index].num_triangles * 3 * 2);
	models[index].triangles = (ModelTriangle *)malloc(sizeof(ModelTriangle) * models[index].num_triangles);
	
	// Points sub-block: 
	models[index].bound_left = 0;
	models[index].bound_right = 0;
	models[index].bound_top = 0;
	
	for (j = 0; j < models[index].num_points; j++)
	{
		Files_Read(&model_file, &models[index].points[j].x, sizeof(float));
		Files_Read(&model_file, &models[index].points[j].y, sizeof(float));
		Files_Read(&model_file, &models[index].points[j].z, sizeof(float));
		
		models[index].points[j].x *= MODELS_DEFAULT_SCALE * scale;
		models[index].points[j].y *= MODELS_DEFAULT_SCALE * scale;
		models[index].points[j].z *= MODELS_DEFAULT_SCALE * scale;
		
		models[index].points[j].x = - models[index].points[j].x;

		Files_Skip(&model_file, sizeof(I32));
		
		if (models[index].bound_top < models[index].points[j].y)
			models[index].bound_top = models[index].points[j].y;
		
		bound_x = models[index].points[j].x;
		if (models[index].bound_left > bound_x)
			models[index].bound_left = bound_x;
		if (models[index].bound_right < bound_x)
			models[index].bound_right = bound_x;
	}
	
	c_pos.x = 0.0f;
	c_pos.y = models[index].bound_top * 0.5f;
	c_pos.z = 0.0f;
	
	models[index].bound_radius_sqr = 0.0f;
	
	for (j = 0; j < models[index].num_points; j++)
	{
		v_pos.x = models[index].points[j].x;
		v_pos.y = models[index].points[j].y;
		v_pos.z = models[index].points[j].z;
		
		dist_sqr = Vector3D_SegmentLengthSQR(&c_pos, &v_pos);
		
		if (models[index].bound_radius_sqr < dist_sqr)
			models[index].bound_radius_sqr = dist_sqr;
	}
	
	models[index].sqr = (models[index].bound_right - models[index].bound_left) * (models[index].bound_top);
	
	// Triangles sub-block:
	models[index].num_indices = 0;
	
	for (j = 0; j < models[index].num_triangles; j++)
	{
		for (k = 0; k < 3; k ++)
		{
			Files_Read(&model_file, &models[index].triangles[j].point_indices[k], sizeof(I32));
			
			models[index].vertex_array[j*9+k*3+0] = models[index].points[models[index].triangles[j].point_indices[k]].x;
			models[index].vertex_array[j*9+k*3+1] = models[index].points[models[index].triangles[j].point_indices[k]].y;
			models[index].vertex_array[j*9+k*3+2] = models[index].points[models[index].triangles[j].point_indices[k]].z;
		}
		
		for (k = 0; k < 6; k ++)
		{
			Files_Read(&model_file, &models[index].tex_coords_array[j*6+k], sizeof(I16));
		}
		
		Files_Read(&model_file, &face_info, sizeof(U32));
		
		models[index].triangles[j].flags = face_info & 0xffff;
		models[index].triangles[j].dmask = (face_info >> 16) & 0xffff;
		
		Files_Skip(&model_file, sizeof(I32) * 6);

		if (models[index].triangles[j].flags & FF_DOUBLE_SIDE)
		{
			models[index].index_array[models[index].num_indices] = j * 3;
			models[index].num_indices ++;
			models[index].index_array[models[index].num_indices] = j * 3 + 1;
			models[index].num_indices ++;
			models[index].index_array[models[index].num_indices] = j * 3 + 2;
			models[index].num_indices ++;
			
			models[index].index_array[models[index].num_indices] = j * 3 + 2;
			models[index].num_indices ++;
			models[index].index_array[models[index].num_indices] = j * 3 + 1;
			models[index].num_indices ++;
			models[index].index_array[models[index].num_indices] = j * 3;
			models[index].num_indices ++;
		}
		else
		{
			models[index].index_array[models[index].num_indices] = j * 3 + 2;
			models[index].num_indices ++;
			models[index].index_array[models[index].num_indices] = j * 3 + 1;
			models[index].num_indices ++;
			models[index].index_array[models[index].num_indices] = j * 3;
			models[index].num_indices ++;
		}
	}
	
	Files_CloseFile(&model_file);
	
	models[index].valid = TRUE;
	
	return index;
}


int  ModelManager_GetModelIndexByName(char *name)
{
	for (int i = 0; i < MAX_MODELS_COUNT; i++)
		if (models[i].valid)
			if (models[i].filename[0] == name[0])
			{
				int result = strcmp(models[i].filename, name);
				if (result == 0)
					return i;
			}
	
	return -1;
}


void ModelManager_PushToRenderListByIndex(int index, ModelPosition3D *pos, float *color, float scale, RenderModelAniState* ani_state)
{
	if (index >= 0 && index < MAX_MODELS_COUNT)
		if (models[index].valid)
			if (models_in_list_count < MAX_MODELS_IN_LIST - 1)
			{
				models_render_list[models_in_list_count].index = index;
				
				models_render_list[models_in_list_count].pos = *pos;
				
				models_render_list[models_in_list_count].scale = scale;
				
				if (color != NULL)
				{
					models_render_list[models_in_list_count].color[0] = color[0];
					models_render_list[models_in_list_count].color[1] = color[1];
					models_render_list[models_in_list_count].color[2] = color[2];
					models_render_list[models_in_list_count].color[3] = color[3];
				}
				else
				{
					models_render_list[models_in_list_count].color[0] = 1.0f;
					models_render_list[models_in_list_count].color[1] = 1.0f;
					models_render_list[models_in_list_count].color[2] = 1.0f;
					models_render_list[models_in_list_count].color[3] = 1.0f;
				}
				
				if (ani_state != NULL)
				{
					models_render_list[models_in_list_count].ani_state = *ani_state;
				}
				else
				{
					models_render_list[models_in_list_count].ani_state.valid = FALSE;
				}
				
				if (models_cache[index].count >= MAX_MODELS_OF_ONE_TYPE_IN_LIST - 1)
					return;
					
				models_cache[index].indices[models_cache[index].count] = models_in_list_count;
				models_cache[index].count ++;

				models_in_list_count ++;
			}
}


void ModelManager_PushToRenderListByName(char *name, ModelPosition3D *pos, float *color, float scale, RenderModelAniState* ani_state)
{
	int index = ModelManager_GetModelIndexByName(name);
	
	ModelManager_PushToRenderListByIndex(index, pos, color, scale, ani_state);
}


void ModelManager_RemoveModelByIndex(int index)
{
	if (!models[index].valid)
		return;
	
	if (index >= 0 && index < MAX_MODELS_COUNT)
	{
		free(models[index].points);
		free(models[index].tex_coords_array);
		free(models[index].vertex_array);
		free(models[index].index_array);
		free(models[index].triangles);
	}
	
	TexManager_RemoveTextureByName(models[index].texture);
	
	models[index].valid = FALSE;
}


void ModelManager_RemoveModelByName(char *name)
{
	int index = ModelManager_GetModelIndexByName(name);
	
	ModelManager_RemoveModelByIndex(index);
}


void ModelManager_RemoveModelsByFlag(U32 user_flag)
{
	for (int i = 0; i < MAX_MODELS_COUNT; i ++)
		if (models[i].user_flags & user_flag)
			ModelManager_RemoveModelByIndex(i);
}


void ModelManager_RemoveAllModels()
{
	for (int i = 0; i < MAX_MODELS_COUNT; i ++)
		ModelManager_RemoveModelByIndex(i);
}


void ModelManager_Release()
{
	ModelManager_RemoveAllModels();
	models_in_list_count = 0;
}

BOOL ModelManager_GetVertexPos(int index, ModelPosition3D *pos, float scale, RenderModelAniState* ani_state, int vertex_index, Vector3D *result)
{
	if (index < 0 && index >= MAX_MODELS_COUNT)
		return FALSE;

	if (result == NULL)
		return FALSE;
	
	Model *mptr = &models[index];

	Vector3D *pptr = NULL;
	
	if (!mptr->valid)
		return FALSE;

	if (vertex_index >= mptr->num_points)
		return FALSE;
	
	float mm[4][4];
	mm[0][0] = 1.0f; mm[1][0] = 0.0f; mm[2][0] = 0.0f; mm[3][0] = 0.0f;
	mm[0][1] = 0.0f; mm[1][1] = 1.0f; mm[2][1] = 0.0f; mm[3][1] = 0.0f;
	mm[0][2] = 0.0f; mm[1][2] = 0.0f; mm[2][2] = 1.0f; mm[3][2] = 0.0f;
	mm[0][3] = 0.0f; mm[1][3] = 0.0f; mm[2][3] = 0.0f; mm[3][3] = 1.0f;
	
	float a;
	float b;
	float g;
	
	float ca;
	float sa;      
	float cb;
	float sb;   
	float cg;
	float sg;
	
	a = -pos->alpha;
	b = -pos->beta;
	g = -pos->gamma;
			
	FastSinCos(a, &sa, &ca);
	FastSinCos(b, &sb, &cb);
	FastSinCos(g, &sg, &cg);
			
	mm[3][0] = pos->x;
	mm[3][1] = pos->y;
	mm[3][2] = pos->z;
			
	if (b == 0.0f && g == 0.0f)
	{
		mm[0][0] = ca * scale;   mm[1][0] = 0.0f;          mm[2][0] = sa * scale;
		mm[0][1] = 0.0f;         mm[1][1] = scale;         mm[2][1] = 0.0f;
		mm[0][2] = -sa * scale;  mm[1][2] = 0.0f;          mm[2][2] = ca * scale;
	}
	else
	{
		mm[0][0] = (cg * ca + sg * sb * sa) * scale;  mm[1][0] = (cg * sb * sa - sg * ca) * scale;  mm[2][0] = (cb * sa) * scale;
		mm[0][1] = (sg * cb) * scale;                 mm[1][1] = (cg * cb) * scale;                 mm[2][1] = (-sb) * scale;
		mm[0][2] = (sg * sb * ca - cg * sa) * scale;  mm[1][2] = (sg * sa + cg * sb * ca) * scale;  mm[2][2] = (cb * ca) * scale;
	}
	
	ModelManager_AnimateCache(mptr, ani_state);
	
	pptr = &points_cache[vertex_index];
			
	result->x = mm[0][0] * pptr->x + mm[1][0] * pptr->y + mm[2][0] * pptr->z + mm[3][0];
	result->y = mm[0][1] * pptr->x + mm[1][1] * pptr->y + mm[2][1] * pptr->z + mm[3][1];
	result->z = mm[0][2] * pptr->x + mm[1][2] * pptr->y + mm[2][2] * pptr->z + mm[3][2];
	
	return TRUE;
}

BOOL ModelManager_GetVertexPosAndNormal(int index, ModelPosition3D *pos, float scale, RenderModelAniState* ani_state, int vertex_index, Vector3D *result_pos, Vector3D *result_normal)
{
	if (index < 0 && index >= MAX_MODELS_COUNT)
		return FALSE;

	if (result_pos == NULL)
		return FALSE;

	if (result_normal == NULL)
		return FALSE;
	
	Model *mptr = &models[index];

	Vector3D *pptr = NULL;
	Vector3D points[3];
	Vector3D triangle_normal, v1, v2;

	int i, j;
	
	if (!mptr->valid)
		return FALSE;

	if (vertex_index >= mptr->num_points)
		return FALSE;
	
	float mm[4][4];
	mm[0][0] = 1.0f; mm[1][0] = 0.0f; mm[2][0] = 0.0f; mm[3][0] = 0.0f;
	mm[0][1] = 0.0f; mm[1][1] = 1.0f; mm[2][1] = 0.0f; mm[3][1] = 0.0f;
	mm[0][2] = 0.0f; mm[1][2] = 0.0f; mm[2][2] = 1.0f; mm[3][2] = 0.0f;
	mm[0][3] = 0.0f; mm[1][3] = 0.0f; mm[2][3] = 0.0f; mm[3][3] = 1.0f;
	
	float a;
	float b;
	float g;
	
	float ca;
	float sa;      
	float cb;
	float sb;   
	float cg;
	float sg;
	
	a = -pos->alpha;
	b = -pos->beta;
	g = -pos->gamma;
			
	FastSinCos(a, &sa, &ca);
	FastSinCos(b, &sb, &cb);
	FastSinCos(g, &sg, &cg);
			
	mm[3][0] = pos->x;
	mm[3][1] = pos->y;
	mm[3][2] = pos->z;
			
	if (b == 0.0f && g == 0.0f)
	{
		mm[0][0] = ca * scale;   mm[1][0] = 0.0f;          mm[2][0] = sa * scale;
		mm[0][1] = 0.0f;         mm[1][1] = scale;         mm[2][1] = 0.0f;
		mm[0][2] = -sa * scale;  mm[1][2] = 0.0f;          mm[2][2] = ca * scale;
	}
	else
	{
		mm[0][0] = (cg * ca + sg * sb * sa) * scale;  mm[1][0] = (cg * sb * sa - sg * ca) * scale;  mm[2][0] = (cb * sa) * scale;
		mm[0][1] = (sg * cb) * scale;                 mm[1][1] = (cg * cb) * scale;                 mm[2][1] = (-sb) * scale;
		mm[0][2] = (sg * sb * ca - cg * sa) * scale;  mm[1][2] = (sg * sa + cg * sb * ca) * scale;  mm[2][2] = (cb * ca) * scale;
	}
	
	ModelManager_AnimateCache(mptr, ani_state);
	
	pptr = &points_cache[vertex_index];
			
	result_pos->x = mm[0][0] * pptr->x + mm[1][0] * pptr->y + mm[2][0] * pptr->z + mm[3][0];
	result_pos->y = mm[0][1] * pptr->x + mm[1][1] * pptr->y + mm[2][1] * pptr->z + mm[3][1];
	result_pos->z = mm[0][2] * pptr->x + mm[1][2] * pptr->y + mm[2][2] * pptr->z + mm[3][2];


	result_normal->x = 0.0f;
	result_normal->y = 0.0f;
	result_normal->z = 0.0f;

	for (i = 0; i < mptr->num_triangles; i++)
	{
		if ((mptr->triangles[i].point_indices[0] == vertex_index) || (mptr->triangles[i].point_indices[1] == vertex_index) || (mptr->triangles[i].point_indices[2] == vertex_index))
		{
			for (j = 0; j < 3; j++)
			{
				pptr = &mptr->points[mptr->triangles[i].point_indices[j]];
			
				points[j].x = mm[0][0] * pptr->x + mm[1][0] * pptr->y + mm[2][0] * pptr->z;
				points[j].y = mm[0][1] * pptr->x + mm[1][1] * pptr->y + mm[2][1] * pptr->z;
				points[j].z = mm[0][2] * pptr->x + mm[1][2] * pptr->y + mm[2][2] * pptr->z;
			}

			v2 = Vector3D_Subtract(&points[1], &points[0]);
			v1 = Vector3D_Subtract(&points[2], &points[0]);
		
			triangle_normal = Vector3D_CrossProduct(&v1, &v2);
		
			Vector3D_Normalize(&triangle_normal);

			result_normal->x += triangle_normal.x;
			result_normal->y += triangle_normal.y;
			result_normal->z += triangle_normal.z;
		}
	}

	Vector3D_Normalize(result_normal);
	
	return TRUE;
}

int ModelManager_TraceModel(int index, ModelPosition3D *pos, float scale, RenderModelAniState* ani_state, Vector3D *trace_a, Vector3D *trace_b, Vector3D *trace_nv, U16 *flags, Vector3D *triangle_normal)
{
	int result = 0;
	
	if (index < 0 && index >= MAX_MODELS_COUNT)
		return 0;
	
	Model *mptr = &models[index];
	
	if (mptr->valid)
		return 0;
	
	int i, j;
	
	Vector3D *pptr;
	Vector3D points[3];
	
	float mm[4][4];
	mm[0][0] = 1.0f; mm[1][0] = 0.0f; mm[2][0] = 0.0f; mm[3][0] = 0.0f;
	mm[0][1] = 0.0f; mm[1][1] = 1.0f; mm[2][1] = 0.0f; mm[3][1] = 0.0f;
	mm[0][2] = 0.0f; mm[1][2] = 0.0f; mm[2][2] = 1.0f; mm[3][2] = 0.0f;
	mm[0][3] = 0.0f; mm[1][3] = 0.0f; mm[2][3] = 0.0f; mm[3][3] = 1.0f;
	
	float a;
	float b;
	float g;
	
	float ca;
	float sa;      
	float cb;
	float sb;   
	float cg;
	float sg;
	
	a = -pos->alpha;
	b = -pos->beta;
	g = -pos->gamma;
			
	FastSinCos(a, &sa, &ca);
	FastSinCos(b, &sb, &cb);
	FastSinCos(g, &sg, &cg);
			
	mm[3][0] = pos->x;
	mm[3][1] = pos->y;
	mm[3][2] = pos->z;
			
	if (b == 0.0f && g == 0.0f)
	{
		mm[0][0] = ca * scale;   mm[1][0] = 0.0f;          mm[2][0] = sa * scale;
		mm[0][1] = 0.0f;         mm[1][1] = scale;         mm[2][1] = 0.0f;
		mm[0][2] = -sa * scale;  mm[1][2] = 0.0f;          mm[2][2] = ca * scale;
	}
	else
	{
		mm[0][0] = (cg * ca + sg * sb * sa) * scale;  mm[1][0] = (cg * sb * sa - sg * ca) * scale;  mm[2][0] = (cb * sa) * scale;
		mm[0][1] = (sg * cb) * scale;                 mm[1][1] = (cg * cb) * scale;                 mm[2][1] = (-sb) * scale;
		mm[0][2] = (sg * sb * ca - cg * sa) * scale;  mm[1][2] = (sg * sa + cg * sb * ca) * scale;  mm[2][2] = (cb * ca) * scale;
	}
			
	
	ModelManager_AnimateCache(mptr, ani_state);
	
	for (i = 0; i < mptr->num_triangles; i++)
	{
		if (mptr->triangles[i].flags & (FF_TRANSPARENT + FF_HIDDEN))
			continue;
		
		for (j = 0; j < 3; j++)
		{
			pptr = &points_cache[mptr->triangles[i].point_indices[j]];
			
			points[j].x = mm[0][0] * pptr->x + mm[1][0] * pptr->y + mm[2][0] * pptr->z + mm[3][0];
			points[j].y = mm[0][1] * pptr->x + mm[1][1] * pptr->y + mm[2][1] * pptr->z + mm[3][1];
			points[j].z = mm[0][2] * pptr->x + mm[1][2] * pptr->y + mm[2][2] * pptr->z + mm[3][2];
		}
		
		if (Trace_CheckPlane(&points[0], &points[1], &points[2], trace_a, trace_b, trace_nv))
		{
			result = 1;
			*flags = mptr->triangles[i].flags;
			if (triangle_normal)
			{
				Vector3D trace_v3 = Vector3D_Subtract(&points[1], &points[0]);
				Vector3D trace_v2 = Vector3D_Subtract(&points[2], &points[0]);
				
				Vector3D trace_plane_nv = Vector3D_CrossProduct(&trace_v3, &trace_v2);
				
				Vector3D_Normalize(&trace_plane_nv);
							
				triangle_normal->x = trace_plane_nv.x;
				triangle_normal->y = trace_plane_nv.y;
				triangle_normal->z = trace_plane_nv.z;
			}
		}
	}
	
	return result;
}

int ModelManager_FindClosestPoint(int index, ModelPosition3D * pos, float scale, Vector3D *distpoint, float spheredist, Vector3D *closest, Vector3D *closest_triangle_normal)
{
	int result = 0;
	
	if (index < 0 && index >= MAX_MODELS_COUNT)
		return 0;
	
	Model *mptr = &models[index];
	
	if (!mptr->valid)
		return 0;
	
	int i, j;
	
	Vector3D *pptr;
	Vector3D points[3];
	Vector3D point_in_triangle;
	Vector3D closest_point_in_triangle;
	int closest_triangle_index;
	float closest_triangle_dist = 0.0f;
	
	float mm[4][4];
	mm[0][0] = 1.0f; mm[1][0] = 0.0f; mm[2][0] = 0.0f; mm[3][0] = 0.0f;
	mm[0][1] = 0.0f; mm[1][1] = 1.0f; mm[2][1] = 0.0f; mm[3][1] = 0.0f;
	mm[0][2] = 0.0f; mm[1][2] = 0.0f; mm[2][2] = 1.0f; mm[3][2] = 0.0f;
	mm[0][3] = 0.0f; mm[1][3] = 0.0f; mm[2][3] = 0.0f; mm[3][3] = 1.0f;
	
	float a;
	float b;
	float g;
	
	float ca;
	float sa;      
	float cb;
	float sb;   
	float cg;
	float sg;
	
	a = -pos->alpha;
	b = -pos->beta;
	g = -pos->gamma;
	
	FastSinCos(a, &sa, &ca);
	FastSinCos(b, &sb, &cb);
	FastSinCos(g, &sg, &cg);
	
	mm[3][0] = pos->x;
	mm[3][1] = pos->y;
	mm[3][2] = pos->z;
	
	if (b == 0.0f && g == 0.0f)
	{
		mm[0][0] = ca * scale;   mm[1][0] = 0.0f;          mm[2][0] = sa * scale;
		mm[0][1] = 0.0f;         mm[1][1] = scale;         mm[2][1] = 0.0f;
		mm[0][2] = -sa * scale;  mm[1][2] = 0.0f;          mm[2][2] = ca * scale;
	}
	else
	{
		mm[0][0] = (cg * ca + sg * sb * sa) * scale;  mm[1][0] = (cg * sb * sa - sg * ca) * scale;  mm[2][0] = (cb * sa) * scale;
		mm[0][1] = (sg * cb) * scale;                 mm[1][1] = (cg * cb) * scale;                 mm[2][1] = (-sb) * scale;
		mm[0][2] = (sg * sb * ca - cg * sa) * scale;  mm[1][2] = (sg * sa + cg * sb * ca) * scale;  mm[2][2] = (cb * ca) * scale;
	}
		
	closest_triangle_normal->x = 0;
	closest_triangle_normal->y = 0;
	closest_triangle_normal->z = 0;
	
	for (i = 0; i < mptr->num_triangles; i++)
	{
		if (mptr->triangles[i].flags & (FF_TRANSPARENT + FF_HIDDEN))
			continue;
		
		for (j = 0; j < 3; j++)
		{
			pptr = &mptr->points[mptr->triangles[i].point_indices[j]];
			
			points[j].x = mm[0][0] * pptr->x + mm[1][0] * pptr->y + mm[2][0] * pptr->z + mm[3][0];
			points[j].y = mm[0][1] * pptr->x + mm[1][1] * pptr->y + mm[2][1] * pptr->z + mm[3][1];
			points[j].z = mm[0][2] * pptr->x + mm[1][2] * pptr->y + mm[2][2] * pptr->z + mm[3][2];
		}
		
		Plane_FindClosestPointInTriangle(&points[0], &points[1], &points[2], distpoint, &point_in_triangle);
		
		float dist = Vector3D_SegmentLengthSQR(distpoint, &point_in_triangle);
		
		if ((dist < closest_triangle_dist) || (!result))
		{
			result = 1;
			closest_triangle_index = i;
			closest_triangle_dist = dist;
			closest_point_in_triangle.x = point_in_triangle.x;
			closest_point_in_triangle.y = point_in_triangle.y;
			closest_point_in_triangle.z = point_in_triangle.z;
		}
		
		if (dist < spheredist * spheredist)
		{
			Vector3D trace_v3 = Vector3D_Subtract(&points[1], &points[0]);
			Vector3D trace_v2 = Vector3D_Subtract(&points[2], &points[0]);
				
			Vector3D trace_plane_nv = Vector3D_CrossProduct(&trace_v3, &trace_v2);
					
			Vector3D_Normalize(&trace_plane_nv);
			
			closest_triangle_normal->x += trace_plane_nv.x;
			closest_triangle_normal->y += trace_plane_nv.y;
			closest_triangle_normal->z += trace_plane_nv.z;
			
			Vector3D_Normalize(closest_triangle_normal);
		}
	}

	closest->x = closest_point_in_triangle.x;
	closest->y = closest_point_in_triangle.y;
	closest->z = closest_point_in_triangle.z;

	return result;
}


