#pragma once
#include <glad/glad.h> 

#include <memory>
#include <iostream>

namespace ml
{
	template <GLenum buffer_type>
	class Buffer
	{
		std::shared_ptr<GLuint> id;
		size_t size = 0;

	public:
		Buffer()
		{
			id = std::make_shared<GLuint>();
			glGenBuffers(1, id.get());
		}

		~Buffer() 
		{
			//TODO: Make Thread-safe
			if (id.use_count() == 1)
			{
				glDeleteBuffers(1, id.get());
			}
		};

		Buffer(Buffer const& other_buffer) noexcept
		{
			if (this != &other_buffer)
			{
				id = other_buffer.id;
				size = other_buffer.size;
			}
		}

		Buffer& operator=(Buffer const& other_buffer) noexcept
		{
			if (this != &other_buffer)
			{
				id = other_buffer.id;
				size = other_buffer.size;
			}
			return *this;
		}

		Buffer(Buffer&& other_buffer) noexcept
		{
			if (this != &other_buffer)
			{
				id = std::move(other_buffer.id);
				size = std::move(other_buffer.size);
			}
		}

		Buffer& operator=(Buffer&& other_buffer) noexcept
		{
			if (this != &other_buffer)
			{
				id = std::move(other_buffer.id);
				size = std::move(other_buffer.size);
			}
			return *this;
		}

		void bind() const
		{
			glBindBuffer(buffer_type, *id);
		}

		void unbind() const
		{
			glBindBuffer(buffer_type, 0);
		}

		void buffer_data(GLsizeiptr const& data_size, GLenum const & usage)
		{
			bind();
			glNamedBufferData(*id, data_size, nullptr, usage);
			this->size = data_size;
			unbind();
		}

		void bind_buffer_range(GLuint const & index, GLintptr const& offset, GLsizeiptr const& data_size) const
			requires(buffer_type == GL_UNIFORM_BUFFER)
		{
			bind();
			glBindBufferRange(buffer_type, index, *id, offset, data_size);
			unbind();
		}

		void buffer_sub_data(GLintptr offset, GLsizeiptr data_size, const void * data) const
		{
			glNamedBufferSubData(*id, offset, data_size, data);
		}
	};
}
