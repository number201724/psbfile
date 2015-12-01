#include <windows.h>
#include "../../common/psb.hpp"
#include "../../common/psb_stream.hpp"

HANDLE g_hHeap;

#include "plugin.h"




/**/
struct MDFHDR {
	unsigned char signature[4];
	unsigned long size;
};
bool ProcessCompressData(unsigned char* buff, unsigned long length, psb_stream_t& stream, bool is_compress) {
	unsigned char* temp_buf;
	unsigned long temp_len;
	MDFHDR hdr;
	if (is_compress) {
		temp_len = length * 2+ 1024 ;
		temp_buf = new unsigned char[temp_len];
		if (_ZlibCompress(temp_buf, &temp_len, buff, length) != 0) {
			delete[] temp_buf;
			return false;
		}

		hdr.signature[0] = 'm'; hdr.signature[1] = 'd'; hdr.signature[2] = 'f'; hdr.signature[3] = '\x00';
		hdr.size = length;
		stream.write((unsigned char*)&hdr, sizeof(hdr));
		stream.write((unsigned char*)temp_buf, temp_len);
		delete[] temp_buf;
	}
	else {
		memcpy(&hdr, buff, sizeof(hdr));
		temp_buf = new unsigned char[hdr.size];
		temp_len = hdr.size;

		if (_ZlibUncompress(temp_buf, &temp_len, &buff[sizeof(hdr)], length - sizeof(hdr)) != 0) {
			delete[] temp_buf;
			return false;
		}

		stream.write(temp_buf, temp_len);

		delete[] temp_buf;
	}

	return true;
}

//===========================================================================================================================

struct scene_text_pack
{
	unsigned long index;
	string texts;
};

vector<scene_text_pack> scene_texts;
//map<unsigned long, string> scene_texts_map;

void
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects,
	string prev_layer = "");

string
format_layer(string prev_layer,
	string entry_name) {
	string layer_name = prev_layer;
	if (layer_name.empty()) {
		layer_name = entry_name;
	}
	else {
		layer_name += "." + entry_name;
	}
	return layer_name;
}
bool filterChars(string& str) {
	bool result = true;
	for (unsigned long i = 0; i < str.length(); i++) {
		if (str[i] >= ' ' && str[i] <= '~')
			continue;

		result = false;
		break;
	}
	return result;
}

void
parse_texts(string layer_name,
	string entry_name,
	const psb_string_t* str) {
	scene_text_pack packs;
	if (layer_name == "scenes.texts" || layer_name == "scenes.title" || layer_name == "scenes.selects.text" || layer_name == "scenes.selectInfo.select.text") {

		packs.index = str->get_index();
		packs.texts = str->get_string();
		if (packs.texts.empty()) return;
		if (filterChars(packs.texts)) return;
		

		//if (scene_texts_map.find(packs.index) == scene_texts_map.end()) {
		//	scene_texts_map[packs.index] = packs.texts;
			scene_texts.push_back(packs);
		//}
	}
}

//It is actually a connecting structure, object and data node separately
void
traversal_offsets_tree(psb_t& psb,
	const psb_offsets_t *offsets,
	string layer_name,
	string entry_name) {
	psb_value_t *value = NULL;

	for (unsigned long i = 0; i < offsets->size(); i++) {
		unsigned char* entry_buff = offsets->get(i);

		psb.unpack(value, entry_buff);

		if (value != NULL) {
			if (value->get_type() == psb_value_t::TYPE_OFFSETS) {
				traversal_offsets_tree(psb, (const psb_offsets_t *)value, layer_name, entry_name);
				delete (const psb_offsets_t *)value;
			}
			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {
				traversal_object_tree(psb, (const psb_objects_t *)value, layer_name);
				delete (const psb_objects_t *)value;
			}
			if (value->get_type() == psb_value_t::TYPE_STRING) {
				parse_texts(layer_name, entry_name, (const psb_string_t *)value);
				delete (const psb_string_t *)value;
			}

			
		}
	}
}
void
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects,
	string prev_layer) {
	psb_value_t *value = NULL;

	for (unsigned long i = 0; i < objects->size(); i++) {
		string entry_name = objects->get_name(i);
		string layer_name = format_layer(prev_layer, entry_name);
		unsigned char* entry_buff = objects->get_data(i);

		psb.unpack(value, entry_buff);

		if (value != NULL) {
			if (value->get_type() == psb_value_t::TYPE_OFFSETS) {
				traversal_offsets_tree(psb, (const psb_offsets_t *)value, layer_name, entry_name);
				delete (const psb_offsets_t *)value;
			}
			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {
				traversal_object_tree(psb, (const psb_objects_t *)value, layer_name);
				delete (const psb_objects_t *)value;
			}
			if (value->get_type() == psb_value_t::TYPE_STRING) {
				parse_texts(layer_name, entry_name, (const psb_string_t *)value);
				delete (const psb_string_t *)value;
			}
		}
	}
}


struct PSBInfo
{
	vector<scene_text_pack> texts;
	bool bIsCompress;
};

void GetFileBuffers(HANDLE hFile, PBYTE& lpBuffer,DWORD& dwSize) {
	DWORD dwNumberByteOfRead;
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	dwSize = GetFileSize(hFile, NULL);

	lpBuffer = (PBYTE)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	ReadFile(hFile, lpBuffer, dwSize, &dwNumberByteOfRead, NULL);
}

void WINAPI InitInfo(LPMEL_INFO2 lpMelInfo)
{
	lpMelInfo->dwInterfaceVersion = INTERFACE_VERSION;
	lpMelInfo->dwCharacteristic = 0;
}

void WINAPI PreProc(LPPRE_DATA lpPreData)
{
	g_hHeap = lpPreData->hGlobalHeap;
}

int WINAPI Match(LPCWSTR lpszName)
{
	FILE* fp;
	unsigned char signature[4];

	fp = _wfopen(lpszName, L"rb");
	if (fp == NULL) {
		return MR_ERR;
	}

	if (fread(&signature, 1, sizeof(signature), fp) != sizeof(signature)) {
		fclose(fp);
		return MR_NO;
	}

	fclose(fp);

	if (signature[0] == 'P' && signature[1] == 'S' && signature[2] == 'B' && signature[3] == '\x00') {
		return MR_YES;
	}
	if (signature[0] == 'm' && signature[1] == 'd' && signature[2] == 'f' && signature[3] == '\x00') {
		return MR_YES;
	}

	return MR_NO;
}
MRESULT WINAPI GetText(LPFILE_INFO lpFileInfo, LPDWORD lpdwRInfo)
{
	psb_stream_t psb_stream;
	DWORD dwSize; LPBYTE lpBuffer;
	bool bIsCompress = false;

	scene_texts.clear();
	//scene_texts_map.clear();

	if (lpFileInfo->dwCharSet == CS_UNKNOWN)
		lpFileInfo->dwCharSet = CS_UTF8;
	else if (lpFileInfo->dwCharSet != CS_UTF8)
		return E_CODEFAILED;

	/*Process Compress MDF Files*/
	
	GetFileBuffers(lpFileInfo->hFile, lpBuffer, dwSize);

	if (lpBuffer[0] == 'm' && lpBuffer[1] == 'd' && lpBuffer[2] == 'f' && lpBuffer[3] == '\x00') {
		bIsCompress = true;

		if (!ProcessCompressData(lpBuffer, dwSize, psb_stream, false)) {
			VirtualFree(lpBuffer, 0, MEM_RELEASE);
			return E_ERROR;
		}
	}
	else {
		psb_stream.write(lpBuffer, dwSize);
	}

	VirtualFree(lpBuffer, 0, MEM_RELEASE);
	dwSize = psb_stream.get_length();
	lpBuffer = (PBYTE)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	memcpy(lpBuffer, psb_stream.get_buffer(), dwSize);
	
	/*Process Compress MDF Files		End*/

	//parse psb files
	psb_t psb(lpBuffer);
	const psb_objects_t* psb_objects = psb.get_objects();
	traversal_object_tree(psb, psb_objects);
	

	//=====================================================================================================================

	PSBInfo *lpPSBInfo = new PSBInfo();
	lpPSBInfo->texts = scene_texts;
	lpPSBInfo->bIsCompress = bIsCompress;

	lpFileInfo->lpStream = lpBuffer;
	lpFileInfo->nStreamSize = dwSize;

	lpFileInfo->lpCustom = lpPSBInfo;
	lpFileInfo->nLine = lpPSBInfo->texts.size();
	lpFileInfo->lpStreamIndex = (STREAM_ENTRY*)VirtualAlloc(0, lpFileInfo->nLine * sizeof(STREAM_ENTRY), MEM_COMMIT, PAGE_READWRITE);

	for (unsigned long i = 0; i < lpPSBInfo->texts.size(); i++) {
		lpFileInfo->lpStreamIndex[i].lpStart = (LPVOID)i;
		lpFileInfo->lpStreamIndex[i].lpInformation = (LPVOID)lpPSBInfo->texts[i].texts.length();
	}

	lpFileInfo->dwMemoryType = MT_POINTERONLY;
	lpFileInfo->dwStringType = ST_ENDWITHZERO;

	return E_SUCCESS;
}
MRESULT WINAPI GetStr(LPFILE_INFO lpFileInfo, LPWSTR* pPos, LPSTREAM_ENTRY lpStreamEntry)
{
	unsigned long i = (unsigned long)lpStreamEntry->lpStart;
	PSBInfo *lpPSBInfo = (PSBInfo *)lpFileInfo->lpCustom;
	const char *s = lpPSBInfo->texts[i].texts.c_str();
	unsigned long nLen = (lpPSBInfo->texts[i].texts.length() + 2);

	wchar_t* pTemp = new wchar_t[nLen * sizeof(wchar_t)];
	int nSize = MultiByteToWideChar(CP_UTF8, 0, s, -1, pTemp, nLen);
	*pPos = _ReplaceCharsW(pTemp, RCH_ENTERS | RCH_TOESCAPE, 0);
	delete[] pTemp;

	return E_SUCCESS;
}
MRESULT WINAPI SetLine(LPCWSTR lpStr, LPSEL_RANGE lpRange)
{
	return _SetLine(lpStr, lpRange);
}
MRESULT WINAPI ModifyLine(LPFILE_INFO lpFileInfo, DWORD nLine)
{
	PSBInfo *lpPSBInfo = (PSBInfo *)lpFileInfo->lpCustom;
	/*GetStr&ConvertUTF8*/
	wchar_t* pWideStr = _GetStringInList(lpFileInfo, nLine);
	pWideStr = _ReplaceCharsW(pWideStr, 0x00000001, 0);
	if (!pWideStr) return E_NOMEM;
	int nNewLen = lstrlenW(pWideStr);
	char* pNewStr = new char[nNewLen * 4];
	nNewLen = WideCharToMultiByte(CP_UTF8, 0, pWideStr, -1, pNewStr, nNewLen * 4, 0, 0);

	lpPSBInfo->texts[nLine].texts = pNewStr;
	delete[] pNewStr;

	return E_SUCCESS;
}
unsigned long
fixed_write_string(psb_fixed_build_t& psb_build, unsigned long& pos, string new_str) {
	unsigned long result;
	const char* buf = new_str.c_str();
	unsigned long buf_len = new_str.length() + 1;

	psb_build.insert((const unsigned char*)buf, buf_len, pos);

	result = pos - psb_build.hdr->offset_strings_data;
	pos += buf_len;

	return result;
}

MRESULT WINAPI SaveText(LPFILE_INFO lpFileInfo)
{
	unsigned long nByteWrite;
	PSBInfo *lpPSBInfo = (PSBInfo *)lpFileInfo->lpCustom;
	vector <unsigned long> strings;
	psb_t psb((unsigned char *)lpFileInfo->lpStream);

	for (unsigned long i = 0; i < psb.strings->size(); i++) {
		strings.push_back(psb.strings->get(i));
	}

	psb_fixed_build_t psb_build((const unsigned char *)lpFileInfo->lpStream, lpFileInfo->nStreamSize);


	unsigned long section_size = 0;
	unsigned long pos;


	for (unsigned long i = 0; i < strings.size(); i++) {
		if (strings[i] > section_size) {
			section_size = strings[i];
		}
	}

	string s = (psb.strings_data + section_size);
	section_size += s.length() + 1;

	pos = psb.hdr->offset_strings_data + section_size;

	for (unsigned long i = 0; i < lpPSBInfo->texts.size(); i++) {
		unsigned long index = lpPSBInfo->texts[i].index;

		if (index > strings.size()) {
			continue;
		}

		string texts = (psb.strings_data + strings[index]);

		if (texts == lpPSBInfo->texts[i].texts) {
			continue;
		}

		strings[index] = fixed_write_string(psb_build, pos, lpPSBInfo->texts[i].texts);
	}

	psb_array_build_t array_table(strings);
	psb_build.replace(psb_build.hdr->offset_strings, psb.strings->data_length, array_table.get_length(), array_table.get_buffer());

	psb_stream_t psb_stream;
	if (lpPSBInfo->bIsCompress) {
		ProcessCompressData(psb_build.get_buffer(), psb_build.get_length(), psb_stream, true);
	}
	else {
		psb_stream.write(psb_build.get_buffer(), psb_build.get_length());
	}

	SetFilePointer(lpFileInfo->hFile, 0, 0, FILE_BEGIN);
	WriteFile(lpFileInfo->hFile, psb_stream.get_buffer(), psb_stream.get_length(), &nByteWrite, 0);
	SetEndOfFile(lpFileInfo->hFile);

	return E_SUCCESS;
}
MRESULT WINAPI Release(LPFILE_INFO lpFileInfo)
{
	PSBInfo *lpPSBInfo = (PSBInfo *)lpFileInfo->lpCustom;

	delete lpPSBInfo;

	lpFileInfo->lpCustom = NULL;

	return E_SUCCESS;
}