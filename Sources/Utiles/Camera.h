#pragma once

/*!
 *	\file Camera.h
 *	\brief Declaration of the Camera class.
 *	\author Thibault LAINE
 */

#include <atomic>
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mutex.h"
#include "../Events/EventHandler.h"

/** \class Camera
 *	\brief Camera object.
 *
 *	A camera can have different mode of usage:
 *	- freefly : no restriction, moving the mouse change the orientation,
 *	- trackball : always looking a point in space, moving the mouse change the orientation and position,
 *	- isometric : moving the mouse move the camera position.
 *
 *	Camera is a thread safe class protected by a mutex.
 *
 */
class Camera
{
    public:
        //  Miscellaneous
		/*!
		 *	\enum CameraMode
		 *	\brief The different camera mode
		 */
        enum CameraMode
        {
            FREEFLY = 0,		//!< Freefly mode
            TRACKBALL = 1,		//!< Trackball mode
            ISOMETRIC = 2,		//!< Isometric mode
            MODE_MASK = 0x03	//!< Mask to extract camera mode
        };
        //

        //  Default
		/*!
		 *  \brief Constructor
		 *  \param screenRatio : the screen ratio
		 */
        Camera(const float& screenRatio = 1.0f);

		/*!
		 *  \brief Destructor
		 */
        ~Camera();
        //

        //  Public functions
		/*!
		 *	\brief Update the camera position, orientation, ...
		 *
		 *	Update all camera attributes depending on camera mode and input
		 *
		 *	\param elapseTime : A time parameter (generaly the elapsed time since last frame)
		 *	\param goForw : The go forward event is activated
		 *	\param goBack : The go back event is activated
		 *	\param goLeft : The go left event is activated
		 *	\param goRight : The go right event is activated
		 *	\param option1 : The option1 event is activated
		 *	\param option2 : The option2 event is activated
		 */
        void animate(float elapseTime,bool goForw = false,bool goBack = false,
                     bool goLeft = false,   bool goRight = false,
                     bool option1 = false,      bool option2 = false);

		/*!
		 *	\brief Compute and retur the view matrix
		 *	\return the view matric computed with glm::lookAt
		 */
		glm::mat4 getViewMatrix();

		/*!
		 *	\brief Translate the camera
		 *
		 *	This function is like setPosition(actual position + v)
		 *
		 *	\param v : the translation vector
		 */
        void translate(glm::vec3 v);
        //

        //  Set/get functions
		/*!
		 *	\brief Set the camera mode
		 *	\param mode : the new mode
		 */
        void setMode(CameraMode mode);

		/*!
		 *	\brief Set the camera speed
		 *	\param s : the new speed
		 */
        void setSpeed(float s);

		/*!
		 *	\brief Set the minimum radius (for trackball camera mode)
		 *	\param r : the new radius
		 */
        void setRadiusMin(float r);

		/*!
		 *	\brief Set the maximum radius (for trackball camera mode)
		 *	\param r : the new radius
		 */
        void setRadiusMax(float r);

		/*!
		 *	\brief Set the actual radius (for trackball camera mode)
		 *	\param r : the new radius
		 */
        void setRadius(float r);

		/*!
		 *	\brief Set all radius parameters (for trackball camera mode)
		 *	\param r : the new radius
		 *	\param rmin : the new minimum radius
		 *	\param rmax : the new maximum radius
		 */
        void setAllRadius(float r, float rmin, float rmax);

		/*!
		 *	\brief Set the camera position
		 *
		 *	If in trackball mode the new position is the closest respection trackball restrictions
		 *
		 *	\param pos : the new position
		 */
        void setPosition(glm::vec3 pos);

		/*!
		 *	\brief Set the camera orientation
		 *
		 *	If in trackball mode change the camera position to respect orientation
		 *
		 *	\param orientation : the new orientation
		 */
        void setOrientation(glm::vec3 orientation);

		/*!
		 *	\brief Set the camera target
		 *
		 *	If in trackball mode the camera position may change.
		 *
		 *	\param target : the new target point
		 */
        void setTarget(glm::vec3 target);

		/*!
		 *	\brief Set the vertical frustrum angle
		 *	\param angle : the new angle in degree
		 */
        void setFrustrumAngleVertical(float angle);

		/*!
		 *	\brief Set the horizontal frustrum angle
		 *	\param angle : the new angle in degree
		 */
        void setFrustrumAngleHorizontal(float angle);

		/*!
		 *	\brief Change the horizontal frustrum angle from screen ratio
		 *	\param screenratio : the screen ratio (width/height)
		 */
		void setFrustrumAngleHorizontalFromScreenRatio(float screenratio);

		/*!
		 *	\brief Set the camera sensibility
		 *	\param sens : the sensibility
		 */
        void setSensitivity(float sens);

		/*!
		 *	\brief Get the camera mode
		 *	\return the camera mode
		 */
        CameraMode getMode();

		/*!
		 *	\brief Get the camera target point
		 *
		 *	even if in trackball mode the target point is always computed as target = position + forward
		 *
		 *	\return the camera target point
		 */
		glm::vec3 getTarget();

		/*!
		 *	\brief Get the camera left vector
		 *	\return the camera left vector
		 */
		glm::vec3 getLeft();

		/*!
		 *	\brief Get the camera forward vector
		 *	\return the camera forward vector
		 */
		glm::vec3 getForward();
		
		/*!
		 *	\brief Get the camera vertical vector
		 *	\return the camera vertical vector
		 */
		glm::vec3 getVertical();
		
		/*!
		 *	\brief Get the camera position
		 *	\return the camera position
		 */
		glm::vec3 getPosition();

		/*!
		 *	\brief Get the camera speed
		 *	\return the camera speed
		 */
        float getSpeed();
		
		/*!
		 *	\brief Get the camera minimum radius (used in trackball mode)
		 *	\return the camera minimum radius
		 */
		float getRadiusMin();
		
		/*!
		 *	\brief Get the camera maximum radius (used in trackball mode)
		 *	\return the camera maximum radius
		 */
		float getRadiusMax();

		/*!
		 *	\brief Get the camera actual radius (used in trackball mode)
		 *	\return the camera actual radius
		 */
		float getRadius();

		/*!
		 *	\brief Get the vertical frustrum angle
		 *	\return the vertical frustrum angle
		 */
        float getFrustrumAngleVertical();
        
		/*!
		 *	\brief Get the horizontal frustrum angle
		 *	\return the horizontal frustrum angle
		 */
		float getFrustrumAngleHorizontal();
        
		/*!
		 *	\brief Get the camera sensibility
		 *	\return the camera sensibility
		 */
		float getSensitivity();
        //

    private:
        //  Private functions
		/*!
		 *	\brief Compute camera vector attributes from phi and teta angles
		 *	\param target : used in trackball mode
		 */
        void vectorsFromAngles(glm::vec3 target = glm::vec3(0,0,0));

		/*!
		 *	\brief Compute camera phi and teta angles from vector attributes
		 */
        void anglesFromVectors();

		/*!
		 *	\brief Clamp the actual radius in min max radius
		 */
        void boundingRadius();
        //

        //  Attributes
        uint8_t configuration;					//!< Configuration byte of the camera. To unpack flags and variables see #CameraMode. enum
        Mutex mutex;							//!< A mutex for thread safe usage.

		glm::vec3 position;						//!< The camera position.
		glm::vec3 forward;						//!< The forward vector. Forward, left and vertical form a orthogonal base of space.
		glm::vec3 left;							//!< The left vector.
        glm::vec3 vertical;						//!< The vertical vector.

		float radius;							//!< Actual rarius for trackball mode.
		float radiusMin;						//!< Minimum radius for trackball mode.
        float radiusMax;						//!< Maximum radius for trackball mode.

		float sensivity;						//!< Camera sensibility.
		float speedMag;							//!< Camera speed.
		float frustrumAngleHorizontal;			//!< Horizontal frustrum angle.
		float frustrumAngleVertical;			//!< Vertical frustrum angle.
		float phi;								//!< Horizontal orientaion angle.
		float teta;								//!< Vertical orientation angle.
        //
};
