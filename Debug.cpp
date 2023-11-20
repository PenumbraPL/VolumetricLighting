#include "GL/glew.h"
#include "Debug.h"
//#include <vector>
#include "iostream"


void debug_init(std::vector<DEBUGPROC> callback_list) {
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	const void* userParam = nullptr;
	for (auto& callback : callback_list) {
		glDebugMessageCallback(callback, userParam);
	}

	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

	/*GLchar messageLog[2048];
	glGetDebugMessageLog(1, 2048, NULL, NULL, NULL, NULL, NULL, messageLog);
	std::cout << messageLog << std::endl << "==================================" << std::endl;*/
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);


	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);


}

void callback1(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	/*std::cout << "Source: " << source << " Type: " << type 
		<< " Id: " << id << " Severity: " << severity << " Length: " << length
		<< " Message: " << message << std::endl 
		<< " ======================================================================== \n";*/

	std::cout << "Severity: " << severity << " Message: " << message << std::endl
		<< " ======================================================================== \n";
};


void callback_list(std::vector<DEBUGPROC>& callback_list) {
	callback_list.push_back(&callback1);
}


void error_callback(int code, const char* description)
{
	std::cout << code << " " << description << std::endl;
}