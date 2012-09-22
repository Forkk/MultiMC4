#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <exception>
#include <wx/ioswrap.h>
#include "endian.h"

namespace java
{
	class classfile_exception : public std::exception {};
	class constant
	{
	public:
		enum type_t:uint8_t
		{
			j_string_data = 1,
			j_int = 3,
			j_float = 4,
			j_long = 5,
			j_double = 6,
			j_class = 7,
			j_string = 8,
			j_fieldref = 9,
			j_methodref = 10,
			j_interface_methodref = 11,
			j_nameandtype = 12
		} type;
		constant(char *& data, std::size_t & max_size )
		{
			// bad class file!
			if(max_size < 1)
				throw new classfile_exception();
			origin = data;
			type = (type_t) *((uint8_t *)data);max_size--;data++;
			// invalid constant type!
			if(type > j_nameandtype || type == (type_t)0 || type == (type_t)2)
				throw new classfile_exception();
			std::size_t length = 0;
			// load data depending on type
			switch(type)
			{
				case j_float:
				case j_int:
					if(max_size < 4)
						throw new classfile_exception();
					int_data = bigswap(*(uint32_t *)data); data += 4; max_size -= 4;
					break;
				case j_double:
				case j_long:
					if(max_size < 8)
						throw new classfile_exception();
					int_data = bigswap(*(uint64_t *)data); data += 8; max_size -= 8;
					break;
				case j_class:
					if(max_size < 2)
						throw new classfile_exception();
					ref_type.class_idx = bigswap(*(uint16_t *)data); data += 2; max_size -= 2;
					break;
				case j_fieldref:
				case j_methodref:
				case j_interface_methodref:
					if(max_size < 4)
						throw new classfile_exception();
					ref_type.class_idx = bigswap(*(uint16_t *)data); data += 2; max_size -= 2;
					ref_type.name_and_type_idx = bigswap(*(uint16_t *)data); data += 2; max_size -= 2;
					break;
				case j_string:
					if(max_size < 2)
						throw new classfile_exception();
					int_data = bigswap(*(uint16_t *)data); data += 2; max_size -= 2;
					break;
				case j_string_data:
					if(max_size < 2)
						throw new classfile_exception();
					length = bigswap(*(uint16_t *)data); data += 2; max_size -= 2;
					if(max_size < length)
						throw new classfile_exception();
					// HACK HACK: for now, we call these UTF-8 and do no further processing.
					// Later, we should do some decoding. It's really modified UTF-8
					// * U+0000 is represented as 0xC0,0x80 invalid character
					// * any single zero byte ends the string
					// * characters above U+10000 are encoded like in CESU-8
					str_data.append(data,length);
					data += length;
					max_size -= length;
					break;
				case j_nameandtype:
					if(max_size < 4)
						throw new classfile_exception();
					name_and_type.name_index = bigswap(*(uint16_t *)data); data += 2; max_size -= 2;
					name_and_type.descriptor_index = bigswap(*(uint16_t *)data); data += 2; max_size -= 2;
					break;
			}
		}

		std::string str_data; /** String data in 'modified utf-8'.*/
		// store everything here.
		union
		{
			int64_t int_data;
			double double_data;
			float float_data;
			struct
			{
				/**
				 * Class reference:
				 *   an index within the constant pool to a UTF-8 string containing
				 *   the fully qualified class name (in internal format)
				 * Used for j_class, j_fieldref, j_methodref and j_interface_methodref
				 */
				uint16_t class_idx;
				// used for j_fieldref, j_methodref and j_interface_methodref
				uint16_t name_and_type_idx;
			} ref_type;
			struct
			{
				uint16_t name_index;
				uint16_t descriptor_index;
			} name_and_type;
		};
		char * origin;
	};
	/**
	 * A helper class that represents the custom container used in Java class file for storage of constants
	 */
	class constant_pool
	{
	public:
		/**
		 * Create a pool of constants
		 */
		constant_pool(){}
		/**
		 * Create a pool of constants given an offset into the parsed file and the remaining file size
		 */
		constant_pool(char *& data, std::size_t & max_size)
		{
			load(data, max_size);
		}
		/**
		 * Load a java constant pool
		 */
		void load(char *& data, std::size_t & max_size)
		{
			if(max_size < 2)
			{
				throw new classfile_exception();
			}
			uint16_t length = bigswap(*((uint16_t *)data)) - 1;
			uint16_t index = 1;
			data +=2; max_size-=2;
			const constant * last_constant = nullptr;
			while(length)
			{
				const constant & cnst = constant(data, max_size);
				constants.push_back(cnst);
				last_constant = &constants[constants.size() - 1];
				if(last_constant->type == constant::j_double || last_constant->type == constant::j_long)
				{
					length-=2;
					index+=2;
				}
				else
				{
					length--;
					index++;
				}
			}
		};
		typedef std::vector<java::constant> container_type;
		/**
		 * Access constants based on jar file index numbers (index of the first element is 1)
		 */
		java::constant & operator[](std::size_t constant_index)
		{
			if(constant_index == 0 || constant_index > constants.size())
			{
				throw new classfile_exception();
			}
			return constants[constant_index - 1];
		};
		container_type::const_iterator begin() const
		{
			return constants.begin();
		};
		container_type::const_iterator end() const
		{
			return constants.end();
		}
	private:
		container_type constants;
	};
	/**
	 * Class representing a Java .class file
	 */
	class classfile
	{
	public:
		classfile(char *& data, std::size_t& size)
		{
			valid = false;
			// the class file can't be shorter than 24 bytes (given its structure).
			if(size < 24)
				throw new classfile_exception();
			magic = bigswap(*(uint32_t *)data); data += 4; size -= 4;
			if(magic != 0xCAFEBABE)
				throw new classfile_exception();
			minor_version = bigswap(*(uint16_t *)data); data += 2; size -= 2;
			major_version = bigswap(*(uint16_t *)data); data += 2; size -= 2;
			constants.load(data,size);
			valid = true;
		};
		bool valid;
		uint32_t magic;
		uint16_t minor_version;
		uint16_t major_version;
		constant_pool constants;
		// TBD
		/*
		uint16_t access_flags;
		uint16_t this_class;
		uint16_t super_class;
		std::vector<uint16_t> interfaces;
		// etc.
		*/
	};
}