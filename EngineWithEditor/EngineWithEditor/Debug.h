#pragma once

// Todo
namespace Debug
{
#if _DEBUG
	void Log(_In_z_ const char* message) noexcept(noexcept(printf))
	{
		printf("Log | %s", message);
	}
	void LogWarning(_In_z_ const char* message) noexcept(noexcept(printf))
	{
		printf("Warning | %s", message);
	}
	void LogError(_In_z_ const char* message) noexcept(noexcept(printf))
	{
		printf("Error | %s", message);
	}
	[[ noreturn ]] void LogAssertion(bool condition, _In_z_ const char* message) noexcept(false)
	{
		if (!condition)
		{
			printf("Assertion Failed | %s", message);
			throw std::exception("Failed assertion");
		}
	}
#else
	void Log(_In_z_ const char* message) noexcept {}
	void LogWarning(_In_z_ const char* message) noexcept {}
	void LogError(_In_z_ const char* message) noexcept {}
	void LogAssertion(bool condition, _In_z_ const char* message) noexcept {}
#endif
}
