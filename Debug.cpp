#include "GL/glew.h"
#include "pch.h"
#include "Debug.h"

extern spdlog::logger logger;

namespace debug 
{
	void turnOnEverything(bool turnOnNotify)
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH,   0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW,    0, NULL, GL_TRUE);
	
		if (turnOnNotify) {
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
		}
		else {
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);
		}
	}

	void turnOnOnlyErrors()
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,  0, NULL, GL_FALSE);

		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH,   0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_LOW,    0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_NOTIFICATION,  0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	}

	void displayLogs()
	{
		char messageLog[2048] = {'\0'};
		//memset(messageLog, '\0', 2048);
		glGetDebugMessageLog(1, 2048, NULL, NULL, NULL, NULL, NULL, messageLog);
		//fwrite(messageLog, sizeof(char), 2048, stdout);
		logger.warn(messageLog);
	}

	void callbackBasicInfo(GLenum source,
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
		std::string text = "Severity: " + sev + " Message: " + message + 
			 "\n ========================================================================";
		logger.warn(text);
	};

	void callbackFullInfo(GLenum source,
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

		std::string text = "Source: " + std::to_string(source) + " Type: " + std::to_string(type)
			+ " Id: " + std::to_string(id)  + " Severity: " + sev + " Length: " + std::to_string(length)
			+ " Message: " + message
			+ "\n ========================================================================";
		logger.warn(text);
	};

	void fillCallbackList(std::vector<DEBUGPROC>& callback_list) 
	{
		callback_list.push_back(&callbackBasicInfo);
		//callback_list.push_back(&gl_callback_full_info);
	}

	void glewCallback(int code, const char* description)
	{
	//	std::cout << code << " " << description << std::endl;
		std::string text{ code + " " };
		text += description;
		logger.warn(text);
	}

	void basicLogfileExample()
	{
		try {
			auto logger{ spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt") };
		}
		catch (const spdlog::spdlog_ex& ex) {
			std::cout << "Log init failed: " << ex.what() << std::endl;
		}
	}

	/* ================================================ */

	void debugInit(std::vector<DEBUGPROC>& callback_list) 
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		const void* userParam = nullptr;
		for (auto& callback : callback_list) {
			glDebugMessageCallback(callback, userParam);
		}
		displayLogs();
		turnOnOnlyErrors();
		//turn_on_everything();
		basicLogfileExample();
	}
}
