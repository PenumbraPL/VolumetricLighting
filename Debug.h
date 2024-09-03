#pragma once
#include <vector>
#include <sstream>

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
	void debugInit(std::vector<DEBUGPROC>& callback_list);
	void fillCallbackList(std::vector<DEBUGPROC>& callback_list);
	void glewCallback(int code, const char* description);

    class BufferLogger : public spdlog::sinks::sink {
    public:
        explicit BufferLogger(std::size_t _maxBufferSize = 4096) : _maxBufferSize(_maxBufferSize) {
            _buffer.reserve(_maxBufferSize);
        }

        void log(const spdlog::details::log_msg& msg) override {
            std::ostringstream oss;
            oss << "[" << spdlog::level::to_string_view(msg.level).data() << "] " << msg.payload.data();
            std::string formattedMsg = oss.str();
            _buffer += formattedMsg;
            _buffer += "\n";

            if (_buffer.size() > _maxBufferSize) {
                _buffer.erase(0, _buffer.size() - _maxBufferSize);
            }
        }


        std::string getBuffer() const {
            return _buffer;
        }

        void set_pattern(const std::string& pattern) override {}
        void flush() override {}
        void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override {}

    private:
        std::string _buffer;
        std::size_t _maxBufferSize;
    };
}

