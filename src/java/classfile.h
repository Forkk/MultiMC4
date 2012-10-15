#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <exception>
#include "endian.h"

namespace java
{
	class membuffer
	{
	public:
		membuffer(char * buffer, std::size_t size)
		{
			current = start = buffer;
			end = start + size;
		}
		~membuffer()
		{
			// maybe? possibly? left out to avoid confusion. for now.
			//delete start;
		}
		/**
		 * Read some value. That's all ;)
		 */
		template <class T>
		void read(T& val)
		{
			val = *(T *)current;
			current += sizeof(T);
		}
		/**
		 * Read a big-endian number
		 * valid for 2-byte, 4-byte and 8-byte variables
		 */
		template <class T>
		void read_be(T& val)
		{
			val = bigswap(*(T *)current);
			current += sizeof(T);
		}
		/**
		 * Read a string in the format:
		 * 2B length (big endian, unsigned)
		 * length bytes data
		 */
		void read_jstr(std::string & str)
		{
			uint16_t length = 0;
			read_be(length);
			str.append(current,length);
			current += length;
		}
		/**
		 * Skip N bytes
		 */
		void skip (std::size_t N)
		{
			current += N;
		}
	private:
		char * start, *end, *current;
	};
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
		constant(membuffer & buf )
		{
			buf.read(type);
			// invalid constant type!
			if(type > j_nameandtype || type == (type_t)0 || type == (type_t)2)
				throw new classfile_exception();
			
			// load data depending on type
			switch(type)
			{
				case j_float:
				case j_int:
					buf.read_be(int_data); // same as float data really
					break;
				case j_double:
				case j_long:
					buf.read_be(long_data); // same as double
					break;
				case j_class:
					buf.read_be(ref_type.class_idx);
					break;
				case j_fieldref:
				case j_methodref:
				case j_interface_methodref:
					buf.read_be(ref_type.class_idx);
					buf.read_be(ref_type.name_and_type_idx);
					break;
				case j_string:
					buf.read_be(index);
					break;
				case j_string_data:
					// HACK HACK: for now, we call these UTF-8 and do no further processing.
					// Later, we should do some decoding. It's really modified UTF-8
					// * U+0000 is represented as 0xC0,0x80 invalid character
					// * any single zero byte ends the string
					// * characters above U+10000 are encoded like in CESU-8
					buf.read_jstr(str_data);
					break;
				case j_nameandtype:
					buf.read_be(name_and_type.name_index);
					buf.read_be(name_and_type.descriptor_index);
					break;
			}
		}

		std::string str_data; /** String data in 'modified utf-8'.*/
		// store everything here.
		union
		{
			int32_t int_data;
			int64_t long_data;
			float float_data;
			double double_data;
			uint16_t index;
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
		 * Load a java constant pool
		 */
		void load(membuffer & buf)
		{
			uint16_t length = 0;
			buf.read_be(length);
			length --;
			uint16_t index = 1;
			const constant * last_constant = nullptr;
			while(length)
			{
				const constant & cnst = constant(buf);
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
	class classfile : public membuffer
	{
	public:
		classfile(char * data, std::size_t size) : membuffer(data, size)
		{
			valid = false;
			read_be(magic);
			if(magic != 0xCAFEBABE)
				throw new classfile_exception();
			read_be(minor_version);
			read_be(major_version);
			constants.load(*this);
			read_be(access_flags);
			read_be(this_class);
			read_be(super_class);
			
			// Interfaces
			uint16_t iface_count = 0;
			read_be(iface_count);
			while (iface_count)
			{
				uint16_t iface;
				read_be(iface);
				interfaces.push_back(iface);
				iface_count --;
			}
			
			// Fields
			// read fields (and attributes from inside fields) (and possible inner classes. yay for recursion!)
			// for now though, we will ignore all attributes
			/*
			 * field_info
			 * {
			 * 	u2 access_flags;
			 * 	u2 name_index;
			 * 	u2 descriptor_index;
			 * 	u2 attributes_count;
			 * 	attribute_info attributes[attributes_count];
			 * }
			 */
			uint16_t field_count = 0;
			read_be(field_count);
			while (field_count)
			{
				// skip field stuff
				skip(6);
				// and skip field attributes
				uint16_t attr_count = 0;
				read_be(attr_count);
				while(attr_count)
				{
					skip(2);
					uint32_t attr_length = 0;
					read_be(attr_length);
					skip(attr_length);
					attr_count --;
				}
				field_count --;
			}

			// class methods
			/*
			 * method_info
			 * {
			 * 	u2 access_flags;
			 * 	u2 name_index;
			 * 	u2 descriptor_index;
			 * 	u2 attributes_count;
			 * 	attribute_info attributes[attributes_count];
			 * }
			 */
			uint16_t method_count = 0;
			read_be(method_count);
			while( method_count )
			{
				skip(6);
				// and skip method attributes
				uint16_t attr_count = 0;
				read_be(attr_count);
				while(attr_count)
				{
					skip(2);
					uint32_t attr_length = 0;
					read_be(attr_length);
					skip(attr_length);
					attr_count --;
				}
				method_count --;
			}

			// class attributes
			// there are many kinds of attributes. this is just the generic wrapper structure.
			// type is decided by attribute name. extensions to the standard are *possible*
			// class annotations are one kind of a attribute (one per class)
			/*
			 * attribute_info
			 * {
			 * 	u2 attribute_name_index;
			 * 	u4 attribute_length;
			 * 	u1 info[attribute_length];
			 * }
			 */
			uint16_t class_attr_count = 0;
			read_be(class_attr_count);
			while(class_attr_count)
			{
				skip(2);
				uint32_t attr_length = 0;
				read_be(attr_length);
				skip(attr_length);
				class_attr_count --;
			}
			valid = true;
		};
		bool valid;
		uint32_t magic;
		uint16_t minor_version;
		uint16_t major_version;
		constant_pool constants;
		uint16_t access_flags;
		uint16_t this_class;
		uint16_t super_class;
		// interfaces this class implements ? must be. investigate.
		std::vector<uint16_t> interfaces;
		
	};
}