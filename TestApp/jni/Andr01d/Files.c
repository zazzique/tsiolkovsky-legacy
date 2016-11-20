#include <jni.h>
#include <errno.h>

#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "main.h"

#include "Common.h"
#include "Files.h"


void Files_Init()
{
	//
}

BOOL Files_OpenFile(FileHandler *file, const char *name)
{

	char *type = strrchr(name,'.');

	if (type == NULL)
		return FALSE;
	
	char base[128];
	
	strncpy(base, name, (type - name));
	base[type - name] = '\0';

	
	return Files_OpenFileOfType(file, base, type + 1);
}

BOOL Files_OpenFileAltType(FileHandler *file, const char *name, const char *type)
{
	char *main_type = strrchr(name,'.');
	
	if (main_type == NULL)
		return FALSE;
	
	char base[128];
    
    strncpy(base, name, (main_type - name));
	base[main_type - name] = '\0';
    
    if (strlen(base) == 0)
        return FALSE;
		
	if (Files_OpenFileOfType(file, base, type) == FALSE)
	{
		return Files_OpenFileOfType(file, base, main_type + 1);
	}
	
	return TRUE;
}

BOOL Files_OpenFileOfType(FileHandler *file, const char *name, const char *type)
{
	char full_name[256];
	sprintf(full_name, "%s.%s", name, type);

	AAsset *asset = AAssetManager_open(GetAssetManager(), full_name, AASSET_MODE_STREAMING);
	if (asset == NULL)
	{
		return FALSE;
	}

	file->size = AAsset_getLength(asset);
	file->data = (U8 *)malloc(file->size * sizeof(U8));

	if (file->data == NULL)
	{
		return FALSE;
	}
	else
	{
		AAsset_read (asset, file->data, file->size);
		AAsset_close(asset);
	}

	file->current_pos = 0;
    
    strcpy(file->file_base, name);
	strcpy(file->file_extention, type);
    
	return TRUE;
}

BOOL Files_GetData(FileHandler *file, void **data, I32 *size)
{
	if (file->data == NULL)
		return FALSE;

	if (data != NULL)
		*data = file->data;

	if (size != NULL)
		*size = file->size;

	return TRUE;
}

I32 Files_GetSize(FileHandler *file)
{
	if (file->data == NULL)
		return -1;
	
	return file->size;
}

I32 Files_GetCurrentPos(FileHandler *file)
{
	if (file->data == NULL)
		return -1;
	
	return file->current_pos;
}

char *Files_GetFileBaseName(FileHandler *file)
{
	return file->file_base;
}

char *Files_GetFileExtension(FileHandler *file)
{
	return file->file_extention;
}

BOOL Files_Read(FileHandler *file, void *data, int size)
{
	if (file->current_pos + size > file->size)
		return FALSE;
    
	U8 *data_src = &file->data[file->current_pos];
    
	memcpy(data, data_src, size);
    
	file->current_pos += size;
    
	return TRUE;
}

BOOL Files_ReadCompressed(FileHandler *file, void *data)
{
	int i, j;
	I32 size, result_size;
	U8 count, data_byte;
	
	U8 *out = (U8*)data;
	
	if (file->data == NULL)
		return FALSE;
	
	memcpy(&size, file->data + file->current_pos, sizeof(I32));
    
	if (size == 0)
		return FALSE;
	
	file->current_pos += sizeof(I32);
	
	result_size = 0;
	
	for (i = 0; i < size; i += 2)
	{
        data_byte = file->data[file->current_pos];
		file->current_pos ++;
        
        count = file->data[file->current_pos];
        file->current_pos ++;
		
		for (j = 0; j < count; j++)
		{
			*out = data_byte;
			out ++;
		}
		
		result_size += count;
	}
	
	return TRUE;
}

void Files_Skip(FileHandler *file, int size)
{
	file->current_pos += size;
    
	if (file->current_pos > file->size - 1)
		file->current_pos = file->size - 1;
}

void Files_SetPos(FileHandler *file, int pos)
{
	file->current_pos = pos;
    
	if (file->current_pos > file->size - 1)
		file->current_pos = file->size - 1;
}

void Files_CloseFile(FileHandler *file)
{
    if (file->data != NULL)
    {
        free(file->data);
        file->data = NULL;
    }
}

void Files_Release()
{
	//
}


