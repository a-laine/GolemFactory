#pragma once

#include "InstanceVirtual.h"

class Animator;

class InstanceDrawable : public InstanceVirtual
{
	public:
		//  Default
		/*!
		 *  \brief Constructor
		 *  \param meshName : the mesh name for the instance. If not specified the default mesh is used.
		 *  \param shaderName : the shader name for the instance. If not specified the default shader is used.
		 */
		InstanceDrawable(std::string meshName = "default", std::string shaderName = "default");

		/*!
		 *  \brief Destructor
		 */
		virtual ~InstanceDrawable();
		//

		//	Set/get functions
		/*!
		 *  \brief Change shader of instance
		 *  \param shaderName : the shader name for the instance.
		 */
		void setShader(std::string shaderName);

		/*!
		 *  \brief Change instance shader
		 *  \param s : the shader for the instance.
		 */
		void setShader(Shader* s);

		/*!
		 *  \brief Change instance mesh
		 *  \param meshName : the mesh name for the instance.
		 */
		void setMesh(std::string meshName);

		/*!
		 *  \brief Change instance mesh
		 *  \param m : the mesh for the instance.
		 */
		void setMesh(Mesh* m);

		/*!
		 *  \brief Get bounding box size of instance (depending on mesh bounding box)
		 *  \return the bounding box as a vector.
		 */
		glm::vec3 getBBSize() const;

		/*!
		*  \brief Get bounding sphere radius of instance (depending on mesh bounding box)
		*  \return the bounding sphere radius as a float.
		*/
		float getBSRadius() const;

		/*!
		 *  \brief Get shader used
		 *  \return the shader used to draw instance
		 */
		Shader* getShader() const;

		/*!
		 *  \brief Get mesh used
		 *  \return the mesh used to draw instance
		 */
		Mesh* getMesh() const;
		//

	protected:
		// Attributes
		Mesh* mesh;				//!< Mesh resource pointer
		Shader* shader;			//!< Shader resource pointer
		//
};
