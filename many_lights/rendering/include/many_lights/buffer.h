#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <memory>

namespace ml
{
	template <GLenum buffer_type>
	class Buffer
	{
	private:
		std::shared_ptr<GLuint> id;
		GLsizeiptr size = 0;

	public:
		//add more constructors maybe?
		Buffer()
		{
			id = std::make_shared<GLuint>();
			glGenBuffers(1, id.get());
		}

		~Buffer()
		{
			if (id.use_count() == 1) // talk about thread safety
			{
				glDeleteBuffers(1, id.get());
			}
		};

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
			glBufferData(buffer_type, size, NULL, usage);
			this->size = size;
		}

		void bind_buffer_range(GLuint const & index, GLintptr const& offset, GLsizeiptr const& size)
			requires(buffer_type == GL_UNIFORM_BUFFER)
		{
			glBindBufferRange(buffer_type, index, *id, offset, size);
		}

		void buffer_sub_data(GLintptr const& offset, GLsizeiptr const& size, const void * const data)
		{
			std::cout << *id << " " << offset << " " << size << " " << data << std::endl;
			std::cout << glGetError() << std::endl;
			glNamedBufferSubData(*id, offset, size, data);
		}
	};
}