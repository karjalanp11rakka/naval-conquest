#include "renderEngine.h"
#include "objectManagement.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Object::use()
{
    static RenderEngine& renderEngineInstance {RenderEngine::getInstance()};

    shader->use();
    unsigned int modelLoc = glGetUniformLocation(shader->getID(), "model");
    unsigned int viewLoc = glGetUniformLocation(shader->getID(), "view");
    unsigned int projectionLoc = glGetUniformLocation(shader->getID(), "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getView()));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(renderEngineInstance.getProjection()));
}
void Object::draw()
{
    m_mesh.use();
    glDrawElements(GL_TRIANGLES, m_mesh.indiciesLength, GL_UNSIGNED_INT, 0);
}