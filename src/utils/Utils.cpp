#include "Utils.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Matrix.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cerrno>

// Fonctions internes (non exposées dans l'interface)
namespace {
	// Charge le contenu d'un fichier en mémoire (doit être libéré par l'appelant)
	char *load_file(const char *path) {
		FILE *file = fopen(path, "rb");
		if (!file) {
			std::fprintf(stderr, "fopen %s failed: %d %s\n", path, errno, std::strerror(errno));
			std::exit(1);
		}
		fseek(file, 0, SEEK_END);
		int length = ftell(file);
		rewind(file);
		char *data = new char[length + 1];
		std::memset(data, 0, length + 1);
		fread(data, 1, length, file);
		fclose(file);
		return data;
	}

	// Fonction helper pour retourner une image verticale inversée
	void flip_image_vertical(unsigned char *data, unsigned int width, unsigned int height) {
		unsigned int stride = width * 4;
		unsigned int size = stride * height;
		unsigned char *new_data = new unsigned char[size];
		for (unsigned int i = 0; i < height; i++) {
			unsigned int j = height - i - 1;
			std::memcpy(new_data + j * stride, data + i * stride, stride);
		}
		std::memcpy(data, new_data, size);
		delete [] new_data;
	}
} // namespace anonyme

namespace Utils {

	// --- Fonctions aléatoires et FPS ---
	int rand_int(int n) {
		int result;
		// On répète tant que le résultat n'est pas dans [0,n)
		do {
			result = std::rand() / (RAND_MAX / n);
		} while (result >= n);
		return result;
	}

	double rand_double() {
		return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
	}

	void update_fps(FPS &fps) {
		fps.frames++;
		double now = glfwGetTime();
		double elapsed = now - fps.since;
		if (elapsed >= 1.0) {
			fps.fps = static_cast<unsigned int>(std::round(fps.frames / elapsed));
			fps.frames = 0;
			fps.since = now;
		}
	}

	// --- Fonctions OpenGL ---
	GLuint gen_buffer(GLsizei size, const GLfloat *data) {
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return buffer;
	}

	void del_buffer(GLuint buffer) {
		glDeleteBuffers(1, &buffer);
	}

	GLfloat *malloc_faces(int components, int faces) {
		// Alloue un tableau de 6 * components * faces flottants
		return new GLfloat[6 * components * faces];
	}

	GLuint gen_faces(int components, int faces, GLfloat *data) {
		GLuint buffer = gen_buffer(sizeof(GLfloat) * 6 * components * faces, data);
		delete [] data;
		return buffer;
	}

	// --- Fonctions de shaders ---
	GLuint make_shader(GLenum type, const char *source) {
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			char *info = new char[length];
			glGetShaderInfoLog(shader, length, nullptr, info);
			std::fprintf(stderr, "glCompileShader failed:\n%s\n", info);
			delete [] info;
		}
		return shader;
	}

	GLuint load_shader(GLenum type, const char *path) {
		char *data = load_file(path);
		GLuint shader = make_shader(type, data);
		delete [] data;
		return shader;
	}

	GLuint make_program(GLuint shader1, GLuint shader2) {
		GLuint program = glCreateProgram();
		glAttachShader(program, shader1);
		glAttachShader(program, shader2);
		glLinkProgram(program);
		GLint status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			GLint length;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			char *info = new char[length];
			glGetProgramInfoLog(program, length, nullptr, info);
			std::fprintf(stderr, "glLinkProgram failed:\n%s\n", info);
			delete [] info;
		}
		glDetachShader(program, shader1);
		glDetachShader(program, shader2);
		glDeleteShader(shader1);
		glDeleteShader(shader2);
		return program;
	}

	GLuint load_program(const char *path1, const char *path2) {
		GLuint shader1 = load_shader(GL_VERTEX_SHADER, path1);
		GLuint shader2 = load_shader(GL_FRAGMENT_SHADER, path2);
		GLuint program = make_program(shader1, shader2);
		return program;
	}

	// --- Chargement d'une texture PNG ---
	void load_png_texture(const char *file_name) {
		int width, height, nrChannels;
		// Force la charge en 4 canaux (RGBA)
		unsigned char *data = stbi_load(file_name, &width, &height, &nrChannels, 4);
		if (!data) {
			std::fprintf(stderr, "Failed to load image %s: %s\n", file_name, stbi_failure_reason());
			std::exit(1);
		}
		flip_image_vertical(data, width, height);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	// --- Tokenisation et fonctions de largeur de texte ---
	char *tokenize(char *str, const char *delim, char **key) {
		char *result;
		if (str == nullptr) {
			str = *key;
		}
		str += std::strspn(str, delim);
		if (*str == '\0') {
			return nullptr;
		}
		result = str;
		str += std::strcspn(str, delim);
		if (*str) {
			*str++ = '\0';
		}
		*key = str;
		return result;
	}

	int char_width(char input) {
		static const int lookup[128] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			4, 2, 4, 7, 6, 9, 7, 2, 3, 3, 4, 6, 3, 5, 2, 7,
			6, 3, 6, 6, 6, 6, 6, 6, 6, 6, 2, 3, 5, 6, 5, 7,
			8, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 6, 5, 8, 8, 6,
			6, 7, 6, 6, 6, 6, 8,10, 8, 6, 6, 3, 6, 3, 6, 6,
			4, 7, 6, 6, 6, 6, 5, 6, 6, 2, 5, 5, 2, 9, 6, 6,
			6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 2, 5, 7, 0
		};
		if (static_cast<unsigned char>(input) < 128)
			return lookup[static_cast<unsigned char>(input)];
		return 0;
	}

	int string_width(const char *input) {
		int result = 0;
		int length = std::strlen(input);
		for (int i = 0; i < length; i++) {
			result += char_width(input[i]);
		}
		return result;
	}

	int wrap(const char *input, int max_width, char *output, int max_length) {
		output[0] = '\0';
		size_t input_len = std::strlen(input);
		char *text = new char[input_len + 1];
		std::strcpy(text, input);
		int space_width = char_width(' ');
		int line_number = 0;
		char *key1 = nullptr, *key2 = nullptr;
		char *line = tokenize(text, "\r\n", &key1);
		while (line) {
			int line_width = 0;
			char *token = tokenize(line, " ", &key2);
			while (token) {
				int token_width = string_width(token);
				if (line_width) {
					if (line_width + token_width > max_width) {
						line_width = 0;
						line_number++;
						std::strncat(output, "\n", max_length - std::strlen(output) - 1);
					} else {
						std::strncat(output, " ", max_length - std::strlen(output) - 1);
					}
				}
				std::strncat(output, token, max_length - std::strlen(output) - 1);
				line_width += token_width + space_width;
				token = tokenize(nullptr, " ", &key2);
			}
			line_number++;
			std::strncat(output, "\n", max_length - std::strlen(output) - 1);
			line = tokenize(nullptr, "\r\n", &key1);
		}
		delete [] text;
		return line_number;
	}

} // namespace Utils
