#include "system/util/util.hpp"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "base64/base64.h"

std::string Util_encode_to_escape(std::string in_data)
{
	int string_length = in_data.length();
	std::string check = "";
	std::string return_data = "";

	for(int i = 0; i < string_length; i++)
	{
		check = in_data.substr(i, 1);
		if(check == "\n")
			return_data += "\\n";
		else if(check == "\u0022")
			return_data += "\\\u0022";
		else if(check == "\u005c")
			return_data += "\\\u005c";
		else
			return_data += in_data.substr(i, 1);
	}

	return return_data;
}

std::string Util_encode_to_base64(char* source, int size)
{
	return base64_encode((const unsigned char*)source, size);
}

std::string Util_decode_from_base64(std::string source)
{
	return base64_decode(source);
}
