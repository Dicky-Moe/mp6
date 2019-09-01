// mp6tool.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

void AppendSO(TCHAR* mp6file, TCHAR* addend, TCHAR* alias) {
	HANDLE f1 = CreateFile(mp6file, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	HANDLE f2 = CreateFile(addend, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	char buf[512];
	DWORD bRead=1;
	BYTE len = (BYTE)_tcslen(alias);
	WORD chunksize = 26 + len;
	DWORD written; //dummy?
	//сначала пишем заголовок SO
	SetFilePointer(f1, 0, NULL, FILE_END);
	WriteFile(f1, &chunksize, 2, &written, NULL);
	WriteFile(f1, &len, 1, &written, NULL);
	WriteFile(f1, alias, len, &written, NULL);
	WriteFile(f1, "application/octet-stream;", 25, &written, NULL);
	//WriteFile(f1, &bRead, 4, NULL, NULL);
	while (bRead > 0) {
		ReadFile(f2, buf, 512, &bRead, NULL);
		WriteFile(f1, buf, bRead, &bRead, NULL);
	}
	CloseHandle(f1); CloseHandle(f2);
}

void CreateMP6(TCHAR* mp6file) {
	HANDLE f = CreateFile(mp6file, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
	WriteFile(f, "MP6", 3, NULL, NULL);
	CloseHandle(f);
}

void ExtractSO(TCHAR* mp6file, TCHAR* target, TCHAR* alias) {
	HANDLE f1 = CreateFile(mp6file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	HANDLE f2;
	//парсим
	char buf[512];
	WORD chunk_len;
	WORD so_len;
	int i;
	ReadFile(f1, buf, 3, NULL, NULL);
	//проверим сигнатуру
	if (strcmp(buf, "MP6"))return;

	ReadFile(f1, buf, 1, NULL, NULL);
	while (GetLastError() == ERROR_SUCCESS) {
		ReadFile(f1, &chunk_len, 2, NULL, NULL);
		ReadFile(f1, buf, chunk_len, NULL, NULL);
		if (buf[0] && 0xc0 == 0) {
			//чанк=SO
			so_len = buf[0];		
			//проверяем имя
			//чанк состоит из длины имени, имени (д.б. совпадать с alias), типа, разделителя ";" и содержимого 
			//WideCharToMultiByte(0, 0, alias, lstrlen(alias), buf, 512, "\0", NULL);
			if (!memcmp(buf, alias,so_len)) {
				//имя совпало, выгружаем
				for (i = so_len + 1; buf[i] != ';'; i++);
				f2 = CreateFile(target, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
				WriteFile(f2, buf + i+1, chunk_len - i-1, NULL, NULL);
				CloseHandle(f2);
				return;
			}


		}

		else if (buf[0] && 0xC0 == 0x40) {
			//чанк=ESDU
		}
		else if (buf[0] && 0xC0 == 0x80) {
			//чанк=SNCP. просто пропускаем +2 байта.
			ReadFile(f1, buf, 2, NULL, NULL);
		}
		//в начало цикла, пока не кончится файл.
	}
}

int wmain(int argc,TCHAR**argv)
{
	if (argc < 2) {
		printf("Usage:...\n");
		return 0;
	}
	if (lstrcmp(argv[1], TEXT("a"))==0) {
		AppendSO(argv[2], argv[3], argv[4]); 
		return 0;
	}
	if (lstrcmp(argv[1], TEXT("x"))==0) {
		ExtractSO(argv[2], argv[3], argv[4]); 
		return 0;
	}
	
	
	return 0;
}

