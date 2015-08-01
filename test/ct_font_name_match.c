#include "ct.h"

extern int font_name_match(const char* key, const char* font);

int main(void)
{
	testeqi(font_name_match("Courier", "Iosevka"), 0);
	testeqi(font_name_match("Iosevka", "Iosevka"), 1);
	testeqi(font_name_match("Iosevka", "Iosevka-Regular"), 1);
	testeqi(font_name_match("Iosevkablah", "Iosevka-Regular"), 0);
	testeqi(font_name_match("Ios", "Iosevka-Regular"), 0);
	return 0;
}
