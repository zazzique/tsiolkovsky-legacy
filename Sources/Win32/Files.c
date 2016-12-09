

#include "windows.h"
#include "Common.h"
#include "Files.h"

#define MAX_FILES 2048

char file_pathes[MAX_FILES][256];
char file_names[MAX_FILES][64];
int files_count = 0;

int GetFileIndex(char *name)
{
	for (int i = 0; i < files_count; i ++)
	{
		if (strcmp(file_names[i], name) == 0)
		{
			return i;
		}
	}

	return -1;
}

int AddFileToList(char *path, char *name)
{
	int current_file_index = files_count;

	char current_path[256];

	if (files_count >= MAX_FILES - 1)
	{
		LogPrint("Error! Too many files");
		return -1;
	}

	sprintf(current_path, "%s\\%s", path, name);

	for (int i = 0; i < files_count; i ++)
	{
		if (strcmp(file_pathes[i], current_path) == 0)
		{
			return i;
		}
	}

	strcpy(file_pathes[current_file_index], current_path);
	strcpy(file_names[current_file_index], name);

	files_count ++;

	return current_file_index;
}

void ScanDir(char *dir)
{
	char filter[MAX_PATH];
	char sub_dir[MAX_PATH];

	HANDLE hFind;
	WIN32_FIND_DATA data;

	sprintf(filter, "%s\\*", dir);

	hFind = FindFirstFile(filter, &data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (data.cFileName[0] != '.')
			{
				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
				
					sprintf(sub_dir, "%s\\%s", dir, data.cFileName);
					ScanDir(sub_dir);

				}
				else
				{
					AddFileToList(dir, data.cFileName);
				}
			}
		}
		while (FindNextFile(hFind, &data));

		FindClose(hFind);
	}
}

BOOL Files_GetFilePathByName(char *name, char *result)
{
	int index = GetFileIndex(name);

	if (index < 0)
		return FALSE;

	strcpy(result, &file_pathes[index][0]);

	return TRUE;
}

void Files_Init()
{
	char dir[MAX_PATH];
	char current_dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, current_dir);

	sprintf(dir, "%s%s", current_dir, "\\Data");

	ScanDir(dir);
}

BOOL Files_OpenFile(FileHandler *file, char *name)
{
	char *type = strrchr(name,'.');
	
	if (type == NULL)
		return FALSE;
	
	char base[128];
	
	strncpy(base, name, (type - name));
	base[type - name] = '\0';

	
	return Files_OpenFileOfType(file, base, type + 1);
}

BOOL Files_OpenFileAltType(FileHandler *file, char *name, char *type)
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

BOOL Files_OpenFileOfType(FileHandler *file, char *name, char *type)
{
	char full_name[64];
	sprintf(full_name, "%s.%s", name, type);
	int index = GetFileIndex(full_name);

	if (index < 0)
		return FALSE;

    FILE *fp;
    
	fp = fopen(file_pathes[index], "rb");
	if (fp == NULL)
	{
		return FALSE;
	}
    
	fseek(fp, 0, SEEK_END);
	file->size = ftell(fp);
	rewind(fp);
    
	file->data = (U8 *)malloc(file->size * sizeof(U8));
    
	if (file->data == NULL)
	{
		return FALSE;
	}
	else
	{
		int result = fread(file->data, sizeof(U8), file->size, fp);
		if (file->size != result)
		{
			free(file->data);
            file->data = NULL;
			return FALSE;
		}
	}
	
	fclose(fp);
    
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


