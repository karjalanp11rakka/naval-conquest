#include "objectManagement.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Object::use()
{
    shader->use();
    unsigned int modelLoc = glGetUniformLocation(shader->getID(), "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}
void Object::draw()
{
    glBindVertexArray(m_mesh.VAO);
    glDrawElements(GL_TRIANGLES, m_mesh.indiciesLength, GL_UNSIGNED_INT, 0);
}