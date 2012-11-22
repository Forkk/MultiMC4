/*
	wxInclude version 1.0
	Kim De Deyn
*/

#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <exception>
#include <fstream>
#include <stdexcept>

//#include <boost/program_options.hpp>
//namespace po = boost::program_options;

#define WXINCLUDE_INFO	"wxInclude originally by Kim De Deyn, this is a gutted version.\n"

#define WXINCLUDE_VERSION "Version 1.0, compiled at " __DATE__ " " __TIME__

#define BUFFER_SIZE 4096

void defineheader_start ( std::ostringstream& data, std::string& headername, bool usemacro = true, bool useconst = false )
{
	/* Write info header */
	data << "/*" << std::endl;
	data << "	Automatic generated header. Do not modify." << std::endl;
	data << "	Generator: " WXINCLUDE_INFO << std::endl << std::endl;
	data << "	Header: " << headername << std::endl;
	data << "	Macros: " << ( usemacro ? "yes" : "no" ) << std::endl;
	data << "	Const: " << ( useconst ? "yes" : "no" ) << std::endl;
	data << "*/" << std::endl << std::endl;

	/* Prevent multiple defines */
	std::string temp ( headername );
	std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
	data << "#ifndef _WXINCLUDE_" << temp << "_H_" << std::endl;
	data << "#define _WXINCLUDE_" << temp << "_H_" << std::endl << std::endl;
}

void defineheader_end ( std::ostringstream& data, std::string& name )
{
	/* Prevent multiple defines */
	data << "#endif" << std::endl << std::endl;
}

void definemacros ( std::ostringstream& data )
{
	/* Include wxWidgets headers */
	data << "#include \"wx/wx.h\"" << std::endl;
	data << "#include \"wx/mstream.h\"" << std::endl;

	data << std::endl;

	/* Define some useful macros */
	data << "#define wxMEMORY_IMAGE( name ) _wxConvertMemoryToImage( name, sizeof( name ) )" << std::endl;
	data << "#define wxMEMORY_IMAGEEX( name, type ) _wxConvertMemoryToImage( name, sizeof( name ), type )" << std::endl;
	data << "#define wxMEMORY_BITMAP( name ) _wxConvertMemoryToBitmap( name, sizeof( name ) )" << std::endl;
	data << "#define wxMEMORY_BITMAPEX( name, type ) _wxConvertMemoryToBitmap( name, sizeof( name ), type )" << std::endl << std::endl;

	data << "inline wxImage _wxConvertMemoryToImage(const unsigned char* data, int length, long type = wxBITMAP_TYPE_ANY )" << std::endl;
	data << "{" << std::endl;
	data << "	wxMemoryInputStream stream( data, length );" << std::endl;
	data << "	wxImage img;" << std::endl;
	data << "	img.LoadFile(stream, (wxBitmapType)type, -1);" << std::endl;
	data << "	return img;" << std::endl;
	data << "}" << std::endl << std::endl;

	data << "inline wxBitmap _wxConvertMemoryToBitmap(const unsigned char* data, int length, long type = wxBITMAP_TYPE_ANY )" << std::endl;
	data << "{" << std::endl;
	data << "	wxMemoryInputStream stream( data, length );" << std::endl;
	data << "	wxImage img;" << std::endl;
	data << "	img.LoadFile(stream, (wxBitmapType)type, -1);" << std::endl;
	data << "	return wxBitmap( img, -1 );" << std::endl;
	data << "}" << std::endl << std::endl;
}

static std::vector<std::string> list;

void definefile ( std::ostringstream& data, std::ifstream& input, std::string& name, bool useconst = false )
{
	/* Check if already defined */
	std::vector<std::string>::iterator search = std::find ( list.begin(), list.end(), name );
	if ( search == list.end() )
	{
		list.push_back ( name );
	}
	else
	{
		/* Show warning, object of this name is already processed! */
		std::cout << "Warning: '" << name << "' already defined, processing of new one stopped." << std::endl;
		return;
	}

	/* Define array */
	data << "static" << ( useconst ? " const " : " " ) << "unsigned char " << name << "[] = {" << std::endl;

	unsigned size = input.tellg();
	input.seekg ( 0, std::ios::beg );

	int c = 0;
	int col = 0;

	for ( unsigned i = 1; i <= size; ++i )
	{
		/* Get character and add to array */
		c = input.get();
		char temp[5];
		temp[4] = 0;

#ifdef WIN32
		_snprintf ( temp, 5, "0x%02X", c );
#else
		snprintf ( temp, 5, "0x%02X", c );
#endif
		
		data << temp;

		if ( i >= size )
		{
			/* Last character */
			data << std::endl;
		}
		else
		{
			/* Next */
			data << ", ";
		}

		/* New colume? */
		int curcol = ( i / 10 );
		if ( col < curcol )
		{
			col = curcol;
			data << std::endl;
		}
	}

	data << "};" << std::endl << std::endl;
}

std::string GetFileExtension(const std::string& FileName)
{
	if(FileName.find_last_of(".") != std::string::npos)
		return FileName.substr(FileName.find_last_of("."));
	return "";
}

std::string removeExtension(const std::string & filename)
{
	std::size_t lastdot = filename.find_last_of(".");
	if (lastdot == std::string::npos)
		return filename;
	return filename.substr(0, lastdot); 
}

std::string GetFileBasename(const std::string& path)
{
	std::size_t last_slash = path.find_last_of("/");
	std::size_t last_backslash = path.find_last_of("/");
	std::string cropped = path;
	if(last_slash != std::string::npos)
	{
		cropped = cropped.substr(last_slash+1);
	}
	else if(last_backslash != std::string::npos)
	{
		cropped = cropped.substr(last_backslash+1);
	}
	if(cropped.find_last_of(".") != std::string::npos)
	{
		return cropped.substr(0,cropped.find_last_of("."));
	}
	else return cropped;
}

int toUnderscores (int __c)
{
	if(__c == '-')
		return '_';
	return __c;
}

int main ( int argc, const char* argv[] )
{
	if(argc <= 2)
	{
		std::cerr << "You need to specify at least the output file and one input file." << std::endl
		          << "Example: " << argv[0] << " output_file input_file" << std::endl;
		return 1;
	}
	try
	{
		std::string headername (argv[1]);
		std::vector<std::string> input_files;
		for(int i = 2; i < argc; i++)
		{
			input_files.push_back(std::string(argv[i]));
		}
		/* Process */
		std::ofstream output ( headername, std::ios::out | std::ios::trunc| std::ios::binary );

		/* Use buffer */
		char outbuffer[BUFFER_SIZE];
		output.rdbuf()->pubsetbuf ( outbuffer, BUFFER_SIZE );

		if ( !output )
			throw std::runtime_error ( "Failed to create output file!" );

		/* Show status */
		std::cout << "Build  : file '" << headername << "'..." << std::endl;

		/* Get base name of file */
		headername = GetFileBasename ( headername );

		/* Data string stream */
		std::ostringstream data;

		/* Write header start when wanted */
		defineheader_start ( data, headername, /*use macro*/ true, /* const */ true );

		/* Write macros */
		definemacros ( data );

		/* Common input buffer */
		char inbuffer[BUFFER_SIZE];

		for ( auto iter = input_files.begin(); iter != input_files.end(); iter++ )
		{
			std::string &file = *iter;
			//std::string fileext = GetFileExtension ( file );

			std::ifstream input ( file, std::ios::in | std::ios::binary | std::ios::ate );
			input.rdbuf()->pubsetbuf ( inbuffer, BUFFER_SIZE );

			if ( input.is_open() )
			{
				/* Show status */
				std::cout << "Process: file '" << file << "'..." << std::endl;

				/* Remove extension */
				file = removeExtension(file);
				std::transform(file.begin(), file.end(), file.begin(), ::tolower);
				std::transform(file.begin(), file.end(), file.begin(), toUnderscores);

				/* Process file */
				definefile ( data, input, file, true );
			}
			else
			{
				/* Only show warning, other files need to be processed */
				std::cout << "Warning: input file '" << file << "' failed to open." << std::endl;
			}
		}

		/* Write header end when wanted */
		defineheader_end ( data, headername );

		/* Write data to output file */
		output.seekp ( 0, std::ios::beg );
		output << data.str();
	}
	catch ( std::exception& e )
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch ( ... )
	{
		std::cerr << "Error: Exception of unknown type!" << std::endl;
	}

	return 0;
}