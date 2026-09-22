//============================================================================
// Name        : hahaha.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <Core/Window.h>

Window g_window(800, 600, "h32engine");

int main() { 
	std::cout << "Hello\n";

	while (!g_window.shouldClose()) {
		g_window.pollEvents();

		// nothing to draw yet — just prove the loop works
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		g_window.swapBuffers();
	}
}