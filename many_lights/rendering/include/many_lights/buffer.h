#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <memory>
#include <iostream>

namespace ml
{
	template <GLenum buffer_type>
	class Buffer
	{
	public:
		std::shared_ptr<GLuint> id;
		size_t size = 0;

	public:
		//add more constructors maybe?
		Buffer()
		{
			id = std::make_shared<GLuint>();
			glGenBuffers(1, id.get());
			std::cout << "Buffer constructed: " << *id << std::endl;
		}

		~Buffer()
		{
			if (id)
			{
				std::cout << "Buffer destructor: " << *id << std::endl;
			}
			else
			{
				std::cout << "Buffer already destroyed" << std::endl;
			}
			if (id.use_count() == 1) // talk about thread safety
			{
				std::cout << "Buffer destroyed: " << *id << std::endl;
				glDeleteBuffers(1, id.get());
			}
		};

		Buffer(Buffer const& other_buffer)
		{
			id = other_buffer.id;
			size = other_buffer.size;
			std::cout << "Buffer copied: " << *id << std::endl;
		}

		Buffer& operator=(Buffer const& other_buffer)
		{
			id = other_buffer.id;
			size = other_buffer.size;
			std::cout << "Buffer copy assigned: " << *id << std::endl;
		}

		Buffer(Buffer&& other_buffer)
		{
			id = std::move(other_buffer.id);
			size = std::move(other_buffer.size);
			std::cout << "Buffer moved: " << *id << std::endl;
		}

		Buffer& operator=(Buffer&& other_buffer)
		{
			if (this == &other_buffer)
			{
				return *this;
			}
			id = std::move(other_buffer.id);
			size = std::move(other_buffer.size);
			std::cout << "Buffer move assigned: " << *id << std::endl;
			return *this;
		}

		void bind()
		{
			glBindBuffer(buffer_type, *id);
		}

		void unbind()
		{
			glBindBuffer(buffer_type, 0);
		}

		void buffer_data(GLsizeiptr const& size, GLenum const & usage)
		{
			bind();
			glNamedBufferData(*id, size, NULL, usage);
			unbind();
		}

		void bind_buffer_range(GLuint const & index, GLintptr const& offset, GLsizeiptr const& size)
			requires(buffer_type == GL_UNIFORM_BUFFER)
		{
			bind();
			glBindBufferRange(buffer_type, index, *id, offset, size);
			unbind();
		}

		void buffer_sub_data(GLintptr offset, GLsizeiptr size, const void * data)
		{
			glNamedBufferSubData(*id, offset, size, data);
		}
	};
}