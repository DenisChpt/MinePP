#include "Renderer.hpp"
#include <iostream>

Renderer::Renderer() : shaderProgram(0) {}

Renderer::~Renderer() {}

void Renderer::initialize() {
	// Set up basic OpenGL state here (e.g., enable depth testing, culling)
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

void Renderer::clear() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
	
void Renderer::draw() const {
	// Placeholder for actual drawing code
}