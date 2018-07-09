#include "seen.hpp"

int main(int arc, const char* argv[])
{
	seen::RendererGL renderer("./data", argv[0]);

	while(renderer.is_running())
	{
		renderer.draw(NULL, { /* passes go here */ });
	}

	return 0;
}
