#pragma once

#include "../Common.hpp"
#include "../Utils/Utils.hpp"

#include <type_traits>

// Base buffer class
class Buffer {
   protected:
	int32_t size = 0;
	uint32_t id = 0;
	uint32_t type;

	Buffer(uint32_t type) : type(type) { glGenBuffers(1, &id); }

   public:
	~Buffer() {
		if (isValid())
			glDeleteBuffers(1, &id);
	};

	void bind() { glBindBuffer(type, id); }

	template <typename T>
	void bufferStaticData(const std::vector<T>& data, int32_t dataSize, int32_t dataOffset = 0) {
		TRACE_FUNCTION();
		assert(isValid() && "Cannot write data to an invalid buffer");
		assert(dataOffset + dataSize <= data.size() && "Data is out of bounds");

		bind();
		size = dataSize;
		glBufferData(type, sizeof(T) * size, &data[dataOffset], GL_STATIC_DRAW);
	}

	template <typename T>
	void bufferStaticData(const std::vector<T>& data) {
		bufferStaticData(data, data.size());
	}

	template <typename T>
	void bufferDynamicData(const std::vector<T>& data, int32_t dataSize, int32_t dataOffset = 0) {
		TRACE_FUNCTION();
		assert(isValid() && "Cannot write data to an invalid buffer");
		assert(dataOffset + dataSize <= data.size() && "Data is out of bounds");

		bind();
		size = dataSize;
		glBufferData(type, sizeof(T) * size, &data[dataOffset], GL_DYNAMIC_DRAW);
	}

	template <typename T>
	void bufferDynamicData(const std::vector<T>& data) {
		bufferDynamicData(data, data.size());
	}

	template <typename T>
	void bufferDynamicSubData(const std::vector<T>& data,
							  int32_t dataSize,
							  int32_t dataOffset = 0,
							  int32_t bufferOffset = 0) {
		TRACE_FUNCTION();
		assert(isValid() && "Cannot write data to an invalid buffer");
		assert(dataOffset + dataSize <= data.size() && "Data is out of bounds");
		assert(dataOffset + dataSize <= size && "Buffer is out of bounds");

		bind();
		size = dataSize;
		glBufferSubData(type, bufferOffset * sizeof(T), sizeof(T) * size, &data[dataOffset]);
	}

	[[nodiscard]] int32_t getSize() const { return size; }
	[[maybe_unused]] [[nodiscard]] uint32_t getId() const { return id; };
	[[nodiscard]] bool isValid() const { return id != 0; };

	Buffer(const Buffer&) = delete;
	Buffer(Buffer&) = delete;
	Buffer(Buffer&&) noexcept = delete;
	Buffer& operator=(Buffer&) = delete;
	Buffer& operator=(Buffer&&) noexcept = delete;
};

// Vertex buffer
class VertexBuffer : public Buffer {
   public:
	VertexBuffer() : Buffer(GL_ARRAY_BUFFER) {}
	static Ref<VertexBuffer> createRef() { return std::make_shared<VertexBuffer>(); }
};

// Index buffer
class IndexBuffer : public Buffer {
	uint32_t type = 0;

	template <typename T>
	int32_t getSizeType() {
		switch (sizeof(T)) {
			case 1:
				return GL_UNSIGNED_BYTE;
			case 2:
				return GL_UNSIGNED_SHORT;
			case 4:
				return GL_UNSIGNED_INT;
		}

		assert(false);
		return 0;
	}

   public:
	template <typename T>
	explicit IndexBuffer(T _) : Buffer(GL_ELEMENT_ARRAY_BUFFER) {
		static_assert(
			std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value ||
				std::is_same<T, uint32_t>::value,
			"The given type must be either unsigned char, unsigned short or unsigned int");
		type = getSizeType<T>();
	}

	template <typename T>
	static Ref<IndexBuffer> createRef() {
		return std::make_shared<IndexBuffer>(T());
	}

	[[nodiscard]] uint32_t getType() const { return type; }
};

// Vertex attribute descriptor
struct VertexAttribute {
	enum ComponentType {
		UShort = GL_UNSIGNED_SHORT,
		Int = GL_INT,
		UInt = GL_UNSIGNED_INT,
		Byte = GL_BYTE,
		Float = GL_FLOAT,
	};

	uint8_t componentCount;
	ComponentType type;
	bool shouldBeNormalized = false;
	int32_t vertexSize;
	uint32_t offset;

	VertexAttribute(uint8_t componentCount, ComponentType type, uint32_t offset)
		: componentCount(componentCount),
		  type(type),
		  shouldBeNormalized(false),
		  vertexSize(0),
		  offset(offset) {}
};

// Vertex array object
class VertexArray {
	uint32_t id = 0;
	Ref<VertexBuffer> vertexBuffer;
	Ref<IndexBuffer> indexBuffer;

   public:
	explicit VertexArray();
	~VertexArray();

	template <typename VertexT>
	explicit VertexArray(const std::vector<VertexT>& vertices, bool dynamic = false) {
		glGenVertexArrays(1, &id);
		bind();

		vertexBuffer = VertexBuffer::createRef();
		if (dynamic) {
			vertexBuffer->bufferDynamicData<VertexT>(vertices);
		} else {
			vertexBuffer->bufferStaticData<VertexT>(vertices);
		}

		unbind();
	};

	template <typename VertexT, typename IndexT>
	VertexArray(const std::vector<VertexT>& vertices,
				const std::vector<IndexT>& indices,
				bool dynamic = false)
		: VertexArray(vertices, dynamic) {
		bind();
		indexBuffer = IndexBuffer::createRef<IndexT>();

		if (dynamic) {
			indexBuffer->bufferDynamicData<IndexT>(indices);
		} else {
			indexBuffer->bufferStaticData<IndexT>(indices);
		}
		unbind();
	};

	template <typename VertexT>
	explicit VertexArray(const std::vector<VertexT>& vertices,
						 const std::vector<VertexAttribute>& vertexAttributes,
						 bool dynamic = false)
		: VertexArray(vertices, dynamic) {
		addVertexAttributes(vertexAttributes, sizeof(VertexT));
	};

	template <typename VertexT, typename IndexT>
	VertexArray(const std::vector<VertexT>& vertices,
				const std::vector<VertexAttribute>& vertexAttributes,
				const std::vector<IndexT>& indices,
				bool dynamic = false)
		: VertexArray(vertices, indices, dynamic) {
		addVertexAttributes(vertexAttributes, sizeof(VertexT));
	};

	void bind();
	void addVertexAttributes(const std::vector<VertexAttribute>& vector, int32_t defaultVertexSize);
	void renderIndexed(int32_t type = GL_TRIANGLES);
	void renderVertexSubStream(int32_t size, int32_t startOffset, int32_t type = GL_TRIANGLES);
	void renderVertexStream(int32_t type = GL_TRIANGLES);
	void unbind();

	Ref<VertexBuffer> getVertexBuffer() { return vertexBuffer; };

	[[nodiscard]] bool isValid() const { return id != 0; };

	VertexArray(const VertexArray&) = delete;
	VertexArray(VertexArray&) = delete;
	VertexArray(VertexArray&&) noexcept = delete;
	VertexArray& operator=(VertexArray&) = delete;
	VertexArray& operator=(VertexArray&&) noexcept = delete;
};

// Fullscreen quad utility
namespace FullscreenQuad {
Ref<VertexArray> getVertexArray();
}

// Inline implementations
inline VertexArray::VertexArray() {
	glGenVertexArrays(1, &id);
	bind();

	vertexBuffer = VertexBuffer::createRef();
	vertexBuffer->bind();

	unbind();
}

inline VertexArray::~VertexArray() {
	if (isValid()) {
		glDeleteVertexArrays(1, &id);
	}
}

inline void VertexArray::bind() {
	glBindVertexArray(id);
	if (vertexBuffer)
		vertexBuffer->bind();
	if (indexBuffer)
		indexBuffer->bind();
}

inline void VertexArray::unbind() {
	glBindVertexArray(0);
}

inline void VertexArray::renderIndexed(int32_t type) {
	TRACE_FUNCTION();
	assert(indexBuffer != nullptr);

	bind();
	glDrawElements(type, indexBuffer->getSize(), indexBuffer->getType(), nullptr);
	unbind();
}

inline void VertexArray::renderVertexStream(int32_t type) {
	TRACE_FUNCTION();
	renderVertexSubStream(vertexBuffer->getSize(), 0, type);
}

inline void VertexArray::renderVertexSubStream(int32_t size, int32_t startOffset, int32_t type) {
	TRACE_FUNCTION();
	if (!isValid())
		return;
	assert(indexBuffer == nullptr);

	bind();
	glDrawArrays(type, startOffset, size);
	unbind();
}

inline void VertexArray::addVertexAttributes(const std::vector<VertexAttribute>& vector,
											 int32_t defaultVertexSize) {
	bind();
	for (size_t i = 0; i < vector.size(); i++) {
		const auto& [componentCount, type, shouldBeNormalized, vertexSize, offset] = vector[i];
		const auto normalize = shouldBeNormalized ? GL_TRUE : GL_FALSE;
		const auto stride = vertexSize ? vertexSize : defaultVertexSize;

		switch (type) {
			case VertexAttribute::UShort:
			case VertexAttribute::Int:
			case VertexAttribute::UInt:
			case VertexAttribute::Byte:
				glVertexAttribIPointer(
					i, componentCount, type, stride, reinterpret_cast<void*>(offset));
				break;
			case VertexAttribute::Float:
				glVertexAttribPointer(
					i, componentCount, type, normalize, stride, reinterpret_cast<void*>(offset));
				break;
		}
		glEnableVertexAttribArray(i);
	}
	unbind();
}

inline Ref<VertexArray> FullscreenQuad::getVertexArray() {
	static Ref<VertexArray> vertexArray = std::make_shared<VertexArray>(
		std::vector<glm::vec3>{{-1, -1, 0}, {-1, 1, 0}, {1, -1, 0}, {1, 1, 0}},
		std::vector<VertexAttribute>{VertexAttribute{3, VertexAttribute::Float, 0}},
		std::vector<uint8_t>{0, 2, 1, 1, 2, 3});
	return vertexArray;
}