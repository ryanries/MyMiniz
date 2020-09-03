// Filename: Main.c
// Project: MyMiniz
// 2020 Joseph Ryan Ries
// A simple command-line tool that adds and extracts files to and from compressed archives,
// using Miniz by Rich Geldreich <richgel99@gmail.com>.


#pragma warning(push, 0)

#include <Windows.h>

#include "miniz.h"

#include <stdio.h>

#include <stdlib.h>

#pragma warning(pop)

#define OPERATION_NONE		0

#define OPERATION_ADD		1

#define OPERATION_EXTRACT	2

BOOL FileExists(_In_ char* FileName)
{
	DWORD Attributes = GetFileAttributesA(FileName);

return (Attributes != INVALID_FILE_ATTRIBUTES && !(Attributes & FILE_ATTRIBUTE_DIRECTORY));
}

int main(int argc, char* argv[])
{
	char* ArchiveName = NULL;

	char* FullyQualifiedFileName = NULL;

	int Operation = OPERATION_NONE;

	if (argc != 4)
	{
		printf("Adds or extracts files from a compressed archive.\n");

		printf("Usage: myminiz.exe <archive> <[+|-]> <filename>\n");

		return 0;
	}

	ArchiveName = argv[1];

	FullyQualifiedFileName = argv[3];

	if (_stricmp(argv[2], "+") == 0)
	{
		Operation = OPERATION_ADD;
	}
	else if (_stricmp(argv[2], "-") == 0)
	{
		Operation = OPERATION_EXTRACT;
	}
	else
	{
		printf("Adds or extracts files from a compressed archive.\n");

		printf("Usage: myminiz.exe <archive> <[+|-]> <filename>\n");

		return(0);
	}

	if (Operation == OPERATION_ADD)
	{
		DWORD BytesRead = 0;

		BYTE* FileBuffer = NULL;

		HANDLE FileHandle = INVALID_HANDLE_VALUE;

		LARGE_INTEGER FileSize = { 0 };

		DWORD Error = ERROR_SUCCESS;

		char FileName[MAX_PATH] = { 0 };
		
		char FileExtension[MAX_PATH] = { 0 };

		if ((FileHandle = CreateFileA(FullyQualifiedFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
		{
			Error = GetLastError();

			printf("ERROR: The file %s cannot be found, or could not be opened for reading! 0x%08lx\n", FullyQualifiedFileName, Error);

			return(Error);
		}

		printf("[+] File %s opened for reading.\n", FullyQualifiedFileName);

		if (GetFileSizeEx(FileHandle, &FileSize) == 0)
		{
			Error = GetLastError();

			return(Error);
		}

		printf("[+] File size: %lld\n", FileSize.QuadPart);

		if ((FileBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, FileSize.QuadPart)) == NULL)
		{
			Error = ERROR_OUTOFMEMORY;

			return(Error);
		}

		if ((ReadFile(FileHandle, FileBuffer, (DWORD)FileSize.QuadPart, &BytesRead, NULL)) == FALSE)
		{
			Error = GetLastError();

			printf("ERROR: ReadFile failed! 0x%08lx\n", Error);

			return(Error);
		}

		if (BytesRead != FileSize.QuadPart)
		{
			Error = ERROR_READ_FAULT;

			printf("ERROR: Bytes read into memory did not match file size!\n");

			return(Error);
		}

		Error = _splitpath_s(FullyQualifiedFileName, NULL, 0, NULL, 0, FileName, sizeof(FileName), FileExtension, sizeof(FileExtension));

		strcat_s(FileName, sizeof(FileName), FileExtension);

		printf("[+] FileName: %s\n", FileName);

		if ((mz_zip_add_mem_to_archive_file_in_place(ArchiveName, FileName, FileBuffer, FileSize.QuadPart, "", 1, MZ_BEST_COMPRESSION)) == MZ_FALSE)
		{			
			Error = ERROR_COMPRESSED_FILE_NOT_SUPPORTED;

			printf("ERROR: Failed to add file %s to archive %s! 0x%08lx\n", FileName, ArchiveName, Error);

			return(Error);
		}

		printf("[+] File %s successfully added to archive %s.\n", FileName, ArchiveName);

		if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE))
		{
			CloseHandle(FileHandle);
		}
	}
	else if (Operation == OPERATION_EXTRACT)
	{
		if (FileExists(ArchiveName) == FALSE)
		{
			printf("ERROR: Archive %s does not exist!\n", ArchiveName);

			return(0);
		}

	}
	else
	{
		printf("ERROR: No operation specified!\n");
	}
}