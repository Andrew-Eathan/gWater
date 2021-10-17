#include "util.h"

void LUA_Print(std::string text)
{
	GlobalLUA->PushSpecial(SPECIAL_GLOB);
	GlobalLUA->GetField(-1, "print");
	GlobalLUA->PushString(text.c_str());
	GlobalLUA->Call(1, 0);
	GlobalLUA->Pop();
}

void LUA_Print(char* text)
{
	GlobalLUA->PushSpecial(SPECIAL_GLOB);
	GlobalLUA->GetField(-1, "print");
	GlobalLUA->PushString(text);
	GlobalLUA->Call(1, 0);
	GlobalLUA->Pop();
}

void LUA_Print(int num)
{
	GlobalLUA->PushSpecial(SPECIAL_GLOB);
	GlobalLUA->GetField(-1, "print");
	GlobalLUA->PushString(std::to_string(num).c_str());
	GlobalLUA->Call(1, 0);
	GlobalLUA->Pop();
}

std::string FLEX_GetErrorType(NvFlexErrorSeverity type) {
	switch (type) {
	case NvFlexErrorSeverity::eNvFlexLogAll:
		return "ALL";
	case NvFlexErrorSeverity::eNvFlexLogDebug:
		return "DEBUG";
	case NvFlexErrorSeverity::eNvFlexLogError:
		return "ERROR";
	case NvFlexErrorSeverity::eNvFlexLogInfo:
		return "INFO";
	case NvFlexErrorSeverity::eNvFlexLogWarning:
		return "WARNING";
	default:
		return "UNKNOWN"; //probably useless to add
	}
}