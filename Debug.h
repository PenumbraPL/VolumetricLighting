#pragma once
#include <vector>

typedef void (* DEBUGPROC)(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

void debug_init(std::vector<DEBUGPROC> callback_list);
void callback_list(std::vector<DEBUGPROC>& callback_list);
void error_callback(int code, const char* description);
void turn_on_only_errors();
void turn_on_everything();
void display_logs();