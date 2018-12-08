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
    if (size_t postd = line_in.find("TD") != std::string::npos)
    {
        _outputFile << line_in;
    }
    return std::string();
}

void Cpdf_to_text::ProcessOutput(char * output, size_t length)
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
					fputc(0x0d, pPdfFile);
                    //new line
					fputc(0x0a, pPdfFile);
				}
				if (num < 1.0)
				{
					fputc('\t', pPdfFile);
				}
			}
			if (rbdepth == 0 && seen2("ET", oc))
			{
				//End of a text object, also go to a new line.
				intextobject = false;
				fputc(0x0d, pPdfFile);
				fputc(0x0a, pPdfFile);
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
						fputc('\t', pPdfFile);
					}
					else if (num > 100.0)
					{
						fputc(' ', pPdfFile);
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
						fputc(c, pPdfFile);
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
