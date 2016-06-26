#pragma once

#include <atomic>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
  
#include "../Events/EventHandler.h"
#include "Mutex.h"


class Camera
{
    public:
        //  Miscellaneous
        enum CameraMode
        {
            FREEFLY = 0,
            TRACKBALL = 1,
            CINEMATIC = 2,
            ISOMETRIC = 3,
            MODE_MASK = 0x03
        };
        enum CinematicState
        {
            PLAY = 1<<2
        };
        //

        //  Default
        Camera(float screenRatio = 1.0f);
        ~Camera();
        //

        //  Public functions
        void animate(float elapseTime,bool goForw = false,bool goBack = false,
                     bool goLeft = false,   bool goRight = false,
                     bool option1 = false,      bool option2 = false);
		glm::mat4 getViewMatrix();
        void move(glm::vec3 v);

        void pause();
        void play();
        //

        //  Set/get functions
        void setMode(CameraMode mode);
        void setSpeed(float s);
        void setRadiusMin(float r);
        void setRadiusMax(float r);
        void setRadius(float r);
        void setAllRadius(float r, float rmin, float rmax);
        void setPosition(glm::vec3 pos);
        void setOrientation(glm::vec3 orientation);
        void setTarget(glm::vec3 target);
        void setFrustrumAngleVertical(float angle);
        void setFrustrumAngleHorizontal(float angle);
		void setFrustrumAngleHorizontalFromScreenRatio(float screenratio);
        void setSensitivity(float sens);

        CameraMode getMode();
		glm::vec3 getTarget();
		glm::vec3 getLeft();
		glm::vec3 getForward();
		glm::vec3 getVertical();
		glm::vec3 getPosition();
        float getSpeed();
		float getRadiusMin();
		float getRadiusMax();
		float getRadius();
        float getFrustrumAngleVertical();
        float getFrustrumAngleHorizontal();
        float getSensitivity();
        //

    private:
        //  Private functions
        void vectorsFromAngles(glm::vec3 target = glm::vec3(0,0,0));
        void anglesFromVectors();
        void boundingRadius();
        //

        //  Attributes
        uint8_t configuration;
        Mutex mutex;

		glm::vec3 position,
                  forward,
                  left,
                  vertical;

        float radius,
              radiusMin,
              radiusMax;

        float sensivity,
              speedMag,
              frustrumAngleHorizontal,
              frustrumAngleVertical,
              phi,
              teta;
        //
};
