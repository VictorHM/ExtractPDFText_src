#include <stdio.h>
#include <windows.h>

//YOur project must also include zdll.lib (ZLIB) as a dependency.
//ZLIB can be freely downloaded from the internet, www.zlib.org
//Use 4 byte struct alignment in your project!

#include "headers\zlib.h"
#include "pdf_to_text.h"



Cpdf_to_text::Cpdf_to_text()
{
}

Cpdf_to_text::Cpdf_to_text(std::string inputPDF)
{
	_filename = inputPDF;
    hasFileSet = true;
}


Cpdf_to_text::~Cpdf_to_text()
{
}

bool Cpdf_to_text::setFilename(std::string inputPDF)
{
	bool isOk = false;
	if (inputPDF.length > 0)
	{
		_filename = inputPDF;
        hasFileSet = true;
		isOk = true;
	}
	return isOk;
}

bool Cpdf_to_text::getFileHandle()
{
	bool isOpen = false;

	if (_filename.empty() || !hasFileSet)
	{
		std::cout << "No se ha proporcionado ruta o nombre de archivo." << std::endl;
	}
	else 
	{
		
		_inputFile.open(_filename, std::ifstream::in);
		//pPdfFile = fopen(_filename.c_str(), "rb");
		if (_inputFile.good())
		{
			isOpen = true;
		}
		else
		{
			//TODO: mejorar la gestión de errores e impresión de los mismos
			std::cout << "Archivo no abierto. Se ha producido un error." << std::endl;
		}
	}

    _outputFile.open("temporary.txt", std::ofstream::out);
	return isOpen;
}

std::string Cpdf_to_text::ProcessLine(const std::string & line_in)
{
    size_t streamstart = line_in.find("stream");
    size_t streamend = line_in.find("endstream");
    if (streamstart != std::string::npos && streamend > streamstart)
    {
        //std::string::iterator it = line_in.at[streamstart + 6];
        streamstart += 6;

        if (line_in.at[streamstart] == 0x0d && line_in.at[streamstart] == 0x0a)
        {
            streamstart += 2;
        }
        else if (line_in.at[streamstart] == 0x0a) streamstart++;

        if (line_in.at[streamend - 2] == 0x0d && line_in.at[streamend - 1] == 0x0a) streamend -= 2;
        else if (line_in.at[streamend - 1] == 0x0a) streamend--;

        //Assume output will fit into 10 times input buffer:
        size_t outsize = (streamend - streamstart) * 10;
        std::string output;
        output.reserve(outsize);
        //Setting values for z_stream structure. C-style forzed by library
        z_stream zstream; ZeroMemory(&zstream, sizeof(zstream));

        zstream.avail_in = streamend - streamstart + 1;
        zstream.avail_out = outsize;
        zstream.next_in = static_cast<Bytef*> (line_in.at[streamstart].c_str);
        zstream.next_out = static_cast<Bytef*> (output.c_str);

        //Lets inflate the block of data
        int rsti = inflate(&zstream);
        if (rsti == Z_OK)
        {
            int rst2 = inflate(&zstream, Z_FINISH);
            if (rst2 >= 0)
            {
                //Ok, got something, extract the text
                size_t totout = zstream.total_out;
                ProcessOutput(output.c_str, totout);
            }
        }


    }
    if (size_t postd = line_in.find("TD") != std::string::npos)
    {
        _outputFile << line_in;
    }
    return std::string();
}

void Cpdf_to_text::ProcessOutput(std::string output, size_t length)
{
	//Are we currently inside a text object?
	bool intextobject = false;

	//Is the next character literal (e.g. \\ to get a \ character or \( to get ( ):
	bool nextliteral = false;

	//() Bracket nesting level. Text appears inside ()
	int rbdepth = 0;

	//Keep previous chars to get extract numbers etc.:
	char oc[oldchar];
	int j = 0;
	for (j = 0; j < oldchar; j++) oc[j] = ' ';
    //TODO use iterators for going trhough output string
    //TODO change the way it is stored on file using streams
	for (size_t i = 0; i < length; i++)
	{
		char c = output[i];
		if (intextobject)
		{
			if (rbdepth == 0 && seen2("TD", oc))
			{
				//Positioning.
				//See if a new line has to start or just a tab:
				float num = ExtractNumber(oc, oldchar - 5);
				if (num > 1.0)
				{
                    //carriage return
					fputc(0x0d, _inputFile.);
                    //new line
					fputc(0x0a, _inputFile);
				}
				if (num < 1.0)
				{
					fputc('\t', _inputFile);
				}
			}
			if (rbdepth == 0 && seen2("ET", oc))
			{
				//End of a text object, also go to a new line.
				intextobject = false;
				fputc(0x0d, _inputFile);
				fputc(0x0a, _inputFile);
			}
			else if (c == '(' && rbdepth == 0 && !nextliteral)
			{
				//Start outputting text!
				rbdepth = 1;
				//See if a space or tab (>1000) is called for by looking
				//at the number in front of (
				int num = ExtractNumber(oc, oldchar - 1);
				if (num > 0)
				{
					if (num > 1000.0)
					{
						fputc('\t', _inputFile);
					}
					else if (num > 100.0)
					{
						fputc(' ', _inputFile);
					}
				}
			}
			else if (c == ')' && rbdepth == 1 && !nextliteral)
			{
				//Stop outputting text
				rbdepth = 0;
			}
			else if (rbdepth == 1)
			{
				//Just a normal text character:
				if (c == '\\' && !nextliteral)
				{
					//Only print out next character no matter what. Do not interpret.
					nextliteral = true;
				}
				else
				{
					nextliteral = false;
					if (((c >= ' ') && (c <= '~')) || ((c >= 128) && (c < 255)))
					{
						fputc(c, _inputFile);
					}
				}
			}
		}
		//Store the recent characters for when we have to go back for a number:
		for (j = 0; j < oldchar - 1; j++) oc[j] = oc[j + 1];
		oc[oldchar - 1] = c;
		if (!intextobject)
		{
			if (seen2("BT", oc))
			{
				//Start of a text object:
				intextobject = true;
			}
		}
	}
}

size_t Cpdf_to_text::findStringInBuffer(char * buffer, char * search, size_t buffersize)
{
	std::vector<char>::iterator it_buff(_buffer.begin());
	for (; it_buff != _buffer.end(); it_buff++)
	{
        
	}
	return size_t();
}


void Cpdf_to_text::ReadDocumentByLine()
{
    //Asume isOpen = true
	//get file data
    std::string line;
    if (_inputFile.is_open())
    {
        while (getline(_inputFile, line))
        {
            //Process line
        }
    }
	//TODO: check for more efficient methods if there are. Examples here https://www.geeksforgeeks.org/ways-copy-vector-c/
	//Also check if std::copy can be used and for what is best.https://stackoverflow.com/questions/5420317/reading-and-writing-binary-file

}
