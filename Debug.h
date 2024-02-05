#pragma once
#include <vector>

typedef void (*DEBUGPROC)(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

namespace debug
{
	void debug_init(std::vector<DEBUGPROC>& callback_list);
	void fill_callback_list(std::vector<DEBUGPROC>& callback_list);
	void glew_callback(int code, const char* description);
}
