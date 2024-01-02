#include "GL/glew.h"
#include "Debug.h"
#include "iostream"
#include "spdlog/sinks/basic_file_sink.h"


void turn_on_everything(bool without_notif) {
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_TRUE);
	
	if (without_notif) {
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	}
	else {
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);
	}
}

void turn_on_only_errors() {
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
}

void gl_display_logs() {
	char messageLog[2048];
	memset(messageLog, '\0', 2048);
	glGetDebugMessageLog(1, 2048, NULL, NULL, NULL, NULL, NULL, messageLog);
	fwrite(messageLog, sizeof(char), 2048, stdout);
}

void gl_callback_basic_info(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	std::string sev;
	switch (severity) {
	case GL_DEBUG_SEVERITY_NOTIFICATION: sev = "[Notification]"; break;
	case GL_DEBUG_SEVERITY_HIGH:		sev = "[High]"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:		sev = "[Medium]"; break;
	case GL_DEBUG_SEVERITY_LOW:			sev = "[Low]"; break;
	default:							sev = "[Unknown]";
	}

	std::cout << "Severity: " << sev << " Message: " << message << std::endl
		<< " ======================================================================== \n";
};

void gl_callback_full_info(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	std::string sev;
	switch (severity) {
	case GL_DEBUG_SEVERITY_NOTIFICATION: sev = "[Notification]"; break;
	case GL_DEBUG_SEVERITY_HIGH:		sev = "[High]"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:		sev = "[Medium]"; break;
	case GL_DEBUG_SEVERITY_LOW:			sev = "[Low]"; break;
	default:							sev = "[Unknown]";
	}

	std::cout << "Source: " << source << " Type: " << type
		<< " Id: " << id << " Severity: " << sev << " Length: " << length
		<< " Message: " << message << std::endl
		<< " ======================================================================== \n";
};

void gl_fill_callback_list(std::vector<DEBUGPROC>& callback_list) {
	callback_list.push_back(&gl_callback_basic_info);
	//callback_list.push_back(&gl_callback_full_info);
}

void glew_callback(int code, const char* description)
{
	std::cout << code << " " << description << std::endl;
}

void basic_logfile_example()
{
	try
	{
		auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log init failed: " << ex.what() << std::endl;
	}
}

/* ================================================ */

void gl_debug_init(std::vector<DEBUGPROC>& callback_list) {
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	const void* userParam = nullptr;
	for (auto& callback : callback_list) {
		glDebugMessageCallback(callback, userParam);
	}
	gl_display_logs();
	turn_on_only_errors();
	//turn_on_everything();
	basic_logfile_example();
}

