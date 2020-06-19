// dxfconv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HDxf.h"

std::wstring output_filepath;

void WritePercentToOutputFile(int percent)
{
	// use the output file to update the progress bar
	ofstream ofs(output_filepath);
	if (!ofs)return;
	ofs << percent << "\n";
}

void DoConvert(const wchar_t* in_filepath, const wchar_t* out_filepath)
{
	HeeksDxfRead dxf_file(in_filepath, WritePercentToOutputFile);
	dxf_file.DoRead(HeeksDxfRead::m_ignore_errors);

	printf("file has been read, converting splines... \n");
	theApp.SplinesToBiarcs(0.01);
	printf("Done!. Splines were converted to arcs\n");


	theApp.SaveDXFFile(theApp.GetChildren(), out_filepath);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		wprintf(L"not enough arguments to dxfconv, first parameter is input file, second parameter is output file");
	}

	std::wstring input_filepath(argv[1]);
	output_filepath.assign(argv[2]);

	wprintf(L"input file is %s\n", input_filepath.c_str());
	wprintf(L"output file is %s\n", output_filepath.c_str());

	DoConvert(input_filepath.c_str(), output_filepath.c_str());

	if(theApp.m_number_of_splines_converted > 0)return 0; // 0 means "all good and some splines converted"
	return 1; // non zero means either a problem or no splines converted
}



