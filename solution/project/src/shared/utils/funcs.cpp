#include "funcs.hpp"

std::wstring utils::utf8ToWide(std::string_view str)
{
	const int count{ MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), 0, 0) };

	std::wstring s(count, 0);

	MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), &s[0], count);

	return s;
}

std::string utils::wideToUtf8(std::wstring_view str)
{
	const int count{ WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), 0, 0, 0, 0) };

	std::string s(count, 0);

	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), &s[0], count, 0, 0);

	return s;
}

Color utils::hsvToRGB(float H, float S, float V)
{
	if (H > 360.0f || H < 0.0f || S > 100.0f || S < 0.0f || V > 100.0f || V < 0.0f)
	{
		return {};
	}

	const float s{ S / 100.0f };
	const float v{ V / 100.0f };
	const float C{ s * v };
	const float X{ C * (1.0f - std::fabs(std::fmod(H / 60.0f, 2.0f) - 1.0f)) };
	const float m{ v - C };

	float r{};
	float g{};
	float b{};

	if (H >= 0.0f && H < 60.0f) {
		r = C;
		g = X;
		b = 0.0f;
	}

	else if (H >= 60.0f && H < 120.0f) {
		r = X;
		g = C;
		b = 0.0f;
	}

	else if (H >= 120.0f && H < 180.0f) {
		r = 0.0f;
		g = C;
		b = X;
	}

	else if (H >= 180.0f && H < 240.0f) {
		r = 0.0f;
		g = X;
		b = C;
	}

	else if (H >= 240.0f && H < 300.0f) {
		r = X;
		g = 0.0f;
		b = C;
	}

	else {
		r = C;
		g = 0.0f;
		b = X;
	}

	return
	{
		static_cast<std::uint8_t>((r + m) * 255.0f),
		static_cast<std::uint8_t>((g + m) * 255.0f),
		static_cast<std::uint8_t>((b + m) * 255.0f),
		255
	};
}

void utils::rgbToHSV(const Color rgb, float &h_out, float &s_out, float &v_out)
{
	const float r{ static_cast<float>(rgb.r) / 255.0f };
	const float g{ static_cast<float>(rgb.g) / 255.0f };
	const float b{ static_cast<float>(rgb.b) / 255.0f };

	const float cmax{ std::max(r, std::max(g, b)) };
	const float cmin{ std::min(r, std::min(g, b)) };
	const float diff{ cmax - cmin };

	float v{ cmax * 100.0f };
	float h{ -1.0f };
	float s{ -1.0f };

	if (cmax == cmin) {
		h = 0.0f;
	}

	else if (cmax == r) {
		h = std::fmodf(60.0f * ((g - b) / diff) + 360.0f, 360.0f);
	}

	else if (cmax == g) {
		h = std::fmodf(60.0f * ((b - r) / diff) + 120.0f, 360.0f);
	}

	else if (cmax == b) {
		h = std::fmodf(60.0f * ((r - g) / diff) + 240.0f, 360.0f);
	}

	if (cmax == 0.0f) {
		s = 0.0f;
	}

	else {
		s = (diff / cmax) * 100.0f;
	}

	h_out = h;
	s_out = s;
	v_out = v;
}

Color utils::rainbow(const float t, const uint8_t a)
{
	const uint8_t r{ static_cast<uint8_t>(std::roundf(std::cosf(t + 0.0f) * 127.5f + 127.5f)) };
	const uint8_t g{ static_cast<uint8_t>(std::roundf(std::cosf(t + 2.0f) * 127.5f + 127.5f)) };
	const uint8_t b{ static_cast<uint8_t>(std::roundf(std::cosf(t + 4.0f) * 127.5f + 127.5f)) };

	return { r, g, b, a };
}

void utils::copyToClipboard(std::string_view str)
{
	const HGLOBAL mem{ GlobalAlloc(GMEM_MOVEABLE, (str.size() + 1) * sizeof(char)) };

	if (!mem || !OpenClipboard(0)) {
		return;
	}

	EmptyClipboard();

	char *const mem_str{ static_cast<char *>(GlobalLock(mem)) };

	if (!mem_str) {
		return;
	}

	memcpy(mem_str, str.data(), str.size());

	GlobalUnlock(mem);
	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();
}

std::string utils::getFilePath()
{
	char *app_data{};
	size_t len{};

	_dupenv_s(&app_data, &len, "APPDATA");

	if (!app_data) {
		return {};
	}

	std::string path{ app_data };

	free(app_data);

	return std::string(path) + "\\seo64\\";
}

std::string utils::keyToStr(const int key)
{
	switch (key)
	{
		case 0: return "key not set";
		case VK_MENU: return "alt";
		case VK_CAPITAL: return "caps";
		case VK_LBUTTON: return "m1";
		case VK_RBUTTON: return "m2";
		case VK_MBUTTON: return "m3";
		case VK_XBUTTON1: return "m4";
		case VK_XBUTTON2: return "m5";
		case VK_NUMPAD0: return "num0";
		case VK_NUMPAD1: return "num1";
		case VK_NUMPAD2: return "num2";
		case VK_NUMPAD3: return "num3";
		case VK_NUMPAD4: return "num4";
		case VK_NUMPAD5: return "num5";
		case VK_NUMPAD6: return "num6";
		case VK_NUMPAD7: return "num7";
		case VK_NUMPAD8: return "num8";
		case VK_NUMPAD9: return "num9";

		default:
		{
			CHAR output[16]{ '\0' };

			if (const int result{ GetKeyNameTextA(MapVirtualKeyW(key, MAPVK_VK_TO_VSC) << 16, output, 16) })
			{
				for (int i{}; i < result; i++) {
					output[i] = std::tolower(output[i]);
				}

				return output;
			}

			return "unknown";
		}
	}
}