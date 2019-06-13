// dxfconv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HDxf.h"


void DoConvert(const wchar_t* in_filepath, const wchar_t* out_filepath)
{
	HeeksDxfRead dxf_file(in_filepath);
	dxf_file.DoRead(HeeksDxfRead::m_ignore_errors);

	theApp.SplinesToBiarcs(0.0001);

	theApp.SaveDXFFile(theApp.GetChildren(), out_filepath);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		wprintf(L"not enough arguments to dxfconv, first parameter is input file, second parameter is output file");
	}

	std::wstring input_filepath(argv[1]);
	std::wstring output_filepath(argv[2]);
	wprintf(L"input file is %s\n", input_filepath.c_str());
	wprintf(L"output file is %s\n", output_filepath.c_str());

	DoConvert(input_filepath.c_str(), output_filepath.c_str());

	return 1;
}



