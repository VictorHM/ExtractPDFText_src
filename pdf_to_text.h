#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include<string>

//TODO: check different ways to read a file in C++ http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
//Keep this many previous recent characters for back reference:
#define oldchar 15

class Cpdf_to_text
{
public:
	Cpdf_to_text();
	Cpdf_to_text(std::string inputPDF);
	~Cpdf_to_text();

	bool setFilename(std::string inputPDF);

private:
		std::string _filename;
		std::ifstream _inputFile;
		std::ofstream _outputFile;
		std::vector<char> _buffer;
		//char* _buffer = nullptr;
        bool hasFileSet = false;

		bool getFileHandle();
        std::string ProcessLine(const std::string& line_in);
		void ProcessOutput(char* output, size_t length);
		float ExtractNumber(const char* search, int lastCharOffset);
		bool seen2(const char* search, char* recent);
		size_t findStringInBuffer(char* buffer, char* search, size_t buffersize);
		void ReadDocumentByLine();
};

