#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Renderer {
public:
	Renderer();
	~Renderer();

	void initialize();
	void clear() const;
	void draw() const; // Placeholder for drawing functionality

private:
	// Add shader program ID and other necessary members here
	GLuint shaderProgram;
};

#endif // RENDERER_HPP
