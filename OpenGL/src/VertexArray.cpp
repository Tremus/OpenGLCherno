#include "VertexArray.h"

#include "Renderer.h"

VertexArray::VertexArray()
{
    //unsigned vao;
    GLCall(glGenVertexArrays(1, &m_RendererID));
    //GLCall(glBindVertexArray(m_RendererID));
}

VertexArray::~VertexArray()
{
    GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
    Bind();
	vb.Bind();

    const auto& elements = layout.GetElements();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < elements.size(); i++)
    {
        const auto& element = elements[i];
        // tell our GPU about the structure of our data (cols & rows)
        GLCall(glEnableVertexAttribArray(i));
        // links the vertex array to the currently bound vertex buffer
        GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalised, layout.GetStride(), (const void*)offset));
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }
}

void VertexArray::Bind() const
{
    GLCall(glBindVertexArray(m_RendererID));
}
void VertexArray::Unbind() const
{
    GLCall(glBindVertexArray(0));
}
